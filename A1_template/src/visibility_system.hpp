#pragma once

#include "common.hpp"
#include "components.hpp"
#include "global.hpp"
#include "tiny_ecs_registry.hpp"
#include "render_system.hpp"

enum class VISIBILITY_STATE {
	VISIBLE, // 0
	NOT_VISIBLE // 1
};

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

	// restart visibility and reference maps to default 0 and -1 respectively
	// IMPORTANT ORDER OF OPERATIONS:
	// (1) Call VisibilitySystem::restart_map -> map to 0, reference_map to -1
	// (2) Call MapSystem::generate_all_tiles -> createTile -> populates reference_map with entities
	// (3) Call VisibilitySystem::init_visibility -> populates map with 1 for floors/walls
	// (4) Call RenderSystem::set_visibility_tiles_instance_buffer_max (takes info from (2))
	// Note: 
	// - Calling VisibilitySystem::restart_map not at (1) will result in error
	// - (3) and (4) are interchangeable
	void restart_map();
	// sets 1s and 0s based on world_map
	void init_visibility();
	void step(float elapsed_ms);
	void init(RenderSystem* renderer_arg);

	// Utilities
	void print_visibility_map();
	void print_reference_map();
private:
	// set tile to be visible by removing visibility tile
	void set_tile_visible(coord grid_pos);

	// Game state
	RenderSystem* renderer;
};