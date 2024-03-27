#include "global.hpp"

vec2 world_center = WORLD_CENTER_DEFAULT;
int world_width = WORLD_WIDTH_DEFAULT;
int world_height = WORLD_HEIGHT_DEFAULT;
int world_tile_size = WORLD_TILE_SIZE_DEFAULT; // In pixels

coord convert_world_to_grid(coord world_coord) {
	return round((world_coord / (float)world_tile_size) + world_center);
}

coord convert_grid_to_world(coord grid_coord) {
	return (grid_coord - world_center) * (float)world_tile_size;
}

void reset_world_default() {
	world_center = WORLD_CENTER_DEFAULT;
	world_width = WORLD_WIDTH_DEFAULT;
	world_height = WORLD_HEIGHT_DEFAULT;
	world_tile_size = WORLD_TILE_SIZE_DEFAULT; // In pixels
}