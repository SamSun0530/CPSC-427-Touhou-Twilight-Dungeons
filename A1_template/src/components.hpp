#pragma once
#include "common.hpp"
#include <vector>
#include <set>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"


struct PlayerBullet {
	int damage = 1;
};

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
	float moving_ms = 1000;
};

enum EnemyType
{
	normal,
	shotgun,
	suicide,
};

struct EntityAnimation {
	State state = State::IDLE;
	float frame_rate_ms = 200;
	vec2 spritesheet_scale = { 0, 0 };
	vec2 render_pos = { 0, 0 };
	bool isCursor = false;
	float offset = 0;
};

struct Deadly
{
	int damage = 1;
};

struct BasicEnemy {
	
};

struct SuicideEnemy
{
	
};

struct ShotgunEnemy
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
	int damage = 1;
};


struct EnemyBullet
{
	int damage = 1;
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

// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other; // the second object involved in the collision
	Collision(Entity& other) { this->other = other; };
};

// Data structure for toggling debug mode
struct Debug {
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};
extern Debug debugging;

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

// A timer that will be associated to dying chicken
struct HitTimer
{
	float counter_ms = 50;
};

struct DeathTimer
{
	float death_counter_ms = 3000;
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
	vec2 original_size = {1,1};
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

// IDs for each tile type
enum class TILE_TYPE {
	EMPTY = 0,
	FLOOR = EMPTY+1,
	WALL = FLOOR+1
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
	ENEMY = BULLET + 1,
	PLAYER = ENEMY + 1,
	ENEMY_BULLET = PLAYER + 1,
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
	TEXTURE_COUNT = WALL_SURFACE + 1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	EGG = COLOURED + 1,
	PLAYER = EGG + 1,
	TEXTURED = PLAYER + 1,
	WIND = TEXTURED + 1,
	UI = WIND + 1,
	EFFECT_COUNT = UI + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

// We won't use geometry as we are mostly using sprites
enum class GEOMETRY_BUFFER_ID {
	CHICKEN = 0,
	SPRITE = CHICKEN + 1,
	EGG = SPRITE + 1,
	DEBUG_LINE = EGG + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	GEOMETRY_COUNT = SCREEN_TRIANGLE + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest {
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
};