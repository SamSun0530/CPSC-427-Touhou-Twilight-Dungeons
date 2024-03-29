#pragma once
#include "common.hpp"
#include <vector>
#include <set>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"


struct PlayerBullet {
	int damage = 10;
};

struct UIUX {

};

struct EnemyBullet
{
	int damage = 1;
};

struct MapLevel {
	enum LEVEL {
		TUTORIAL,
		MAIN
	} level = LEVEL::MAIN;
};
extern MapLevel map_level;

/*
Bullet actions:
Floats:
SPEED - float - change in velocity magnitude
ROTATE - float - change in bullet direction
DELAY - float - wait until executing next command
DEL - float - bullet death timer to remove bullet

Vec2s:
LOOP - vec2 - loop back to specified index
	- vec2[0] = number to loop (specify 1 - loops once)
	- vec2[1] = 0-indexed to loop to (should be index >= 0 && index < commands.size)
DIRECTION - vec2 - change direction to x,y

Vec3s:
SPLIT - vec3 - split one bullets into multiple bullets based on angle
	- vec2[0] = number of bullets to split into (<= 1 - won't do anything)
	- vec2[1] = angle for the bullet spread
	- vec2[2] = initial bullet speed
*/
enum class BULLET_ACTION {
	SPEED,
	ROTATE,
	DELAY,
	LOOP,
	DEL,
	SPLIT,
	DIRECTION
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

struct Boss {
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
};

// Player component
struct Player
{
	bool invulnerability = false;
	int bullet_damage = 10;
	int coin_amount = 0;
	int key_amount = 0;
	float invulnerability_time_ms = 1000;
	float fire_rate = 0.2;
	float critical_hit = 0.05;
	float critical_demage = 1.5;
};

enum class State {
	IDLE = 0,
	MOVE = IDLE + 1,
	ALERT = MOVE + 1,
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
};

struct BeeEnemy {

};

struct BomberEnemy
{

};

struct WolfEnemy
{

};

struct SubmachineGunEnemy
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

};

struct Key
{
};
	
struct BossHealthBarUI {
	bool is_visible = false;
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

// A non interactable tile of the map
struct Floor
{
};

// A interactable tile of the map
struct Wall {
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
};
extern FocusMode focus_mode;

// Sets the brightness of the screen
struct ScreenState
{
	float darken_screen_factor = -1;
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
	bool in_world;
};

struct RenderTextPermanent {
	std::string content;
	bool in_world;
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

enum class KEYS {
	A = 1,
	D = A + 1,
	ESC = D + 1,
	F = ESC + 1,
	S = F + 1,
	SHIFT = S + 1,
	W = SHIFT + 1,
	MOUSE_1 = W + 1,
	SCROLL = MOUSE_1 + 1,
	R = SCROLL + 1,
	SPACE = R + 1,
	P = SPACE + 1,
};


// IDs for each tile type
enum class TILE_TYPE {
	EMPTY = 0,
	FLOOR = EMPTY + 1,
	WALL = FLOOR + 1
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
	REIMU_HEALTH = RIGHT_BOTTOM_CORNER_WALL + 1,
	REIMU_HEAD= REIMU_HEALTH + 1,
	EMPTY_HEART = REIMU_HEAD + 1,
	BOTTOM_WALL = EMPTY_HEART + 1,
	WALL_EDGE = BOTTOM_WALL + 1,
	WALL_SURFACE = WALL_EDGE + 1,
	PILLAR_TOP = WALL_SURFACE + 1,
	PILLAR_BOTTOM = PILLAR_TOP + 1,
	HEALTH_1 = PILLAR_BOTTOM + 1,
	HEALTH_2 = HEALTH_1 + 1,
	REGENERATE_HEALTH = HEALTH_2 + 1,
	BOSS = REGENERATE_HEALTH + 1,
	BOSS_HEALTH_BAR = BOSS + 1,
	REIMU_BULLET_DISAPPEAR = BOSS_HEALTH_BAR + 1,
	FOCUS_DOT = REIMU_BULLET_DISAPPEAR + 1,
	KEYS = FOCUS_DOT + 1,
	COIN = KEYS + 1,
	ATTACKDMG = COIN + 1,
	ATTACKSPEED = ATTACKDMG + 1,
	CRTCHANCE = ATTACKSPEED + 1,
	CRTDMG = CRTCHANCE + 1,
	CRTHITICON = CRTDMG + 1,
	FOCUS_BAR = CRTHITICON + 1,
	COIN_STATIC = FOCUS_BAR + 1,
	TEXTURE_COUNT = COIN_STATIC + 1
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
	EFFECT_COUNT = BOSSHEALTHBAR + 1
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