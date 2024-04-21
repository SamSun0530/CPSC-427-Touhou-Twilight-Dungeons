#pragma once

#include "common.hpp"
#include "components.hpp"

// World Map Data
extern std::vector<std::vector<int>> world_map;
extern int world_width;
extern int world_height;
extern int world_tile_size; // In pixels
// Shift POSITIVE x,y grid cells to center the map at (0,0)
extern vec2 world_center;
// Converts (x,y) in world coordinates to grid coordinates
coord convert_world_to_grid(coord world_coord);
// Converts (x,y) in grid coordinates to world coordinates
coord convert_grid_to_world(coord grid_coord);
// Reset all world attributes to default specified in common.hpp
void reset_world_default();
