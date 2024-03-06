
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stlib
#include <chrono>

// internal
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "ai_system.hpp"
#include "bullet_system.hpp"
#include "audio.hpp"
#include "animation.hpp"

// debug
#include "time_debug.hpp"
#include <iostream>

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
	Animation animation;
	
	// Global classes
	Audio audio;
	TimeDebug time_debug;

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
	world.init(&renderer, &audio);
	bullets.init(&renderer, window, &audio);
	ai.init();
	animation.init(&renderer, window);

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
		
		//printf("=============\n");

		//time_debug.initTime();
		world.step(elapsed_ms);
		//time_debug.getTime("world");

		//time_debug.initTime();
		animation.step(elapsed_ms);
		//time_debug.getTime("animation");

		//time_debug.initTime();
		physics.step(elapsed_ms);
		//time_debug.getTime("physics");

		//time_debug.initTime();
		ai.step(elapsed_ms);
		//time_debug.getTime("ai");

		//time_debug.initTime();
		bullets.step(elapsed_ms);
		//time_debug.getTime("bullets");

		//time_debug.initTime();
		world.handle_collisions();
		//time_debug.getTime("handle_collisions");

		//time_debug.initTime();
		renderer.draw();
		//time_debug.getTime("renderer");
	}

	return EXIT_SUCCESS;
}
