#pragma once

#include <vector>
#include <random>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"
#include "decision_tree.hpp"

#include "world_system.hpp"
#include "world_init.hpp"

class AISystem
{
public:
	AISystem();

	// Initialize decision trees
	void init();

	void step(float elapsed_ms);

private:
	// Decision trees for different ai entities
	DecisionTree ghost;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist{ -1,1 }; // number between -1..1
};