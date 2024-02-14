// internal
#include "ai_system.hpp"

void AISystem::step(float elapsed_ms)
{
	// Randomized enemy position based on their state
	for (Entity entity : registry.deadlys.entities) {
		// progress timer
		Deadly& deadly = registry.deadlys.get(entity);
		deadly.timer_ms = deadly.timer_ms < elapsed_ms ? 0.f : deadly.timer_ms - elapsed_ms;
		if (deadly.timer_ms <= 0) {
			Motion& motion = registry.motions.get(entity);
			switch (deadly.state) {
			case EnemyState::IDLE:
				deadly.state = EnemyState::MOVE;
				deadly.timer_ms = deadly.moving_ms;
				motion.velocity = { uniform_dist(rng), uniform_dist(rng) };
				break;
			case EnemyState::MOVE:
				deadly.state = EnemyState::IDLE;
				deadly.timer_ms = deadly.idle_ms;
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