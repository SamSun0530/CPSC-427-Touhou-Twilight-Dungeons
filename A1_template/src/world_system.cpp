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
const size_t MAX_ENEMIES = 15;
const size_t MAX_COINS = 5;

const size_t ENEMY_SPAWN_DELAY_MS = 2000 * 3;
bool is_alive = true;

// Create the world
WorldSystem::WorldSystem()
	: points(0)
	, next_enemy_spawn(0.f) {
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {
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
	window = glfwCreateWindow(window_width_px, window_height_px, "Touhou: Twilight Dungeons", nullptr, nullptr);
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

	return window;
}

void WorldSystem::init(RenderSystem* renderer_arg, Audio* audio) {
	this->renderer = renderer_arg;
	this->audio = audio;

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
	// sharpness_factor_camera = 0 (not following) -> 0.5 (delay) -> 1 (always following)
	// Adapted from: https://gamedev.stackexchange.com/questions/152465/smoothly-move-camera-to-follow-player
	float sharpness_factor_camera = 0.95f;
	float K = 1.0f - pow(1.0f - sharpness_factor_camera, elapsed_ms_since_last_update / 1000.f);
	renderer->camera.setPosition(vec2_lerp(renderer->camera.getPosition(), motions_registry.get(player).position, K));
	renderer->ui.setPosition(motions_registry.get(player).position);

	// Interpolate mouse-camera offset
	// sharpness_factor_camera_offset possible values:
	// - 0.0 - offset
	// - 0.0 to 1.0 - slow offset transition speed
	// - 10.0 - medium offset transition speed
	// - 30.0 - very fast offset transition speed
	float sharpness_factor_camera_offset = 10.0f;
	renderer->camera.offset = vec2_lerp(renderer->camera.offset, renderer->camera.offset_target, elapsed_ms_since_last_update / 1000.f * sharpness_factor_camera_offset);

	// User interface
	vec2 ui_pos = { 50.f, 50.f };
	for (int i = 0; i < ui.size(); i++) {
		vec2 padding = { i * 60, 0 };
		motions_registry.get(ui[i]).position = motions_registry.get(player).position - window_px_half + ui_pos + padding;
	}
	for (int i = registry.hps.get(player).curr_hp; i < ui.size(); i++) {
		registry.renderRequests.get(ui[i]).used_texture = TEXTURE_ASSET_ID::EMPTY_HEART;
	}

	//// Spawning new enemies
	//next_enemy_spawn -= elapsed_ms_since_last_update;
	//if (registry.deadlys.components.size() <= MAX_ENEMIES && next_enemy_spawn < 0.f) {
	//	// Reset timer
	//	next_enemy_spawn = (ENEMY_SPAWN_DELAY_MS / 2) + uniform_dist(rng) * (ENEMY_SPAWN_DELAY_MS / 2);
	//	Motion& motion = registry.motions.get(player);
	//	// Create enemy with random initial position
	//	float spawn_x = (uniform_dist(rng) * (world_width - 3) * world_tile_size) - (world_width - 1) / 2.3 * world_tile_size;
	//	float spawn_y = (uniform_dist(rng) * (world_height - 3) * world_tile_size) - (world_height - 1) / 2.3 * world_tile_size;
	//	createEnemy(renderer, vec2(spawn_x, spawn_y));
	//}

	// Processing the player state
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

		// turn the entity back to original color once the hit timer expired
		if (counter.counter_ms < 0) {
			registry.hitTimers.remove(entity);
			registry.colors.get(entity) = vec3(1, 1, 1);
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
		registry.kinematics.get(player).direction = { 0,0 };
		Mix_HaltMusic();
		Mix_PlayChannel(-1, audio->game_ending_sound, 0);
		registry.realDeathTimers.emplace(player);
	}

	for (Entity entity : registry.hps.entities) {
		HP& hp = registry.hps.get(entity);
		if (hp.curr_hp <= 0.0f) {
			registry.remove_all_components_of(entity);
		}
	}
	// reduce window brightness if any of the present players is dying
	screen.darken_screen_factor = 1 - min_death_time_ms / 3000;

	return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	// Reset keyboard presses
	pressed = { 0 };

	// Reset bgm
	Mix_PlayMusic(audio->background_music, -1);

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all enemies, coins, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());

	// Debugging for memory/component leaks
	registry.list_all_components();

	// Create rooms

	//std::vector<TEXTURE_ASSET_ID> textureIDs{ TEXTURE_ASSET_ID::LEFT_WALL };
	//createWall(renderer, { 0, -200 }, textureIDs); // for testing collision
	//createWall(renderer, { -124, -324 }, textureIDs); // for testing collision

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
				createWall(renderer, { xPos,yPos }, textureIDs);
				break;
			case (int)TILE_TYPE::FLOOR:
				createFloor(renderer, { xPos,yPos }, textureIDs);
				break;
			default:
				break;
			}
		}
	}

	// Create a new player
	player = createPlayer(renderer, { 0, 0 });
	is_alive = true;
	ui = createUI(renderer, registry.hps.get(player).max_hp);

	renderer->camera.setPosition({ 0, 0 });
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
	// Loop over all collisions detected by the physics system
	auto& collisionsRegistry = registry.collisions;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other;

		if (registry.players.has(entity)) {
			// Checking Player - Deadly collisions
			if (registry.deadlys.has(entity_other)) {
				// initiate death unless already dying
				if (!registry.hitTimers.has(entity) && !registry.realDeathTimers.has(player)) {

					// player turn red and decrease hp
					if (!registry.players.get(player).invulnerability) {
						registry.hitTimers.emplace(entity);
						Mix_PlayChannel(-1, audio->damage_sound, 0);
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
						Mix_PlayChannel(-1, audio->damage_sound, 0);
						registry.colors.get(entity) = vec3(1.0f, 0.0f, 0.0f);

						registry.hps.get(player).curr_hp -= registry.enemyBullets.get(entity_other).damage;
						registry.remove_all_components_of(entity_other);
						registry.players.get(player).invulnerability = true;
						registry.invulnerableTimers.emplace(entity);
					}

				}
			}
		}
		else if (registry.deadlys.has(entity)) {
			if (registry.playerBullets.has(entity_other)) {
				if (!registry.hitTimers.has(entity)) {
					// enemy turn red and decrease hp, bullet disappear
					registry.hitTimers.emplace(entity);
					registry.colors.get(entity) = vec3(1.0f, 0.0f, 0.0f);

					Mix_PlayChannel(-1, audio->hit_spell, 0);
					registry.hps.get(entity).curr_hp -= registry.playerBullets.get(entity_other).damage;
					registry.remove_all_components_of(entity_other);
				}
			}
		}
		else if (registry.walls.has(entity)) {
			if (registry.playerBullets.has(entity_other) || registry.enemyBullets.has(entity_other)) {
				registry.remove_all_components_of(entity_other);
			}
			else if (registry.players.has(entity_other)) {
				Motion& wall_motion = registry.motions.get(entity);
				Motion& motion = registry.motions.get(entity_other);
				Kinematic& kinematic = registry.kinematics.get(entity_other);
				vec2 normal = motion.position - wall_motion.position;

				// clamp vector from entity to wall to get wall normal
				if (abs(normal.x) > abs(normal.y)) {
					normal = normal.x > 0 ? vec2(1, 0) : vec2(-1, 0);
				}
				else {
					normal = normal.y > 0 ? vec2(0, 1) : vec2(0, -1);
				}

				if (normal.x == 0) {
					kinematic.direction = { kinematic.direction.x, 0 };
					kinematic.velocity = { kinematic.velocity.x, 0 };
					if (normal.y > 0) {
						pressed[GLFW_KEY_W] = false;
					}
					else {
						pressed[GLFW_KEY_S] = false;
					}
				}
				else {
					kinematic.direction = { 0, kinematic.direction.y };
					kinematic.velocity = { 0, kinematic.velocity.y };
					if (normal.x > 0) {
						pressed[GLFW_KEY_A] = false;
					}
					else {
						pressed[GLFW_KEY_D] = false;
					}
				}

				motion.position = motion.last_position;
			}
			else if (registry.deadlys.has(entity_other)) {
				Motion& wall_motion = registry.motions.get(entity);
				Motion& motion = registry.motions.get(entity_other);
				Kinematic& kinematic = registry.kinematics.get(entity_other);
				vec2 normal = motion.position - wall_motion.position;

				// clamp vector from entity to wall to get wall normal
				if (abs(normal.x) > abs(normal.y)) {
					normal = normal.x > 0 ? vec2(1, 0) : vec2(-1, 0);
				}
				else {
					normal = normal.y > 0 ? vec2(0, 1) : vec2(0, -1);
				}

				if (normal.x == 0) {
					kinematic.direction = { kinematic.direction.x, 0 };
					kinematic.velocity = { kinematic.velocity.x, 0 };
				}
				else {
					kinematic.direction = { 0, kinematic.direction.y };
					kinematic.velocity = { 0, kinematic.velocity.y };
				}

				motion.position = motion.last_position;
			}
		}
	}
	//std::cout << "collision size before: " << registry.collisions.size() << std::endl;

	// Remove all collisions from this simulation step
	registry.collisions.clear();

	//std::cout << "collision size after: " << registry.collisions.size() << std::endl;

}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {
	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);
		restart_game();
	}

	// Handle player movement
	if (is_alive) {
		Kinematic& kinematic = registry.kinematics.get(player);
		switch (key) {
		case GLFW_KEY_W:
			if (!pressed[key] && action == GLFW_PRESS) {
				kinematic.direction.y -= 1;
				pressed[key] = true;
			}
			else if (pressed[key] && action == GLFW_RELEASE) {
				kinematic.direction.y += 1;
				pressed[key] = false;
			}
			break;
		case GLFW_KEY_A:
			if (!pressed[key] && action == GLFW_PRESS) {
				kinematic.direction.x -= 1;
				pressed[key] = true;
			}
			else if (pressed[key] && action == GLFW_RELEASE) {
				kinematic.direction.x += 1;
				pressed[key] = false;
			}
			break;
		case GLFW_KEY_S:
			if (!pressed[key] && action == GLFW_PRESS) {
				kinematic.direction.y += 1;
				pressed[key] = true;
			}
			else if (pressed[key] && action == GLFW_RELEASE) {
				kinematic.direction.y -= 1;
				pressed[key] = false;
			}
			break;
		case GLFW_KEY_D:
			if (!pressed[key] && action == GLFW_PRESS) {
				kinematic.direction.x += 1;
				pressed[key] = true;
			}
			else if (pressed[key] && action == GLFW_RELEASE) {
				kinematic.direction.x -= 1;
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
				renderer->camera.offset_target = { 0, 0 };
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
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
	if (renderer->camera.isFreeCam) {
		vec2& player_position = registry.motions.get(player).position;
		// Set the camera offset to be in between the cursor and the player
		// Center the mouse position, get the half distance between mouse cursor and player, update offset relative to player position
		renderer->camera.offset_target = ((mouse_position - window_px_half) - player_position) / 2.f + player_position / 2.f;
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

void WorldSystem::on_scroll(vec2 scroll_offset) {
	renderer->camera.addZoom(scroll_offset.y);

	(vec2)scroll_offset;
}