// Adpated from: https://www.gamedeveloper.com/programming/procedural-dungeon-generation-algorithm#close-modal

// internal
#include "map_system.hpp"
#include "map_system.hpp"
#include "world_init.hpp"
#include "common.hpp"
#include <iostream>

const vec2 small_world_map_size = {500,500};
const vec2 med_world_map_size = {1000,1000};
const vec2 large_world_map_size = {2000,2000};
int room_size = 11;

// Static map var
std::vector<std::vector<int>> MapSystem::world_map;


MapSystem::MapSystem() {
    // Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

void MapSystem::init(RenderSystem* renderer_arg) {
	this->renderer = renderer_arg;
}

Room generateBasicRoom(int x, int y);
void addRoomToMap(const Room& room, std::vector<std::vector<int>>& map);
void addHallwayBetweenRoom(const Room& room1, const Room& room2, std::vector<std::vector<int>>& map);
void MapSystem::generateBasicMap() {
    
     std::vector<std::vector<int>> map(world_height, std::vector<int>(world_width, 0));
     std::vector<Room> rooms;
     rooms.push_back(generateBasicRoom(0,0));
     rooms.push_back(generateBasicRoom(room_size+5,0));
     rooms.push_back(generateBasicRoom(2*(room_size+5),0));
     rooms.push_back(generateBasicRoom(0,room_size+5));
     rooms.push_back(generateBasicRoom(room_size+5,room_size+5));
     rooms.push_back(generateBasicRoom(2*(room_size+5),room_size+5));

     for(Room room : rooms) {
        addRoomToMap(room, map);
     }

    addHallwayBetweenRoom(rooms[0], rooms[3], map);
    addHallwayBetweenRoom(rooms[0], rooms[1], map);
    addHallwayBetweenRoom(rooms[1], rooms[2], map);
    addHallwayBetweenRoom(rooms[2], rooms[3], map);
    addHallwayBetweenRoom(rooms[2],rooms[5], map);
    addHallwayBetweenRoom(rooms[4],rooms[5], map);

    // // Print the initialized array
    // for (int i = 0; i < map.size(); ++i) {
    //     for (int j = 0; j < map[i].size(); ++j) {
    //         std::cout << map[i][j] << " ";
    //     }
    //     std::cout << std::endl;
    // }

    MapSystem::generateAllEntityTiles(map);

    world_map = map;
}

void MapSystem::generateMap(int floor) {
    // Resets all values
    registry.roomHitbox.clear();

    int max_rooms = 100;
    int generation_circle_radius = 10*world_tile_size;
    vec2 widthRange = {world_tile_size*11, world_tile_size*11};
    vec2 heightRange = {world_tile_size*11, world_tile_size*11};
    // vec2 aspectRatioRange = {0.5, 2.0};

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
        //std::cout << " Start: Position of Room: " << i << " is (" << motion.position.x <<" , " << motion.position.y << ")" << std::endl;
        //std::cout << " Size of Obj: (" << motion.scale.x << "," << motion.scale.y << ")" << std::endl;
        // printf("Position of Room:%d is %f,%f\n", i, motion.position.x, motion.position.y);
    }

    // // Seperates the rooms if collided
    // std::vector<Motion> motion_container;
    // for (Entity roomBox : registry.roomHitbox.entities) {
    //     motion_container.insert(registry.motions.get(roomBox));
    // }

    // Checks for collisions and move all rooms until no more collisions are detected
    
    do {
        // Resets all collision flags
        registry.collisions.clear();
    
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
                    registry.collisions.emplace_with_duplicates(entity_j, entity_i);
                    // Exit loop since entity_i has collided at least 1 time
                    break;
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
                vec2 dist = center_room - center_other_room;
                if(dist.x != 0 && dist.y != 0) {
                    // vec2 temp1 = normalize(dist);
                    // vec2 temp2 = ceil(temp1);
                    dir = vec2(sign(dist.x),sign(dist.y));
                } else {
                    //Sets the direction to a random direction
                    dir = vec2{roundf(uniform_dist(rng)*2-1),roundf(uniform_dist(rng)*2-1)};
                }
                //std::cout << "Direction: (" << dir.x << "," << dir.y << ")" << std::endl;

                // Moves the room 1 world tile away from the other room in both x and y directions
                room_motion.position = room_motion.position + vec2{world_tile_size * dir.x, world_tile_size * dir.y};
                //std::cout << "Moved: Position of Room: " << i << " is (" << room_motion.position.x <<" , " << room_motion.position.y << ")" << std::endl;
            }
        } 

    // Removes all collisions, ie marks them as resolved
    } while (registry.collisions.size() > 0);

    // creates rooms and insert it into map
    // creates entities and textures from the map

    //Triangulation: https://www.gorillasun.de/blog/bowyer-watson-algorithm-for-delaunay-triangulation/
}

void MapSystem::debug() {
    // createLine({0,0},{100,100});
    for(Entity room: registry.roomHitbox.entities) {
        if(!registry.motions.has(room)) {
            continue;
        }
        Motion motion = registry.motions.get(room);
        vec2 pos = (motion.position/vec2(20,20) + window_px_half);
        vec2 scale = motion.scale/vec2(20,20);
        createLine(pos,scale);
    }
}

// Takes a value and a tile size and floors it to be a multiple of tile size
float roundToTileSize(float input, float tileSize) {
    return {floorf(((input+tileSize-1)/tileSize))*tileSize};
}

// Generates the dimention of a rectangle following a uniform distribution. All values are measured in pixels
vec2 MapSystem::getUniformRectangleDimentions(vec2 widthRange, vec2 heightRange) {
    auto width = roundToTileSize(clamp(normal_dist(rng)*world_tile_size,widthRange[0],widthRange[1]), world_tile_size);
    auto height = roundToTileSize(clamp(normal_dist(rng)*world_tile_size,heightRange[0],heightRange[1]), world_tile_size);
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

//Triangulation: https://www.gorillasun.de/blog/bowyer-watson-algorithm-for-delaunay-triangulation/
std::vector<Triangle> addVertex(const Vertex& vertex, std::vector<Triangle>& triangles) {
    return triangles; // TODO
}

bool inCircumCircle(Triangle triangle, Vertex vertex) {
    vec2 center = triangle.circumcircle.center;
    vec2 manhatten_distance = center- vec2(vertex.x,vertex.y);
    float euclidian_distance =  sqrt(dot(manhatten_distance, manhatten_distance));
    return euclidian_distance < triangle.circumcircle.radius;
}

//Triangulation: https://www.gorillasun.de/blog/bowyer-watson-algorithm-for-delaunay-triangulation/
Triangle generateSuperTriangle(std::vector<Vertex> points) {
    int min_x, min_y = INT_MAX;
    int max_x, max_y = -INT_MAX;

    // gets the max/min x,y values of all points
    for(Vertex point: points) {
        min_x = min(min_x, point.x);
        min_y = min(min_y, point.y);
        max_x = max(max_x, point.x);
        max_y = max(max_y, point.y);
    }

    // Calculates the offset from the min/max points so that the super triangle is sure to
    // be large enough to cover all points
    int distance_x = (max_x-min_x) * 10;
    int distance_y = (max_y-min_y) * 10;

    Vertex v0 = {min_x - distance_x, min_y - distance_y  * 3};
    Vertex v1 = {min_x - distance_x, max_y + distance_y }; 
    Vertex v2 = {max_x + distance_x * 3, max_y + distance_y }; 
    return {v0,v1,v2};
}

// The Main Triangulation method
std::vector<Triangle> triangulate(std::vector<Vertex> verticies) {
    // Create bounding super triangle
    Triangle super_triangle = generateSuperTriangle(verticies);

    // Set the super triangle as the only triangle
    std::vector<Triangle> triangles;
    triangles.push_back(super_triangle);

    // Triangulate each vertex
    for(Vertex vertex: verticies) {
        triangles = addVertex(vertex, triangles);
    }

    return {};
}

// Calculates and returns the circumcircle of a triangle represented as 3 verticies
Circle calcCircumCircle(Vertex v1, Vertex v2, Vertex v3) {

    // Calculates the midpoints of 2 sides
    vec2 midPoint1 = {(v1.x + v2.x)/2 , (v1.y + v2.y)/2};
    vec2 midPoint2 = {(v1.x + v3.x)/2 , (v1.y + v3.y)/2};

    // Calculates the slope of the sides
    float slope1 = (v2.y - v1.y) / (v2.x - v1.x);
    float slope2 = (v3.y - v1.y) / (v3.x - v1.x);

    // Calculates the negative reciprical of the slope, the slope of the perpendicular bisector
    float perpendicular_slope1 = -1.0f / slope1;
    float perpendicular_slope2 = -1.0f / slope2;

    //Calculates the y-intecept for each bisector
    float y_intercept1 = midPoint1.y - (perpendicular_slope1 * midPoint1.x);
    float y_intercept2 = midPoint2.y - (perpendicular_slope2 * midPoint2.x);

    // Calculates the circumcenter coordinates by solving the intersection point of 2 lines
    float circumcenterX = (y_intercept2 - y_intercept1) / (perpendicular_slope1 - perpendicular_slope2);
    float circumcenterY = (perpendicular_slope1 * circumcenterX) + y_intercept1;

    // Calculates radius of circumcircle    
    float radius = sqrt(pow(v1.x-circumcenterX,2)+pow(v1.y-circumcenterY,2));

    return {vec2(circumcenterX,circumcenterY), radius};
}

Room generateBasicRoom(int x, int y) {
    Room room;
    room.id = room_id++; // Increments room id after assignment
    room.x = x;
    room.y = y;
    
    std::vector<std::vector<int>> grid = {
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
        {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
        {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
        {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
        {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
        {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
        {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
        {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
        {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
        {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
    };
    room.grid = grid;
    return room;
}

void addRoomToMap(const Room& room, std::vector<std::vector<int>>& map) {
    int room_height = room.grid.size();
    for(int row = room.y; row < room_height+ room.y; row++) {
        int room_width = room.grid[row-room.y].size();
        for(int col = room.x; col < room_width + room.x; col++) {
            // int max = room.grid[row-room.y].size() + room.x;
            // int shiftedY = row-room.y;
            // int shiftedX = col-room.x;
            // int roomVal = room.grid[row-room.y][col-room.x];

            map[row][col] = room.grid[row-room.y][col-room.x];
        }
    }
}

void addHallwayBetweenRoom(const Room& room1, const Room& room2, std::vector<std::vector<int>>& map) {
    int room1_half_height = room1.grid.size() >> 1;
    int room1_half_width =  room1.grid[0].size() >> 1;

    int room2_half_height = room2.grid.size() >> 1;
    int room2_half_width =  room2.grid[0].size() >> 1;

    vec2 midpoint1 = vec2(room2_half_width + room1.x, room1_half_height + room1.y);
    vec2 midpoint2 = vec2(room2_half_width + room2.x, room2_half_height + room2.y);

    if(room1.x == room2.x) { // 2 Rooms on top of each other
        int minY = (min(room1.y, room2.y) == room1.y) ? midpoint1.y+room1_half_height : midpoint2.y+room2_half_height;
        int maxY = (max(room1.y, room2.y) == room1.y) ? midpoint1.y-room1_half_height : midpoint2.y-room2_half_height;

        for(int y = minY; y <= maxY; ++y) {
            map[y][room1.x+room1_half_width] = 1; // Set floor tile
            map[y][room1.x+room1_half_width+1] = 2; // Set hallway right wall tile
            map[y][room1.x+room1_half_width-1] = 2; // Set hallway left wall tile
        }
    }
    else if(room1.y == room2.y) { // 2 Rooms on horizontal of each other
        int minX = (min(room1.x, room2.x) == room1.x) ? midpoint1.x+room1_half_width : midpoint2.x+room2_half_width;
        int maxX = (max(room1.x, room2.x) == room1.x) ? midpoint1.x-room1_half_width : midpoint2.x-room2_half_width;

        for(int x = minX; x <= maxX; ++x) {
            map[room1.y+room1_half_height][x] = 1; // Set floor tile
            map[room1.y+room1_half_height+1][x] = 2; // Set hallway right wall tile
            map[room1.y+room1_half_height-1][x] = 2; // Set hallway left wall tile
        }
    }
}

void MapSystem::addTile(int x, int y, std::vector<TEXTURE_ASSET_ID>& textureIDs, std::vector<std::vector<int>>& map) {

    // Sets the center of the map to 0,0
    // Comment out to center on top left
    // int xPos = (x - (map[0].size() >> 1)) * world_tile_size;
    // int yPos = (y - (map.size() >> 1)) * world_tile_size;
    int xPos = x;
    int yPos = y;
    switch (map[y][x])
    {
    case (int)TILE_TYPE::WALL:
        // textureIDs.push_back(TEXTURE_ASSET_ID::WALL_EDGE);
        // createPhysTile(renderer, { xPos,yPos }, textureIDs);
        break;
    case (int)TILE_TYPE::FLOOR:
        // textureIDs.push_back(TEXTURE_ASSET_ID::TILE_1);
        // createDecoTile(renderer, { xPos,yPos }, textureIDs);
        break;
    default:
        break;
    }
}

void MapSystem::generateAllEntityTiles(std::vector<std::vector<int>>& map) {
    for(int row = 0; row < map.size(); row++) {
        for( int col = 0; col < map[row].size(); col++) {
			std::vector<TEXTURE_ASSET_ID> textureIDs;
            MapSystem::addTile(row, col, textureIDs, map);
        }
    }
}