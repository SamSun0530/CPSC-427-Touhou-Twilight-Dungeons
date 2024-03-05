// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include <iostream>

//// Returns the local bounding coordinates scaled by the current size of the entity
//vec2 get_bounding_box(const Motion& motion)
//{
//	// abs is to avoid negative scale due to the facing direction.
//	return { abs(motion.scale.x), abs(motion.scale.y) };
//}
//
//// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
//// if the center point of either object is inside the other's bounding-box-circle. You can
//// surely implement a more accurate detection
//bool collides(const Motion& motion1, const Motion& motion2)
//{
//	vec2 dp = motion1.position - motion2.position;
//	float dist_squared = dot(dp, dp);
//	const vec2 other_bonding_box = get_bounding_box(motion1) / 2.f;
//	const float other_r_squared = dot(other_bonding_box, other_bonding_box);
//	const vec2 my_bonding_box = get_bounding_box(motion2) / 2.f;
//	const float my_r_squared = dot(my_bonding_box, my_bonding_box);
//	const float r_squared = max(other_r_squared, my_r_squared);
//	if (dist_squared < r_squared)
//		return true;
//	return false;
//}

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

	if (left2 <= right1 && left1 <= right2 && top2 <= bottom1 && top1 <= bottom2)
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

int orientation(vec3 p, vec3 q, vec3 r) {
	int val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
	if (val == 0) return 0;  // Collinear
	return (val > 0) ? 1 : 2; // Clock or counterclockwise
}

bool do_intersect(vec3 p1, vec3 q1, vec3 p2, vec3 q2) {
	int o1 = orientation(p1, q1, p2);
	int o2 = orientation(p1, q1, q2);
	int o3 = orientation(p2, q2, p1);
	int o4 = orientation(p2, q2, q1);
	if (o1 != o2 && o3 != o4)
		return true;
	if (o1 == 0 && on_segment(p1, p2, q1)) return true;
	if (o2 == 0 && on_segment(p1, q2, q1)) return true;
	if (o3 == 0 && on_segment(p2, p1, q2)) return true;
	if (o4 == 0 && on_segment(p2, q1, q2)) return true;
	return false;
}

std::set<std::pair<uint16_t, uint16_t>> get_edges(const std::vector<uint16_t>& vertex_indices) {
	std::set<std::pair<uint16_t, uint16_t>> edge_set;
	for (size_t i = 0; i < vertex_indices.size(); i += 3) {
		uint16_t v1 = vertex_indices[i];
		uint16_t v2 = vertex_indices[i + 1];
		uint16_t v3 = vertex_indices[i + 2];
		edge_set.insert({ std::min(v1, v2), std::max(v1, v2) });
		edge_set.insert({ std::min(v2, v3), std::max(v2, v3) });
		edge_set.insert({ std::min(v3, v1), std::max(v3, v1) });
	}
	return edge_set;
}

bool collides_mesh_AABB(const Entity& e1, const Motion& motion1, const Motion& motion2, const Collidable& collidable2) {
	Mesh* e1_mesh = registry.meshPtrs.get(e1);
	vec2 mesh_scale = { motion1.scale.x/32*24 , -motion1.scale.y };
	Transform transform;
	transform.scale(mesh_scale);
	const vec2 bounding_box2 = abs(collidable2.size) / 2.f;
	const vec2 box_center2 = motion2.position + collidable2.shift;
	const float top2 = box_center2.y - bounding_box2.y;
	const float bottom2 = box_center2.y + bounding_box2.y;
	const float left2 = box_center2.x - bounding_box2.x;
	const float right2 = box_center2.x + bounding_box2.x;
	// Checking if vertices are colliding with AABB
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
	std::set<std::pair<uint16_t, uint16_t>> mesh_edges = get_edges(e1_mesh->vertex_indices);
	// Checking if edges are colliding with AABB
	for (const auto& meshEdge : mesh_edges) {
		for (const auto& boxEdge : box_edges) {
			vec3 v1 = e1_mesh->vertices[meshEdge.first].position;
			vec3 v2 = e1_mesh->vertices[meshEdge.second].position;
			vec3 transformed_v1 = transform.mat * v1 + vec3(motion1.position.x, motion1.position.y, 0);
			vec3 transformed_v2 = transform.mat * v2 + vec3(motion1.position.x, motion1.position.y, 0);
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
		if (registry.walls.has(entity_i)) continue;
		Collidable& collidable_i = collidable_container.components[i];
		Motion& motion_i = motion_container.get(entity_i);

		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for (uint j = i + 1; j < collidable_container.components.size(); j++)
		{
			Entity entity_j = collidable_container.entities[j];
			if (registry.walls.has(entity_j)) continue;
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
				if (collides_mesh_AABB(entity_j, motion_j, motion_i, collidable_i)) {
					std::cout << "colliding" << std::endl;
					registry.collisions.emplace_with_duplicates(entity_i, entity_j);
					registry.collisions.emplace_with_duplicates(entity_j, entity_i);
				}
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

		// Wall to enemy bullet collision
		for (Entity& entity_j : registry.enemyBullets.entities) {
			Collidable& collidable_j = registry.collidables.get(entity_j);
			Motion& motion_j = registry.motions.get(entity_j);
			if (collides_AABB_AABB(motion_i, motion_j, collidable_i, collidable_j))
			{
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				registry.collisions.emplace_with_duplicates(entity_j, entity_i);
			}
		}
	}

	// Other collisions here

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
}

