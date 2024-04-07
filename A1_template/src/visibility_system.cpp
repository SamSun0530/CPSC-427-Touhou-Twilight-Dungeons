#include "visibility_system.hpp"

void VisibilitySystem::restart_map()
{
	// restart visibility map
	map = std::vector<std::vector<int>>(WORLD_HEIGHT_DEFAULT, std::vector<int>(WORLD_WIDTH_DEFAULT, 0));

	for (int i = 0; i < WORLD_HEIGHT_DEFAULT; ++i) {
		for (int j = 0; j < WORLD_WIDTH_DEFAULT; ++j) {
			map[i][j] = world_map[i][j] == (int)TILE_TYPE::EMPTY ? 0 : 1;
		}
	}

	// restart reference map
	reference_map = std::vector<std::vector<int>>(WORLD_HEIGHT_DEFAULT, std::vector<int>(WORLD_WIDTH_DEFAULT, -1));
}

void VisibilitySystem::init()
{
	restart_map();
}

void VisibilitySystem::step(float elapsed_ms) 
{
	
}