#pragma once

#include "tiny_ecs_registry.hpp"
#include "common.hpp"

static int bullet_phase_id_count = 0;

struct BulletPhase {
	int id;
	BulletPattern bullet_pattern;
	BulletSpawner bullet_spawner;
};

class BossSystem
{
private:
	// phase-indexed bullet patterns and spawner (e.g. 1st phase indexed at 0)
	std::vector<std::vector<BulletPhase>> bullet_phases;

	void set_random_phase(Boss& boss, std::mt19937& gen, const Entity& entity);
public:
	BossSystem();
	void init_phases();
	void step(float elapsed_ms);
};