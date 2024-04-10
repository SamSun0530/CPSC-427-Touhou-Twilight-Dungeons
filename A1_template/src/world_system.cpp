// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include "physics_system.hpp"
#include <glm/trigonometric.hpp>
#include <iostream>
#include <GLFW/glfw3.h>
#include <fstream>
#include <map>
#include <string>
#include <SDL_opengl.h>
#include <glm/gtc/type_ptr.hpp>

// Game configuration
const size_t MAX_ENEMIES = 10;
const size_t ENEMY_SPAWN_DELAY_MS = 2000 * 3;
bool is_alive = true;

// Create the world
WorldSystem::WorldSystem()
	: points(0)
	, next_enemy_spawn(0.f)
	, display_instruction(true) {
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
	loadScript("start.txt", start_script);
	loadScript("cirno.txt", cirno_script);
	loadScript("cirno_after.txt", cirno_after_script);
}

WorldSystem::~WorldSystem() {
	// Destroy all created components
	registry.clear_all_components();

	// Destroy all cursors
	glfwTerminate();

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

	// https://kenney-assets.itch.io/crosshair-pack
	std::string path = misc_path("crosshair038.png");

	GLFWimage image;
	image.width = 64;
	image.height = 64;
	stbi_uc* data;
	data = stbi_load(path.c_str(), &image.width, &image.height, NULL, 4);

	if (data == NULL)
	{
		const std::string message = "Could not load the file " + path + ".";
		fprintf(stderr, "%s", message.c_str());
		assert(false);
	}

	image.pixels = data;
	GLFWcursor* cursor = glfwCreateCursor(&image, image.width / 2, image.height / 2);
	glfwSetCursor(window, cursor);
	stbi_image_free(data);

	// Set the cursor origin to start at the center of the screen
	glfwSetCursorPos(window, window_px_half.x, window_px_half.y);

	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, mouse_key_redirect);
	glfwSetScrollCallback(window, scroll_offset_redirect);

	return window;
}

void WorldSystem::init(RenderSystem* renderer_arg, Audio* audio, MapSystem* map, AISystem* ai, VisibilitySystem* visibility_arg) {
	this->renderer = renderer_arg;
	this->audio = audio;
	this->map = map;
	this->ai = ai;
	this->visibility_system = visibility_arg;
	renderer->initFont(window, font_filename, font_default_size);
	//Sets the size of the empty world
	//world_map = std::vector<std::vector<int>>(world_width, std::vector<int>(world_height, (int)TILE_TYPE::EMPTY));
	Mix_PlayChannel(1, audio->menu_music, -1);
	// TODO: remove, redundant
	if (menu.state == MENU_STATE::PLAY) {
		// Set all states to default
		restart_game();
	}
}

void WorldSystem::init_menu() {
	// create buttons
	int num_buttons = game_info.has_started ? 3 : 2;
	const float button_scale = 0.8f;
	const float button_padding_y = 5.f;
	const float offset_y_padding = 50.f;
	const float offset_y_delta = BUTTON_HOVER_HEIGHT * button_scale + button_padding_y;
	float offset_y = -(offset_y_delta * (num_buttons - 1) - button_padding_y) / 2.f + offset_y_padding;
	float offset_x = window_px_half.x / 2.2f;

	// create main menu title and background
	createMainMenu(renderer, { -window_px_half.x / 2.2f, 0.f }, 0.38f, { offset_x , offset_y_padding }, 1.f);

	if (game_info.has_started) {
		createButton(renderer, { offset_x, offset_y }, button_scale, MENU_STATE::MAIN_MENU, "Resume", 0.9f, [&]() {
			//Mix_ResumeMusic();
			//Mix_HaltChannel(1);
			//Mix_PlayChannel(0, audio->background_music, -1);

;			resume_game();
			});
		offset_y += offset_y_delta;
	}
	createButton(renderer, { offset_x, offset_y }, button_scale, MENU_STATE::MAIN_MENU, "New Game", 1.f, [&]() { restart_game(); });
	offset_y += offset_y_delta;
	createButton(renderer, { offset_x, offset_y }, button_scale, MENU_STATE::MAIN_MENU, "Exit", 1.f, [&]() { glfwSetWindowShouldClose(window, true); });
}

void WorldSystem::init_pause_menu() {
	// background
	createPauseMenu(renderer, { 0, 0 }, 1.f);

	// create buttons
	// vertically center the buttons
	int num_buttons = 2;
	const float button_scale = 0.7f;
	const float button_padding_y = 5.f;
	const float offset_y_delta = BUTTON_HOVER_HEIGHT * button_scale + button_padding_y;
	float offset_y = -(offset_y_delta * (num_buttons - 1) - button_padding_y) / 2.f;
	createButton(renderer, { 0, offset_y }, button_scale, MENU_STATE::PAUSE, "Resume", 0.9f, [&]() { 
		resume_game(); 
		});
	offset_y += offset_y_delta;
	createButton(renderer, { 0, offset_y }, button_scale * 1.1f, MENU_STATE::PAUSE, "Exit to Menu", 0.85f, [&]() {
		////Mix_ResumeMusic();
		//Mix_HaltChannel(0);
		//Mix_PlayChannel(1, audio->menu_music, -1);

		menu.state = MENU_STATE::MAIN_MENU;
		});
}

void WorldSystem::init_win_menu() {
	createWin(renderer);
	const float button_scale = 0.7f;
	createButton(renderer, { 0, 200 }, button_scale*1.1, MENU_STATE::WIN, "Exit to Menu", 0.85f, [&]() {
		game_info.has_started = false;
		Mix_PauseMusic();
		for (Entity entity : registry.buttons.entities) {
			Button& button = registry.buttons.get(entity);
			if (button.state == MENU_STATE::MAIN_MENU)
				registry.remove_all_components_of(entity);
		}
		registry.mainMenus.clear();
		init_menu();
		menu.state = MENU_STATE::MAIN_MENU;
		});
}

void WorldSystem::init_lose_menu() {
	createLose(renderer);
	const float button_scale = 0.7f;
	createButton(renderer, { 0, 200 }, button_scale*1.1, MENU_STATE::LOSE, "Exit to Menu", 0.85f, [&]() {
		game_info.has_started = false;
		Mix_PauseMusic();
		for (Entity entity : registry.buttons.entities) {
			Button& button = registry.buttons.get(entity);
			if (button.state == MENU_STATE::MAIN_MENU)
				registry.remove_all_components_of(entity);
		}
		registry.mainMenus.clear();
		init_menu();
		menu.state = MENU_STATE::MAIN_MENU;
		});
}

void WorldSystem::resume_game() {
	////Mix_ResumeMusic();
	menu.state = MENU_STATE::PLAY;
	pressed = { 0 };
	registry.kinematics.get(player).direction = { 0, 0 };
	if (registry.bulletSpawners.has(player)) {
		registry.bulletSpawners.get(player).is_firing = false;
	}
	focus_mode.in_focus_mode = false;
	focus_mode.speed_constant = 1.0f;
	while (registry.focusdots.entities.size() > 0)
		registry.remove_all_components_of(registry.focusdots.entities.back());
	pressed[GLFW_KEY_LEFT_SHIFT] = false;
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	tutorial_counter--;

	elapsedSinceLastFPSUpdate += elapsed_ms_since_last_update;
	if (elapsedSinceLastFPSUpdate >= 1000.0) {
		// Calculate FPS
		getInstance().fps = static_cast<int>(1000.0f / (elapsed_ms_since_last_update / combo_mode.combo_meter));
		elapsedSinceLastFPSUpdate = 0.0f;
	}

	tutorial_timer -= tutorial_timer <= 0 ? 0 : elapsed_ms_since_last_update;
	if (tutorial_timer <= 0) {
		getInstance().display_instruction = false;
	}

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
		registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Remove texts in ui and world that are not permanent
	while (registry.texts.entities.size() > 0)
		registry.remove_all_components_of(registry.texts.entities.back());
	while (registry.textsWorld.entities.size() > 0)
		registry.remove_all_components_of(registry.textsWorld.entities.back());

	// Create combo meter ui
	if (combo_mode.combo_meter <= 1.04) {
		registry.renderRequests.get(display_combo).used_texture = TEXTURE_ASSET_ID::C;
	}
	else if (combo_mode.combo_meter <= 1.1) {
		registry.renderRequests.get(display_combo).used_texture = TEXTURE_ASSET_ID::B;
	}
	else if (combo_mode.combo_meter <= 1.2) {
		registry.renderRequests.get(display_combo).used_texture = TEXTURE_ASSET_ID::A;
	}
	else {
		registry.renderRequests.get(display_combo).used_texture = TEXTURE_ASSET_ID::S;
	}


	// Create on screen player attributes ui
	HP& player_hp = registry.hps.get(player);
	//createText({ 128 * 1.3 + 70 - 4, window_height_px - 77 }, { 1,1 }, std::to_string(player_hp.curr_hp) + " / " + std::to_string(player_hp.max_hp), vec3(1, 1, 1), false);
	createText(-window_px_half + vec2(260, 70), { 1,1 }, std::to_string(player_hp.curr_hp) + " / " + std::to_string(player_hp.max_hp), vec3(1, 1, 1), false);
	Player& player_att = registry.players.get(player);
	std::string fire_rate = std::to_string(1 / player_att.fire_rate);
	std::string critical_hit = std::to_string(player_att.critical_hit * 100);
	std::string critical_dmg = std::to_string(player_att.critical_demage * 100);
	createText(-window_px_half + vec2(68, 153), { 1,1 }, std::to_string(player_att.coin_amount), vec3(1), false);
	createText(-window_px_half + vec2(72, 200), { 1,1 }, std::to_string(player_att.bullet_damage), vec3(1), false);
	createText(-window_px_half + vec2(86, 250), { 1,1 }, fire_rate.substr(0, fire_rate.find(".") + 3), vec3(1), false);
	createText(-window_px_half + vec2(86, 300), { 1,1 }, critical_hit.substr(0, critical_hit.find(".") + 3), vec3(1), false);
	createText(-window_px_half + vec2(103, 350), { 1,1 }, critical_dmg.substr(0, critical_dmg.find(".") + 3), vec3(1), false);

	// Interpolate camera to smoothly follow player based on sharpness factor - elapsed time for independent of fps
	// sharpness_factor_camera = 0 (not following) -> 0.5 (delay) -> 1 (always following)
	// Adapted from: https://gamedev.stackexchange.com/questions/152465/smoothly-move-camera-to-follow-player
	float sharpness_factor_camera = 0.95f;
	float K = 1.0f - pow(1.0f - sharpness_factor_camera, elapsed_ms_since_last_update / 1000.f);

	vec2 player_position = registry.motions.get(player).position;
	renderer->camera.setPosition(vec2_lerp(renderer->camera.getPosition(), player_position, K));

	// Interpolate mouse-camera offset
	// sharpness_factor_camera_offset possible values:
	// - 0.0 - offset
	// - 0.0 to 1.0 - slow offset transition speed
	// - 10.0 - medium offset transition speed
	// - 30.0 - very fast offset transition speed
	float sharpness_factor_camera_offset = 10.0f;
	renderer->camera.offset = vec2_lerp(renderer->camera.offset, renderer->camera.offset_target, elapsed_ms_since_last_update / 1000.f * sharpness_factor_camera_offset);

	// Render boss health bar ui boss name
	for (Entity entity : registry.bossHealthBarUIs.entities) {
		BossHealthBarUI& bar = registry.bossHealthBarUIs.get(entity);
		if (bar.is_visible) {
			Motion& motion = registry.motions.get(entity);
			createText(motion.position - vec2(100.f, 40.f), vec2(0.7f), bar.boss_name, vec3(1.f, 0.f, 0.f), false, false);
		}
	}

	// Focus mode ui
	if (focus_mode.in_focus_mode) {
		float counter_ms_temp = focus_mode.counter_ms - elapsed_ms_since_last_update;
		focus_mode.counter_ms = counter_ms_temp <= 0.f ? 0.f : counter_ms_temp;
		if (focus_mode.counter_ms <= 0.f) {
			// disable focus mode
			focus_mode.speed_constant = 1.0f;
			while (registry.focusdots.entities.size() > 0)
				registry.remove_all_components_of(registry.focusdots.entities.back());
			pressed[GLFW_KEY_LEFT_SHIFT] = false;
			focus_mode.in_focus_mode = false;
		}
	}
	else if (focus_mode.counter_ms + elapsed_ms_since_last_update < focus_mode.max_counter_ms) {
		focus_mode.counter_ms = min(focus_mode.counter_ms + elapsed_ms_since_last_update, focus_mode.max_counter_ms);
	}

	// Tutorial dummy spawner
	if (map_info.level == MAP_LEVEL::TUTORIAL) {
		for (Entity entity : registry.dummyenemyspawners.entities) {
			Motion& motion = registry.motions.get(entity);
			DummyEnemySpawner& spawner = registry.dummyenemyspawners.get(entity);
			if (spawner.number_spawned < spawner.max_spawn) {
				Entity dummy_enemy = createDummyEnemy(renderer, motion.position);
				registry.dummyEnemyLink.emplace(dummy_enemy, entity);
				spawner.number_spawned++;
			}
		}
	}


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
			return true;
		}
	}

	float min_death_time_ms = 3000.f;
	for (Entity entity : registry.realDeathTimers.entities) {

		DeathTimer& death_counter = registry.realDeathTimers.get(entity);
		death_counter.death_counter_ms -= elapsed_ms_since_last_update;

		if (death_counter.death_counter_ms < min_death_time_ms && registry.players.has(entity)) {
			min_death_time_ms = death_counter.death_counter_ms;
		}

		if (death_counter.death_counter_ms < 0) {
			if (registry.players.has(entity)) {
				registry.realDeathTimers.remove(entity);
				screen.darken_screen_factor = 0;
				menu.state = MENU_STATE::LOSE;
				return true;
			}
			else if (registry.deadlys.has(entity)) {
				std::random_device rd;
				std::mt19937 gen(rd());
				std::uniform_real_distribution<> distrib(0, 1);
				double number = distrib(gen);
				createCoin(renderer, registry.motions.get(entity).position, 1);
				if (registry.dummyenemies.has(entity)) {
					DummyEnemyLink& link = registry.dummyEnemyLink.get(entity);
					DummyEnemySpawner& spawner = registry.dummyenemyspawners.get(link.other);
					spawner.number_spawned--;
					registry.dummyEnemyLink.remove(entity);
				}

				if (number <= 0.1)
					createFood(renderer, registry.motions.get(entity).position);

				if (game_info.in_room != -1) {
					auto& deadly_entities = game_info.room_index[game_info.in_room].enemies;
					for (int i = 0; i < deadly_entities.size(); ++i) {
						if (entity == deadly_entities[i]) {
							std::swap(deadly_entities[i], deadly_entities[deadly_entities.size() - 1]);
							deadly_entities.pop_back();
							break;
						}
					}
				}

				registry.remove_all_components_of(entity);
			}
			else {
				registry.remove_all_components_of(entity);
			}
		}
	}

	for (Entity entity : registry.bezierCurves.entities) {

		BezierCurve& curve_counter = registry.bezierCurves.get(entity);
		curve_counter.curve_counter_ms -= elapsed_ms_since_last_update;

		if (curve_counter.curve_counter_ms < 0) {
			registry.bezierCurves.remove(entity);
		}
	}

	if (is_alive && registry.hps.get(player).curr_hp <= 0) {
		registry.bulletSpawners.get(player).is_firing = false;
		is_alive = false;
		pressed = { 0 };
		registry.kinematics.get(player).direction = { 0,0 };
		Mix_HaltChannel(-1);
		Mix_PlayChannel(-1, audio->game_ending_sound, 0);
		registry.realDeathTimers.emplace(player);
	}

	for (Entity entity : registry.hps.entities) {
		if (registry.players.has(entity)) continue;
		HP& hp = registry.hps.get(entity);
		if (hp.curr_hp <= 0.0f) {
			if (registry.deadlys.has(entity)) {
				// remove boss health bar ui
				if (registry.bossHealthBarLink.has(entity)) {
					registry.remove_all_components_of(registry.bossHealthBarLink.get(entity).other);

					// remove all bullets when boss hp < 0
					while (registry.enemyBullets.entities.size() > 0)
						registry.remove_all_components_of(registry.enemyBullets.entities.back());
				}
			}
			registry.remove_all_components_of(entity);
		}
	}
	// reduce window brightness if any of the present players is dying
	screen.darken_screen_factor = 1 - min_death_time_ms / 3000;

	return true;
}

std::string replaceNewlines(std::string str) {
	size_t start_pos = 0;
	while ((start_pos = str.find("\\n", start_pos)) != std::string::npos) {
		str.replace(start_pos, 2, "\n");
		start_pos += 1;
	}
	return str;
}

unsigned int WorldSystem::loadScript(std::string file_name, std::vector<std::string>& scripts) {
	std::ifstream script_file("../../../script/" + file_name);
	if (script_file.is_open()) {
		std::cout << "opened" << std::endl;
		std::string line;
		while (getline(script_file, line)) {
			scripts.push_back(replaceNewlines(line));
		}
		script_file.close();
	}
	else {
		std::cout << "Unable to open file" << std::endl;
	}
	return 0;
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	menu.state = MENU_STATE::PLAY;
	game_info.has_started = true;
	if (map_info.level != MAP_LEVEL::TUTORIAL) {
		start_pt = 0;
		dialogue_info.cirno_pt = 1000;
		dialogue_info.cirno_after_pt = 1000;
		dialogue_info.cirno_played = false;
		start_dialogue_timer = 1000.f;
	}
	
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	// Reset keyboard presses
	pressed = { 0 };

	// Reset bgm
	//Mix_HaltChannel(1);
	//Mix_PlayChannel(0, audio->background_music, -1);

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all enemies, coins, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());

	// Since visibility tile do not have motion, iterate and remove here
	while (registry.visibilityTileInstanceData.entities.size() > 0)
		registry.remove_all_components_of(registry.visibilityTileInstanceData.entities.back());

	// initialize menus
	init_menu();
	init_pause_menu();

	// Debugging for memory/component leaks
	registry.list_all_components();

	// Generate map
	reset_world_default();
	map->restart_map();

	game_info.reset_room_info();

	if (map_info.level == MAP_LEVEL::TUTORIAL) {
		map->generateTutorialMap();
		renderer->set_tiles_instance_buffer();
		player = map->spawnPlayer(world_center);
	}
	else {
		map->generateRandomMap(11); // room_size must be > 3
		player = map->spawnPlayerInRoom(0);
	}

	//createPillar(renderer, { world_center.x, world_center.y - 2 }, std::vector<TEXTURE_ASSET_ID>{TEXTURE_ASSET_ID::PILLAR_BOTTOM, TEXTURE_ASSET_ID::PILLAR_TOP});

	// Create a new player
	is_alive = true;
	createHealthUI(renderer);
	createAttributeUI(renderer);
	combo_mode.restart();
	focus_mode.restart();
	ai->restart_flow_field_map();
	display_combo = createCombo(renderer);
	game_info.set_player_id(player);

	init_win_menu();
	init_lose_menu();
	renderer->camera.setPosition({ 0, 0 });

}

// handle_wall_collisions parameter entity IS WALL ENTITY!
void WorldSystem::handle_wall_collisions(Entity& entity, Entity& entity_other) {
	Motion& wall_motion = registry.motions.get(entity);
	Motion& entity_motion = registry.motions.get(entity_other);
	Kinematic& kinematic = registry.kinematics.get(entity_other);
	Collidable& wall_collidable = registry.collidables.get(entity);
	Collidable& entity_collidable = registry.collidables.get(entity_other);
	vec2 wall_center = wall_motion.position + wall_collidable.shift;
	vec2 entity_center = entity_motion.position + entity_collidable.shift;

	// Minkowski Sum adapted from "sam hocevar": https://gamedev.stackexchange.com/a/24091
	// Find the normal of object B, or the wall given two rectangles
	float wy = (wall_collidable.size.x + entity_collidable.size.x) * (entity_center.y - wall_center.y);
	float hx = (wall_collidable.size.y + entity_collidable.size.y) * (entity_center.x - wall_center.x);

	if (wy > hx) {
		if (wy > -hx) {
			// top
			entity_motion.position = { entity_motion.position.x , wall_center.y + wall_collidable.size.y / 2 + entity_collidable.size.y / 2 - entity_collidable.shift.y };
			kinematic.direction = { kinematic.direction.x, 0 };
			kinematic.velocity = { kinematic.velocity.x, 0 };
		}
		else {
			// left
			entity_motion.position = { wall_center.x - wall_collidable.size.x / 2 - entity_collidable.size.x / 2 - entity_collidable.shift.x, entity_motion.position.y };
			kinematic.direction = { 0, kinematic.direction.y };
			kinematic.velocity = { 0, kinematic.velocity.y };
		}
	}
	else {
		if (wy > -hx) {
			// right
			entity_motion.position = { wall_center.x + wall_collidable.size.x / 2 + entity_collidable.size.x / 2 - entity_collidable.shift.x, entity_motion.position.y };
			kinematic.direction = { 0, kinematic.direction.y };
			kinematic.velocity = { 0, kinematic.velocity.y };
		}
		else {
			// bottom
			entity_motion.position = { entity_motion.position.x , wall_center.y - wall_collidable.size.y / 2 - entity_collidable.size.y / 2 - entity_collidable.shift.y };
			kinematic.direction = { kinematic.direction.x, 0 };
			kinematic.velocity = { kinematic.velocity.x, 0 };
		}
	}
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
	// Loop over all collisions detected by the physics system
	auto& collisionsRegistry = registry.collisions;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other;

		// Player collisions
		if (registry.players.has(entity)) {
			// Checking Player - Deadly collisions
			if (registry.deadlys.has(entity_other)) {
				// initiate death unless already dying
				if (!registry.hitTimers.has(entity) && !registry.realDeathTimers.has(entity)) {
					// player turn red and decrease hp
					if (!registry.invulnerableTimers.has(entity)) {
						// decrease HP
						if (!registry.realDeathTimers.has(entity_other)) {
							registry.hitTimers.emplace(entity);
							Mix_PlayChannel(-1, audio->damage_sound, 0);
							registry.colors.get(player) = vec3(-1.f);
							HP& player_hp = registry.hps.get(player);
							player_hp.curr_hp -= registry.deadlys.get(entity_other).damage;
							if (player_hp.curr_hp < 0) player_hp.curr_hp = 0;
							combo_mode.restart();
							if (registry.bomberEnemies.has(entity_other)) {
								registry.realDeathTimers.emplace(entity_other).death_counter_ms = 1000;
								registry.hps.remove(entity_other);
								registry.aitimers.remove(entity_other);
								registry.followpaths.remove(entity_other);
								registry.followFlowField.remove(entity);
								if (registry.bulletSpawners.has(entity_other)) {
									registry.bulletSpawners.get(entity_other).is_firing = false;
								}
								Kinematic& kin = registry.kinematics.get(entity_other);
								kin.velocity = { 0,0 };
								kin.direction = { 0,0 };
								kin.speed_modified = 0.f;
								registry.motions.get(entity_other).scale = 2.f * vec2({ ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT });
								registry.bomberEnemies.get(entity_other).touch_player = true;
							}
						}

						registry.players.get(player).invulnerability = true;
						registry.invulnerableTimers.emplace(entity).invulnerable_counter_ms = registry.players.components[0].invulnerability_time_ms;
					}
				}
			}
			// Checking Player - enemy bullet collisions
			else if (registry.enemyBullets.has(entity_other)) {
				if (!registry.hitTimers.has(entity)) {
					// player turn red and decrease hp, bullet disappear
					if (!registry.invulnerableTimers.has(entity)) {
						registry.hitTimers.emplace(entity);
						Mix_PlayChannel(-1, audio->damage_sound, 0);
						registry.colors.get(entity) = vec3(-1.f);
						combo_mode.restart();
						HP& player_hp = registry.hps.get(player);
						player_hp.curr_hp -= registry.enemyBullets.get(entity_other).damage;
						if (player_hp.curr_hp < 0) player_hp.curr_hp = 0;
						registry.remove_all_components_of(entity_other);

						registry.players.get(player).invulnerability = true;
						registry.invulnerableTimers.emplace(entity).invulnerable_counter_ms = registry.players.components[0].invulnerability_time_ms;
					}
				}
			}
			// Checking player - Pick ups collisions
			else if (registry.pickupables.has(entity_other)) {
				if (!registry.realDeathTimers.has(entity)) {
					HP& hp = registry.hps.get(entity);
					Pickupable& pickupable = registry.pickupables.get(entity_other);
					hp.curr_hp = min(hp.curr_hp + pickupable.health_change, hp.max_hp);
					Motion& motion = registry.motions.get(player);
					Entity text_entity = createText({ motion.position.x, motion.position.y - 40.f }, vec2(0.5f), "+" + std::to_string(pickupable.health_change) + " HP!", vec3(0.0f, 1.0f, 0.0f), true, true);
					registry.realDeathTimers.emplace(text_entity).death_counter_ms = 2000;
					registry.remove_all_components_of(entity_other);
				}
			}
			// Checking player - coins collisions
			else if (registry.coins.has(entity_other)) {
				if (!registry.realDeathTimers.has(entity)) {
					registry.players.get(entity).coin_amount += registry.coins.get(entity_other).coin_amount;
					registry.remove_all_components_of(entity_other);
				}
			}
			// Checking player - keys collision
			else if (registry.keys.has(entity_other)) {
				registry.players.get(entity).key_amount++;
				registry.remove_all_components_of(entity_other);
			}
			// Checking player - power up items collisions
			else if (registry.maxhpIncreases.has(entity_other)) {

				registry.hps.get(entity).max_hp += registry.maxhpIncreases.get(entity_other).max_health_increase;
				if (registry.hps.get(entity).max_hp > 12) {
					registry.hps.get(entity).max_hp = 12;
				}
				registry.hps.get(entity).curr_hp += registry.maxhpIncreases.get(entity_other).max_health_increase;
				registry.remove_all_components_of(entity_other);

			}
			else if (registry.attackUps.has(entity_other)) {
				registry.players.get(entity).bullet_damage += 1;
				registry.remove_all_components_of(entity_other);
			}
		}
		// Checks enemy collisions
		else if (registry.deadlys.has(entity)) {
			if (registry.playerBullets.has(entity_other) && !registry.invulnerableTimers.has(entity)) {
				if (!registry.hitTimers.has(entity) && !registry.realDeathTimers.has(entity)) {
					// enemy turn red and decrease hp, bullet disappear
					registry.hitTimers.emplace(entity);
					registry.colors.get(entity) = vec3(-1.f);
					Motion& deadly_motion = registry.motions.get(entity);
					Mix_PlayChannel(-1, audio->hit_spell, 0);

					std::random_device rd;
					std::mt19937 gen(rd());
					std::uniform_real_distribution<> distrib(0, 1);
					double number = distrib(gen);
					Player& player_att = registry.players.get(player);
					if (number < player_att.critical_hit) {
						registry.hps.get(entity).curr_hp -= registry.playerBullets.get(entity_other).damage * player_att.critical_demage;
						registry.realDeathTimers.emplace(createCriHit(renderer, deadly_motion.position - vec2(30, 0))).death_counter_ms = 300;
					}
					else {
						registry.hps.get(entity).curr_hp -= registry.playerBullets.get(entity_other).damage;
					}

					if (!registry.bosses.has(entity)) {
						// Knockback
						// TODO: knockback factor based on player items
						float knockback_factor = 50.f;
						Kinematic& bullet_kin = registry.kinematics.get(entity_other);
						Kinematic& kin = registry.kinematics.get(entity);
						kin.velocity += bullet_kin.direction * knockback_factor;
					}

					HP& hp = registry.hps.get(entity);
					if (hp.curr_hp <= 0.0f) {
						combo_mode.combo_meter = min(combo_mode.combo_meter + 0.02f, combo_mode.COMBO_METER_MAX);
						if (registry.beeEnemies.has(entity) || registry.wolfEnemies.has(entity) || registry.bomberEnemies.has(entity) || registry.dummyenemies.has(entity)) {
							registry.realDeathTimers.emplace(entity).death_counter_ms = 1000;
							registry.hps.remove(entity);
							registry.aitimers.remove(entity);
							registry.followpaths.remove(entity);
							registry.followFlowField.remove(entity);
							if (registry.bulletSpawners.has(entity)) {
								registry.bulletSpawners.get(entity).is_firing = false;
							}
							Kinematic& kin = registry.kinematics.get(entity);
							kin.velocity = { 0,0 };
							kin.direction = { 0,0 };
							kin.speed_modified = 0.f;
						}
					}
					registry.remove_all_components_of(entity_other);
				}
			}
			else if (registry.deadlys.has(entity_other)) {
				Motion& motion1 = registry.motions.get(entity);
				Motion& motion2 = registry.motions.get(entity_other);
				Kinematic& kinematic = registry.kinematics.get(entity_other);
				Collidable& collidable1 = registry.collidables.get(entity);
				Collidable& collidable2 = registry.collidables.get(entity_other);
				vec2 center1 = motion1.position + collidable1.shift;
				vec2 center2 = motion2.position + collidable2.shift;

				// Adapted from Minkowski Sum below
				vec2 center_delta = center2 - center1;
				vec2 center_delta_non_collide = collidable1.size / 2.f + collidable2.size / 2.f;

				// Handle case when both entities are perfectly on top of each other
				if (length(center_delta) < 0.001) {
					std::random_device ran;
					std::mt19937 gen(ran());
					std::uniform_real_distribution<> dis(0, 1);
					float x_sign = dis(gen) < 0.5 ? 1 : -1;
					float y_sign = dis(gen) < 0.5 ? 1 : -1;
					center_delta = { 0.5 * x_sign, 0.5 * y_sign };
				}
				// Penetration depth from collision
				// if center2 is to the right/below of center1, depth is positive, otherwise negative
				// depth corresponds to the entity_other's velocity change.
				// e.g. if entity collides on left of entity_other, depth.x is positive -> entity_other's velocity.x will increase
				vec2 depth = { center_delta.x > 0 ? center_delta_non_collide.x - center_delta.x : -(center_delta_non_collide.x + center_delta.x),
								center_delta.y > 0 ? center_delta_non_collide.y - center_delta.y : -(center_delta_non_collide.y + center_delta.y) };

				// check how deep the collision between the two entities are, if passes threshold, apply delta velocity
				float threshold = 10.f; // in pixels
				float depth_size = length(depth);
				if (depth_size > threshold) {
					float strength = 0.5f;
					kinematic.velocity += depth * strength;
				}
			}
		}
		// Checks wall collisions
		// Checks locked door collisions
		else if (registry.walls.has(entity)) {
			// enemy/player bullets to wall are handled in physics system 
			if (registry.players.has(entity_other) || registry.deadlys.has(entity_other)) {
				handle_wall_collisions(entity, entity_other);
			}
		}
		else if (registry.doors.has(entity)) {
			if (registry.players.has(entity_other)) {
				Door& door = registry.doors.get(entity);
				if (door.is_locked) {
					handle_wall_collisions(entity, entity_other);
				}
				else {
					Room_struct& room = game_info.room_index[door.room_index];
					if (room.need_to_spawn) {
						room.need_to_spawn = false;
						// spawn enemies in room
						map->spawnEnemiesInRoom(room);
						Mix_PlayChannel(-1, audio->open_gate_sound, 0);
					}

					if (door.is_closed) {
						renderer->switch_door_texture(entity, false);
						Mix_PlayChannel(-1, audio->open_gate_sound, 0);
						door.is_closed = false;
					}

					if (!door.is_visited) {
						door.is_visited = true;
					}
				}
			}
			else if (registry.deadlys.has(entity_other)) {
				handle_wall_collisions(entity, entity_other);
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

bool WorldSystem::get_display_instruction()
{
	return display_instruction;
}
bool WorldSystem::get_show_fps()
{
	return show_fps;
}

std::string WorldSystem::get_fps_in_string()
{
	return std::to_string(fps);
}

int WorldSystem::get_tutorial_counter() {
	return tutorial_counter;
}

// Helper for updating player direction
void WorldSystem::updatePlayerDirection(Kinematic& player_kinematic) {
	float direction_x = 0;
	float direction_y = 0;
	if (pressed[GLFW_KEY_W]) direction_y -= 1;
	if (pressed[GLFW_KEY_A]) direction_x -= 1;
	if (pressed[GLFW_KEY_S]) direction_y += 1;
	if (pressed[GLFW_KEY_D]) direction_x += 1;
	player_kinematic.direction = { direction_x, direction_y };
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {
	if (menu.state == MENU_STATE::PLAY) {
		// Resetting game
		if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
			int w, h;
			glfwGetWindowSize(window, &w, &h);
			map_info.level = MAP_LEVEL::LEVEL1;
			restart_game();
		}

		// Debugging
		if (key == GLFW_KEY_G) {
			if (action == GLFW_RELEASE)
				debugging.in_debug_mode = !debugging.in_debug_mode;
		}

		// Toggle FPS display
		if (key == GLFW_KEY_F && action == GLFW_RELEASE) {
			getInstance().toggle_show_fps();
		}

		// Player can only act when alive
		if (!is_alive) {
			return;
		}

		// Handle player movement
		// Added key checks at the beginning so don't have to fetch kinematics / update player direction for
		// every key press that is not related to WASD
		if (is_alive && (key == GLFW_KEY_W || key == GLFW_KEY_A || key == GLFW_KEY_S || key == GLFW_KEY_D)) {
			Kinematic& kinematic = registry.kinematics.get(player);
			switch (key) {
			case GLFW_KEY_W:
				if (action == GLFW_PRESS) {
					pressed[key] = true;
				}
				else if (action == GLFW_RELEASE) {
					pressed[key] = false;
				}
				break;
			case GLFW_KEY_A:
				if (action == GLFW_PRESS) {
					pressed[key] = true;
				}
				else if (action == GLFW_RELEASE) {
					pressed[key] = false;
				}
				break;
			case GLFW_KEY_S:
				if (action == GLFW_PRESS) {
					pressed[key] = true;
				}
				else if (action == GLFW_RELEASE) {
					pressed[key] = false;
				}
				break;
			case GLFW_KEY_D:
				if (action == GLFW_PRESS) {
					pressed[key] = true;
				}
				else if (action == GLFW_RELEASE) {
					pressed[key] = false;
				}
				break;
			default:
				break;
			}

			// Update player direction
			updatePlayerDirection(kinematic);
		}

		// Debug mode: M to print mapsystem/worldsystem world map
		if (debugging.in_debug_mode && action == GLFW_RELEASE && key == GLFW_KEY_M) {
			printf("mapsystem map:\n");
			for (int i = 0; i < world_map.size(); i++) {
				for (int j = 0; j < world_map[0].size(); j++) {
					printf("%d ", world_map[i][j]);
				}
				printf("\n");
			}
			printf("\n");
		}


		// Toggle between camera-cursor offset
		if (key == GLFW_KEY_P) {
			if (action == GLFW_RELEASE) {
				renderer->camera.isFreeCam = !renderer->camera.isFreeCam;
				if (!renderer->camera.isFreeCam) {
					renderer->camera.offset_target = { 0, 0 };
				}
				else {
					// set camera offset immediately when button is pressed
					if (registry.players.has(player)) {
						double mouse_pos_x;
						double mouse_pos_y;
						glfwGetCursorPos(window, &mouse_pos_x, &mouse_pos_y);
						vec2 player_position = registry.motions.get(player).position;
						renderer->camera.offset_target = ((vec2(mouse_pos_x, mouse_pos_y) - window_px_half) - player_position) / 2.f + player_position / 2.f;
					}
				}
			}
		}

		// Fire bullets at mouse cursor (Also mouse 1)
		if (key == GLFW_KEY_SPACE) {
			BulletSpawner& bullet_spawner = registry.bulletSpawners.get(player);
			if (action == GLFW_PRESS) {
				bullet_spawner.is_firing = true;
			}
			else if (action == GLFW_RELEASE) {
				bullet_spawner.is_firing = false;
			}
		}

		// Toggle tutorial display
		if (key == GLFW_KEY_T && action == GLFW_RELEASE) {
			getInstance().display_instruction = false;
			map_info.level = MAP_LEVEL::TUTORIAL;
			restart_game();
		}

		// Hold for focus mode
		if (key == GLFW_KEY_LEFT_SHIFT &&
			!pressed[key] &&
			action == GLFW_PRESS &&
			!focus_mode.in_focus_mode) {
			focus_mode.in_focus_mode = !focus_mode.in_focus_mode;
			focus_mode.speed_constant = 0.5f;
			Motion& motion = registry.motions.get(player);
			CircleCollidable& circle_collidable = registry.circleCollidables.get(player);
			createFocusDot(renderer, motion.position + circle_collidable.shift, vec2(circle_collidable.radius * 2.f));
			pressed[key] = true;
		}
		else if (key == GLFW_KEY_LEFT_SHIFT &&
			pressed[key] &&
			action == GLFW_RELEASE &&
			focus_mode.in_focus_mode) {
			focus_mode.in_focus_mode = !focus_mode.in_focus_mode;
			focus_mode.speed_constant = 1.0f;
			while (registry.focusdots.entities.size() > 0)
				registry.remove_all_components_of(registry.focusdots.entities.back());
			pressed[key] = false;
		}
		

		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			// open pause menu
			Mix_PlayChannel(3, audio->pause_menu_sound, 0);
			menu.state = MENU_STATE::PAUSE;
		}
	}
	else if (menu.state == MENU_STATE::MAIN_MENU) {
		// Exit the program
		//if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
		//	glfwSetWindowShouldClose(window, true);
		//}
	}
	else if (menu.state == MENU_STATE::PAUSE) {
		// back to game
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			resume_game();
		}
	}
	else if (menu.state == MENU_STATE::DIALOGUE) {
		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
			start_pt += 1;
			dialogue_info.cirno_pt += 1;
			dialogue_info.cirno_after_pt += 1;
			curr_word = 0;
			start_buffer = "";
		}
	}
}

void WorldSystem::dialogue_step(float elapsed_time) {
	start_dialogue_timer -= elapsed_time;

	if (start_dialogue_timer > 0) {
		return;
	}
	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
		registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Remove texts in ui and world that are not permanent
	while (registry.texts.entities.size() > 0)
		registry.remove_all_components_of(registry.texts.entities.back());
	while (registry.textsWorld.entities.size() > 0)
		registry.remove_all_components_of(registry.texts.entities.back());
	while (registry.dialogueMenus.entities.size() > 0)
		registry.remove_all_components_of(registry.dialogueMenus.entities.back());

	if (registry.bosses.size() == 0 && dialogue_info.cirno_played) {
		dialogue_info.cirno_played = false;
		dialogue_info.cirno_after_pt = 0;
	}
	if (start_pt < start_script.size()) {
		word_up_ms -= elapsed_time;
		CHARACTER speaking_chara = CHARACTER::REIMU;
		EMOTION emotion = EMOTION::NORMAL;
		std::istringstream ss(start_script[start_pt]);
		std::string token;
		std::getline(ss, token, ' ');

		if (token == "Cirno:") {
			speaking_chara = CHARACTER::CIRNO;
		}

		std::getline(ss, token, ' ');
		if (token == "(laugh)") {
			emotion = EMOTION::LAUGH;
		}
		else if (token == "(cry)") {
			emotion = EMOTION::CRY;
		}
		else if (token == "(special)") {
			emotion = EMOTION::SPECIAL;
		}
		else if (token == "(shock)"){
			emotion = EMOTION::SHOCK;
		}
		else if (token == "(angry)") {
			emotion = EMOTION::ANGRY;
		}
		if (word_up_ms < 0) {
			unsigned int i = 0;
			while (std::getline(ss, token, ' ')) {
				if (i == curr_word) {
					start_buffer += " " + token;
				}
				i += 1;
			}
			word_up_ms = 50.f;
			curr_word += 1;
		}
		menu.state = MENU_STATE::DIALOGUE;
		createDialogue(speaking_chara, start_buffer, CHARACTER::NONE, emotion);
	}
	else if (start_pt == start_script.size()) {
		start_pt += 1;
		resume_game();
	}
	else if (dialogue_info.cirno_pt < cirno_script.size()) {
		word_up_ms -= elapsed_time;
		CHARACTER speaking_chara = CHARACTER::REIMU;
		EMOTION emotion = EMOTION::NORMAL;
		std::istringstream ss(cirno_script[dialogue_info.cirno_pt]);
		std::string token;
		std::getline(ss, token, ' ');
		if (token == "Cirno:") {
			speaking_chara = CHARACTER::CIRNO;
		}
		std::getline(ss, token, ' ');
		if (token == "(laugh)") {
			emotion = EMOTION::LAUGH;
		}
		else if (token == "(cry)") {
			emotion = EMOTION::CRY;
		}
		else if (token == "(special)") {
			emotion = EMOTION::SPECIAL;
		}
		else if (token == "(shock)") {
			emotion = EMOTION::SHOCK;
		}
		else if (token == "(angry)") {
			emotion = EMOTION::ANGRY;
		}
		if (word_up_ms < 0) {
			unsigned int i = 0;
			while (std::getline(ss, token, ' ')) {
				if (i == curr_word) {
					start_buffer += " " + token;
				}
				i += 1;
			}
			word_up_ms = 50.f;
			curr_word += 1;
		}
		menu.state = MENU_STATE::DIALOGUE;
		createDialogue(speaking_chara, start_buffer, CHARACTER::CIRNO, emotion);
	}
	else if (dialogue_info.cirno_pt == cirno_script.size()) {
		dialogue_info.cirno_pt += 1;
		dialogue_info.cirno_played = true;
		resume_game();
	}else if (dialogue_info.cirno_after_pt < cirno_after_script.size()) {
		word_up_ms -= elapsed_time;
		CHARACTER speaking_chara = CHARACTER::REIMU;
		EMOTION emotion = EMOTION::NORMAL;
		std::istringstream ss(cirno_after_script[dialogue_info.cirno_after_pt]);
		std::string token;
		std::getline(ss, token, ' ');
		if (token == "Cirno:") {
			speaking_chara = CHARACTER::CIRNO;
		}
		std::getline(ss, token, ' ');
		if (token == "(laugh)") {
			emotion = EMOTION::LAUGH;
		}
		else if (token == "(cry)") {
			emotion = EMOTION::CRY;
		}
		else if (token == "(special)") {
			emotion = EMOTION::SPECIAL;
		}
		else if (token == "(shock)") {
			emotion = EMOTION::SHOCK;
		}
		else if (token == "(angry)") {
			emotion = EMOTION::ANGRY;
		}
		if (word_up_ms < 0) {
			unsigned int i = 0;
			while (std::getline(ss, token, ' ')) {
				if (i == curr_word) {
					start_buffer += " " + token;
				}
				i += 1;
			}
			word_up_ms = 50.f;
			curr_word += 1;
		}
		menu.state = MENU_STATE::DIALOGUE;
		createDialogue(speaking_chara, start_buffer, CHARACTER::CIRNO, emotion);
	}
	else if (dialogue_info.cirno_after_pt == cirno_after_script.size()) {
		dialogue_info.cirno_after_pt += 1;
		menu.state = MENU_STATE::WIN;
	}

}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
	if (menu.state == MENU_STATE::PLAY) {
		if (renderer->camera.isFreeCam) {
			vec2& player_position = registry.motions.get(player).position;
			// Set the camera offset to be in between the cursor and the player
			// Center the mouse position, get the half distance between mouse cursor and player, update offset relative to player position
			renderer->camera.offset_target = ((mouse_position - window_px_half) - player_position) / 2.f + player_position / 2.f;
		}
	}
	else {
		ComponentContainer<Button>& button_container = registry.buttons;
		int button_container_size = button_container.size();
		for (int i = 0; i < button_container_size; ++i) {
			Button& button = button_container.components[i];
			if (button.state != menu.state) continue; // don't check other state buttons
			Entity& entity = button_container.entities[i];
			const Motion& motion = registry.motions.get(entity);

			vec2 mouse_pos = mouse_position - window_px_half;
			// check if point clicked is inside button
			vec2 half_extent = motion.scale / 2.f;
			vec2 min = motion.position - half_extent;
			vec2 max = motion.position + half_extent;
			if (mouse_pos.x >= min.x && mouse_pos.x <= max.x &&
				mouse_pos.y >= min.y && mouse_pos.y <= max.y) {
				button.is_hovered = true;
			}
			else {
				button.is_hovered = false;
			}
		}
	}
}

void WorldSystem::on_mouse_key(int button, int action, int mods) {
	if (menu.state == MENU_STATE::PLAY) {
		if (is_alive && button == GLFW_MOUSE_BUTTON_LEFT) {
			BulletSpawner& bullet_spawner = registry.bulletSpawners.get(player);
			if (action == GLFW_PRESS) {
				// Start firing
				bullet_spawner.is_firing = true;
			}
			else if (action == GLFW_RELEASE) {
				// Stop firing
				bullet_spawner.is_firing = false;
			}
		}
	}
	else if (menu.state == MENU_STATE::DIALOGUE) {
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			start_pt += 1;
			dialogue_info.cirno_pt += 1;
			dialogue_info.cirno_after_pt += 1;
			curr_word = 0;
			start_buffer = "";
		}
	}
	else {
		// prevent double presses by setting pressed array
		if (pressed[GLFW_MOUSE_BUTTON_LEFT] && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
			ComponentContainer<Button>& button_container = registry.buttons;
			int button_container_size = button_container.size();
			for (int i = 0; i < button_container_size; ++i) {
				Button& button = button_container.components[i];
				if (button.state != menu.state) continue; // don't check other state buttons
				Entity& entity = button_container.entities[i];
				const Motion& motion = registry.motions.get(entity);

				double mouse_pos_x;
				double mouse_pos_y;
				glfwGetCursorPos(window, &mouse_pos_x, &mouse_pos_y);
				// normalize mouse position
				mouse_pos_x -= window_px_half.x;
				mouse_pos_y -= window_px_half.y;
				// check if point clicked is inside button
				vec2 half_extent = motion.scale / 2.f;
				vec2 min = motion.position - half_extent;
				vec2 max = motion.position + half_extent;
				if (mouse_pos_x >= min.x && mouse_pos_x <= max.x &&
					mouse_pos_y >= min.y && mouse_pos_y <= max.y) {
					// execute function
					// this function should handle menu state change
					// e.g. restart_game should set menu.state == MENU_STATE::PLAY
					button.func();

					break; // don't check any more buttons
				}
			}
			pressed[GLFW_MOUSE_BUTTON_LEFT] = false;
		}
		else if (!pressed[GLFW_MOUSE_BUTTON_LEFT] && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			pressed[GLFW_MOUSE_BUTTON_LEFT] = true;
		}
	}
}

void WorldSystem::on_scroll(vec2 scroll_offset) {
	if (menu.state == MENU_STATE::PLAY) {
		renderer->camera.addZoom(scroll_offset.y);
	}
}

void WorldSystem::update_focus_dot() {
	if (menu.state == MENU_STATE::PLAY) {
		if (focus_mode.in_focus_mode) {
			// Only reimu should have focus dot
			for (Entity entity : registry.focusdots.entities) {
				Motion& motion = registry.motions.get(entity);
				Motion& player_motion = registry.motions.get(player);
				CircleCollidable& player_circle_collidable = registry.circleCollidables.get(player);
				motion.position = player_motion.position + player_circle_collidable.shift;
			}

			// This is for testing circleCollidable size if focus mode texture is not present
			//for (Entity entity : registry.circleCollidables.entities) {
			//	Motion& motion = registry.motions.get(entity);
			//	CircleCollidable& circle_collidable = registry.circleCollidables.get(entity);
			//	createEgg(motion.position + circle_collidable.shift, vec2(circle_collidable.radius * 2.f));
			//}
		}
	}
}