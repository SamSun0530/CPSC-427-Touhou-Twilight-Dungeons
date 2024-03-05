#include "ai_system.hpp"

// checks if (x,y) on the map is valid, this is not screen coordinates
bool is_valid_cell(int x, int y) {
	return !(y < 0 || x < 0 || y >= world_height || x >= world_width
		|| WorldSystem::world_map[y][x] == (int)TILE_TYPE::WALL
		|| WorldSystem::world_map[y][x] == (int)TILE_TYPE::EMPTY);
}

// checks if entity has a line of sight of the player
bool canSeePlayer(Entity& entity) {
	int center_x = world_width >> 1;
	int center_y = world_height >> 1;
	// assume we have a player
	Entity& player_entity = registry.players.entities[0];
	Motion& player_motion = registry.motions.get(player_entity);
	Motion& entity_motion = registry.motions.get(entity);
	int e_y = round((entity_motion.position.y / world_tile_size) + center_y);
	int e_x = round((entity_motion.position.x / world_tile_size) + center_x);
	int p_y = round((player_motion.position.y / world_tile_size) + center_y);
	int p_x = round((player_motion.position.x / world_tile_size) + center_x);
	// Bresenham's line Credit: https://www.codeproject.com/Articles/15604/Ray-casting-in-a-2D-tile-based-environment
	// TODO: Optimize with better algorithm: raycast, others (?)
	// Bug: Enemy sometimes shoot even if player still behind a wall
	void (*swap)(int&, int&) = [](int& a, int& b) {
		int temp = a;
		a = b;
		b = temp;
		};
	bool steep = abs(p_y - e_y) > abs(p_x - e_x);
	if (steep) {
		swap(p_y, p_x);
		swap(e_y, e_x);
	}
	if (e_x > p_x) {
		swap(e_x, p_x);
		swap(e_y, p_y);
	}
	int deltax = p_x - e_x;
	int deltay = abs(p_y - e_y);
	int error = 0;
	int ystep;
	int y = e_y;
	if (e_y < p_y) ystep = 1; else ystep = -1;
	for (int x = e_x; x <= p_x; x++) {
		int x_pos;
		int y_pos;
		int x_grid;
		int y_grid;
		if (steep) {
			x_pos = (y - center_x) * world_tile_size;
			y_pos = (x - center_y) * world_tile_size;
			x_grid = y;
			y_grid = x;
		}
		else {
			x_pos = (x - center_x) * world_tile_size;
			y_pos = (y - center_y) * world_tile_size;
			x_grid = x;
			y_grid = y;
		}
		if (!is_valid_cell(x_grid, y_grid)) return false;
		// debugging
		//createLine({ x_pos, y_pos }, vec2(world_tile_size / 2, world_tile_size / 2));
		error += deltay;
		if (2 * error >= deltax) {
			y += ystep;
			error -= deltax;
		}
	}
	return true;
}

// heuristic for astar
float astar_heuristic(coord n, coord goal) {
	vec2 dp = goal - n;
	return sqrt(dot(dp, dp));
}

// get all possible actions and their costs (1 for direct, 1.4 for diagonal since sqrt(2)=1.4)
// Bug (potential): entities should not search diagonal when there are walls there
// Consider the scenario where a wall is to the left and below the entity, the entity should not check through the cracks
// This can be fixed if we seal the cracks
std::vector<std::pair<float, coord>> astar_actioncosts() {
	return (std::vector<std::pair<float, coord>>{
		std::make_pair(1.f, vec2(0, -1)), // UP
			std::make_pair(1.f, vec2(0, 1)), // DOWN
			std::make_pair(1.f, vec2(-1, 0)), // LEFT
			std::make_pair(1.f, vec2(1, 0)), // RIGHT
			std::make_pair(1.4f, vec2(-1, -1)), // UP LEFT
			std::make_pair(1.4f, vec2(1, -1)), // UP RIGHT
			std::make_pair(1.4f, vec2(-1, 1)), // DOWN LEFT
			std::make_pair(1.4f, vec2(1, 1)), // DOWN RIGHT
	});
}

// backtrack and reconstruct the path stored in came_from
path reconstruct_path(std::unordered_map<coord, coord>& came_from, coord& current, coord& start) {
	path optimal_path;
	while (current != start) {
		optimal_path.push_back(current);
		current = came_from[current];
	}
	std::reverse(optimal_path.begin(), optimal_path.end());
	return optimal_path;
}

// comparator for sorting min heap
struct CompareGreater
{
	bool operator()(const std::pair<float, coord>& l, const std::pair<float, coord>& r) const { return l.first > r.first; }
};

// set default value of infinity
struct GScore
{
	float score = std::numeric_limits<float>::max();
	operator float() { return score; }
	void operator=(float x) { score = x; }
};

// A-star path finding algorithm (does not store paths inside frontier)
// Adapted from: https://en.wikipedia.org/wiki/A*_search_algorithm
// Note: parameter coords are in grid coordinates NOT screen coordinates
// e.g. input (x,y) should be (1,1) on the grid instead of (550.53, -102.33)
path astar(coord start, coord goal) {
	std::priority_queue<std::pair<float, coord>, std::vector<std::pair<float, coord>>, CompareGreater> open_list;
	std::unordered_set<coord> close_list; // visited set
	std::unordered_map<coord, coord> came_from;
	std::unordered_map<coord, GScore> g_score;

	g_score[start] = 0.f;
	open_list.push(std::make_pair(astar_heuristic(start, goal), start));

	while (!open_list.empty()) {
		std::pair<float, coord> current = open_list.top();
		if (current.second == goal) return reconstruct_path(came_from, current.second, start);
		open_list.pop();
		close_list.insert(current.second);
		for (std::pair<float, coord> actioncost : astar_actioncosts()) {
			vec2 candidate = current.second + actioncost.second;
			// bug: we check if goal/player is in a wall as we support wall collision boxes -> results in enemy being stuck when in wall
			if (close_list.count(candidate) || (is_valid_cell(goal.x, goal.y) && !is_valid_cell(candidate.x, candidate.y))) continue;
			// f_value = total_cost + heuristic, therefore total_cost = f_value - heuristic
			float candidate_g = current.first - astar_heuristic(current.second, goal) + actioncost.first;
			if (candidate_g < g_score[candidate]) {
				came_from[candidate] = current.second;
				g_score[candidate] = candidate_g;
				open_list.push(std::make_pair(candidate_g + astar_heuristic(candidate, goal), candidate));
			}
		}
	}
	return path();
}

void set_follow_path(Entity& entity, Motion& from, Motion& to) {
	// convert to grid coords
	int center_x = world_width >> 1;
	int center_y = world_height >> 1;
	vec2 grid_from = { round((from.position.x / world_tile_size) + center_x),
					round((from.position.y / world_tile_size) + center_y) };
	vec2 grid_to = { round((to.position.x / world_tile_size) + center_x),
					round((to.position.y / world_tile_size) + center_y) };
	path path_to_player = astar(grid_from, grid_to);
	if (path_to_player.size() == 0) return;
	if (!registry.followpaths.has(entity)) {
		registry.followpaths.emplace(entity);
	}
	FollowPath& fp = registry.followpaths.get(entity);
	fp.path = path_to_player;
	fp.next_path_index = 0;
}

void AISystem::init() {
	// A list of function pointers for conditionals and actions
	// checks if entity is within range of player
	bool (*isInRange)(Entity & entity) = [](Entity& entity) {
		float minimum_range_to_check = 360000; // sqrt(360000)=600 pixels
		Motion& motion = registry.motions.get(entity);
		// asusme there is only one player
		for (Entity& player_entity : registry.players.entities) {
			Motion& player_motion = registry.motions.get(player_entity);
			vec2 dp = player_motion.position - motion.position;
			if (dot(dp, dp) < minimum_range_to_check) return true;
		}
		registry.followpaths.remove(entity);
		return false;
		};
	// checks if entity can shoot, throws error if entity does not have bulletfirerate component
	bool (*canShoot)(Entity & entity) = [](Entity& entity) {
		float current_time = glfwGetTime();
		BulletFireRate& fireRate = registry.bulletFireRates.get(entity);
		return current_time - fireRate.last_time >= fireRate.fire_rate;
		};
	// do nothing
	void (*doNothing)(Entity & entity) = [](Entity& entity) {};
	// handles random idle movement, if entity do not have idlemoveactions -> do nothing
	void (*moveRandomDirection)(Entity & entity) = [](Entity& entity) {
		registry.bulletFireRates.get(entity).is_firing = false; // stop firing
		if (!registry.idleMoveActions.has(entity)) return;
		std::random_device ran;
		std::mt19937 gen(ran());
		std::uniform_real_distribution<> dis(0.0, 1.0);
		IdleMoveAction& action = registry.idleMoveActions.get(entity);
		if (action.timer_ms <= 0) {
			Kinematic& kinematic = registry.kinematics.get(entity);
			switch (action.state) {
			case State::IDLE:
				action.state = State::MOVE;
				action.timer_ms = action.moving_ms;
				kinematic.direction = { dis(gen), dis(gen) };
				break;
			case State::MOVE:
				action.state = State::IDLE;
				action.timer_ms = action.idle_ms;
				kinematic.direction = { 0, 0 };
				break;
			default:
				break;
			}
		}
		};
	// stops entity from firing, throws error if entity does not have bulletfirerate component
	void (*stopFiring)(Entity & entity) = [](Entity& entity) {
		registry.bulletFireRates.get(entity).is_firing = false;
		};
	// make entity fire at player and stop motion
	void (*fireAtPlayer)(Entity & entity) = [](Entity& entity) {
		registry.bulletFireRates.get(entity).is_firing = true;
		registry.kinematics.get(entity).direction = { 0, 0 };
		registry.followpaths.remove(entity);
		};
	// find player with a star and sets followpath component
	void (*findPlayer)(Entity & entity) = [](Entity& entity) {
		if (registry.bulletFireRates.has(entity)) registry.bulletFireRates.get(entity).is_firing = false;
		Entity& player = registry.players.entities[0];
		Motion& player_motion = registry.motions.get(player);
		Motion& entity_motion = registry.motions.get(entity);
		set_follow_path(entity, entity_motion, player_motion);
		};
	// find player with a star only if outside of range, otherwise stop motion
	void (*findPlayerThreshold)(Entity & entity) = [](Entity& entity) {
		// does not find player if within a threshold
		Entity& player = registry.players.entities[0];
		Motion& player_motion = registry.motions.get(player);
		Motion& entity_motion = registry.motions.get(entity);
		float minimum_range_to_check = 90000; // sqrt(160000)=300 pixels
		vec2 dp = player_motion.position - entity_motion.position;
		if (dot(dp, dp) > minimum_range_to_check) {
			set_follow_path(entity, entity_motion, player_motion);
		}
		else {
			registry.kinematics.get(entity).direction = { 0, 0 };
			registry.followpaths.remove(entity);
		}
		};

	ConditionalNode* can_see_player_bee = new ConditionalNode(canSeePlayer);
	ConditionalNode* is_in_range_bee = new ConditionalNode(isInRange);
	ConditionalNode* can_shoot_bee = new ConditionalNode(canShoot);

	ActionNode* move_random_direction_bee = new ActionNode(moveRandomDirection);
	ActionNode* stop_firing_bee = new ActionNode(stopFiring);
	ActionNode* fire_at_player_bee = new ActionNode(fireAtPlayer);
	ActionNode* find_player_bee = new ActionNode(findPlayer);
	ActionNode* find_player_threshold_bee = new ActionNode(findPlayerThreshold);

	can_shoot_bee->setTrue(fire_at_player_bee);
	can_shoot_bee->setFalse(find_player_threshold_bee);

	can_see_player_bee->setTrue(can_shoot_bee);
	can_see_player_bee->setFalse(find_player_bee);

	is_in_range_bee->setTrue(can_see_player_bee);
	is_in_range_bee->setFalse(move_random_direction_bee);

	/*
	Bee enemy decision tree
	COND in range global?
		F -> move random idle
		T -> can see player?
		COND can see player?
			F -> find player with a star
			T -> can shoot?
			COND can shoot?
				F -> move closer and stop at distance
				T -> stop and shoot
	*/
	this->bee_tree.setRoot(is_in_range_bee);

	/*
	Bomber enemy decision tree
	COND in range global?
		F -> move random idle
		T -> find player with a star
	*/
	ActionNode* move_random_direction_bomber = new ActionNode(moveRandomDirection);
	ActionNode* find_player_bomber = new ActionNode(findPlayer);
	ConditionalNode* is_in_range_bomber = new ConditionalNode(find_player_bomber, move_random_direction_bomber, isInRange);
	this->bomber_tree.setRoot(is_in_range_bomber);

	// TODO: create decision trees/condition/action functions here for different enemies
}