#pragma once

// internal
#include "common.hpp"
#include "global.hpp"
// stlib
#include <vector>
#include <random>
#include <array>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>
#include <iostream>
#include <sstream>

#include "render_system.hpp"
#include "audio.hpp"
#include "map_system.hpp"
#include "ai_system.hpp"
#include <map>
#include "visibility_system.hpp"
#include "boss_system.hpp"

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
	void WorldSystem::init(RenderSystem* renderer_arg, Audio* audio, MapSystem* map, AISystem* ai, VisibilitySystem* visibility_arg, BossSystem* boss_arg);

	// initialize the menu
	void init_menu();
	void init_pause_menu();
	void init_win_menu();
	void init_lose_menu();
	void init_infographic_menu();
	void resume_game();

	// Releases all associated resources
	~WorldSystem();

	// Ai system - for restarting flow field
	AISystem* ai;

	// Bullet system - for initializing boss phases
	BossSystem* boss_system;

	// Steps the game ahead by ms milliseconds
	bool step(float elapsed_ms);

	// Check for collisions
	void handle_collisions();
	// handle_wall_collisions parameter entity IS WALL ENTITY!
	void handle_wall_collisions(Entity& entity, Entity& entity_other);

	// Should the game be over ?
	bool is_over() const;

	// font and instructions
	bool get_display_instruction();
	int get_tutorial_counter();
	bool get_show_fps();
	std::string get_fps_in_string();
	void toggle_display_instruction() { display_instruction = !display_instruction; }
	void WorldSystem::dialogue_step(float elapsed_time);
	void toggle_show_fps() { show_fps = !show_fps; }
	// Updates focus mode position
	// Should be called after physics step
	// Fixes issue where dot lags behind player due to physics lerp step after setting position
	void update_focus_dot();
private:
	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 pos);
	void on_mouse_key(int button, int action, int mods);
	void on_scroll(vec2 scroll_offset);
	unsigned int loadScript(std::string file_name, std::vector<std::string>& scripts);
	//void renderText(const std::string& text, float x, float y, float scale, const glm::vec3& color, const glm::mat4& trans);

	// State of keyboard
	// Initial state is all false
	std::array<bool, 512> pressed = { 0 };

	// Dialogue scripts
	std::vector<std::string> start_script;
	std::vector<std::string> cirno_script;
	std::vector<std::string> marisa_script;
	std::vector<std::string> flandre_script;
	std::vector<std::string> cirno_after_script;
	std::vector<std::string> flandre_after_script;
	unsigned int start_pt = 0;
	float start_dialogue_timer = 1000.f;
	float word_up_ms = 50.f;
	unsigned int curr_word = 0;
	std::string start_buffer;

	// Bomb stuff (resets at restart_game)
	float bomb_timer = 0.f;
	float bomb_timer_max = 500.f;
	int bomb_damage = 50;
	void deal_damage_to_deadly(const Entity& entity, int damage);

	// Update player direction based on pressed
	void updatePlayerDirection(Kinematic& player_kinematic);

	// restart level
	void restart_game();
	void WorldSystem::next_level();
	// OpenGL window handle
	GLFWwindow* window;

	// TEMPORARY
	// Number of coins picked by the player, displayed in the window title
	unsigned int points;

	// Game state
	RenderSystem* renderer;
	VisibilitySystem* visibility_system;

	float next_enemy_spawn;
	Audio* audio;
	int tutorial_counter = 10;
	int fps;
	bool show_fps = false;
	bool display_instruction;
	float elapsedSinceLastFPSUpdate = 0.0f;
	float tutorial_timer = 10000.0f;
	Entity display_combo;

	// fonts seting
	//std::string font_filename = "..//..//..//data//fonts//OpenSans-Bold.ttf";
	std::string font_filename = "..//..//..//data//fonts//pixelmix//pixelmix.ttf";
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
