
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stlib
#include <chrono>

// internal
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "map_system.hpp"
#include "ai_system.hpp"
#include "bullet_system.hpp"
#include "audio.hpp"
#include "animation.hpp"
#include "boss_system.hpp"
#include "components.hpp"

using Clock = std::chrono::high_resolution_clock;

// Entry point
int main()
{
	// Global systems
	WorldSystem world;
	RenderSystem renderer;
	PhysicsSystem physics;
	AISystem ai;
	BulletSystem bullets;
	MapSystem map;
	Animation animation;
	BossSystem boss_system;

	// Global classes
	Audio audio;

	// Initializing window
	GLFWwindow* window = world.create_window();
	if (!window) {
		// Time to read the error message
		printf("Press any key to exit");
		getchar();
		return EXIT_FAILURE;
	}


	// initialize the main systems
	renderer.init(window);
	world.init(&renderer, &audio, &map, &ai);
	bullets.init(&renderer, window, &audio);
	ai.init();
	map.init(&renderer);
	animation.init(&renderer, window);
	world.init_menu();

	// variable timestep loop
	auto t = Clock::now();
	while (!world.is_over()) {
		// Processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// Calculating elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;

		if (menu.state == MENU_STATE::MAIN_MENU) {

		}
		else if (menu.state == MENU_STATE::PLAY) {
			elapsed_ms = combo_mode.combo_meter * elapsed_ms;
			world.step(elapsed_ms);
			boss_system.step(elapsed_ms);
			animation.step(elapsed_ms);
			physics.step(elapsed_ms);
			world.update_focus_dot();
			ai.step(elapsed_ms);
			bullets.step(elapsed_ms);
			world.handle_collisions();
		}
		else if (menu.state == MENU_STATE::PAUSE) {

		}

		// map.debug(); // Just to visualize the map

		renderer.draw();
	}

	return EXIT_SUCCESS;
}
