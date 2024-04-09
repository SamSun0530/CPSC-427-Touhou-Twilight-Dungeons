#include "visibility_system.hpp"

void VisibilitySystem::restart_map()
{
	// restart visibility map
	map = std::vector<std::vector<int>>(WORLD_HEIGHT_DEFAULT, std::vector<int>(WORLD_WIDTH_DEFAULT, (int)VISIBILITY_STATE::VISIBLE));

	// restart reference map
	reference_map = std::vector<std::vector<int>>(WORLD_HEIGHT_DEFAULT, std::vector<int>(WORLD_WIDTH_DEFAULT, -1));

	close_list.clear();
	next_pos.clear();
	next_num = 0;
	curr_num = 0;
	is_door_found = false;
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
	if (map_level.level == MapLevel::TUTORIAL) return;
	if (!game_info.is_player_id_set) return;

	counter_ms = counter_ms - elapsed_ms <= 0 ? 0 : counter_ms - elapsed_ms;

	// unreadable code - maybe refactor
	// MAYBE??????
	if (counter_ms <= 0) {
		Motion& player_motion = registry.motions.get(game_info.player_id);
		vec2 grid_pos = convert_world_to_grid(player_motion.position);
		if (grid_pos.y >= 0 && grid_pos.y < WORLD_HEIGHT_DEFAULT && grid_pos.x >= 0 && grid_pos.x < WORLD_WIDTH_DEFAULT) {
			if (game_info.in_room != -1) {
				// in a room
				Room_struct& room = game_info.room_index[game_info.in_room];

				// check if it's the first tile that is not visible, otherwise expand on previous
				if (map[grid_pos.y][grid_pos.x] == (int)VISIBILITY_STATE::NOT_VISIBLE && next_pos.empty()) {
					registry.visibilityTileInstanceData.get((Entity)reference_map[grid_pos.y][grid_pos.x]).alpha = 0.5;
					game_info.room_visited[game_info.in_room] = true;
					next_pos.push_back(grid_pos);
					curr_num = 1;
				}
				else if (!next_pos.empty()) {
					next_num = 0;
					while (curr_num > 0 && !next_pos.empty()) {
						coord current = next_pos.front();
						next_pos.pop_front();
						set_tile_visible(current);
						close_list.insert(current);
						for (const coord& ACTION : ACTIONS) {
							coord candidate = current + ACTION;
							if (close_list.count(candidate) == 0 &&
								candidate.x >= 0 && candidate.x < WORLD_WIDTH_DEFAULT &&
								candidate.y >= 0 && candidate.y < WORLD_HEIGHT_DEFAULT &&
								candidate.x >= room.top_left.x - 1 && candidate.x <= room.bottom_right.x + 1 &&
								candidate.y >= room.top_left.y - 1 && candidate.y <= room.bottom_right.y + 1 &&
								map[candidate.y][candidate.x] == (int)VISIBILITY_STATE::NOT_VISIBLE &&
								reference_map[candidate.y][candidate.x] != -1) {
								registry.visibilityTileInstanceData.get((Entity)reference_map[candidate.y][candidate.x]).alpha = 0.5;
								close_list.insert(candidate);
								next_pos.push_back(candidate);
								next_num++;
								if (world_map[candidate.y][candidate.x] == (int)TILE_TYPE::DOOR) {
									// get adjacent neighbors and set their transparency
									for (const coord& ACTION2 : ACTIONS_DIAGONALS) {
										coord door_candidate = candidate + ACTION2;
										if (door_candidate.x >= 0 && door_candidate.x < WORLD_WIDTH_DEFAULT &&
											door_candidate.y >= 0 && door_candidate.y < WORLD_HEIGHT_DEFAULT &&
											map[door_candidate.y][door_candidate.x] == (int)VISIBILITY_STATE::NOT_VISIBLE &&
											reference_map[door_candidate.y][door_candidate.x] != -1) {
											registry.visibilityTileInstanceData.get((Entity)reference_map[door_candidate.y][door_candidate.x]).alpha = 0.5;
										}
									}
								}
							}
						}
						curr_num--;
					}
					curr_num = next_num;
					renderer->set_visibility_tiles_instance_buffer();
				}
			}
			else {
				// in a corridor
				if (map[grid_pos.y][grid_pos.x] == (int)VISIBILITY_STATE::NOT_VISIBLE && next_pos.empty()) {
					registry.visibilityTileInstanceData.get((Entity)reference_map[grid_pos.y][grid_pos.x]).alpha = 0.5;
					is_door_found = false;
					next_pos.push_back(grid_pos);
					curr_num = 1;
				}
				else if (!next_pos.empty()) {
					next_num = 0;
					while (curr_num > 0 && !next_pos.empty()) {
						coord current = next_pos.front();
						next_pos.pop_front();
						set_tile_visible(current);
						close_list.insert(current);
						if (!is_door_found) {
							for (const coord& ACTION : ACTIONS) {
								coord candidate = current + ACTION;
								if (close_list.count(candidate) == 0 &&
									candidate.x >= 0 && candidate.x < WORLD_WIDTH_DEFAULT &&
									candidate.y >= 0 && candidate.y < WORLD_HEIGHT_DEFAULT &&
									map[candidate.y][candidate.x] == (int)VISIBILITY_STATE::NOT_VISIBLE &&
									reference_map[candidate.y][candidate.x] != -1) {
									registry.visibilityTileInstanceData.get((Entity)reference_map[candidate.y][candidate.x]).alpha = 0.5;
									close_list.insert(candidate);
									next_pos.push_back(candidate);
									next_num++;
									if (world_map[candidate.y][candidate.x] == (int)TILE_TYPE::DOOR) {
										is_door_found = true;
										// get adjacent neighbors and set their transparency
										for (const coord& ACTION2 : ACTIONS_DIAGONALS) {
											coord door_candidate = candidate + ACTION2;
											if (door_candidate.x >= 0 && door_candidate.x < WORLD_WIDTH_DEFAULT &&
												door_candidate.y >= 0 && door_candidate.y < WORLD_HEIGHT_DEFAULT &&
												map[door_candidate.y][door_candidate.x] == (int)VISIBILITY_STATE::NOT_VISIBLE &&
												reference_map[door_candidate.y][door_candidate.x] != -1) {
												registry.visibilityTileInstanceData.get((Entity)reference_map[door_candidate.y][door_candidate.x]).alpha = 0.5;
											}
										}
									}
								}
							}
						}
						curr_num--;
					}
					curr_num = next_num;
					renderer->set_visibility_tiles_instance_buffer();
				}
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