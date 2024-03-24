#include "boss_system.hpp"

BossSystem::BossSystem() {
	init_phases();
}

void BossSystem::step(float elapsed_ms) {
	for (Entity entity : registry.bosses.entities) {
		Boss& boss = registry.bosses.get(entity);
		HP& hp = registry.hps.get(entity);
		if (boss.phase_index < boss.health_phase_thresholds.size() && 
			hp.curr_hp <= boss.health_phase_thresholds[boss.phase_index]) {
			boss.phase_index++;
			std::random_device ran;
			std::mt19937 gen(ran());
			set_random_phase(boss, gen, entity);
		}

		if (boss.duration != -1) {
			boss.current_duration += elapsed_ms;
			if (boss.current_duration >= boss.duration) {
				std::random_device ran;
				std::mt19937 gen(ran());
				set_random_phase(boss, gen, entity);
				boss.current_duration = 0;
			}
		}
	}
}

void BossSystem::set_random_phase(Boss& boss, std::mt19937& gen, const Entity& entity)
{
	// check if phase index is in bounds
	if (boss.phase_index < bullet_phases.size()) {
		// given a random premade bullet phase,
		// if no bullet pattern -> uses bullet spawner only
		// if no bullet spawner -> uses previous spawner
		std::vector<BulletPhase>& phases = bullet_phases[boss.phase_index];
		if (phases.size() == 0) return; // no need to set phase if there are no phases
		std::uniform_int_distribution<> int_distrib(0, phases.size() - 1);
		int random_index = int_distrib(gen);
		BulletPhase& phase = phases[random_index];
		// if the new randomly generated phase is the same, don't bother setting phase
		if (boss.current_bullet_phase_id == phase.id) return;
		if (phase.bullet_spawner != BulletSpawner()) {
			if (!registry.bulletSpawners.has(entity)) registry.bulletSpawners.emplace(entity);
			BulletSpawner& boss_spawner = registry.bulletSpawners.get(entity);
			boss_spawner = phase.bullet_spawner;
		}
		boss.bullet_pattern = phase.bullet_pattern;
		boss.current_bullet_phase_id = phase.id;
	}
}

void BossSystem::init_phases() {
	BulletPhase b_phase;
	BulletPattern b_pattern;
	BulletSpawner bs;

	// currently assume we have 4 phases (index 0 phase empty)
	bullet_phases = std::vector<std::vector<BulletPhase>>(5);

	bs = BulletSpawner();
	bs.fire_rate = 2;
	bs.is_firing = true;
	bs.spin_rate = 1;
	bs.invert = false;
	bs.spin_delta = 0.f;
	bs.max_spin_rate = 20.f;
	bs.total_bullet_array = 1;
	bs.spread_between_array = 0;
	bs.bullets_per_array = 4;
	bs.spread_within_array = 90;
	bs.bullet_initial_speed = 100;
	b_phase.bullet_spawner = bs;
	b_phase.id = bullet_phase_id_count++;
	bullet_phases[1].push_back(b_phase);

	bs = BulletSpawner();
	bs.fire_rate = 2;
	bs.is_firing = true;
	bs.spin_rate = 13;
	bs.invert = false;
	bs.spin_delta = 0.f;
	bs.max_spin_rate = 20.f;
	bs.total_bullet_array = 3;
	bs.spread_between_array = 118;
	bs.bullets_per_array = 1;
	bs.spread_within_array = 21;
	bs.bullet_initial_speed = 50;
	b_phase.bullet_spawner = bs;
	b_phase.id = bullet_phase_id_count++;
	bullet_phases[2].push_back(b_phase);

	bs = BulletSpawner();
	bs.fire_rate = 1;
	bs.is_firing = true;
	bs.spin_rate = 20;
	bs.invert = false;
	bs.spin_delta = 0.f;
	bs.max_spin_rate = 20.f;
	bs.total_bullet_array = 3;
	bs.spread_between_array = 120;
	bs.bullets_per_array = 3;
	bs.spread_within_array = 30;
	bs.bullet_initial_speed = 50;
	b_phase.bullet_spawner = bs;
	b_phase.id = bullet_phase_id_count++;
	bullet_phases[3].push_back(b_phase);

	bs = BulletSpawner();
	b_pattern = BulletPattern();
	bs.fire_rate = 2;
	bs.is_firing = true;
	bs.spin_rate = 2;
	bs.invert = false;
	bs.spin_delta = 1.f;
	bs.max_spin_rate = 20.f;
	bs.total_bullet_array = 3;
	bs.spread_between_array = 30;
	bs.bullets_per_array = 4;
	bs.spread_within_array = 90;
	bs.bullet_initial_speed = 100;
	bs.cooldown_rate = 70;
	bs.number_to_fire = 10;
	b_pattern.commands = {
		{ BULLET_ACTION::DELAY, 1000.f },
		{ BULLET_ACTION::ROTATE, 20.f },
		{ BULLET_ACTION::DELAY, 100.f },
		{ BULLET_ACTION::LOOP, vec2(10, 1)},
		{ BULLET_ACTION::DELAY, 1500.f },
		{ BULLET_ACTION::ROTATE, 30.f },
		{ BULLET_ACTION::DELAY, 150.f },
		{ BULLET_ACTION::LOOP, vec2(10, 4)},
		{ BULLET_ACTION::DELAY, 5000.f },
		{ BULLET_ACTION::DEL, 0.f },
	};
	b_phase.bullet_pattern = b_pattern;
	b_phase.bullet_spawner = bs;
	b_phase.id = bullet_phase_id_count++;
	bullet_phases[4].push_back(b_phase);

	bs = BulletSpawner();
	b_pattern = BulletPattern();
	bs.fire_rate = 1;
	bs.is_firing = true;
	bs.spin_rate = 2;
	bs.invert = false;
	bs.spin_delta = 1.f;
	bs.max_spin_rate = 20.f;
	bs.total_bullet_array = 3;
	bs.spread_between_array = 30;
	bs.bullets_per_array = 4;
	bs.spread_within_array = 90;
	bs.bullet_initial_speed = 100;
	bs.cooldown_rate = 100;
	bs.number_to_fire = 5;
	b_pattern.commands = {
		{ BULLET_ACTION::DELAY, 500.f },
		{ BULLET_ACTION::DIRECTION, vec2(0,1) },
		{ BULLET_ACTION::DELAY, 1500.f },
		{ BULLET_ACTION::DIRECTION, vec2(0,0) },
		{ BULLET_ACTION::DELAY, 1000.f },
		{ BULLET_ACTION::DIRECTION, vec2(1,1) },
		{ BULLET_ACTION::SPLIT, vec3(10, 36, -100)},
	};
	b_phase.bullet_pattern = b_pattern;
	b_phase.bullet_spawner = bs;
	b_phase.id = bullet_phase_id_count++;
	bullet_phases[4].push_back(b_phase);

}