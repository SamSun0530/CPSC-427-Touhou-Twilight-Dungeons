#pragma once

#include "common.hpp"
#include "components.hpp"
#include "global.hpp"

class VisibilitySystem {
public:
	// copy of the world_map size
	// 0 - visible, no quad rendered on top
	// 1 - not visible, black quad rendered on top
	std::vector<std::vector<int>> map;
	// copy of the world_map size
	// mapping between map and entity id, allows for deletion
	// each cell is either:
	// -1 - no entity
	// number != -1 - entity id
	std::vector<std::vector<int>> reference_map;

	// restart visibility and reference maps
	void restart_map();
	void step(float elapsed_ms);
	void init();
private:
};