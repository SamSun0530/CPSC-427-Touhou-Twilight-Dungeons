#pragma once

#include <vector>
#include <random>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"

class AISystem
{
public:
	AISystem();

	void step(float elapsed_ms);

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist{ -1,1 }; // number between -1..1
};