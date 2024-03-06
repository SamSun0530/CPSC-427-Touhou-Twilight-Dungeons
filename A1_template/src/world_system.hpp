#pragma once

// internal
#include "common.hpp"

// stlib
#include <vector>
#include <random>
#include <array>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "render_system.hpp"
#include "audio.hpp"
#include "map_system.hpp"

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class WorldSystem
{
public:
	WorldSystem();

	// Creates a window
	GLFWwindow* create_window();

	// starts the game
	void init(RenderSystem* renderer, Audio* audio, MapSystem* map);

	// Releases all associated resources
	~WorldSystem();

	// Steps the game ahead by ms milliseconds
	bool step(float elapsed_ms);

	// Check for collisions
	void handle_collisions();

	// Should the game be over ?
	bool is_over()const;
private:
	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 pos);
	void on_mouse_key(int button, int action, int mods);
	void on_scroll(vec2 scroll_offset);

	// State of keyboard
	// Initial state is all false
	std::array<bool, 512> pressed = { 0 };
	// Update player direction based on pressed
	void updatePlayerDirection(Kinematic& player_kinematic);

	// restart level
	void restart_game();

	// OpenGL window handle
	GLFWwindow* window;

	// TEMPORARY
	// Number of coins picked by the player, displayed in the window title
	unsigned int points;

	// Game state
	RenderSystem* renderer;
	float next_enemy_spawn;
	Audio* audio;

	// Player state
	Entity player;
	std::vector<Entity> ui;

	// World Map
	std::vector<std::vector<int>> world_map; // world_map[Row][Col]
	MapSystem* map;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1
};
