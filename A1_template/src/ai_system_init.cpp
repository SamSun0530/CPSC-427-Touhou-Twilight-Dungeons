#include "ai_system.hpp"
#include "world_system.hpp"

// checks if (x,y) on the map grid is valid, this is not world coordinates
bool is_valid_cell(int x, int y) {
	return !(y < 0 || x < 0 || y >= world_height || x >= world_width
		|| world_map[y][x] == (int)TILE_TYPE::WALL
		|| world_map[y][x] == (int)TILE_TYPE::EMPTY
		|| world_map[y][x] == (int)TILE_TYPE::DOOR);
}

// checks if entity has a line of sight of the player
bool canSeePlayer(Entity& entity) {
	// assume we have a player
	Entity& player_entity = registry.players.entities[0];
	Motion& player_motion = registry.motions.get(player_entity);
	Motion& entity_motion = registry.motions.get(entity);
	ivec2 e = convert_world_to_grid(entity_motion.position);
	ivec2 p = convert_world_to_grid(player_motion.position);
	// Bresenham's line Credit: https://www.codeproject.com/Articles/15604/Ray-casting-in-a-2D-tile-based-environment
	// TODO: Optimize with better algorithm: raycast, others (?)
	// Bug: Enemy sometimes shoot even if player still behind a wall
	void (*swap)(int&, int&) = [](int& a, int& b) {
		int temp = a;
		a = b;
		b = temp;
		};
	bool steep = abs(p.y - e.y) > abs(p.x - e.x);
	if (steep) {
		swap(p.y, p.x);
		swap(e.y, e.x);
	}
	if (e.x > p.x) {
		swap(e.x, p.x);
		swap(e.y, p.y);
	}
	int deltax = p.x - e.x;
	int deltay = abs(p.y - e.y);
	int error = 0;
	int ystep;
	int y = e.y;
	if (e.y < p.y) ystep = 1; else ystep = -1;
	for (int x = e.x; x <= p.x; x++) {
		//int x_pos;
		//int y_pos;
		int x_grid;
		int y_grid;
		if (steep) {
			//x_pos = (y - world_center.x) * world_tile_size;
			//y_pos = (x - world_center.y) * world_tile_size;
			x_grid = y;
			y_grid = x;
		}
		else {
			//x_pos = (x - world_center.x) * world_tile_size;
			//y_pos = (y - world_center.y) * world_tile_size;
			x_grid = x;
			y_grid = y;
		}
		if (!is_valid_cell(x_grid, y_grid)) return false;
		// Debug line to visualize line of sight path
		//createLine({ x_pos, y_pos }, vec2(world_tile_size / 2));
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
	// Euclidean - 8 direction, diagonal different cost
	//vec2 dp = goal - n;
	//return sqrt(dot(dp, dp));

	// Manhattan - 4 direction
	return abs(goal.x - n.x) + abs(goal.y - n.y);

	// Chebyshev - 8 direction, same costs
	//return max(abs(goal.y - n.y), abs(goal.x - n.x));
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
			//std::make_pair(1.4f, vec2(-1, -1)), // UP LEFT
			//std::make_pair(1.4f, vec2(1, -1)), // UP RIGHT
			//std::make_pair(1.4f, vec2(-1, 1)), // DOWN LEFT
			//std::make_pair(1.4f, vec2(1, 1)), // DOWN RIGHT
	});
}

// backtrack and reconstruct the path stored in came_from
path reconstruct_path(std::unordered_map<coord, coord>& came_from, coord& current, coord& start) {
	path optimal_path;
	while (current != start) {
		optimal_path.push_back(current);
		current = came_from[current];
	}

	// Debug line by visualizing the path
	//for (int i = 0; i < optimal_path.size(); i++) {
	//	createLine(convert_grid_to_world(optimal_path[i]), vec2(world_tile_size / 2));
	//}

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
// Note: parameter coords are in grid coordinates NOT world coordinates
// e.g. input (x,y) should be (1,1) on the grid instead of (550.53, -102.33)
path astar(coord start, coord goal) {
	// unlikely to find path between floats, so round
	start = round(start);
	goal = round(goal);
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
		for (std::pair<float, coord>& actioncost : astar_actioncosts()) {
			vec2 candidate = current.second + actioncost.second;
			if (close_list.count(candidate) || (!is_valid_cell(candidate.x, candidate.y))) continue;
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

// Takes in from & to in world coordinates
void set_follow_path(Entity& entity, coord from, coord to) {
	path path_to_player = astar(convert_world_to_grid(from), convert_world_to_grid(to));
	if (path_to_player.size() == 0) return;
	if (!registry.followpaths.has(entity)) {
		registry.followpaths.emplace(entity);
	}
	FollowPath& fp = registry.followpaths.get(entity);
	fp.path = path_to_player;
	fp.next_path_index = 0;
}

void AISystem::init() {
	// Initialize flow field
	restart_flow_field_map();

	// A list of function pointers for conditionals and actions
	// checks if entity is within range of player
	bool (*isInRangeRemoveFollow)(Entity & entity) = [](Entity& entity) {
		float minimum_range_to_check = 360000; // sqrt(360000)=600 pixels
		Motion& motion = registry.motions.get(entity);
		// asume there is only one player
		for (Entity& player_entity : registry.players.entities) {
			Motion& player_motion = registry.motions.get(player_entity);
			vec2 dp = player_motion.position - motion.position;
			if (dot(dp, dp) < minimum_range_to_check) return true;
		}
		registry.followpaths.remove(entity);
		registry.followFlowField.remove(entity);
		return false;
		};
	bool (*isInRange)(Entity & entity) = [](Entity& entity) {
		float minimum_range_to_check = 850000; // sqrt(minimum_range_to_check) = x, where x = # of pixels
		Motion& motion = registry.motions.get(entity);
		// asume there is only one player
		for (Entity& player_entity : registry.players.entities) {
			Motion& player_motion = registry.motions.get(player_entity);
			vec2 dp = player_motion.position - motion.position;
			if (dot(dp, dp) < minimum_range_to_check) return true;
		}
		return false;
		};
	// checks if entity can shoot, throws error if entity does not have bullet spawner component
	bool (*canShoot)(Entity & entity) = [](Entity& entity) {
		float current_time = glfwGetTime();
		BulletSpawner& bullet_spawner = registry.bulletSpawners.get(entity);
		return current_time - bullet_spawner.last_fire_time >= bullet_spawner.fire_rate;
		};
	// do nothing
	void (*doNothing)(Entity & entity) = [](Entity& entity) {};
	// handles random idle movement, if entity do not have idlemoveactions -> do nothing
	void (*moveRandomDirection)(Entity & entity) = [](Entity& entity) {
		if (registry.bulletSpawners.has(entity)) registry.bulletSpawners.get(entity).is_firing = false; // stop firing
		if (!registry.idleMoveActions.has(entity)) return;
		std::random_device ran;
		std::mt19937 gen(ran());
		std::uniform_real_distribution<> dis(-1.0, 1.0);
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
	// stops entity from firing, throws error if entity does not have bullet spawner component
	void (*stopFiring)(Entity & entity) = [](Entity& entity) {
		registry.bulletSpawners.get(entity).is_firing = false;
		};
	// make entity fire at player and stop motion
	void (*fireAtPlayer)(Entity & entity) = [](Entity& entity) {
		registry.bulletSpawners.get(entity).is_firing = true;
		registry.kinematics.get(entity).direction = { 0, 0 };
		registry.followpaths.remove(entity);
		registry.followFlowField.remove(entity);
		};
	// find player with a star and sets followpath component
	void (*findPlayerAStar)(Entity & entity) = [](Entity& entity) {
		if (registry.bulletSpawners.has(entity)) registry.bulletSpawners.get(entity).is_firing = false;
		Entity& player = registry.players.entities[0];
		Motion& player_motion = registry.motions.get(player);
		Motion& entity_motion = registry.motions.get(entity);
		Collidable& player_collidable = registry.collidables.get(player);
		Collidable& entity_collidable = registry.collidables.get(entity);
		set_follow_path(entity, entity_motion.position + entity_collidable.shift, player_motion.position + player_collidable.shift);
		};
	// find player with a star only if outside of range, otherwise stop motion
	void (*findPlayerThresholdAStar)(Entity & entity) = [](Entity& entity) {
		// does not find player if within a threshold
		Entity& player = registry.players.entities[0];
		Motion& player_motion = registry.motions.get(player);
		Motion& entity_motion = registry.motions.get(entity);
		float minimum_range_to_check = 90000; // sqrt(160000)=300 pixels
		vec2 dp = player_motion.position - entity_motion.position;
		if (dot(dp, dp) > minimum_range_to_check) {
			Collidable& player_collidable = registry.collidables.get(player);
			Collidable& entity_collidable = registry.collidables.get(entity);
			set_follow_path(entity, entity_motion.position + entity_collidable.shift, player_motion.position + player_collidable.shift);
		}
		else {
			registry.kinematics.get(entity).direction = { 0, 0 };
			registry.followpaths.remove(entity);
		}
		};
	// find player by following flow field
	void (*followFlowField)(Entity & entity) = [](Entity& entity) {
		if (registry.bulletSpawners.has(entity)) registry.bulletSpawners.get(entity).is_firing = false;
		if (!registry.followFlowField.has(entity)) {
			registry.followFlowField.emplace(entity);
		}
		};
	// find player by following flow field if outside of range threshold
	void (*followFlowFieldThreshold)(Entity & entity) = [](Entity& entity) {
		if (registry.bulletSpawners.has(entity)) registry.bulletSpawners.get(entity).is_firing = false;
		Entity& player = registry.players.entities[0];
		Motion& player_motion = registry.motions.get(player);
		Motion& entity_motion = registry.motions.get(entity);
		float minimum_range_to_check = 90000; // sqrt(160000)=300 pixels
		vec2 dp = player_motion.position - entity_motion.position;
		if (dot(dp, dp) > minimum_range_to_check) {
			if (!registry.followFlowField.has(entity)) {
				registry.followFlowField.emplace(entity);
			}
		}
		else {
			registry.kinematics.get(entity).direction = { 0, 0 };
			registry.followFlowField.remove(entity);
		}
		};
	// show/hide boss info based on boss & player distance:
	// - boss is active -> process phase changes
	// - bullet spawner -> firing
	// - boss health bar ui
	void (*showBossInfo)(Entity & entity) = [](Entity& entity) {
		if (!registry.bosses.has(entity)) return;
		Boss& boss = registry.bosses.get(entity);
		boss.is_active = true;
		BossHealthBarLink& link = registry.bossHealthBarLink.get(entity);
		BossHealthBarUI& ui = registry.bossHealthBarUIs.get(link.other);
		if (registry.bulletSpawners.has(entity)) {
			BulletSpawner& bs = registry.bulletSpawners.get(entity);
			bs.is_firing = true;
		}
		if (!ui.is_visible) ui.is_visible = true;
		};
	void (*hideBossInfo)(Entity & entity) = [](Entity& entity) {
		if (!registry.bosses.has(entity)) return;
		Boss& boss = registry.bosses.get(entity);
		boss.is_active = false;
		BossHealthBarLink& link = registry.bossHealthBarLink.get(entity);
		BossHealthBarUI& ui = registry.bossHealthBarUIs.get(link.other);
		if (registry.bulletSpawners.has(entity)) {
			BulletSpawner& bs = registry.bulletSpawners.get(entity);
			bs.is_firing = false;
		}
		if (ui.is_visible) ui.is_visible = false;
		};

	ConditionalNode* can_see_player_bee = new ConditionalNode(canSeePlayer);
	ConditionalNode* is_in_range_bee = new ConditionalNode(isInRangeRemoveFollow);
	ConditionalNode* can_shoot_bee = new ConditionalNode(canShoot);

	ActionNode* move_random_direction_bee = new ActionNode(moveRandomDirection);
	ActionNode* fire_at_player_bee = new ActionNode(fireAtPlayer);
	//ActionNode* find_player_bee = new ActionNode(findPlayerAStar);
	ActionNode* follow_flow_field_threshold_bee = new ActionNode(followFlowFieldThreshold);

	can_shoot_bee->setTrue(fire_at_player_bee);
	can_shoot_bee->setFalse(follow_flow_field_threshold_bee);

	can_see_player_bee->setTrue(can_shoot_bee);
	can_see_player_bee->setFalse(follow_flow_field_threshold_bee);

	is_in_range_bee->setTrue(can_see_player_bee);
	is_in_range_bee->setFalse(move_random_direction_bee);

	// Fixes memory leak issue where wolf points to bee's allocated nodes
	// When deleting nodes in destructor, bee deletes first, then wolf tries to delete
	// the same node, resulting in error
	// IMPORTANT: for now it's a quick fix, do not reuse nodes
	// TODO: use shared_ptr instead
	ConditionalNode* can_see_player_wolf = new ConditionalNode(canSeePlayer);
	ConditionalNode* is_in_range_wolf = new ConditionalNode(isInRange);
	ConditionalNode* can_shoot_wolf = new ConditionalNode(canShoot);

	ActionNode* move_random_direction_wolf = new ActionNode(moveRandomDirection);
	ActionNode* fire_at_player_wolf = new ActionNode(fireAtPlayer);
	//ActionNode* find_player_wolf = new ActionNode(findPlayerThresholdAStar);
	ActionNode* find_player_threshold_wolf = new ActionNode(followFlowFieldThreshold);

	can_shoot_wolf->setTrue(fire_at_player_wolf);
	can_shoot_wolf->setFalse(find_player_threshold_wolf);

	can_see_player_wolf->setTrue(can_shoot_wolf);
	can_see_player_wolf->setFalse(find_player_threshold_wolf);

	is_in_range_wolf->setTrue(can_see_player_wolf);
	is_in_range_wolf->setFalse(move_random_direction_wolf);

	/*
	Wolf has same tree (temporary)
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
	this->wolf_tree.setRoot(is_in_range_wolf);

	/*
	Bomber enemy decision tree
	COND in range global?
		F -> move random idle
		T -> find player with a star
	*/
	ActionNode* move_random_direction_bomber = new ActionNode(moveRandomDirection);
	//ActionNode* find_player_bomber = new ActionNode(findPlayerAStar);
	ActionNode* follow_flow_field_bomber = new ActionNode(followFlowField);
	ConditionalNode* is_in_range_bomber = new ConditionalNode(follow_flow_field_bomber, move_random_direction_bomber, isInRangeRemoveFollow);
	this->bomber_tree.setRoot(is_in_range_bomber);

	/*
	Cirno boss decision tree
	COND in range global?
		F -> hide boss health bar
		T -> show boss health bar
	*/
	ActionNode* show_boss_health_bar_cirno = new ActionNode(showBossInfo);
	ActionNode* hide_boss_health_bar_cirno = new ActionNode(hideBossInfo);
	ConditionalNode* is_in_range_cirno = new ConditionalNode(show_boss_health_bar_cirno, hide_boss_health_bar_cirno, isInRange);
	this->cirno_boss_tree.setRoot(is_in_range_cirno);

	// TODO: create decision trees/condition/action functions here for different enemies
}

int AISystem::get_flow_field_value(vec2 grid_pos) {
	return flow_field_map[grid_pos.y][grid_pos.x];
}

void AISystem::restart_flow_field_map()
{
	flow_field_map = std::vector<std::vector<int>>(world_height, std::vector<int>(world_width, -1));
	//update_flow_field_map();
	//for (int i = 0; i < flow_field_map.size(); i++) {
	//	for (int j = 0; j < flow_field_map.size(); j++) {
	//		printf("%d ", flow_field_map[i][j]);
	//	}
	//	printf("\n");
	//}
}

void AISystem::update_flow_field_map()
{
	Entity& player = registry.players.entities[0];
	coord start = convert_world_to_grid(registry.motions.get(player).position);
	if (!is_valid_cell(start.x, start.y)) return;

	std::queue<std::pair<coord, int>> open_list;
	std::unordered_set<coord> close_list;

	open_list.push(std::make_pair(start, 0));

	while (!open_list.empty()) {
		auto current = open_list.front();
		open_list.pop();
		close_list.insert(current.first);
		flow_field_map[current.first.y][current.first.x] = current.second;
		if (current.second + 1 <= flow_field.max_length) {
			for (const coord& action : ACTIONS) {
				coord candidate = current.first + action;
				if (close_list.count(candidate) || (!is_valid_cell(candidate.x, candidate.y))) continue;
				close_list.insert(candidate);
				open_list.push(std::make_pair(candidate, current.second + 1));
			}
		}
	}
}