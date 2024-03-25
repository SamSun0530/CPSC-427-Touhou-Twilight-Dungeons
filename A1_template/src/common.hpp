#pragma once

// stlib
#include <fstream> // stdout, stderr..
#include <string>
#include <tuple>
#include <vector>
#include <random>
#include <memory>

// glfw (OpenGL)
#define NOMINMAX
#include <gl3w.h>
#include <GLFW/glfw3.h>

// The glm library provides vector and matrix operations as in GLSL
#include <glm/vec2.hpp>				// vec2
#include <glm/ext/vector_int2.hpp>  // ivec2
#include <glm/vec3.hpp>             // vec3
#include <glm/mat3x3.hpp>           // mat3
using namespace glm;

#include "tiny_ecs.hpp"

typedef vec2 coord;
typedef std::vector<coord> path;

// Simple utility functions to avoid mistyping directory name
// audio_path("audio.ogg") -> data/audio/audio.ogg
// Get defintion of PROJECT_SOURCE_DIR from:
#include "../ext/project_path.hpp"
inline std::string data_path() { return std::string(PROJECT_SOURCE_DIR) + "data"; };
inline std::string shader_path(const std::string& name) { return std::string(PROJECT_SOURCE_DIR) + "/shaders/" + name; };
inline std::string textures_path(const std::string& name) { return data_path() + "/textures/" + std::string(name); };
inline std::string audio_path(const std::string& name) { return data_path() + "/audio/" + std::string(name); };
inline std::string mesh_path(const std::string& name) { return data_path() + "/meshes/" + std::string(name); };
inline std::string misc_path(const std::string& name) { return data_path() + "/misc/" + std::string(name); };
inline std::string font_paht(const std::string& name) { return data_path() + "/fonts" + std::string(name); };

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// Window Data
const int window_width_px = 1280;
const int window_height_px = 720;
const vec2 window_px_half = { window_width_px / 2, window_height_px / 2 };

// World Map Data
const int world_width = 60;
const int world_height = 50;
const int world_tile_size = 64; // In pixels
// Shift POSITIVE x,y grid cells to center the map at (0,0)
const vec2 world_center = { 11,11 };
// Converts (x,y) in world coordinates to grid coordinates
coord convert_world_to_grid(coord world_coord);
// Converts (x,y) in grid coordinates to world coordinates
coord convert_grid_to_world(coord grid_coord);

// The 'Transform' component handles transformations passed to the Vertex shader
// (similar to the gl Immediate mode equivalent, e.g., glTranslate()...)
// We recomment making all components non-copyable by derving from ComponentNonCopyable
struct Transform {
	mat3 mat = { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f}, { 0.f, 0.f, 1.f} }; // start with the identity
	void scale(vec2 scale);
	void rotate(float radians);
	void translate(vec2 offset);
};

bool gl_has_errors();

vec2 vec2_lerp(vec2 start, vec2 end, float t);

// Checks if (x,y) on the map is valid, this is not world coordinates
bool is_valid_cell(int x, int y);

// TEMPORARY
struct Room2 {
	vec2 top_left;
	vec2 bottom_left;
};