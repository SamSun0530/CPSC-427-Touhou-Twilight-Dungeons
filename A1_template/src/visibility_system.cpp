#include "visibility_system.hpp"

void VisibilitySystem::restart_map()
{
	// restart visibility map
	map = std::vector<std::vector<int>>(WORLD_HEIGHT_DEFAULT, std::vector<int>(WORLD_WIDTH_DEFAULT, (int)VISIBILITY_STATE::VISIBLE));

	// restart reference map
	reference_map = std::vector<std::vector<int>>(WORLD_HEIGHT_DEFAULT, std::vector<int>(WORLD_WIDTH_DEFAULT, -1));

	next_pos.clear();
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

	counter_ms = counter_ms - elapsed_ms <= 0 ? 0 : counter_ms - elapsed_ms;

	if (counter_ms <= 0) {
		// in a room
		Motion& player_motion = registry.motions.get(game_info.player_id);
		vec2 grid_pos = convert_world_to_grid(player_motion.position);
		if (grid_pos.y >= 0 && grid_pos.y < WORLD_HEIGHT_DEFAULT && grid_pos.x >= 0 && grid_pos.x < WORLD_WIDTH_DEFAULT) {
			if (game_info.in_room != -1) {
				Room2& room = game_info.room_index[game_info.in_room];

				// check if it's the first tile that is not visible, otherwise expand on previous
				if (next_pos.empty() && map[grid_pos.y][grid_pos.x] == (int)VISIBILITY_STATE::NOT_VISIBLE) {
					game_info.room_visited[game_info.in_room] = true;
					next_pos.push_back(grid_pos);
					curr_num = 1;
				}
				else {
					next_num = 0;
					while (curr_num > 0 && !next_pos.empty()) {
						coord current = next_pos.front();
						next_pos.pop_front();
						set_tile_visible(current);
						for (const coord& ACTION : ACTIONS) {
							coord candidate = current + ACTION;
							if (candidate.x >= 0 && candidate.x < WORLD_WIDTH_DEFAULT &&
								candidate.y >= 0 && candidate.y < WORLD_HEIGHT_DEFAULT &&
								candidate.x >= room.top_left.x - 1 && candidate.x <= room.bottom_left.x + 1 &&
								candidate.y >= room.top_left.y - 1 && candidate.y <= room.bottom_left.y + 1 &&
								map[candidate.y][candidate.x] == (int)VISIBILITY_STATE::NOT_VISIBLE &&
								reference_map[candidate.y][candidate.x] != -1) {
								next_pos.push_back(candidate);
								next_num++;
							}
						}
						curr_num--;
					}
					curr_num = next_num;
					renderer->set_visibility_tiles_instance_buffer();
				}
			}
			else {

			}
		}

		counter_ms = counter_ms_default;
	}

}

void VisibilitySystem::set_tile_visible(coord grid_pos) {
	if (reference_map[grid_pos.y][grid_pos.x] != -1 && map[grid_pos.y][grid_pos.x] == (int)VISIBILITY_STATE::NOT_VISIBLE) {
		Entity entity = (Entity)reference_map[grid_pos.y][grid_pos.x];
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