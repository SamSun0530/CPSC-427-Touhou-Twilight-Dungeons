#include "global.hpp"

vec2 world_center = { 9, 9 };
int world_width = 60;
int world_height = 50;
int world_tile_size = 64; // In pixels

coord convert_world_to_grid(coord world_coord) {
	return round((world_coord / (float)world_tile_size) + world_center);
}

coord convert_grid_to_world(coord grid_coord) {
	return (grid_coord - world_center) * (float)world_tile_size;
}
