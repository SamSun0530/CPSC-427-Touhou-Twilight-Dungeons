// internal
#include "ai_system.hpp"

void AISystem::step(float elapsed_ms)
{
	// Randomized enemy position based on their state
	for (Entity entity : registry.idleMoveActions.entities) {
		// progress timer
		IdleMoveAction& action = registry.idleMoveActions.get(entity);
		action.timer_ms = action.timer_ms < elapsed_ms ? 0.f : action.timer_ms - elapsed_ms;
		if (action.timer_ms <= 0) {
			Motion& motion = registry.motions.get(entity);
			switch (action.state) {
			case State::IDLE:
				action.state = State::MOVE;
				action.timer_ms = action.moving_ms;
				motion.velocity = { uniform_dist(rng), uniform_dist(rng) };
				break;
			case State::MOVE:
				action.state = State::IDLE;
				action.timer_ms = action.idle_ms;
				motion.velocity = { 0, 0 };
				break;
			default:
				break;
			}
		}
	}
}

AISystem::AISystem() {
	rng = std::default_random_engine(std::random_device()());
}