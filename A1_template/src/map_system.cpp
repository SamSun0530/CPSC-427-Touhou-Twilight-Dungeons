// Adpated from: https://www.gamedeveloper.com/programming/procedural-dungeon-generation-algorithm#close-modal

// internal
#include "map_system.hpp"
#include <iostream>

const vec2 small_world_map_size = {500,500};
const vec2 med_world_map_size = {1000,1000};
const vec2 large_world_map_size = {2000,2000};

std::vector<Entity> rooms;

MapSystem::MapSystem() {
    // Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

void MapSystem::generateMap(int floor) {
    // Resets all values
    registry.roomHitbox.clear();

    int max_rooms = 5;
    int generation_circle_radius = 3*world_tile_size;
    vec2 widthRange = {world_tile_size*5, world_tile_size*11};
    vec2 heightRange = {world_tile_size*5, world_tile_size*11};
    vec2 aspectRatioRange = {0.5, 2.0};

    Entity room;

    // Creates all the rooms entities uniformly inside a radius
    // with normal distributed room size
    for (int i = 0; i < max_rooms; i++) {
        room = Entity();
        registry.roomHitbox.emplace(room);

        		// Initialize the motion
		auto& motion = registry.motions.emplace(room);
		motion.angle = 0.f;
		motion.direction = { 0, 0 };
		motion.position = getRandomPointInCircle(generation_circle_radius);
		motion.scale = getUniformRectangleDimentions(widthRange, heightRange);
        std::cout << "Position of Room: " << i << " is (" << motion.position.x <<" , " << motion.position.y << ")" << std::endl; 
        // printf("Position of Room:%d is %f,%f\n", i, motion.position.x, motion.position.y);
    }

    // // Seperates the rooms if collided
    // std::vector<Motion> motion_container;
    // for (Entity roomBox : registry.roomHitbox.entities) {
    //     motion_container.insert(registry.motions.get(roomBox));
    // }


    // Checks collision between all rooms
    // Similar collision code as physics system
    for (uint i = 0; i <  registry.roomHitbox.size(); i++) {
        
        Entity entity_i = registry.roomHitbox.entities[i];
        Motion motion_i = registry.motions.get(entity_i);

		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
        for(uint j = i+1; j < registry.roomHitbox.size(); j++) {
            Entity entity_j = registry.roomHitbox.entities[j];
			Motion motion_j = registry.motions.get(entity_j);

            if(PhysicsSystem::collides_AABB(motion_i,motion_j)) {
                // Create collision event
                // Only 1 collsion is taken into consideration
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
                // Exit loop since entity_i has collided at least 1 time
                continue;
            }
        }
    }

    	// Loop over all collisions detected by the physics system
	auto& collisionsRegistry = registry.collisions;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other;

        if (registry.roomHitbox.has(entity)) {
            Motion& room_motion = registry.motions.get(entity);
            Motion& other_room_motion = registry.motions.get(entity_other);
            vec2 dir;

            // Calculates the unormalized vector between the center points of the 2 colliding rooms
            vec2 center_room = {room_motion.position.x + room_motion.scale.x/2, 
                                room_motion.position.y + room_motion.scale.y/2};
            vec2 center_other_room = {other_room_motion.position.x + other_room_motion.scale.x/2, 
                                      other_room_motion.position.y + other_room_motion.scale.y/2};
            dir = normalize(center_room - center_other_room);

            // Moves the room 1 world tile away from the other room in both x and y directions
            room_motion.position = room_motion.position + vec2{world_tile_size * dir.x, world_tile_size * dir.y};
        }
    }

    // creates rooms and insert it into map
    // creates entities and textures from the map
}

// Takes a value and a tile size and floors it to be a multiple of tile size
float roundToTileSize(float input, float tileSize) {
    return {floorf(((input+tileSize-1)/tileSize))*tileSize};
}

// Generates the dimention of a rectangle following a uniform distribution. All values are measured in pixels
vec2 MapSystem::getUniformRectangleDimentions(vec2 widthRange, vec2 heightRange) {
    auto width = roundToTileSize(clamp(normal_dist(rng)*world_tile_size,widthRange[0],widthRange[1]), world_tile_size);
    auto height = roundToTileSize((normal_dist(rng)*world_tile_size,heightRange[0],heightRange[1]), world_tile_size);
    return {width, height};
}

// Adapted from: https://stackoverflow.com/questions/5837572/generate-a-random-point-within-a-circle-uniformly
// Samples a uniform point from a Circle centered at (0,0)
vec2 MapSystem::getRandomPointInCircle(int maxRadius) {
    // Randomly generates a point in polar coordinates with uniform distrubution
    auto radius = maxRadius * sqrt(uniform_dist(rng)); // sqrt so that the center is more dense than the outer area to maintain uniformity
    auto theta = 2*M_PI*uniform_dist(rng);

    // Convert to cartisian coordinates, convert to tile size, and return
    vec2 output = {roundToTileSize(radius * cos(theta), world_tile_size), roundToTileSize(radius * sin(theta), world_tile_size)};
    return output;
}
