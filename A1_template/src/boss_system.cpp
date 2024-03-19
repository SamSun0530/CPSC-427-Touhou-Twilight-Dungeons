#include "boss_system.hpp"

void BossSystem::step(float elapsed_ms) {
	for (Entity entity : registry.bosses.entities) {
		Boss& boss = registry.bosses.get(entity);
		HP& hp = registry.hps.get(entity);

		// TODO: phase changes
		if (hp.curr_hp < 5) {
		}
	}
}
