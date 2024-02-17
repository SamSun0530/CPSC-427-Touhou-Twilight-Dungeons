// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>

#include "physics_system.hpp"
#include <glm/trigonometric.hpp>
#include <iostream>

// Game configuration
const size_t MAX_EAGLES = 15;
const size_t MAX_BUG = 5;
const size_t MAX_BULLETS = 999;

const size_t EAGLE_DELAY_MS = 2000 * 3;
const size_t BUG_DELAY_MS = 5000 * 3;
bool is_alive = true;

// Create the bug world
WorldSystem::WorldSystem()
	: points(0)
	, next_eagle_spawn(0.f)
	, next_bug_spawn(0.f) {
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {
	// Destroy music components
	if (background_music != nullptr)
		Mix_FreeMusic(background_music);
	if (chicken_dead_sound != nullptr)
		Mix_FreeChunk(chicken_dead_sound);
	if (chicken_eat_sound != nullptr)
		Mix_FreeChunk(chicken_eat_sound);
	Mix_CloseAudio();

	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
}

// Debugging
namespace {
	void glfw_err_cb(int error, const char* desc) {
		fprintf(stderr, "%d: %s", error, desc);
	}
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow* WorldSystem::create_window() {
	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW");
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(window_width_px, window_height_px, "Chicken Game Assignment", nullptr, nullptr);
	if (window == nullptr) {
		fprintf(stderr, "Failed to glfwCreateWindow");
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	auto mouse_key_redirect = [](GLFWwindow* wnd, int button, int action, int mods) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_key(button, action, mods); };
	auto scroll_offset_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_scroll({ _0, _1 }); };

	// Set the cursor origin to start at the center of the screen
	glfwSetCursorPos(window, window_px_half.x, window_px_half.y);

	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, mouse_key_redirect);
	glfwSetScrollCallback(window, scroll_offset_redirect);

	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		return nullptr;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		return nullptr;
	}

	background_music = Mix_LoadMUS(audio_path("backgroundmusic.wav").c_str());
	chicken_dead_sound = Mix_LoadWAV(audio_path("chicken_dead.wav").c_str());
	chicken_eat_sound = Mix_LoadWAV(audio_path("chicken_eat.wav").c_str());
	game_ending_sound = Mix_LoadWAV(audio_path("game_ending_sound.wav").c_str());
	firing_sound = Mix_LoadWAV(audio_path("spell_sound.wav").c_str());
	damage_sound = Mix_LoadWAV(audio_path("damage_sound.wav").c_str());
	hit_spell = Mix_LoadWAV(audio_path("hit_spell.wav").c_str());

	// Set the music volume
	Mix_VolumeMusic(15);
	Mix_Volume(-1, 30);

	if (background_music == nullptr || chicken_dead_sound == nullptr || chicken_eat_sound == nullptr ||
		game_ending_sound == nullptr || firing_sound == nullptr || damage_sound == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("backgroundmusic.wav").c_str(),
			audio_path("game_ending_sound.wav").c_str(),
			audio_path("spell_sound.wav").c_str(),
			audio_path("damage_sound.wav").c_str(),
			audio_path("chicken_dead.wav").c_str(),
			audio_path("chicken_eat.wav").c_str(),
			audio_path("hit_spell.wav").c_str());

		return nullptr;
	}

	return window;
}

void WorldSystem::init(RenderSystem* renderer_arg) {
	this->renderer = renderer_arg;
	// Playing background music indefinitely
	Mix_PlayMusic(background_music, -1);
	fprintf(stderr, "Loaded music\n");

	//Sets the size of the empty world
	world_map = std::vector<std::vector<int>>(world_width, std::vector<int>(world_height, (int)TILE_TYPE::EMPTY));

	// Set all states to default
	restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	// Updating window title with points
	std::stringstream title_ss;
	title_ss << "Points: " << points;
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
		registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Removing out of screen entities
	auto& motions_registry = registry.motions;

	// Interpolate camera to smoothly follow player based on sharpness factor - elapsed time for independent of fps
	// sharpness_factor = 0 (not following) -> 0.5 (delay) -> 1 (always following)
	// Adapted from: https://gamedev.stackexchange.com/questions/152465/smoothly-move-camera-to-follow-player
	float sharpness_factor = 0.95f;
	float K = 1.0f - pow(1.0f - sharpness_factor, elapsed_ms_since_last_update / 1000.f);
	renderer->camera.setPosition(vec2_lerp(renderer->camera.getPosition(), motions_registry.get(player).position, K));
	renderer->ui.setPosition(motions_registry.get(player).position);
	vec2 ui_pos = { 50.f, 50.f };
	for (int i = 0; i < ui.size(); i++) {
		vec2 padding = { i * 60, 0 };
		motions_registry.get(ui[i]).position = motions_registry.get(player).position - window_px_half + ui_pos + padding;
	}
	for (int i = registry.hps.get(player).curr_hp; i < ui.size(); i++) {
		registry.renderRequests.get(ui[i]).used_texture = TEXTURE_ASSET_ID::EMPTY_HEART;
	}

	//// Remove entities that leave the screen on the left side
	//// Iterate backwards to be able to remove without unterfering with the next object to visit
	//// (the containers exchange the last element with the current)
	//for (int i = (int)motions_registry.components.size()-1; i>=0; --i) {
	//    Motion& motion = motions_registry.components[i];
	//	if (motion.position.x + abs(motion.scale.x) < 0.f) {
	//		if(!registry.players.has(motions_registry.entities[i])) // don't remove the player
	//			registry.remove_all_components_of(motions_registry.entities[i]);
	//	}
	//}

	// Spawning new eagles
	next_eagle_spawn -= elapsed_ms_since_last_update * current_speed;
	if (registry.deadlys.components.size() <= MAX_EAGLES && next_eagle_spawn < 0.f) {
		// Reset timer
		next_eagle_spawn = (EAGLE_DELAY_MS / 2) + uniform_dist(rng) * (EAGLE_DELAY_MS / 2);
		Motion& motion = registry.motions.get(player);
		// Create eagle with random initial position
		//createEagle(renderer, vec2(50.f + uniform_dist(rng) * (window_width_px - 100.f), 100.f));
		float spawn_x = (uniform_dist(rng) * (world_width - 3) * world_tile_size) - (world_width - 1) / 2.3 * world_tile_size;
		float spawn_y = (uniform_dist(rng) * (world_height - 3) * world_tile_size) - (world_height - 1) / 2.3 * world_tile_size;
		createEagle(renderer, vec2(spawn_x, spawn_y));
	}

	//// Spawning new bug
	//next_bug_spawn -= elapsed_ms_since_last_update * current_speed;
	//if (registry.eatables.components.size() <= MAX_BUG && next_bug_spawn < 0.f) {
	//	// !!!  TODO A1: Create new bug with createBug({0,0}), as for the Eagles above
	//}

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: HANDLE EGG SPAWN HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// Processing the chicken state
	assert(registry.screenStates.components.size() <= 1);
	ScreenState& screen = registry.screenStates.components[0];

	float min_counter_ms = 50.f;
	for (Entity entity : registry.hitTimers.entities) {
		// progress timer
		HitTimer& counter = registry.hitTimers.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if (counter.counter_ms < min_counter_ms) {
			min_counter_ms = counter.counter_ms;
		}

		// restart the game once the death timer expired
		if (counter.counter_ms < 0) {
			registry.hitTimers.remove(entity);
			registry.colors.get(entity) = vec3(1, 1, 1);
			//color = { 1, 0.8f, 0.8f };
			return true;
		}
	}

	float min_invulnerable_time_ms = 1000.f;
	for (Entity entity : registry.invulnerableTimers.entities) {
		InvulnerableTimer& counter = registry.invulnerableTimers.get(entity);
		counter.invulnerable_counter_ms -= elapsed_ms_since_last_update;
		if (counter.invulnerable_counter_ms < min_invulnerable_time_ms) {
			min_invulnerable_time_ms = counter.invulnerable_counter_ms;
		}

		if (counter.invulnerable_counter_ms < 0) {
			registry.invulnerableTimers.remove(entity);
			registry.players.get(entity).invulnerability = false;
			return true;
		}
	}

	float min_death_time_ms = 3000.f;
	for (Entity entity : registry.realDeathTimers.entities) {

		DeathTimer& death_counter = registry.realDeathTimers.get(entity);
		death_counter.death_counter_ms -= elapsed_ms_since_last_update;

		if (death_counter.death_counter_ms < min_death_time_ms) {
			min_death_time_ms = death_counter.death_counter_ms;
		}

		if (death_counter.death_counter_ms < 0) {
			registry.realDeathTimers.remove(entity);

			restart_game();
			return true;
		}
	}


	if (registry.hps.get(player).curr_hp <= 0) {
		registry.hps.get(player).curr_hp = registry.hps.get(player).max_hp;
		is_alive = false; 
		pressed = { 0 };
		registry.motions.get(player).direction = { 0,0 };
		registry.realDeathTimers.emplace(player);
		//screen.darken_screen_factor = 1 - min_counter_ms / 3000;
	}

	for (Entity entity : registry.hps.entities) {
		HP& hp = registry.hps.get(entity);
		if (hp.curr_hp <= 0.0f) {
			registry.remove_all_components_of(entity);
		}
	}
	// reduce window brightness if any of the present chickens is dying
	screen.darken_screen_factor = 1 - min_death_time_ms / 3000;

	// !!! TODO A1: update LightUp timers and remove if time drops below zero, similar to the death counter

	return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	// Reset keyboard presses
	pressed = { 0 };

	// Reset the game speed
	current_speed = 1.f;
	// Reset bgm
	Mix_PlayMusic(background_music, -1);

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all bug, eagles, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());

	// Debugging for memory/component leaks
	registry.list_all_components();

	// Create rooms

	// createPhysTile(renderer, { 0, -200 }); // for testing collision
	// createPhysTile(renderer, { -124, -324 }); // for testing collision

	// Creates 1 room the size of the map
	for (int row = 0; row < world_map.size(); row++) {
		for (int col = 0; col < world_map[row].size(); col++) {
			if (row == 0 || col == 0 || row == world_height - 1 || col == world_width - 1) {
				world_map[row][col] = (int)TILE_TYPE::WALL;
			}
			else {
				world_map[row][col] = (int)TILE_TYPE::FLOOR;
			}
		}
	}
	int centerX = (world_width >> 1);
	int centerY = (world_height >> 1);

	//Creates entitiy tiles based on the world map
	for (int row = 0; row < (int)world_map.size(); row++) { //i=row, j=col
		for (int col = 0; col < world_map[row].size(); col++) {
			// if (row == 0 || col == 0 || row == world_height-1 || col == world_width-1 ) {
			// 	world_map[row][col] = (int)TILE_TYPE::WALL;
			// }
			std::vector<TEXTURE_ASSET_ID> textureIDs;
			if (row == 0 && col == 0) {
				textureIDs.push_back(TEXTURE_ASSET_ID::LEFT_TOP_CORNER_WALL);
			}
			else if (row == world_height - 1 && col == world_width - 1) {
				textureIDs.push_back(TEXTURE_ASSET_ID::RIGHT_BOTTOM_CORNER_WALL);
			}
			else if (row == 0 && col == world_width - 1) {
				textureIDs.push_back(TEXTURE_ASSET_ID::RIGHT_TOP_CORNER_WALL);
			}
			else if (row == world_height - 1 && col == 0) {
				textureIDs.push_back(TEXTURE_ASSET_ID::LEFT_BOTTOM_CORNER_WALL);
			}
			else if (row == 0) {
				textureIDs.push_back(TEXTURE_ASSET_ID::TOP_WALL);
			}
			else if (row == world_height - 1) {
				float rand = uniform_dist(rng);
				if (rand < 0.5f) {
					textureIDs.push_back(TEXTURE_ASSET_ID::TILE_1);
				}
				else {
					textureIDs.push_back(TEXTURE_ASSET_ID::TILE_2);
				}
				textureIDs.push_back(TEXTURE_ASSET_ID::WALL_EDGE);
			}
			else if (col == 0) {
				textureIDs.push_back(TEXTURE_ASSET_ID::LEFT_WALL);
			}
			else if (col == world_width - 1) {
				textureIDs.push_back(TEXTURE_ASSET_ID::RIGHT_WALL);
			}
			else {
				float rand = uniform_dist(rng);
				if (rand < 0.5f) {
					textureIDs.push_back(TEXTURE_ASSET_ID::TILE_1);
				}
				else {
					textureIDs.push_back(TEXTURE_ASSET_ID::TILE_2);
				}
			}

			int xPos = (col - centerX) * world_tile_size;
			int yPos = (row - centerY) * world_tile_size;
			switch (world_map[col][row])
			{
			case (int)TILE_TYPE::WALL:
				createPhysTile(renderer, { xPos,yPos }, textureIDs);
				break;
			case (int)TILE_TYPE::FLOOR:
				createDecoTile(renderer, { xPos,yPos }, textureIDs);
				break;
			default:
				break;
			}
		}
	}

	// Create a new chicken
	player = createChicken(renderer, { 0, 0 });
	is_alive = true;
	ui = createUI(renderer, registry.hps.get(player).max_hp);

	renderer->camera.setPosition({ 0, 0 });

	// !! TODO A2: Enable static eggs on the ground, for reference
	// Create eggs on the floor, use this for reference
	/*
	for (uint i = 0; i < 20; i++) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);
		float radius = 30 * (uniform_dist(rng) + 0.3f); // range 0.3 .. 1.3
		Entity egg = createEgg({ uniform_dist(rng) * w, h - uniform_dist(rng) * 20 },
					 { radius, radius });
		float brightness = uniform_dist(rng) * 0.5 + 0.5;
		registry.colors.insert(egg, { brightness, brightness, brightness});
	}
	*/
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
	// Loop over all collisions detected by the physics system
	auto& collisionsRegistry = registry.collisions;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other;

		// For now, we are only interested in collisions that involve the chicken

		if (registry.players.has(entity)) {

			// Checking Player - Deadly collisions
			if (registry.deadlys.has(entity_other)) {
				// initiate death unless already dying
				if (!registry.hitTimers.has(entity) && !registry.realDeathTimers.has(player)) {

					// player turn red and decrease hp
					if (!registry.players.get(player).invulnerability) {
						registry.hitTimers.emplace(entity);
						Mix_PlayChannel(-1, damage_sound, 0);
						registry.colors.get(player) = vec3(1.0f, 0.0f, 0.0f);
						// should decrease HP but not yet implemented
						registry.hps.get(player).curr_hp -= registry.deadlys.get(entity_other).damage;
						registry.players.get(player).invulnerability = true;
						registry.invulnerableTimers.emplace(entity);
					}
				}
			}
			// Checking Player - enemy bullet collisions
			else if (registry.enemyBullets.has(entity_other)) {
				if (!registry.hitTimers.has(entity)) {
					// player turn red and decrease hp, bullet disappear
					if (!registry.players.get(player).invulnerability) {
						registry.hitTimers.emplace(entity);
						Mix_PlayChannel(-1, damage_sound, 0);
						registry.colors.get(entity) = vec3(1.0f, 0.0f, 0.0f);

						registry.hps.get(player).curr_hp -= registry.enemyBullets.get(entity_other).damage;
						registry.remove_all_components_of(entity_other);
						registry.players.get(player).invulnerability = true;
						registry.invulnerableTimers.emplace(entity);
					}

				}
			}
			else if (registry.physTiles.has(entity_other)) {
				Motion& motion = registry.motions.get(entity);
				Motion& wall_motion = registry.motions.get(entity_other);
				vec2 normal = motion.position - wall_motion.position;

				// clamp vector from entity to wall to get wall normal
				if (abs(normal.x) > abs(normal.y)) {
					normal = normal.x > 0 ? vec2(1, 0) : vec2(-1, 0);
				}
				else {
					normal = normal.y > 0 ? vec2(0, 1) : vec2(0, -1);
				}

				if (normal.x == 0) {
					motion.direction = { motion.direction.x, 0 };
					motion.velocity = { motion.velocity.x, 0 };
					if (normal.y > 0) {
						pressed[GLFW_KEY_W] = false;
					}
					else {
						pressed[GLFW_KEY_S] = false;
					}
				}
				else {
					motion.direction = { 0, motion.direction.y };
					motion.velocity = { 0, motion.velocity.y };
					if (normal.x > 0) {
						pressed[GLFW_KEY_A] = false;
					}
					else {
						pressed[GLFW_KEY_D] = false;
					}
				}

				motion.position = motion.last_position;
			}
		}
		else if (registry.deadlys.has(entity)) {
			if (registry.bullets.has(entity_other)) {
				if (!registry.hitTimers.has(entity)) {
					// enemy turn red and decrease hp, bullet disappear
					registry.hitTimers.emplace(entity);
					registry.colors.get(entity) = vec3(1.0f, 0.0f, 0.0f);

					Mix_PlayChannel(-1, hit_spell, 0);
					registry.hps.get(entity).curr_hp -= registry.bullets.get(entity_other).damage;
					registry.remove_all_components_of(entity_other);
				}
			}
		}
		else if (registry.physTiles.has(entity)) {
			if (registry.bullets.has(entity_other) || registry.enemyBullets.has(entity_other)) {
				registry.remove_all_components_of(entity_other);
			}
			else if (registry.deadlys.has(entity_other)) {
				Motion& wall_motion = registry.motions.get(entity);
				Motion& motion = registry.motions.get(entity_other);
				vec2 normal = motion.position - wall_motion.position;

				// clamp vector from entity to wall to get wall normal
				if (abs(normal.x) > abs(normal.y)) {
					normal = normal.x > 0 ? vec2(1, 0) : vec2(-1, 0);
				}
				else {
					normal = normal.y > 0 ? vec2(0, 1) : vec2(0, -1);
				}

				if (normal.x == 0) {
					motion.direction = { motion.direction.x, 0 };
					motion.velocity = { motion.velocity.x, 0 };
				}
				else {
					motion.direction = { 0, motion.direction.y };
					motion.velocity = { 0, motion.velocity.y };
				}

				motion.position = motion.last_position;
			}
		}
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: HANDLE CHICKEN MOVEMENT HERE
	// key is of 'type' GLFW_KEY_
	// action can be GLFW_PRESS GLFW_RELEASE GLFW_REPEAT
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);

		restart_game();
	}

	// Handle player movement
	Motion& motion = registry.motions.get(player);
	if (is_alive) {
		switch (key) {
		case GLFW_KEY_W:
			if (!pressed[key] && action == GLFW_PRESS) {
				motion.direction.y -= 1;
				pressed[key] = true;
			}
			else if (pressed[key] && action == GLFW_RELEASE) {
				motion.direction.y += 1;
				pressed[key] = false;
			}
			break;
		case GLFW_KEY_A:
			if (!pressed[key] && action == GLFW_PRESS) {
				motion.direction.x -= 1;
				pressed[key] = true;
			}
			else if (pressed[key] && action == GLFW_RELEASE) {
				motion.direction.x += 1;
				pressed[key] = false;
			}
			break;
		case GLFW_KEY_S:
			if (!pressed[key] && action == GLFW_PRESS) {
				motion.direction.y += 1;
				pressed[key] = true;
			}
			else if (pressed[key] && action == GLFW_RELEASE) {
				motion.direction.y -= 1;
				pressed[key] = false;
			}
			break;
		case GLFW_KEY_D:
			if (!pressed[key] && action == GLFW_PRESS) {
				motion.direction.x += 1;
				pressed[key] = true;
			}
			else if (pressed[key] && action == GLFW_RELEASE) {
				motion.direction.x -= 1;
				pressed[key] = false;
			}
			break;
		default:
			break;
		}
	}
	

	// Toggle between camera-cursor offset
	if (key == GLFW_KEY_P) {
		if (action == GLFW_RELEASE) {
			renderer->camera.isFreeCam = !renderer->camera.isFreeCam;
			if (!renderer->camera.isFreeCam) {
				renderer->camera.setOffset({ 0, 0 });
			}
		}
	}

	// Fire bullets at mouse cursor (Also mouse1)
	if (key == GLFW_KEY_SPACE) {
		BulletFireRate& fireRate = registry.bulletFireRates.get(player);
		if (action == GLFW_PRESS) {
			fireRate.is_firing = true;
		}
		else if (action == GLFW_RELEASE) {
			fireRate.is_firing = false;
		}
	}

	// Debugging
	if (key == GLFW_KEY_D) {
		if (action == GLFW_RELEASE)
			debugging.in_debug_mode = false;
		else
			debugging.in_debug_mode = true;
	}

	// Exit the program
	if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
		glfwSetWindowShouldClose(window, true);
	}

	// Control the current speed with `<` `>`
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA) {
		current_speed -= 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD) {
		current_speed += 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	current_speed = fmax(0.f, current_speed);
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
	if (renderer->camera.isFreeCam) {
		vec2& player_position = registry.motions.get(player).position;
		// Set the camera offset to be in between the cursor and the player
		// Center the mouse position, get the half distance between mouse cursor and player, update offset relative to player position
		renderer->camera.setOffset(((mouse_position - window_px_half) - player_position) / 2.f + player_position / 2.f);
	}
}

void WorldSystem::on_mouse_key(int button, int action, int mods) {
	if (!is_alive) {
		return;
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		BulletFireRate& fireRate = registry.bulletFireRates.get(player);
		if (action == GLFW_PRESS) {
			// Start firing
			fireRate.is_firing = true;
		}
		else if (action == GLFW_RELEASE) {
			// Stop firing
			fireRate.is_firing = false;
		}
	}
}

void WorldSystem::updateBulletFiring(float elapsed_ms_since_last_update) {
	// Update bullet fire timer
	if (registry.bullets.components.size() + registry.enemyBullets.components.size() <= MAX_BULLETS) {
		for (Entity entity : registry.bulletFireRates.entities) {
			BulletFireRate& fireRate = registry.bulletFireRates.get(entity);

			// Fire rate will use time to become independent of FPS
			// Adapted from: https://forum.unity.com/threads/gun-fire-rate-is-frame-rate-dependent.661588/
			float current_time = glfwGetTime();
			if (fireRate.is_firing && current_time - fireRate.last_time >= fireRate.fire_rate) {
				Motion& motion = registry.motions.get(entity);
				if (entity == player) {
					// player fires bullet towards mouse position
					double mouse_pos_x;
					double mouse_pos_y;
					glfwGetCursorPos(window, &mouse_pos_x, &mouse_pos_y);
					last_mouse_position = vec2(mouse_pos_x, mouse_pos_y) - window_px_half + motion.position;
					float x = last_mouse_position.x - motion.position.x;
					float y = last_mouse_position.y - motion.position.y;
					mouse_rotation_angle = -atan2(x, y) - glm::radians(90.0f);

					Mix_PlayChannel(-1, firing_sound, 0);
					createBullet(renderer, motion.speed_modified, motion.position, mouse_rotation_angle, last_mouse_position - motion.position, true);
				}
				else {
					// TODO: Spawn enemy bullets here (ai)
					Motion& player_motion = registry.motions.get(player);
					float x = player_motion.position.x - motion.position.x;
					float y = player_motion.position.y - motion.position.y;
					float enemy_fire_angle = -atan2(x, y) - glm::radians(90.0f);
					createBullet(renderer, motion.speed_modified, motion.position, enemy_fire_angle, { x, y });
				}
				fireRate.last_time = current_time;
			}
		}
	}
}

void WorldSystem::on_scroll(vec2 scroll_offset) {
	renderer->camera.addZoom(scroll_offset.y);

	(vec2)scroll_offset;
}