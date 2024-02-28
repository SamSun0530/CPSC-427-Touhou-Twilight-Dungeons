// internal
#include "physics_system.hpp"
#include "world_init.hpp"

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion& motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(motion.scale.x), abs(motion.scale.y) };
}

// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box-circle. You can
// surely implement a more accurate detection
bool collides(const Motion& motion1, const Motion& motion2)
{
	vec2 dp = motion1.position - motion2.position;
	float dist_squared = dot(dp, dp);
	const vec2 other_bonding_box = get_bounding_box(motion1) / 2.f;
	const float other_r_squared = dot(other_bonding_box, other_bonding_box);
	const vec2 my_bonding_box = get_bounding_box(motion2) / 2.f;
	const float my_r_squared = dot(my_bonding_box, my_bonding_box);
	const float r_squared = max(other_r_squared, my_r_squared);
	if (dist_squared < r_squared)
		return true;
	return false;
}

bool collides_AABB(const Motion& motion1, const Motion& motion2)
{
	const vec2 bounding_box1 = get_bounding_box(motion1) / 2.f;
	const vec2 bounding_box2 = get_bounding_box(motion2) / 2.f;

	const float top1 = motion1.position.y - bounding_box1.y;
	const float bottom1 = motion1.position.y + bounding_box1.y;
	const float left1 = motion1.position.x - bounding_box1.x;
	const float right1 = motion1.position.x + bounding_box1.x;

	const float top2 = motion2.position.y - bounding_box2.y;
	const float bottom2 = motion2.position.y + bounding_box2.y;
	const float left2 = motion2.position.x - bounding_box2.x;
	const float right2 = motion2.position.x + bounding_box2.x;

	if (left2 <= right1 && left1 <= right2 && top2 <= bottom1 && top1 <= bottom2)
		return true;
	return false;
}

bool collides_AABB2(const Motion& motion1, const Motion& motion2, const Collidable& collidable1, const Collidable& collidable2)
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
#include <iostream>
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
		motion.last_position = motion.position;
		motion.position += kinematic.velocity * step_seconds;
	}

	//// Check for collisions between all moving entities
 //   ComponentContainer<Motion> &motion_container = registry.motions;
	//for(uint i = 0; i<motion_container.components.size(); i++)
	//{
	//	Motion& motion_i = motion_container.components[i];
	//	Entity entity_i = motion_container.entities[i];
	//	
	//	// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
	//	for(uint j = i+1; j<motion_container.components.size(); j++)
	//	{
	//		Motion& motion_j = motion_container.components[j];
	//		if (collides_AABB(motion_i, motion_j))
	//		{
	//			Entity entity_j = motion_container.entities[j];
	//			// Create a collisions event
	//			// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
	//			registry.collisions.emplace_with_duplicates(entity_i, entity_j);
	//			registry.collisions.emplace_with_duplicates(entity_j, entity_i);
	//		}
	//	}
	//}

	//ComponentContainer<Collidable>& collidable_container = registry.collidables;
	//ComponentContainer<Motion>& motion_container = registry.motions;
	//for (uint i = 0; i < collidable_container.components.size(); i++)
	//{
	//	Collidable& collidable_i = collidable_container.components[i];
	//	Entity entity_i = collidable_container.entities[i];
	//	Motion& motion_i = motion_container.get(entity_i);

	//	// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
	//	for (uint j = i + 1; j < collidable_container.components.size(); j++)
	//	{
	//		Collidable& collidable_j = collidable_container.components[j];
	//		Entity entity_j = motion_container.entities[j];
	//		Motion& motion_j = motion_container.get(entity_j);
	//		if (collides_AABB2(motion_i, motion_j, collidable_i, collidable_j))
	//		{
	//			// Create a collisions event
	//			// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
	//			registry.collisions.emplace_with_duplicates(entity_i, entity_j);
	//			registry.collisions.emplace_with_duplicates(entity_j, entity_i);
	//		}
	//	}
	//}


	ComponentContainer<Wall>& wall_container = registry.walls;
	ComponentContainer<Motion>& motion_container = registry.motions;
	for (uint i = 0; i < wall_container.components.size(); i++)
	{
		Wall& wall = wall_container.components[i];
		Entity entity_i = wall_container.entities[i];
		Collidable& collidable_i = registry.collidables.get(entity_i);
		Motion& motion_i = motion_container.get(entity_i);

		for (Entity& entity_j : registry.players.entities) {
			Collidable& collidable_j = registry.collidables.get(entity_j);
			Motion& motion_j = registry.motions.get(entity_j);
			if (collides_AABB2(motion_i, motion_j, collidable_i, collidable_j))
			{
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				registry.collisions.emplace_with_duplicates(entity_j, entity_i);
			}
		}
	}

	// Other collisions here
}

