#pragma once

#include <vector>
#include <random>
#include <algorithm>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"
#include "physics_system.hpp"

struct roomNode{
    Entity room;
    std::vector<roomNode*> neighbors;
};

struct Vertex{
    int id;
    int x;
    int y;
};

struct Edge{
    int id;
    Vertex point_start;
    Vertex point_end;
};

struct Triangle{
    int id;
    Vertex point_A;
    Vertex point_B;
    Vertex point_C;
};

struct Circle{
    vec2 center;
    float radius;
};
class MapSystem{
private:
    //bar
    const float mean = 10.0f;
    const float standard_deviation = 3.0f;

    vec2 getRandomPointInCircle(int radius);
    vec2 getUniformRectangleDimentions(vec2 widthRange, vec2 heightRange);
    vec2 roundToTileSizeVec2(vec2 input, float tileSize);

    static std::vector<Entity> rooms;



    std::vector<Vertex*> verticies;
    std::vector<Edge*> edges;
    std::unordered_map<int, Vertex*> vertexLookup;
    std::unordered_map<int, Edge*> edgeLookup;
    int nodeIdCounter;
    int edgeIdCounter;

    // C++ random number generator
    std::default_random_engine rng;
    std::uniform_real_distribution<float> uniform_dist; // number between 0..1
    std::normal_distribution<float>  normal_dist{mean,standard_deviation};
public:
    MapSystem();
    void generateMap(int floor);
    void debug();
    //foo
};