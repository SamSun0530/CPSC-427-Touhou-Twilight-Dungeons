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
	float dist_squared = dot(dp,dp);
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

void PhysicsSystem::step(float elapsed_ms)
{
	// Move bug based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	auto& motion_registry = registry.motions;
	for(uint i = 0; i< motion_registry.size(); i++)
	{
		Motion& motion = motion_registry.components[i];
		Entity entity = motion_registry.entities[i];
		float step_seconds = elapsed_ms / 1000.f;

		// Normalize direction vector if either x or y is not 0 (prevents divide by 0 when normalizing)
		vec2 direction_normalized = motion.direction;
		if (motion.direction.x != 0 || motion.direction.y != 0) {
			direction_normalized = normalize(direction_normalized);
		}

		// Linear interpolation of velocity
		// K factor (0,30] = ~0 (not zero, slippery, ice) -> 10-20 (quick start up/slow down, natural) -> 30 (instant velocity, jittery)
		float K = 10.f;
		motion.velocity = vec2_lerp(motion.velocity, direction_normalized * motion.speed_modified, step_seconds * K);
		motion.last_position = motion.position;
		motion.position += motion.velocity * step_seconds;
	}

	// Check for collisions between all moving entities
    ComponentContainer<Motion> &motion_container = registry.motions;
	for(uint i = 0; i<motion_container.components.size(); i++)
	{
		Motion& motion_i = motion_container.components[i];
		Entity entity_i = motion_container.entities[i];
		
		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for(uint j = i+1; j<motion_container.components.size(); j++)
		{
			Motion& motion_j = motion_container.components[j];
			if (collides_AABB(motion_i, motion_j))
			{
				Entity entity_j = motion_container.entities[j];
				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				registry.collisions.emplace_with_duplicates(entity_j, entity_i);
			}
		}
	}

	// Other collisions here
}