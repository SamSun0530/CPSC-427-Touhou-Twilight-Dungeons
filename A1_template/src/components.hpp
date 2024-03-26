#pragma once
#include "common.hpp"
#include <vector>
#include <set>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"


struct PlayerBullet {
	int damage = 1;
};

struct UIUX {

};

struct MapLevel {
	enum LEVEL {
		TUTORIAL,
		MAIN
	} level = LEVEL::MAIN;
};
extern MapLevel map_level;

// Manages when entity is able to fire a bullet again
struct BulletFireRate
{
	// IMPORTANT: set this to -fire_rate so entity can fire immediately
	float last_time = -0.2;
	// fire rate is (fire_rate) second/shot or (fire_rate)^-1 shots/second
	// e.g. fire_rate = 0.1 s/shot = 10 shots/s
	float fire_rate = 0.2;
	bool is_firing = false;
};

// Player component
struct Player
{
	bool invulnerability = false;
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

struct Pickupable
{
	int health_change = 1;
};


struct EnemyBullet
{
	int damage = 1;
};

struct RoomHitbox {
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

// Data structure for toggling debug mode
struct Debug {
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};
extern Debug debugging;

struct FocusMode {
	bool in_focus_mode = 0;
	float speed_constant = 1.0f;
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

// Update entity ai behavior tree after update ms
struct AiTimer {
	float update_timer_ms = 500;
	float update_base = 500;
};

// A timer that will be associated to dying chicken
struct HitTimer
{
	float counter_ms = 50;
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
	FULL_HEART = RIGHT_BOTTOM_CORNER_WALL + 1,
	HALF_HEART = FULL_HEART + 1,
	EMPTY_HEART = HALF_HEART + 1,
	BOTTOM_WALL = EMPTY_HEART + 1,
	WALL_EDGE = BOTTOM_WALL + 1,
	WALL_SURFACE = WALL_EDGE + 1,
	PILLAR_TOP = WALL_SURFACE + 1,
	PILLAR_BOTTOM = PILLAR_TOP + 1,
	HEALTH_1 = PILLAR_BOTTOM + 1,
	HEALTH_2 = HEALTH_1 + 1,
	REGENERATE_HEALTH = HEALTH_2 + 1,
	REIMU_BULLET_DISAPPEAR = REGENERATE_HEALTH + 1,
	FOCUS_DOT = REIMU_BULLET_DISAPPEAR + 1,
	KEYS = FOCUS_DOT + 1,
	TEXTURE_COUNT = KEYS + 1
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
	EFFECT_COUNT = FONT + 1
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