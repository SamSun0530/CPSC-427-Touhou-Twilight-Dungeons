// internal
#include "ai_system.hpp"

void AISystem::step(float elapsed_ms)
{
	// Randomized enemy position based on their state
	for (Entity& entity : registry.idleMoveActions.entities) {
		// progress timer
		IdleMoveAction& action = registry.idleMoveActions.get(entity);
		action.timer_ms = action.timer_ms < elapsed_ms ? 0.f : action.timer_ms - elapsed_ms;
		if (action.timer_ms <= 0) {
			// Prevent entity to continue moving
			if (action.state == State::MOVE) {
				Kinematic& kinematic = registry.kinematics.get(entity);
				action.state = State::IDLE;
				action.timer_ms = action.idle_ms;
				kinematic.direction = { 0, 0 };
			}
		}
	}

	// Limit number of checks based on update timer
	ComponentContainer<AiTimer>& aitimer_components = registry.aitimers;
	for (uint i = 0; i < aitimer_components.components.size(); i++) {
		AiTimer& aitimer = aitimer_components.components[i];
		Entity& entity = aitimer_components.entities[i];
		
		aitimer.update_timer_ms = aitimer.update_timer_ms < elapsed_ms ? 0.f : aitimer.update_timer_ms - elapsed_ms;
		if (aitimer.update_timer_ms <= 0) {
			if (registry.deadlys.has(entity)) {
				ghost.update(entity);
			}
			aitimer.update_timer_ms = aitimer.update_base;
		}
	}
}

AISystem::AISystem() {
	rng = std::default_random_engine(std::random_device()());
}