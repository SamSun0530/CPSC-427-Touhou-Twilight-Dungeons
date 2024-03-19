#pragma once

#include <vector>
#include <random>
#include <algorithm>

#include "tiny_ecs_registry.hpp"
#include "render_system.hpp"
#include "common.hpp"
#include "physics_system.hpp"

// Grid coordinates: x, y, size
struct Room {
	int id;
	int x;
	int y;
	vec2 size; // (x,y) = (width,height) of room
	std::vector<std::vector<int>> grid;
};

struct roomNode {
	Entity room;
	std::vector<roomNode*> neighbors;
};

struct Vertex {
	int id;
	int x;
	int y;
};

struct Edge {
	int id;
	Vertex point_start;
	Vertex point_end;
};

struct Circle {
	vec2 center;
	float radius;
};

struct Triangle {
	Vertex point_A;
	Vertex point_B;
	Vertex point_C;
	Circle circumcircle;
};

static int room_id = 0;

class MapSystem {
private:
	const float mean = 5.0f;
	const float standard_deviation = 3.0f;

	// game systems
	RenderSystem* renderer;

	vec2 getRandomPointInCircle(int radius);
	vec2 getUniformRectangleDimentions(vec2 widthRange, vec2 heightRange);
	vec2 roundToTileSizeVec2(vec2 input, float tileSize);
	void addTile(int x, int y, std::vector<TEXTURE_ASSET_ID>& textureIDs, std::vector<std::vector<int>>& map);
	void generateAllEntityTiles(std::vector<std::vector<int>>& map);
	std::vector<TEXTURE_ASSET_ID> getTileAssetID(int row, int col, std::vector<std::vector<int>>& map);

	std::vector<Room> rooms;

	std::vector<Vertex*> verticies;
	std::vector<Edge*> edges;
	std::unordered_map<int, Vertex*> vertexLookup;
	std::unordered_map<int, Edge*> edgeLookup;
	int nodeIdCounter;
	int edgeIdCounter;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1
	std::normal_distribution<float>  normal_dist{ mean,standard_deviation };
public:
	MapSystem();
	void init(RenderSystem* renderer);
	std::vector<std::vector<int>> world_map; // world_map[Row][Col]
	void generateMap(int floor);
	void generateBasicMap();
	void generateBossRoom();
	void spawnEnemies();
	void debug();
};