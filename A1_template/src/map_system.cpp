// Adpated from: https://www.gamedeveloper.com/programming/procedural-dungeon-generation-algorithm#close-modal

// internal
#include "map_system.hpp"

const vec2 small_world_map_size = {500,500};
const vec2 med_world_map_size = {1000,1000};
const vec2 large_world_map_size = {2000,2000};

// struct room {
//     int id;
//     vec2 pos;
//     vec2 size; // X, Y
// };

// const enum room_type {
//     startRoom,
//     openRoom
// };

std::vector<Entity> rooms;

MapSystem::MapSystem() {
    // Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

void MapSystem::generateMap(int floor) {
    int max_rooms = 50;
    int generation_circle_radius = 1000;
    vec2 widthRange = {world_tile_size*5, world_tile_size*11};
    vec2 heightRange = {world_tile_size*5, world_tile_size*11};
    vec2 aspectRatioRange = {0.5, 2.0};

    Entity room;

    // Creates all the rooms entities uniformly inside a radius
    for (int i = 0; i < max_rooms; i++) {
        room = Entity();
        registry.roomHitbox.emplace(room);

        		// Initialize the motion
		auto& motion = registry.motions.emplace(room);
		motion.angle = 0.f;
		motion.direction = { 0, 0 };
		motion.position = getRandomPointInCircle(generation_circle_radius);
		motion.scale = getUniformRectangleDimentions(widthRange, heightRange);

    }

    // creates rooms and insert it into map
    // creates entities and textures from the map
}

// void spawn_enemty(enemyType, room, spawnpoint)

/**
 * Pos: vec2 of the top left corner of the room
 * Type: a predefined emum of room_type
*/
// room createRoom(room& outputRoom,vec2) {
//     switch (type)
//     {
//     case startRoom:
//         return generateStartRoom(outputRoom, pos, type);
//         break;
//     default:
//         //throw error, not valid room
//         break;
//     }
// }

// room generateStartRoom(room& outputRoom, vec2 pos, room_type type) {
//     vec2 startRoomSize = {10,10};

//     outputRoom.pos = pos;
//     outputRoom.size = startRoomSize;
//     // Fill walls
//     return outputRoom;
// }

// Generates the dimention of a rectangle following a uniform distribution. All values are measured in pixels
vec2 MapSystem::getUniformRectangleDimentions(vec2 widthRange, vec2 heightRange) {
    auto width = clamp(normal_dist(rng)*world_tile_size,widthRange[0],widthRange[1]);
    auto height = clamp(normal_dist(rng)*world_tile_size,heightRange[0],heightRange[1]);
    return {width, height};
}

// Takes a value and a tile size and floors it to be a multiple of tile size
float roundToTileSize(float input, float tileSize) {
    return {floorf(((input+tileSize-1)/tileSize))*tileSize};
}

// Adapted from: https://stackoverflow.com/questions/5837572/generate-a-random-point-within-a-circle-uniformly
// Samples a uniform point from a Circle centered at (0,0)
vec2 MapSystem::getRandomPointInCircle(int maxRadius) {
    // Randomly generates a point in polar coordinates with uniform distrubution
    auto radius = maxRadius * sqrt(uniform_dist(rng)); // sqrt so that the center is more dense than the outer area to maintain uniformity
    auto theta = 2*M_PI*uniform_dist(rng);

    // Convert to cartisian coordinates, convert to tile size, and return
    return {roundToTileSize(radius * cos(theta), world_tile_size), roundToTileSize(radius * sin(theta), world_tile_size)};
}



// // Takes in a value, min, and max and clamps the value such that it is in the range of min max
// float clamp(float value, float max, float min) {
//     return std::min(std::max(value, min), max);
// }