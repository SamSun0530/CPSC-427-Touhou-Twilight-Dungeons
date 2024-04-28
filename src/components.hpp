#pragma once
#include "common.hpp"
#include <vector>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include "../ext/stb_image/stb_image.h"

// statistics to show at end screen
struct Statistic {
	float enemies_killed = 0;
	float enemies_hit = 0;
	float bullets_fired = 0;
	float accuracy = 1; // this will be enemies_hit / bullets_fired
	float time_taken_to_win = 0; // accumulate via elapsed ms
	// TODO: add some more interesting facts

	void reset() {
		enemies_killed = 0;
		enemies_hit = 0;
		bullets_fired = 0;
		accuracy = 1;
		time_taken_to_win = 0;
	}
};
extern Statistic stats;

// all purpose timer, create your own global timer
struct UniversalTimer {
	// Nearest enemy check timer
	float closest_enemy_timer = 0;
	float closest_enemy_timer_default = 500;
	int closest_enemy = -1;

	// For aimbot1bullet, shoot a bullet per every aimbot_bullet_timer_default ms
	float aimbot_bullet_timer = 0;
	float aimbot_bullet_timer_default = 2000;

	void restart() {
		closest_enemy_timer = 0;
		closest_enemy = -1;
		aimbot_bullet_timer = 0;
	}
};
extern UniversalTimer uni_timer;

struct DialogueInfo {
	unsigned int cirno_pt = 1000000;
	unsigned int cirno_after_pt = 1000000;
	unsigned int flandre_pt = 1000000;
	unsigned int flandre_after_pt = 1000000;
	unsigned int marisa_pt = 1000000;
	bool cirno_played = false;
	bool flandre_played = false;
	bool marisa_played = false;
	bool sakuya_played = false;
	bool remilia_played = false;
};
extern DialogueInfo dialogue_info;

enum class MAP_LEVEL {
	TUTORIAL,
	LEVEL1,
	LEVEL2,
	LEVEL3,
	LEVEL4
};

// World map loader
struct MapInfo {
	MAP_LEVEL level = MAP_LEVEL::LEVEL1;
};
extern MapInfo map_info;

// Menu loader
enum class MENU_STATE {
	MAIN_MENU,
	PLAY,
	PAUSE,
	DIALOGUE,
	INVENTORY,
	WIN,
	LOSE,
	INFOGRAPHIC
};

struct Menu {
	MENU_STATE state = MENU_STATE::MAIN_MENU;
};
extern Menu menu;

// Data structure for toggling debug mode
struct Debug {
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};
extern Debug debugging;

struct FocusMode {
	bool in_focus_mode = 0;
	float speed_constant = 1.0f;
	// limit usage of focus mode through time
	float counter_ms = 10000;
	float max_counter_ms = 10000; // 10 seconds

	void restart() {
		in_focus_mode = false;
		speed_constant = 1.0f;
		counter_ms = max_counter_ms;
	}
};
extern FocusMode focus_mode;

struct ComboMode {
	// Combo meter
	float combo_meter = 1.0f;
	const float COMBO_METER_MAX = 1.5f;

	void restart() {
		combo_meter = 1.0f;
	}
};
extern ComboMode combo_mode;

struct VisibilityInfo {
	// exclude these map levels from using visibility tiles
	std::unordered_set<MAP_LEVEL> excluded{
		MAP_LEVEL::TUTORIAL,
		MAP_LEVEL::LEVEL4
		//MAP_LEVEL::LEVEL3
	};
};
extern VisibilityInfo visibility_info;

// Game related information
struct GameInfo {
	// for "resume" in button
	bool has_started = false;

	// Player information
	// store player entity id
	Entity player_id;
	bool is_player_id_set = false;
	void set_player_id(unsigned int player_actual) {
		player_id = (Entity)player_actual;
		is_player_id_set = true;
	}
	bool is_player_frozen = false;

	int in_room = -1; // -1 - not in room
	int prev_in_room = -1; // used for out of bounds error
	// each index represents a room
	std::vector<Room_struct> room_index;

	void add_room(Room_struct& room) {
		room_index.push_back(room);
	}

	void reset_room_info() {
		in_room = -1;
		room_index.clear();
	}
};
extern GameInfo game_info;

struct BossInfo {
	bool should_use_flandre_bullet = false;

	bool has_cirno_talked = false;
	bool has_flandre_talked = false;
	bool has_sakuya_talked = false;
	bool has_remilia_talked = false;

	void reset() {
		should_use_flandre_bullet = false;
		has_cirno_talked = false;
		has_flandre_talked = false;
		has_sakuya_talked = false;
		has_remilia_talked = false;
	}
};
extern BossInfo boss_info;

/*
NORMAL - nothing special
AOE - area of effect splash damage
AIMBOT - follow closest enemy to cursor
TRIPLE - three normal bullets towards cursor
AIMBOT1BULLET - normal bullet with one aimbot bullet every time interval
*/
enum AMMO_TYPE {
	NORMAL,
	AOE,
	AIMBOT,
	TRIPLE,
	AIMBOT1BULLET,
	AMMO_TYPE_COUNT
};

// Player component
struct Player
{
	bool invulnerability = false;
	int bullet_damage = 10;
	int coin_amount = 0;
	int key_amount = 0;
	float invulnerability_time_ms = 1000;
	float fire_rate = 1 / 3.f;
	float critical_hit = 0.05;
	float critical_damage = 1.5;
	int bomb = 3;

	// we store this here because 
	// - carry it to next level
	// - exclusive to the player only (enemies should not have this)
	AMMO_TYPE ammo_type = AMMO_TYPE::NORMAL;
};

struct Teleporter {
	bool player_overlap = false;
	bool is_teleporting = false;
	MAP_LEVEL destination;
	float teleport_time; // teleports when time < 0
	std::string teleporter_text;
	std::string optional_text_above_teleporter;
};

// make entity fly to player
struct FlyToPlayer {

};

// Menu stuff
struct MainMenu {

};

struct PauseMenu {

};

struct Button {
	MENU_STATE state; // button related to which state
	std::string text = ""; // button text
	float text_scale = 1.f;
	std::function<void()> func;
	bool is_hovered = false;
};

struct VisibilityTile {

};

struct PlayerBullet {
	int damage = 10;
};

struct UIUX {

};

struct Dialogue {

};

struct UIUXWorld {

};

struct EnemyBullet
{
	int damage = 1;
};

struct AimbotBullet {

};

struct AoeBullet {

};

struct NormalBullet {

};

/*
Bullet actions: (parameter argument specification)
None:
PLAYER_DIRECTION - change bullet to face player direction (only for enemies)
ENEMY_DIRECTION - change bullet to face direction of deadly closest to cursor (only for players)
CURSOR_DIRECTION - change bullet to face direction cursor (only for players)

Floats:
SPEED - float - change in velocity magnitude
ROTATE - float - change in bullet direction
DELAY - float - wait until executing next command (Blocking - blocks next action)
DEL - float - bullet death timer to remove bullet

Vec2s:
LOOP - vec2 - loop back to specified index
	- vec2[0] = number to loop (specify 1 - loops once)
	- vec2[1] = 0-indexed to loop to (should be index >= 0 && index < commands.size)
DIRECTION - vec2 - change direction to x,y

Vec3s:
SPLIT - vec3 - split one bullets into multiple bullets based on angle
	- vec3[0] = number of bullets to split into (<= 1 - won't do anything)
	- vec3[1] = angle for the bullet spread
	- vec3[2] = initial bullet speed
SPEED_TIMER - vec3 - lerp velocity magnitude over a time interval
	- vec3[0] = start bullet velocity
	- vec3[1] = end bullet velocity
	- vec3[2] = time interval in ms to lerp from start to end velocity
	- Note: 
	-	if second timer interval overlap with first, does not do the action
*/
enum class BULLET_ACTION {
	SPEED,
	ROTATE,
	DELAY,
	LOOP,
	DEL,
	SPLIT,
	DIRECTION,
	PLAYER_DIRECTION,
	ENEMY_DIRECTION,
	CURSOR_DIRECTION,
	SPEED_TIMER
};

enum class CHARACTER {
	REIMU,
	CIRNO,
	FlANDRE,
	MARISA,
	NONE,
};

struct BulletCommand {
	BULLET_ACTION action;
	union {
		vec2 value_vec2;
		vec3 value_vec3;
		float value;
	};

	BulletCommand(BULLET_ACTION a, float v) : action(a), value(v) {}
	BulletCommand(BULLET_ACTION a, vec2 v) : action(a), value_vec2(v) {}
	BulletCommand(BULLET_ACTION a, vec3 v) : action(a), value_vec3(v) {}
};

// Bullet follows pattern based on list of command
struct BulletPattern {
	// Adapted from https://redd.it/1490tat
	std::vector<BulletCommand> commands;
	int bc_index = 0; // current bullet command index
};

// Manages when entity is able to fire a bullet again
// Inspiration/Credit from: https://youtu.be/whrInb6Z7QI
struct BulletSpawner
{
	// determines if bullet can be fired
	bool is_firing = false;
	// set this to -1 so entity can fire immediately
	float last_fire_time = -1.f;
	// 1 is high fire rate, 10+ is slow fire rate
	// fires every fire_rate * 100ms
	float fire_rate = 1.f;
	// number of rounds to fire, -1 for no cooldown
	int number_to_fire = -1;
	// number of current rounds already fired, set cooldown if number_fired == number_to_fire
	int number_current_fired = 0;
	// determines next set of bullets to fire, 
	// set number_to_fire = -1 for no cooldown
	// updates every cooldown_rate * 100ms
	bool is_cooldown = false;
	float last_cooldown = 30.f;
	float cooldown_rate = 30.f;
	// number of bullet arrays to spawn
	// total_bullet_array separated by between spread
	// bullets_per_array separated by within spread
	int total_bullet_array = 1;
	int bullets_per_array = 1;
	float spread_between_array = 1.f; // in degrees
	float spread_within_array = 1.f; // in degrees
	// current angle to spawn
	float start_angle = 0.f;
	// adjust angle until max rate, delta increases/decreases rate
	float spin_rate = 0.f;
	float max_spin_rate = 1.f;
	float spin_delta = 0.f;
	// negate delta once hit max rate
	bool invert = false;
	// updates spin rate, needed for independence of fire rate
	// updates every update_rate * 100ms
	float last_update = -1.f;
	float update_rate = 1.f;
	float bullet_initial_speed = 100;

	// initial bullet cooldown (so when player enters room, does not immediately fire)
	// only used once
	float initial_bullet_cooldown = 1000;

	inline bool operator==(const BulletSpawner& o) {
		return (is_firing == o.is_firing &&
			last_fire_time == o.last_fire_time &&
			fire_rate == o.fire_rate &&
			number_current_fired == o.number_to_fire &&
			number_current_fired == o.number_current_fired &&
			is_cooldown == o.is_cooldown &&
			last_cooldown == o.last_cooldown &&
			cooldown_rate == o.cooldown_rate &&
			total_bullet_array == o.total_bullet_array &&
			bullets_per_array == o.bullets_per_array &&
			spread_between_array == o.spread_between_array &&
			spread_within_array == o.spread_within_array &&
			start_angle == o.start_angle &&
			spin_rate == o.spin_rate &&
			max_spin_rate == o.max_spin_rate &&
			spin_delta == o.spin_delta &&
			invert == o.invert &&
			last_update == o.last_update &&
			update_rate == o.update_rate &&
			bullet_initial_speed == o.bullet_initial_speed);
	}
	inline bool operator!=(const BulletSpawner& o) {
		return !(*this == o);
	}
};

enum class BOSS_ID {
	CIRNO,
	FLANDRE,
	SAKUYA,
	REMILIA
};

struct Boss {
	// boss identifier
	BOSS_ID boss_id;
	// determines if boss system should process state
	bool is_active = false;
	// current pattern to use during phase
	BulletPattern bullet_pattern;
	// duration of the pattern to change to next one
	float duration = -1;
	float current_duration = -1;
	// phases determined by health thresholds
	// e.g. 4 phases -> [75, 50, 25, -1] (from hp 0-25,26-50,51-75,76-?)
	// if health lower than hp threshold, move on to next phase
	std::vector<int> health_phase_thresholds;
	// current boss phase index into health_phase_thresholds
	int phase_index = 0;
	// current BulletPhase id
	// used in boss system to check if two bullet phase id are the same
	// prevents choosing the same bullet phase
	int current_bullet_phase_id = -1;
	// between phase change time
	float phase_change_time = -1;

	// optional invisible entity for extra bullet spawner/pattern
	// IMPORTANT: remember to remove this if removing this boss
	Entity invis_spawner;
};

struct BossInvisible {
	BulletPattern bullet_pattern;
};

// Keeps track of what aura this belongs to
// Allows for deleting the other
struct AuraLink {
	Entity other;
	AuraLink(Entity& other) { this->other = other; };
};

struct Aura {

};

struct AimbotCursor {

};

struct CoinFountain
{
	int remain_coins = 30;
	float time_per_coins = 100.f;
};

enum class State {
	IDLE = 0,
	MOVE = IDLE + 1,
	ALERT = MOVE + 1,
};

enum VFX_TYPE {
	AOE_AMMO_EXPLOSION,
	AOE_AMMO_DISAPPEAR,
	AIMBOT_AMMO_DISAPPEAR,
	HIT_SPARK
};

struct IdleMoveAction {
	State state = State::IDLE;
	float timer_ms = 5000;
	float idle_ms = 5000;
	float moving_ms = 1500;
};

struct EntityAnimation {
	State state = State::IDLE;
	float frame_rate_ms = 200;
	float full_rate_ms = 200;
	vec2 spritesheet_scale = { 0, 0 };
	vec2 render_pos = { 0, 0 };
	bool isCursor = false;
	float offset = 0;
	bool is_active = true;
};

struct DummyEnemySpawner {
	int max_spawn = 5;
	int number_spawned = 0;
};

struct DummyEnemy {

};

// Link between dummy enemy and spawner
struct DummyEnemyLink {
	Entity other;
	DummyEnemyLink(Entity& other) { this->other = other; };
};

struct Deadly
{
	int damage = 1;
	// if bullet_pattern is defined, set has_bullet_pattern to true
	// so bullet system will apply the pattern to the bullet shot by this entity
	bool has_bullet_pattern = false;
	BulletPattern bullet_pattern;
};

struct BeeEnemy {

};

struct BomberEnemy
{
	bool touch_player = false;
};

struct WolfEnemy
{

};

struct Bee2Enemy {

};

struct LizardEnemy {

};

struct GargoyleEnemy {

};

struct WormEnemy {

};

struct NPC
{

};

struct HP {
	int max_hp = 6;
	int curr_hp = 6;
};

struct PlayerHeart {
};

struct Coin
{
	int coin_amount = 1;
};

struct Product
{
	int price = 0;
};

struct MaxHPIncrease
{
	int max_health_increase = 1;
};

struct Shield
{
	int defense = 5;
};

struct AttackUp
{
	int damageUp = 1;
};

struct Chest
{
	bool is_close = true;
};

struct Key
{
};

struct BossHealthBarUI {
	bool is_visible = false;
	std::string boss_name;
	vec3 name_color;
};

// keep track of which boss this ui belongs to
struct BossHealthBarLink {
	Entity other;
	BossHealthBarLink(Entity& other) { this->other = other; };
};

struct Pickupable
{
	int health_change = 1;
};

// IMPORTANT: we have separators in between each section of effects
// Why? There are different implementations which clash together when
// using the same enum. For example, the initial shop system and the health increase
enum EFFECT_TYPE {
	EFFECT_TYPE_SEPARATOR_0, // 0
	BULLET_DAMAGE,
	FIRE_RATE,
	CRITICAL_HIT,
	CRITICAL_DAMAGE,
	INCR_MAXIMUM_HP,
	BOMB,
	EFFECT_TYPE_SEPARATOR_1, // 7
	INCR_CURRENT_HP,
	EFFECT_TYPE_SEPARATOR_2, // 9
	NORMAL_AMMO,
	AIMBOT_AMMO,
	AOE_AMMO,
	TRIPLE_AMMO,
	AIMBOT1BULLET_AMMO,
	EFFECT_TYPE_SEPARATOR_3 // 15
};

struct Purchasableable
{
	// 1=bullet_damage, 2=fire_rate, or 3=critical_hit
	EFFECT_TYPE effect_type;
	int cost;
	bool player_overlap = false;
	int effect_strength;
};

// Tile set names specifically mapped from texture atlas
// comments are in (x,y), indexed by 0
enum class TILE_NAME {
	NONE,
	DEFAULT_FLOOR, // 0,0
	FLOOR_1_0, // 1,0
	FLOOR_2_0, // 2,0
	FLOOR_3_0, // 3,0
	FLOOR_0_1, // 0,1
	FLOOR_1_1, // 1,1
	FLOOR_2_1, // 2,1
	FLOOR_3_1, // 3,1
	LEFT_WALL, // 0,2
	TOP_WALL, // 1,2 - Corridors included
	RIGHT_WALL, // 2,2
	CORRIDOR_BOTTOM_RIGHT, // 0,5
	CORRIDOR_BOTTOM_LEFT, // 2,5
	BOTTOM_LEFT, // 0,6
	BOTTOM_WALL, // 1,6
	BOTTOM_RIGHT, // 2,6
	BOTTOM_RIGHT_LIGHT, // 2,4
	TOP_RIGHT, // 3,2
	TOP_LEFT, // 3,3
	CORRIDOR_TOP_RIGHT, // 3,5
	CORRIDOR_TOP_LEFT, // 3,4
	TRANSPARENT_TILE, // 3,6
	// SKY TILES
	S02,
	S12,
	S22,
	S03,
	S13,
	S23,
	S04,
	S14,
	S24,
	S05,
	S15,
	S25,
	// SKY TILES FOR ENTRANCE OF CORRIDOR
	S00,
	S10,
	S20,
	S30,
	S01,
	S11,
	S21,
	S31,
	S32,
	S33,
	S34,
	S35,
	S06,
	S16,
	S26,
	S36,
	S40,
	S50,
	S41,
	S51,
	S42,
	S52,
	S43,
	S53,
	S44,
	S54,
	S45,
	S55,
	S46,
	S56,
};

// A non interactable tile of the map
struct Floor
{
};

// A interactable tile of the map
struct Wall {
};

// Tile data to be instance rendered
struct TileInstanceData {
	vec4 spriteloc;
	mat3 transform;
};

struct Door {
	bool is_locked = false;
	bool is_closed = true;
	bool is_visited = false; // for doors to remain open
	DIRECTION dir;
	int room_index;
	Entity top_texture; // none if dir is UP OR DOWN
};

struct VisibilityTileInstanceData {
	mat3 transform;
	float alpha;
};

// All data relevant to the shape and motion of entities
struct Motion {
	vec2 position = { 0, 0 };
	//vec2 prev_pos = { 0, 0 };
	float angle = 0;
	vec2 scale = { 10, 10 };
};

// Velocity related data
struct Kinematic {
	float speed_base = 0.f;
	float speed_modified = 0.f;
	vec2 velocity = { 0, 0 };
	// x, y are either -1,0,1
	vec2 direction = { 0, 0 };
};

// Represents collision box with shift transform and size of box
struct Collidable {
	// if is not active, do not check for collisions 
	// (currently only implemented for enemy-enemy collisions when one is dead)
	bool active = true;
	// translate box relative to motion position
	vec2 shift = { 0, 0 };
	// width and height of box
	// IMPORTANT: size MUST be positive
	vec2 size = { 1, 1 };
};

// Represents collision circle with shift transform and radius
struct CircleCollidable {
	vec2 shift = { 0, 0 };
	float radius;
};

// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other; // the second object involved in the collision
	Collision(Entity& other) { this->other = other; };
};

// Focus dot rendering sprite for reimu
struct FocusDot {
};

// Entity follows given path
struct FollowPath
{
	path path;
	int next_path_index = 0;
	// whether to continue chasing or stop at grid
	// set to true to prevent enemy from stopping early
	bool is_player_target = true;
};

// Entity follows flow field
struct FollowFlowField
{
	coord next_grid_pos = { -1, -1 };
	// whether to continue chasing or stop at grid
	// set to true to prevent enemy from stopping early
	bool is_player_target = true;
};

// Sets the brightness of the screen
struct ScreenState
{
	float darken_screen_factor = -1;
	float bomb_screen_factor = -1;
};

struct WinMenu
{

};

struct LoseMenu
{

};

struct InfographicMenu {

};

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};

// Used for bullet control - loop to specified index
// This could potentially stall if amount to loop is large with no delays
struct BulletLoop {
	float index_to_loop = -1;
	float amount_to_loop = -1;
};

// Start firing after specified amount of time
struct BulletStartFiringTimer {
	float counter_ms = -1;
};

// Used for bullet control - Halts bullet pattern by delay ms
struct BulletDelayTimer {
	float delay_counter_ms = -1;
};

// for BULLET_ACTION::SPEED_TIMER
struct BulletSpeedTimer {
	float start_speed = 0;
	float end_speed = 0;
	float timer_ms = 0;
	float max_timer_ms = 0;
};

// Update entity ai behavior tree after update ms
struct AiTimer {
	float update_timer_ms = 500;
	float update_base = 500;
};

// A timer that will be associated to dying chicken
struct HitTimer
{
	float counter_ms = 80;
};

struct DeathTimer
{
	float death_counter_ms = 3000;
	float first_animation_frame = false;
};


struct BezierCurve
{
	float curve_counter_ms = 500;
	float t = 0.0f;
	std::vector<vec2> bezier_pts;
};

struct BulletDeathTimer
{
	float death_counter_ms = -1;
};

struct InvulnerableTimer {
	float invulnerable_counter_ms = 1000;
};

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl & chicken.vs.glsl)
struct ColoredVertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

struct RenderText {
	std::string content;
	float transparency = 1.0;
};

struct RenderTextPermanent {
	std::string content;
	float transparency = 1.0;
};

struct RenderTextWorld {
	std::string content;
	float transparency = 1.0;
};

struct RenderTextPermanentWorld {
	std::string content;
	float transparency = 1.0;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
	vec2 original_size = { 1,1 };
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
	std::vector<vec3> ordered_vertices;
};

// order is important, reflects keyboard.png
enum class KEYS {
	W = 1,
	A = W + 1,
	S = A + 1,
	D = S + 1,
	F = D + 1,
	Q = F + 1,
	E = Q + 1,
	R = E + 1,
	H = R + 1,
	ESC = H + 1,
	MOUSE_1 = ESC + 1,
	MOUSE_2 = MOUSE_1 + 1,
	SCROLL = MOUSE_2 + 1,
	SHIFT_0 = SCROLL + 1,
	SHIFT_1 = SHIFT_0 + 1,
	SPACE_0 = SHIFT_1 + 1,
	SPACE_1 = SPACE_0 + 1,
	SPACE_2 = SPACE_1 + 1
};

// IDs for each tile type
enum class TILE_TYPE {
	EMPTY = 0,
	FLOOR = EMPTY + 1,
	WALL = FLOOR + 1,
	DOOR = WALL + 1
};

enum class EMOTION {
	ANGRY = 0,
	CRY = ANGRY + 1,
	SPECIAL = CRY + 1,
	LAUGH = SPECIAL + 1,
	NORMAL = LAUGH + 1,
	SHOCK = NORMAL + 1,
};

/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID {
	BULLET = 0,
	ENEMY_BEE = BULLET + 1,
	PLAYER = ENEMY_BEE + 1,
	ENEMY_WOLF = PLAYER + 1,
	ENEMY_BOMBER = ENEMY_WOLF + 1,
	ENEMY_BULLET = ENEMY_BOMBER + 1,
	TILE_1 = ENEMY_BULLET + 1,
	TILE_2 = TILE_1 + 1,
	INNER_WALL = TILE_2 + 1,
	TOP_WALL = INNER_WALL + 1,
	DOOR = TOP_WALL + 1,
	DOOR_OPEN = DOOR + 1,
	LEFT_WALL = DOOR_OPEN + 1,
	RIGHT_WALL = LEFT_WALL + 1,
	LEFT_TOP_CORNER_WALL = RIGHT_WALL + 1,
	LEFT_BOTTOM_CORNER_WALL = LEFT_TOP_CORNER_WALL + 1,
	RIGHT_TOP_CORNER_WALL = LEFT_BOTTOM_CORNER_WALL + 1,
	RIGHT_BOTTOM_CORNER_WALL = RIGHT_TOP_CORNER_WALL + 1,
	ROCK = RIGHT_BOTTOM_CORNER_WALL + 1,
	REIMU_HEALTH = ROCK + 1,
	REIMU_HEAD = REIMU_HEALTH + 1,
	EMPTY_HEART = REIMU_HEAD + 1,
	BOTTOM_WALL = EMPTY_HEART + 1,
	WALL_EDGE = BOTTOM_WALL + 1,
	WALL_SURFACE = WALL_EDGE + 1,
	PILLAR_TOP = WALL_SURFACE + 1,
	PILLAR_BOTTOM = PILLAR_TOP + 1,
	HEALTH_1 = PILLAR_BOTTOM + 1,
	HEALTH_2 = HEALTH_1 + 1,
	REGENERATE_HEALTH = HEALTH_2 + 1,
	BOSS_CIRNO = REGENERATE_HEALTH + 1,
	BOSS_HEALTH_BAR = BOSS_CIRNO + 1,
	REIMU_BULLET_DISAPPEAR = BOSS_HEALTH_BAR + 1,
	FOCUS_DOT = REIMU_BULLET_DISAPPEAR + 1,
	KEYS = FOCUS_DOT + 1,
	COIN = KEYS + 1,
	ATTACKDMG = COIN + 1,
	ATTACKSPEED = ATTACKDMG + 1,
	CRTCHANCE = ATTACKSPEED + 1,
	CRTDMG = CRTCHANCE + 1,
	BOMB = CRTDMG + 1,
	CRTHITICON = BOMB + 1,
	FOCUS_BAR = CRTHITICON + 1,
	COIN_STATIC = FOCUS_BAR + 1,
	TILES_ATLAS_SANDSTONE = COIN_STATIC + 1,
	TILES_ATLAS_RUINS = TILES_ATLAS_SANDSTONE + 1,
	BUTTON = TILES_ATLAS_RUINS + 1,
	BUTTON_HOVERED = BUTTON + 1,
	NONE = BUTTON_HOVERED + 1,
	MENU_TITLE = NONE + 1,
	MENU_BACKGROUND = MENU_TITLE + 1,
	PAUSE_BACKGROUND = MENU_BACKGROUND + 1,
	C = PAUSE_BACKGROUND + 1,
	B = C + 1,
	A = B + 1,
	S = A + 1,
	ITEM_R = S + 1,
	ITEM_G = ITEM_R + 1,
	ITEM_B = ITEM_G + 1,
	ITEM_Y = ITEM_B + 1,
	ITEM_P = ITEM_Y + 1,
	DOOR_HORIZONTAL_OPEN = ITEM_P + 1,
	DOOR_HORIZONTAL_CLOSE = DOOR_HORIZONTAL_OPEN + 1,
	DOOR_VERTICAL_OPEN_DOWN = DOOR_HORIZONTAL_CLOSE + 1,
	DOOR_VERTICAL_OPEN_UP = DOOR_VERTICAL_OPEN_DOWN + 1,
	DOOR_VERTICAL_CLOSE_DOWN = DOOR_VERTICAL_OPEN_UP + 1,
	DOOR_VERTICAL_CLOSE_UP = DOOR_VERTICAL_CLOSE_DOWN + 1,
	REIMU_PORTRAIT = DOOR_VERTICAL_CLOSE_UP + 1,
	CIRNO_PORTRAIT = REIMU_PORTRAIT + 1,
	FLANDRE_PORTRAIT = CIRNO_PORTRAIT + 1,
	MARISA_PORTRAIT = FLANDRE_PORTRAIT + 1,
	DIALOGUE_BOX = MARISA_PORTRAIT + 1,
	TELEPORTER = DIALOGUE_BOX + 1,
	WINDEATH_SCREEN = TELEPORTER + 1,
	BOSS_FLANDRE = WINDEATH_SCREEN + 1,
	FLANDRE_BULLET = BOSS_FLANDRE + 1,
	INFOGRAPHIC = FLANDRE_BULLET + 1,
	NPC_MARISA = INFOGRAPHIC + 1,
	ENEMY_LIZARD = NPC_MARISA + 1,
	ENEMY_WORM = ENEMY_LIZARD + 1,
	ENEMY_BEE2 = ENEMY_WORM + 1,
	ENEMY_GARGOYLE = ENEMY_BEE2 + 1,
	AIMBOT_CURSOR = ENEMY_GARGOYLE + 1,
	AOE_AMMO_EXPLOSION = AIMBOT_CURSOR + 1,
	AOE_AMMO_BULLET = AOE_AMMO_EXPLOSION + 1,
	AIMBOT_AMMO_BULLET = AOE_AMMO_BULLET + 1,
	AOE_AMMO_BULLET_DISAPPEAR = AIMBOT_AMMO_BULLET + 1,
	AIMBOT_AMMO_BULLET_DISAPPEAR = AOE_AMMO_BULLET_DISAPPEAR + 1,
	HIT_SPARK = AIMBOT_AMMO_BULLET_DISAPPEAR + 1,
	CHEST_OPEN = HIT_SPARK + 1,
	CHEST_CLOSE = CHEST_OPEN + 1,
	SKELETON = CHEST_CLOSE + 1,
	BARREL = SKELETON + 1,
	POTTERY = BARREL + 1,
	FIREPLACE = POTTERY + 1,
	AIMBOT_AMMO_ITEM = FIREPLACE + 1,
	AOE_AMMO_ITEM = AIMBOT_AMMO_ITEM + 1,
	TRIPLE_AMMO_ITEM = AOE_AMMO_ITEM + 1,
	AIMBOT1BULLET_AMMO_ITEM = TRIPLE_AMMO_ITEM + 1,
	CIRNO_AURA = AIMBOT1BULLET_AMMO_ITEM + 1,
	FLANDRE_AURA = CIRNO_AURA + 1,
	TELEPORTER_SMALL = FLANDRE_AURA + 1,
	NORMAL_SIGN = TELEPORTER_SMALL + 1,
	BOSS_SIGN = NORMAL_SIGN + 1,
	START_SIGN = BOSS_SIGN + 1,
	SHOP_SIGN = START_SIGN + 1,
	TILES_ATLAS_WATER = SHOP_SIGN + 1,
	TILES_ATLAS_SKY = TILES_ATLAS_WATER + 1,
	BOSS_SAKUYA = TILES_ATLAS_SKY + 1,
	BOSS_REMILIA = BOSS_SAKUYA + 1,
	TEXTURE_COUNT = BOSS_REMILIA + 1,
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	EGG = COLOURED + 1,
	PLAYER = EGG + 1,
	TEXTURED = PLAYER + 1,
	WIND = TEXTURED + 1,
	UI = WIND + 1,
	FONT = UI + 1,
	PLAYER_HB = FONT + 1,
	BOSSHEALTHBAR = PLAYER_HB + 1,
	COMBO = BOSSHEALTHBAR + 1,
	GREY = COMBO + 1,
	EFFECT_COUNT = GREY + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

// We won't use geometry as we are mostly using sprites
enum class GEOMETRY_BUFFER_ID {
	REIMU_FRONT = 0,
	SPRITE = REIMU_FRONT + 1,
	EGG = SPRITE + 1,
	DEBUG_LINE = EGG + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	REIMU_LEFT = SCREEN_TRIANGLE + 1,
	REIMU_RIGHT = REIMU_LEFT + 1,
	GEOMETRY_COUNT = REIMU_RIGHT + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest {
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
};

// Struct for Font
// This adapted from lecture material (Wednesday Feb 28th 2024)
struct Character {
	unsigned int TextureID;  // ID handle of the glyph texture
	glm::ivec2   Size;       // Size of glyph
	glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
	unsigned int Advance;    // Offset to advance to next glyph
	char character;
};