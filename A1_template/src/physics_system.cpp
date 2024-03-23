// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include <iostream>

struct CollisionInfo {
	bool hasCollision = false;
	vec3 collisionPoint;
	vec3 collisionNormal;
	float penetrationDepth = 0.0f;
};

// Collision test between AABB and AABB
bool collides_AABB_AABB(const Motion& motion1, const Motion& motion2, const Collidable& collidable1, const Collidable& collidable2)
{
	const vec2 bounding_box1 = abs(collidable1.size) / 2.f;
	const vec2 bounding_box2 = abs(collidable2.size) / 2.f;
	const vec2 box_center1 = motion1.position + collidable1.shift;
	const vec2 box_center2 = motion2.position + collidable2.shift;

	const float top1 = box_center1.y - bounding_box1.y;
	const float bottom1 = box_center1.y + bounding_box1.y;
	const float left1 = box_center1.x - bounding_box1.x;
	const float right1 = box_center1.x + bounding_box1.x;

	const float top2 = box_center2.y - bounding_box2.y;
	const float bottom2 = box_center2.y + bounding_box2.y;
	const float left2 = box_center2.x - bounding_box2.x;
	const float right2 = box_center2.x + bounding_box2.x;

	if (left2 < right1 && left1 < right2 && top2 < bottom1 && top1 < bottom2)
		return true;
	return false;
}


// Below check edge to edge collision reference Geometric Algorithm: https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/
bool on_segment(vec3 p, vec3 q, vec3 r) {
	if (q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) &&
		q.y <= std::max(p.y, r.y) && q.y >= std::min(p.y, r.y))
		return true;
	return false;
}

int detect_turn(vec3 p, vec3 q, vec3 r) {
	int val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
	if (val == 0) return 0;  // Collinear
	return (val > 0) ? 1 : 2; // LeftTurn or RightTurn
}

// Below point in triangle collision reference "Kornel Kisielewicz" and "xaedes": https://stackoverflow.com/questions/2049582/how-to-determine-if-a-point-is-in-a-2d-triangle
bool in_triangle(vec3 pt, vec3 v1, vec3 v2, vec3 v3) {
	int o1 = detect_turn(v1, v2, pt);
	int o2 = detect_turn(v2, v3, pt);
	int o3 = detect_turn(v3, v1, pt);
	bool is_same_dir = (o1 == o2) && (o2 == o3);
	bool is_not_all_colinear = (o1 != 0) || (o2 != 0) || (o3 != 0);
	return is_same_dir && is_not_all_colinear;
}

bool do_intersect(vec3 p1, vec3 q1, vec3 p2, vec3 q2) {
	int o1 = detect_turn(p1, q1, p2);
	int o2 = detect_turn(p1, q1, q2);
	int o3 = detect_turn(p2, q2, p1);
	int o4 = detect_turn(p2, q2, q1);
	if (o1 != o2 && o3 != o4)
		return true;
	if (o1 == 0 && on_segment(p1, p2, q1)) return true;
	if (o2 == 0 && on_segment(p1, q2, q1)) return true;
	if (o3 == 0 && on_segment(p2, p1, q2)) return true;
	if (o4 == 0 && on_segment(p2, q1, q2)) return true;
	return false;
}


bool collides_mesh_AABB(const Entity& e1, const Motion& motion1, const Motion& motion2, const Collidable& collidable2) {
	Mesh* e1_mesh = registry.meshPtrs.get(e1);
	vec2 mesh_scale = { motion1.scale.x / 32 * 24 , -motion1.scale.y };
	Transform transform;
	transform.scale(mesh_scale);
	const vec2 bounding_box2 = abs(collidable2.size) / 2.f;
	const vec2 box_center2 = motion2.position + collidable2.shift;
	const float top2 = box_center2.y - bounding_box2.y;
	const float bottom2 = box_center2.y + bounding_box2.y;
	const float left2 = box_center2.x - bounding_box2.x;
	const float right2 = box_center2.x + bounding_box2.x;
	// Checking if vertices of mesh are colliding with AABB
	for (const auto& vertex : e1_mesh->vertices) {
		vec3 transformed = transform.mat * vertex.position + vec3(motion1.position.x, motion1.position.y, 0);
		if (transformed.x >= left2 && transformed.x <= right2 && transformed.y <= bottom2 && transformed.y >= top2) {
			return true;
		}
	}
	std::vector<std::pair<vec3, vec3>> box_edges = {
	   {{left2, bottom2, 0}, {right2, bottom2, 0}}, // Bottom edge
	   {{right2, bottom2, 0}, {right2, top2, 0}}, // Right edge
	   {{right2, top2, 0}, {left2, top2, 0}}, // Top edge
	   {{left2, top2, 0}, {left2, bottom2, 0}}  // Left edge
	};
	std::vector<vec3> box_vertices = {
		{left2, bottom2, 0}, {right2, bottom2, 0}, {right2, top2, 0}, {left2, top2, 0}
	};
	// Checking if vertices of AABB are colliding with mesh
	std::set<std::pair<uint16_t, uint16_t>> mesh_edges;
	vec3 v1;
	vec3 v2;
	vec3 v3;
	vec3 transformed_v1;
	vec3 transformed_v2;
	vec3 transformed_v3;
	for (size_t i = 0; i < e1_mesh->vertex_indices.size(); i += 3) {
		uint16_t vid_1 = e1_mesh->vertex_indices[i];
		uint16_t vid_2 = e1_mesh->vertex_indices[i + 1];
		uint16_t vid_3 = e1_mesh->vertex_indices[i + 2];
		v1 = e1_mesh->vertices[vid_1].position;
		v2 = e1_mesh->vertices[vid_2].position;
		v3 = e1_mesh->vertices[vid_3].position;
		transformed_v1 = transform.mat * v1 + vec3(motion1.position.x, motion1.position.y, 0);
		transformed_v2 = transform.mat * v2 + vec3(motion1.position.x, motion1.position.y, 0);
		transformed_v3 = transform.mat * v3 + vec3(motion1.position.x, motion1.position.y, 0);
		for (vec3 box_vertex : box_vertices) {
			if (in_triangle(box_vertex, transformed_v1, transformed_v2, transformed_v3)) return true;
		}
		mesh_edges.insert({ std::min(vid_1, vid_2), std::max(vid_1, vid_2) });
		mesh_edges.insert({ std::min(vid_2, vid_3), std::max(vid_2, vid_3) });
		mesh_edges.insert({ std::min(vid_3, vid_1), std::max(vid_3, vid_1) });
	}


	// Checking if edges of AABB and mesh are colliding
	for (const auto& meshEdge : mesh_edges) {
		for (const auto& boxEdge : box_edges) {
			v1 = e1_mesh->vertices[meshEdge.first].position;
			v2 = e1_mesh->vertices[meshEdge.second].position;
			transformed_v1 = transform.mat * v1 + vec3(motion1.position.x, motion1.position.y, 0);
			transformed_v2 = transform.mat * v2 + vec3(motion1.position.x, motion1.position.y, 0);
			if (do_intersect(transformed_v1, transformed_v2, boxEdge.first, boxEdge.second)) {
				return true;
			}
		}
	}
	return false;
}

void PhysicsSystem::step(float elapsed_ms)
{
	// Move entities based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	auto& kinematic_registry = registry.kinematics;
	for (uint i = 0; i < kinematic_registry.size(); i++)
	{
		Kinematic& kinematic = kinematic_registry.components[i];
		Entity& entity = kinematic_registry.entities[i];
		Motion& motion = registry.motions.get(entity); // kinematic will always have motion
		//motion.prev_pos = motion.position;
		float step_seconds = elapsed_ms / 1000.f;

		// Normalize direction vector if either x or y is not 0 (prevents divide by 0 when normalizing)
		vec2 direction_normalized = kinematic.direction;
		if (kinematic.direction.x != 0 || kinematic.direction.y != 0) {
			direction_normalized = normalize(direction_normalized);
		}

		// Linear interpolation of velocity
		// K factor (0,30] = ~0 (not zero, slippery, ice) -> 10-20 (quick start up/slow down, natural) -> 30 (instant velocity, jittery)
		float K = 10.f;
		kinematic.velocity = vec2_lerp(kinematic.velocity, direction_normalized * kinematic.speed_modified, step_seconds * K);
		motion.position += kinematic.velocity * step_seconds;
	}

	// Check for collisions between all collidable entities
	// Ignores wall collisions as it is checked after
	ComponentContainer<Collidable>& collidable_container = registry.collidables;
	ComponentContainer<Motion>& motion_container = registry.motions;
	for (uint i = 0; i < collidable_container.components.size(); i++)
	{
		Entity entity_i = collidable_container.entities[i];
		if (registry.walls.has(entity_i) || registry.enemyBullets.has(entity_i)) continue;
		Collidable& collidable_i = collidable_container.components[i];
		Motion& motion_i = motion_container.get(entity_i);

		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for (uint j = i + 1; j < collidable_container.components.size(); j++)
		{
			Entity entity_j = collidable_container.entities[j];
			if (registry.walls.has(entity_j) || registry.enemyBullets.has(entity_j)) continue;
			Collidable& collidable_j = collidable_container.components[j];
			Motion& motion_j = motion_container.get(entity_j);

			if (collides_AABB_AABB(motion_i, motion_j, collidable_i, collidable_j))
			{
				if (registry.players.has(entity_i)) {
					if (collides_mesh_AABB(entity_i, motion_i, motion_j, collidable_j)) {
						registry.collisions.emplace_with_duplicates(entity_i, entity_j);
						registry.collisions.emplace_with_duplicates(entity_j, entity_i);
					}
				}
				else if (registry.players.has(entity_j)) {
					if (collides_mesh_AABB(entity_j, motion_j, motion_i, collidable_i)) {
						registry.collisions.emplace_with_duplicates(entity_i, entity_j);
						registry.collisions.emplace_with_duplicates(entity_j, entity_i);
					}
				}
				else {
					registry.collisions.emplace_with_duplicates(entity_i, entity_j);
					registry.collisions.emplace_with_duplicates(entity_j, entity_i);
				}
			}
		}
	}

	// Check for collision between enemy bullets and wall, enemy bullets and player
	// Optimizes performance so do not have to check all wall with all enemy bullets
	for (Entity player_entity : registry.players.entities) {
		Motion& player_motion = registry.motions.get(player_entity);
		//Entity& wall_entity = registry.walls.entities[0]; // quick hack
		Collidable& player_collidable = registry.collidables.get(player_entity);

		for (Entity entity : registry.enemyBullets.entities) {
			Motion& motion = registry.motions.get(entity);
			Collidable& collidable = registry.collidables.get(entity);
			coord grid_coord = convert_world_to_grid(motion.position);
			if (!is_valid_cell(grid_coord.x, grid_coord.y)) {
				//registry.collisions.emplace(entity, wall_entity); // causes bullet to go through walls
				registry.remove_all_components_of(entity);
			}
			//else if (collides_AABB_AABB(motion, player_motion, collidable, player_collidable) && 
			//	collides_mesh_AABB(player_entity, player_motion, motion, collidable)) {
			//	registry.collisions.emplace_with_duplicates(player_entity, entity);
			//}
		}
	}

	// Wall collisions - handled manually since wall to wall collisions has a huge performance hit
	ComponentContainer<Wall>& wall_container = registry.walls;
	for (uint i = 0; i < wall_container.components.size(); i++)
	{
		Wall& wall = wall_container.components[i];
		Entity entity_i = wall_container.entities[i];
		Collidable& collidable_i = registry.collidables.get(entity_i);
		Motion& motion_i = motion_container.get(entity_i);

		// Wall to player collision
		for (Entity& entity_j : registry.players.entities) {
			Collidable& collidable_j = registry.collidables.get(entity_j);
			Motion& motion_j = registry.motions.get(entity_j);
			if (collides_AABB_AABB(motion_i, motion_j, collidable_i, collidable_j))
			{
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				registry.collisions.emplace_with_duplicates(entity_j, entity_i);
			}
		}

		// Wall to enemy collision
		for (Entity& entity_j : registry.deadlys.entities) {
			Collidable& collidable_j = registry.collidables.get(entity_j);
			Motion& motion_j = registry.motions.get(entity_j);
			if (collides_AABB_AABB(motion_i, motion_j, collidable_i, collidable_j))
			{
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				registry.collisions.emplace_with_duplicates(entity_j, entity_i);
			}
		}

		// Wall to player bullet collision
		for (Entity& entity_j : registry.playerBullets.entities) {
			Collidable& collidable_j = registry.collidables.get(entity_j);
			Motion& motion_j = registry.motions.get(entity_j);
			if (collides_AABB_AABB(motion_i, motion_j, collidable_i, collidable_j))
			{
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				registry.collisions.emplace_with_duplicates(entity_j, entity_i);
			}
		}
	}
	// Visualize bounding boxes
	if (debugging.in_debug_mode) {
		for (Entity& entity : registry.collidables.entities) {
			Motion& motion = registry.motions.get(entity);
			Collidable& collidable = registry.collidables.get(entity);
			const vec2 bounding_box = abs(collidable.size);
			const vec2 box_center = motion.position + collidable.shift;
			createLine(box_center, bounding_box);
		}
	}
	// Other collisions here
}