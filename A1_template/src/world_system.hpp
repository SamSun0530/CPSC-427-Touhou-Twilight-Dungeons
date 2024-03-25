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
#include <map>

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class WorldSystem
{
public:
	static WorldSystem& getInstance() {
		static WorldSystem instance; // Guaranteed to be destroyed and instantiated on first use.
		return instance;
	}
	WorldSystem(WorldSystem const&) = delete;
	void operator=(WorldSystem const&) = delete;

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

	// World Map
	static std::vector<std::vector<int>> world_map; // world_map[Row][Col]

	// font and instructions
	bool get_display_instruction();
	int get_tutorial_counter();
	bool get_show_fps();
	std::string get_fps_in_string();
	void toggle_display_instruction() { display_instruction = !display_instruction; }
	void toggle_show_fps() { show_fps = !show_fps; }
	float combo_meter;
	float COMBO_METER_MAX = 1.5f;
private:
	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 pos);
	void on_mouse_key(int button, int action, int mods);
	void on_scroll(vec2 scroll_offset);
	//void renderText(const std::string& text, float x, float y, float scale, const glm::vec3& color, const glm::mat4& trans);

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
	int fps;
	bool show_fps = false;
	bool display_instruction = false;
	float elapsedSinceLastFPSUpdate = 0.0f;

	// fonts seting
	std::string font_filename = "..//..//..//data//fonts//Kenney_Future_Narrow.ttf";
	unsigned int font_default_size = 25;

	// Player state
	Entity player;
	std::vector<Entity> ui;
	
	// World Map
	MapSystem* map;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1

	// fonts
	std::map<char, Character> m_ftCharacters;
	GLuint m_font_shaderProgram;
	GLuint m_font_VAO;
	GLuint m_font_VBO;
};
