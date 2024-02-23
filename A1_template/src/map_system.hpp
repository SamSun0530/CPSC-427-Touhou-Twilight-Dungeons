#pragma once

#include <vector>
#include <random>
#include <algorithm>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"
#include "physics_system.hpp"

class MapSystem{
private:
    //bar
    const float mean = 10.0f;
    const float standard_deviation = 3.0f;

    vec2 getRandomPointInCircle(int radius);
    vec2 getUniformRectangleDimentions(vec2 widthRange, vec2 heightRange);

    // C++ random number generator
    std::default_random_engine rng;
    std::uniform_real_distribution<float> uniform_dist; // number between 0..1
    std::normal_distribution<float>  normal_dist{mean,standard_deviation};
public:
    MapSystem();
    void generateMap(int floor);
    //foo
};