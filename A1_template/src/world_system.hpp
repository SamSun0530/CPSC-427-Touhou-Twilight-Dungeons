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
#include <map>

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class WorldSystem
{
public:
	WorldSystem();

	// Creates a window
	GLFWwindow* create_window();

	// starts the game
	void init(RenderSystem* renderer, Audio* audio);

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
	//void renderText(const std::string& text, float x, float y, float scale, const glm::vec3& color, const glm::mat4& trans);

	// State of keyboard
	// Initial state is all false
	std::array<bool, 512> pressed = { 0 };
	// Update player direction based on pressed
	void WorldSystem::updatePlayerDirection(Kinematic& player_kinematic);

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

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1

	int fps;
	bool show_fps = false;
	bool display_instruction = false;

	//struct Character {
	//	unsigned int TextureID;  // ID handle of the glyph texture
	//	glm::ivec2   Size;       // Size of glyph
	//	glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
	//	unsigned int Advance;    // Offset to advance to next glyph
	//	char character;
	//};

	GLuint m_shaderProgram;
	GLuint m_VAO;
	GLuint m_VBO;
	GLuint m_dirt_texture;
	std::vector<GLuint> m_alien_textures;

	// fonts
	std::map<char, Character> m_ftCharacters;
	GLuint m_font_shaderProgram;
	GLuint m_font_VAO;
	GLuint m_font_VBO;
};
