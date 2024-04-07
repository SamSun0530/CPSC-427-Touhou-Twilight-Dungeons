#include "visibility_system.hpp"

void VisibilitySystem::restart_map()
{
	map = std::vector<std::vector<int>>(WORLD_HEIGHT_DEFAULT, std::vector<int>(WORLD_WIDTH_DEFAULT, 0));

	for (int i = 0; i < WORLD_HEIGHT_DEFAULT; ++i) {
		for (int j = 0; j < WORLD_WIDTH_DEFAULT; ++j) {
			map = world_map[i][j] == (int)TILE_TYPE::EMPTY ? 0 : 1;
		}
	}
}

void VisibilitySystem::step(float elapsed_ms) 
{
	
}