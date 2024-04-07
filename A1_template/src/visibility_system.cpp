#include "visibility_system.hpp"

void VisibilitySystem::restart_map()
{
	// restart visibility map
	map = std::vector<std::vector<int>>(WORLD_HEIGHT_DEFAULT, std::vector<int>(WORLD_WIDTH_DEFAULT, (int)VISIBILITY_STATE::VISIBLE));

	// restart reference map
	reference_map = std::vector<std::vector<int>>(WORLD_HEIGHT_DEFAULT, std::vector<int>(WORLD_WIDTH_DEFAULT, -1));
}

void VisibilitySystem::init_visibility()
{
	for (int i = 0; i < WORLD_HEIGHT_DEFAULT; ++i) {
		for (int j = 0; j < WORLD_WIDTH_DEFAULT; ++j) {
			map[i][j] = world_map[i][j] == (int)TILE_TYPE::EMPTY ? (int)VISIBILITY_STATE::VISIBLE : (int)VISIBILITY_STATE::NOT_VISIBLE;
		}
	}
}

// must be after world init
void VisibilitySystem::init(RenderSystem* renderer_arg)
{
	this->renderer = renderer_arg;

	restart_map();
}

void VisibilitySystem::step(float elapsed_ms)
{
	if (!game_info.is_player_id_set) return;

	Motion& player_motion = registry.motions.get(game_info.player_id);
	vec2 grid_pos = convert_world_to_grid(player_motion.position);
	if (grid_pos.y >= 0 && grid_pos.y < WORLD_HEIGHT_DEFAULT && grid_pos.x >= 0 && grid_pos.x < WORLD_WIDTH_DEFAULT &&
		map[grid_pos.y][grid_pos.x] == (int)VISIBILITY_STATE::NOT_VISIBLE) {
		//set_tile_visible(grid_pos); // TODO: instead of one tile, use flood fill -> need rooms!



		renderer->set_visibility_tiles_instance_buffer();
	}
}

void VisibilitySystem::set_tile_visible(coord grid_pos) {
	if (reference_map[grid_pos.y][grid_pos.x] != -1 && map[grid_pos.y][grid_pos.x] == (int)VISIBILITY_STATE::NOT_VISIBLE ) {
		Entity entity = (Entity) reference_map[grid_pos.y][grid_pos.x];
		map[grid_pos.y][grid_pos.x] = (int)VISIBILITY_STATE::VISIBLE;
		registry.remove_all_components_of(entity);
		reference_map[grid_pos.y][grid_pos.x] = -1;
	}
}

void VisibilitySystem::print_visibility_map()
{
	printf(">>>>>>>>>>>>>>> VISIBILITY MAP <<<<<<<<<<<<<<<<\n");
	for (int i = 0; i < map.size(); ++i) {
		for (int j = 0; j < map[i].size(); ++j) {
			printf("%d ", map[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

void VisibilitySystem::print_reference_map()
{
	printf(">>>>>>>>>>>>>>> REFERENCE MAP <<<<<<<<<<<<<<<<\n");
	for (int i = 0; i < reference_map.size(); ++i) {
		for (int j = 0; j < reference_map[i].size(); ++j) {
			printf("%d ", reference_map[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}