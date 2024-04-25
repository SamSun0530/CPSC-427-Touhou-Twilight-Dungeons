#include "world_init.hpp"
#include <iostream>

//int playerBullet_damage_playerBullet_damage = 10;

Entity createBullet(RenderSystem* renderer, float entity_speed, vec2 entity_position, float rotation_angle, vec2 direction, float bullet_speed, bool is_player_bullet, BulletPattern* bullet_pattern, int damageBoost)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = rotation_angle;
	motion.position = entity_position; // bullet spawns from entity's center position
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ BULLET_BB_WIDTH, BULLET_BB_HEIGHT });

	auto& kinematic = registry.kinematics.emplace(entity);
	kinematic.speed_base = bullet_speed;
	kinematic.speed_modified = 1.f * kinematic.speed_base + entity_speed; // bullet speed takes into account of entity's speed
	kinematic.direction = direction;

	// Set the collision box
	auto& collidable = registry.collidables.emplace(entity);
	collidable.size = abs(motion.scale / 2.f);
	// Create and (empty) bullet component to be able to refer to all bullets
	if (is_player_bullet) {
		auto& playerBullet = registry.playerBullets.emplace(entity);

		playerBullet.damage = registry.players.components[0].bullet_damage;

		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::BULLET,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE });
	}
	else {
		registry.enemyBullets.emplace(entity);
	}

	if (bullet_pattern) {
		registry.bulletPatterns.insert(entity, *bullet_pattern);
	}

	return entity;
}

Entity createBulletDisappear(RenderSystem* renderer, vec2 entity_position, float rotation_angle, bool is_player_bullet)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = rotation_angle;
	motion.position = entity_position; // bullet spawns from entity's center position
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ BULLET_BB_WIDTH, BULLET_BB_HEIGHT });

	auto& kinematic = registry.kinematics.emplace(entity);

	// Set the collision box
	//auto& collidable = registry.collidables.emplace(entity);
	//collidable.size = abs(motion.scale);

	// Create and (empty) bullet component to be able to refer to all bullets
	if (is_player_bullet) {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::REIMU_BULLET_DISAPPEAR,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE });
	}
	else {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::ENEMY_BULLET,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE });
	}
	EntityAnimation ani;
	ani.isCursor = false;
	ani.offset = 0;
	ani.frame_rate_ms = 50;
	ani.full_rate_ms = 50;
	ani.spritesheet_scale = { 0.25, 1 };
	ani.render_pos = { 0.25, 1 };
	registry.alwaysplayAni.insert(entity, ani);
	return entity;
}

Entity createHealth(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.scale = vec2({ HEALTH_WIDTH, HEALTH_HEIGHT });
	registry.kinematics.emplace(entity);
	auto& collidable = registry.collidables.emplace(entity);
	collidable.size = abs(motion.scale);
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> distrib(0, 1);
	double number = distrib(gen);
	double number_y = distrib(gen) / 2;
	auto& pickupable = registry.pickupables.emplace(entity);
	registry.colors.insert(entity, { 1,1,1 });
	if (number <= 0.6) {
		pickupable.health_change = 1;
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::HEALTH_1, // TEXTURE_COUNT indicates that no txture is needed
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE });

	}
	else if (number <= 0.9) {
		pickupable.health_change = 2;
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::HEALTH_2, // TEXTURE_COUNT indicates that no txture is needed
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE });
	}
	else {
		pickupable.health_change = 100;
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::REGENERATE_HEALTH, // TEXTURE_COUNT indicates that no txture is needed
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE });
	}
	vec2 dir = { 60 * (number - 0.5), 50 * (number_y) };
	BezierCurve curve;
	curve.bezier_pts.push_back(position);
	curve.bezier_pts.push_back(position + vec2(0, -20));
	curve.bezier_pts.push_back(position + dir);
	registry.bezierCurves.insert(entity, curve);
	return entity;
}

Entity createPurchasableHealth(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.scale = vec2({ HEALTH_WIDTH, HEALTH_HEIGHT });
	registry.kinematics.emplace(entity);
	auto& collidable = registry.collidables.emplace(entity);
	collidable.size = abs(motion.scale);
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> distrib(0, 1);
	double number = distrib(gen);
	auto& purchasable = registry.purchasableables.emplace(entity);
	registry.colors.insert(entity, { 1,1,1 });
	if (number <= 0.6) {
		purchasable.cost = 6;
		purchasable.effect_strength = 1;
		purchasable.effect_type = 7;
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::HEALTH_1, // TEXTURE_COUNT indicates that no txture is needed
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE });

	}
	else if (number <= 0.9) {
		purchasable.cost = 10;
		purchasable.effect_strength = 2;
		purchasable.effect_type = 7;
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::HEALTH_2, // TEXTURE_COUNT indicates that no txture is needed
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE });
	}
	else {
		purchasable.cost = 20;
		purchasable.effect_strength = 100;
		purchasable.effect_type = 7;
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::REGENERATE_HEALTH, // TEXTURE_COUNT indicates that no txture is needed
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE });
	}
	return entity;
}

// creat purchase able items (boost/buff)
Entity createTreasure(RenderSystem* renderer, vec2 position, bool is_bezier)
{
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.scale = vec2({ ITEM_WIDTH, ITEM_HEIGHT });
	registry.kinematics.emplace(entity);
	auto& collidable = registry.collidables.emplace(entity);
	collidable.size = abs(motion.scale);

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> distrib(0, 1);
	double number = distrib(gen);
	double number_y = distrib(gen) / 2;


	// Generate a random number between 1 and 6 for type
	// 1=bullet_damage, 2=fire_rate, 3=critical_hit, 4=max_health, 5=critical_dmg, 6=bomb

	std::uniform_int_distribution<> typeDistrib(1, 5);
	int type = typeDistrib(gen);

	

	Purchasableable& purchasableable = registry.purchasableables.emplace(entity);
	registry.colors.insert(entity, { 1,1,1 });
	if (number < 0.1) {
		type = 6;
		purchasableable.effect_type = type;
		purchasableable.cost = 10;
		motion.scale = vec2({ 32, 32 });
	}
	else {
		purchasableable.cost = number * 10;
		purchasableable.effect_strength = number * 10;

		if (purchasableable.cost == 0) {
			purchasableable.cost = 1;
			purchasableable.effect_strength = 1;
		}
		purchasableable.cost *= 3;
		purchasableable.effect_type = type;
	}
	
	
	if (type == 1) {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::ITEM_R, // TEXTURE_COUNT indicates that no txture is needed
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE });

	}
	else if (type == 2) {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::ITEM_B, // TEXTURE_COUNT indicates that no txture is needed
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE });
	}
	else if (type == 3) {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::ITEM_G, // TEXTURE_COUNT indicates that no txture is needed
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE });
	}else if (type == 4) {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::ITEM_Y, // TEXTURE_COUNT indicates that no txture is needed
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE });
	}else if (type == 5) {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::ITEM_P, // TEXTURE_COUNT indicates that no txture is needed
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE });
	}else {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::BOMB, // TEXTURE_COUNT indicates that no txture is needed
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE });
	}
	vec2 dir = { 60 * (number - 0.5), 50 * (number_y) };
	if (is_bezier) {
		BezierCurve curve;
		curve.bezier_pts.push_back(position);
		curve.bezier_pts.push_back(position + vec2(0, -20));
		curve.bezier_pts.push_back(position + dir);
		registry.bezierCurves.insert(entity, curve);
	}
	return entity;
}

Entity createCoin(RenderSystem* renderer, vec2 position, int value, float bezier_start, float bezier_end, vec2 bezier_up, float bezier_x_rand)
{
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.position = position;
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ BULLET_BB_WIDTH, BULLET_BB_HEIGHT });

	/*auto& kinematic = registry.kinematics.emplace(entity);
	kinematic.speed_base = 50.f;
	kinematic.speed_modified = 1.f * kinematic.speed_base;
	kinematic.direction = { 0, 1 };*/

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> distrib(bezier_start, bezier_end);
	std::uniform_real_distribution<> x_distrib(0.0, 1.0);

	// Generate a positive number
	double number = distrib(gen);
	double x_number = x_distrib(gen);
	
	// collidable
	auto& collidable = registry.collidables.emplace(entity);
	collidable.size = motion.scale / 2.f;

	// value
	Coin& coin = registry.coins.emplace(entity);
	coin.coin_amount = value;
	vec2 dir = { bezier_x_rand * (x_number - 0.5f), 50 * (number) };
	BezierCurve curve;
	curve.bezier_pts.push_back(position);
	curve.bezier_pts.push_back(position + bezier_up);
	curve.bezier_pts.push_back(position + dir);
	registry.bezierCurves.insert(entity, curve);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::COIN,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });


	EntityAnimation ani;
	ani.isCursor = false;
	ani.offset = 0;
	ani.frame_rate_ms = 200;
	ani.full_rate_ms = 200;
	ani.spritesheet_scale = { 0.2, 1 };
	ani.render_pos = { 0.2, 1 };
	registry.alwaysplayAni.insert(entity, ani);

	return entity;
}

Entity createMaxHPIncrease(RenderSystem* renderer, vec2 position)
{
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.position = position;
	// Setting initial values, scale is negative to make it face the opposite way

	/*need new scale*/
	motion.scale = vec2({ BULLET_BB_WIDTH, BULLET_BB_HEIGHT });

	/*auto& kinematic = registry.kinematics.emplace(entity);
	kinematic.speed_base = 50.f;
	kinematic.speed_modified = 1.f * kinematic.speed_base;
	kinematic.direction = { 0, 1 };*/

	// collidable
	auto& collidable = registry.collidables.emplace(entity);
	collidable.size = motion.scale / 2.f;

	MaxHPIncrease& maxhpIncrease = registry.maxhpIncreases.emplace(entity);
	maxhpIncrease.max_health_increase = 1;

	// value
	Product& price = registry.products.emplace(entity);
	price.price = 60;

	/*need new texture*/
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BULLET,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createAttackUp(RenderSystem* renderer, vec2 position)
{
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.position = position;
	// Setting initial values, scale is negative to make it face the opposite way

	/*need new scale*/
	motion.scale = vec2({ BULLET_BB_WIDTH, BULLET_BB_HEIGHT });

	/*auto& kinematic = registry.kinematics.emplace(entity);
	kinematic.speed_base = 50.f;
	kinematic.speed_modified = 1.f * kinematic.speed_base;
	kinematic.direction = { 0, 1 };*/

	// collidable
	auto& collidable = registry.collidables.emplace(entity);
	collidable.size = motion.scale / 2.f;

	AttackUp& attackUp = registry.attackUps.emplace(entity);
	attackUp.damageUp = 1;

	// value
	Product& price = registry.products.emplace(entity);
	price.price = 40;

	/*need new texture*/
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BULLET,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createChest(RenderSystem* renderer, vec2 position)
{
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.position = position;
	motion.scale = vec2({ world_tile_size, world_tile_size });

	// collidable
	auto& collidable = registry.collidables.emplace(entity);
	collidable.size = vec2(motion.scale.x * 2 / 3.f, motion.scale.y / 2.f);
	Chest& chest = registry.chests.emplace(entity);

	/*need new texture*/
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::CHEST_CLOSE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createKey(RenderSystem* renderer, vec2 position)
{
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.position = position;
	// Setting initial values, scale is negative to make it face the opposite way

	/*need new scale*/
	motion.scale = vec2({ BULLET_BB_WIDTH, BULLET_BB_HEIGHT });

	/*auto& kinematic = registry.kinematics.emplace(entity);
	kinematic.speed_base = 50.f;
	kinematic.speed_modified = 1.f * kinematic.speed_base;
	kinematic.direction = { 0, 1 };*/

	// collidable
	auto& collidable = registry.collidables.emplace(entity);
	collidable.size = motion.scale / 2.f;

	Key& key = registry.keys.emplace(entity);

	/*need new texture*/
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BULLET,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createPlayer(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::REIMU_FRONT);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.scale = vec2({ PLAYER_BB_WIDTH, PLAYER_BB_HEIGHT });

	auto& kinematic = registry.kinematics.emplace(entity);
	kinematic.speed_base = 300.f;
	kinematic.speed_modified = 1.f * kinematic.speed_base;
	kinematic.direction = { 0, 0 };

	// Set the collision box
	auto& collidable = registry.collidables.emplace(entity);
	// Set player collision box at the feet of the player
	collidable.size = { motion.scale.x / 32 * 24, motion.scale.y / 2.f };
	collidable.shift = { 0, motion.scale.y / 4.f };

	// Set the collision circle
	auto& collidable_circle = registry.circleCollidables.emplace(entity);
	collidable_circle.radius = 6.f;
	collidable_circle.shift = { 0, motion.scale.y / 6.f };

	// HP
	HP& hp = registry.hps.emplace(entity);
	hp.max_hp = 6;
	hp.curr_hp = hp.max_hp;

	registry.players.emplace(entity);
	EntityAnimation player_ani;
	player_ani.isCursor = true;
	player_ani.spritesheet_scale = { 0.25, 0.125 };
	player_ani.render_pos = { 0.5, 0.125 };
	registry.animation.insert(entity, player_ani);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLAYER, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::PLAYER,
			GEOMETRY_BUFFER_ID::SPRITE });

	BulletSpawner bs;
	bs.fire_rate = 3;
	bs.is_firing = false;
	bs.bullet_initial_speed = 200;
	bs.initial_bullet_cooldown = 0;

	registry.bulletSpawners.insert(entity, bs);
	registry.colors.insert(entity, { 1,1,1 });

	return entity;
}

Entity createFocusDot(RenderSystem* renderer, vec2 pos, vec2 size)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.scale = size;

	registry.focusdots.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::FOCUS_DOT, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createCombo(RenderSystem* renderer)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = vec2(window_px_half.x, -window_px_half.y) - vec2(150, -150);
	motion.scale = vec2(160, 160);

	registry.UIUX.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::C, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::COMBO,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createWin(RenderSystem* renderer) {
	// Pause menu background
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = vec2(0, 0);
	motion.scale = vec2(1000, 700);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::WINDEATH_SCREEN,
			EFFECT_ASSET_ID::UI,
			GEOMETRY_BUFFER_ID::SPRITE });
	registry.winMenus.emplace(entity);
	registry.winMenus.emplace(createText(vec2(0, -200), vec2(2, 2), "You WIN !!!", vec3(0, 0, 0), true, false));

	return entity;

}

Entity createInfographic(RenderSystem* renderer) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = vec2(0, 0);
	motion.scale = vec2(1100, 690);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::WINDEATH_SCREEN,
			EFFECT_ASSET_ID::UI,
			GEOMETRY_BUFFER_ID::SPRITE });
	registry.infographicsMenus.emplace(entity);

	//Picture
	auto entity2 = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh2 = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity2, &mesh2);

	Motion& motion2 = registry.motions.emplace(entity2);
	motion2.position = vec2(0, -25);
	motion2.scale = vec2(886, 364);

	registry.renderRequests.insert(
		entity2,
		{ TEXTURE_ASSET_ID::INFOGRAPHIC,
			EFFECT_ASSET_ID::UI,
			GEOMETRY_BUFFER_ID::SPRITE });
	registry.infographicsMenus.emplace(entity2);

	return entity;
}

Entity createLose(RenderSystem* renderer) {
	// Pause menu background
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = vec2(0, 0);
	motion.scale = vec2(1000, 700);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::WINDEATH_SCREEN,
			EFFECT_ASSET_ID::UI,
			GEOMETRY_BUFFER_ID::SPRITE });
	registry.loseMenus.emplace(entity);
	registry.loseMenus.emplace(createText(vec2(0, -200), vec2(1.5, 1.5), "Game Over!!!", vec3(0, 0, 0), true, false));

	return entity;

}

void createDialogue(CHARACTER character, std::string sentence, CHARACTER talk_2, EMOTION emotion) {
	auto reimu_entity = Entity();

	// Setting initial motion values
	Motion& motion_reimu = registry.motions.emplace(reimu_entity);
	motion_reimu.position = vec2(-250, 0);
	motion_reimu.angle = 0.f;
	motion_reimu.scale = vec2({ 550.f, 600.f });
	EntityAnimation& ani_reimu = registry.alwaysplayAni.emplace(reimu_entity);
	ani_reimu.spritesheet_scale = { 1 / 6.f, 1 };
	registry.dialogueMenus.emplace(reimu_entity);
	if (character == CHARACTER::REIMU) {
		ani_reimu.render_pos = { 1 / 6.f * (1 + (int)emotion), 1 };
		registry.renderRequests.insert(
			reimu_entity,
			{ TEXTURE_ASSET_ID::REIMU_PORTRAIT, // TEXTURE_COUNT indicates that no txture is needed
				EFFECT_ASSET_ID::UI,
				GEOMETRY_BUFFER_ID::SPRITE });
	}
	else {
		ani_reimu.render_pos = { 1 / 6.f * (1 + (int)EMOTION::NORMAL), 1 };
		registry.renderRequests.insert(
			reimu_entity,
			{ TEXTURE_ASSET_ID::REIMU_PORTRAIT, // TEXTURE_COUNT indicates that no txture is needed
				EFFECT_ASSET_ID::GREY,
				GEOMETRY_BUFFER_ID::SPRITE });
	}

	if (talk_2 != CHARACTER::NONE) {
		auto other_entity = Entity();

		// Setting initial motion values
		Motion& motion_other = registry.motions.emplace(other_entity);
		motion_other.position = vec2(250, 0);
		motion_other.angle = 0.f;
		motion_other.scale = vec2({ 550.f, 600.f });
		EntityAnimation& ani_other = registry.alwaysplayAni.emplace(other_entity);
		ani_other.spritesheet_scale = { 1 / 6.f, 1 };

		registry.dialogueMenus.emplace(other_entity);
		if (character == talk_2) {
			ani_other.render_pos = { 1 / 6.f * (1 + (int)emotion), 1 };
			registry.renderRequests.insert(
				other_entity,
				{ static_cast<TEXTURE_ASSET_ID>((int)TEXTURE_ASSET_ID::REIMU_PORTRAIT + (int)talk_2), // TEXTURE_COUNT indicates that no txture is needed
					EFFECT_ASSET_ID::UI,
					GEOMETRY_BUFFER_ID::SPRITE });
		}
		else {
			ani_other.render_pos = { 1 / 6.f * (1 + (int)EMOTION::NORMAL), 1 };
			registry.renderRequests.insert(
				other_entity,
				{ static_cast<TEXTURE_ASSET_ID>((int)TEXTURE_ASSET_ID::REIMU_PORTRAIT + (int)talk_2), // TEXTURE_COUNT indicates that no txture is needed
					EFFECT_ASSET_ID::GREY,
					GEOMETRY_BUFFER_ID::SPRITE });
		}
	}

	auto dialogue_entity = Entity();
	// Setting initial motion values
	Motion& motion_dialogue = registry.motions.emplace(dialogue_entity);
	motion_dialogue.position = vec2(0, window_px_half.y - 130);
	motion_dialogue.angle = 0.f;
	motion_dialogue.scale = vec2({ 1.6 * 688.f, 1.2 * 224.f });

	registry.dialogueMenus.emplace(dialogue_entity);
	registry.renderRequests.insert(
		dialogue_entity,
		{ TEXTURE_ASSET_ID::DIALOGUE_BOX, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::UI,
			GEOMETRY_BUFFER_ID::SPRITE });

	createText({ 0,window_px_half.y - 170 }, { 0.8,0.8 }, sentence, vec3(0, 0, 0), false, false);
}

Entity createKey(vec2 pos, vec2 size, KEYS key, bool is_on_ui, bool is_active, float frame_rate)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.scale = size;

	EntityAnimation key_ani;
	key_ani.isCursor = false;
	//std::cout << static_cast<int>(key) << std::endl;
	key_ani.spritesheet_scale = { 0.5, 1 / 13.0f };
	key_ani.render_pos = { 0.0, 1 / 13.0f * static_cast<int>(key) };
	key_ani.frame_rate_ms = frame_rate;
	key_ani.full_rate_ms = frame_rate;
	key_ani.is_active = is_active;
	registry.alwaysplayAni.insert(entity, key_ani);
	if (is_on_ui) registry.UIUX.emplace(entity); else registry.UIUXWorld.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::KEYS, // TEXTURE_COUNT indicates that no txture is needed
			is_on_ui ? EFFECT_ASSET_ID::UI : EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createCriHit(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = M_PI;
	motion.scale = { 128 * 0.4, 128 * 0.4 };

	Kinematic& kin = registry.kinematics.emplace(entity);
	kin.velocity = { 0, -300 };
	kin.direction = { 0,-1 };

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::CRTHITICON, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createBossHealthBarUI(RenderSystem* renderer, Entity boss, std::string boss_name, vec3 name_color) {
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	vec2 padding = { 0, -60 };
	motion.position = vec2(0, window_px_half.y) + padding;
	motion.scale = vec2({ BOSS_HEALTH_BAR_WIDTH, BOSS_HEALTH_BAR_HEIGHT });

	BossHealthBarUI& hb = registry.bossHealthBarUIs.emplace(entity);
	hb.is_visible = false;
	hb.name_color = name_color;
	hb.boss_name = boss_name;
	registry.bossHealthBarLink.emplace(entity, boss);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BOSS_HEALTH_BAR,
			EFFECT_ASSET_ID::BOSSHEALTHBAR,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createHealthUI(RenderSystem* renderer)
{
	auto entity_head = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh_head = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity_head, &mesh_head);

	// Setting initial motion values
	Motion& motion_head = registry.motions.emplace(entity_head);
	motion_head.position = vec2(0, 0) - window_px_half + vec2(70, 70);
	motion_head.scale = vec2({ 128 * 1.3, 128 * 1.3 });
	registry.UIUX.emplace(entity_head);
	registry.renderRequests.insert(
		entity_head,
		{ TEXTURE_ASSET_ID::REIMU_HEAD, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::UI,
			GEOMETRY_BUFFER_ID::SPRITE });
	registry.colors.insert(entity_head, { 1,1,1 });


	auto entity_inv = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh_inv = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity_inv, &mesh_inv);

	// Setting initial motion values
	Motion& motion_inv = registry.motions.emplace(entity_inv);
	motion_inv.position = vec2(0, 0) - window_px_half + vec2(128 * 1.3 + 110 + 20, 105);
	motion_inv.scale = vec2({ VP_BB_WIDTH, VP_BB_HEIGHT });
	registry.UIUX.emplace(entity_inv);
	registry.renderRequests.insert(
		entity_inv,
		{ TEXTURE_ASSET_ID::FOCUS_BAR, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::PLAYER_HB,
			GEOMETRY_BUFFER_ID::SPRITE });
	registry.colors.insert(entity_inv, { 1,1,1 });

	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = vec2(0, 0) - window_px_half + vec2(128 * 1.3 + 70 + 20, 70);
	motion.scale = vec2({ HP_BB_WIDTH, HP_BB_HEIGHT });
	registry.UIUX.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::REIMU_HEALTH, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::PLAYER_HB,
			GEOMETRY_BUFFER_ID::SPRITE });
	registry.colors.insert(entity, { 1,1,1 });
	return entity;
}

std::vector<Entity> createAttributeUI(RenderSystem* renderer)
{

	std::vector<Entity> entity_array;
	auto entity_coin = Entity();
	Mesh& mesh_coin = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity_coin, &mesh_coin);
	Motion& motion_coin = registry.motions.emplace(entity_coin);
	motion_coin.position = vec2(0, 0) - window_px_half + vec2(30, 200 - 50);
	motion_coin.scale = vec2({ 128 * 0.2, 128 * 0.2 });
	registry.UIUX.emplace(entity_coin);
	registry.renderRequests.insert(
		entity_coin,
		{ TEXTURE_ASSET_ID::COIN_STATIC, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::UI,
			GEOMETRY_BUFFER_ID::SPRITE });
	registry.colors.insert(entity_coin, { 1,1,1 });
	entity_array.push_back(entity_coin);


	for (int i = 0; i < 5; i++) {
		auto entity = Entity();

		// Store a reference to the potentially re-used mesh object
		Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
		registry.meshPtrs.emplace(entity, &mesh);

		// Setting initial motion values
		Motion& motion = registry.motions.emplace(entity);
		motion.position = vec2(0, 0) - window_px_half + vec2(30, 200 + 50 * i);
		motion.scale = vec2({ 128 * 0.2, 128 * 0.2 });
		registry.UIUX.emplace(entity);
		registry.renderRequests.insert(
			entity,
			{ static_cast<TEXTURE_ASSET_ID>((int)TEXTURE_ASSET_ID::ATTACKDMG + i), // TEXTURE_COUNT indicates that no txture is needed
				EFFECT_ASSET_ID::UI,
				GEOMETRY_BUFFER_ID::SPRITE });
		registry.colors.insert(entity, { 1,1,1 });
		entity_array.push_back(entity);
	}
	return entity_array;
}

Entity createInvisible(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();
	auto& motion = registry.motions.emplace(entity);
	motion.position = position;
	registry.kinematics.emplace(entity);
	return entity;
}

Entity createDummyEnemySpawner(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();
	auto& motion = registry.motions.emplace(entity);
	motion.position = position;
	registry.dummyenemyspawners.emplace(entity);
	return entity;
}

Entity createDummyEnemy(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.position = position;
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ ENEMY_BB_WIDTH * 2.f, ENEMY_BB_HEIGHT * 2.f });

	auto& kinematic = registry.kinematics.emplace(entity);
	kinematic.speed_base = 100.f;
	kinematic.speed_modified = 1.f * kinematic.speed_base;
	kinematic.direction = { 0, 0 };

	// Set the collision box
	auto& collidable = registry.collidables.emplace(entity);
	collidable.size = motion.scale / 2.f;

	// HP
	HP& hp = registry.hps.emplace(entity);
	hp.max_hp = 100;
	hp.curr_hp = hp.max_hp;

	// Collision damage
	Deadly& deadly = registry.deadlys.emplace(entity);
	deadly.damage = 1;

	registry.dummyenemies.emplace(entity);
	EntityAnimation enemy_ani;
	enemy_ani.spritesheet_scale = { 1.f / 6.f, 1.f / 12.f };
	enemy_ani.render_pos = { 1.f / 6.f, 5.f / 12.f };
	registry.animation.insert(entity, enemy_ani);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::ENEMY_WOLF,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE });

	registry.colors.insert(entity, { 1,1,1 });

	return entity;
}

Entity createRoomSignifier(RenderSystem* renderer, vec2 position, ROOM_TYPE room_type) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.position = position;
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ world_tile_size*3/4, world_tile_size * 3 / 4 });
	registry.renderRequests.insert(
		entity,
		{ static_cast<TEXTURE_ASSET_ID>((int)TEXTURE_ASSET_ID::NORMAL_SIGN + (int)room_type),
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE });

	registry.colors.insert(entity, { 1,1,1 });

	return entity;
}

Entity createBoss(RenderSystem* renderer, vec2 position, std::string boss_name, BOSS_ID boss_id, vec3 name_color)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.position = position;
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ ENEMY_BB_WIDTH / 1.2f, ENEMY_BB_HEIGHT / 1.2f });

	auto& kinematic = registry.kinematics.emplace(entity);
	kinematic.speed_base = 100.f;
	kinematic.speed_modified = 1.f * kinematic.speed_base;
	kinematic.direction = { 0, 0 };

	// Set the collision box
	auto& collidable = registry.collidables.emplace(entity);
	collidable.size = abs(vec2(motion.scale.x / 1.4f, motion.scale.y / 1.2f));

	// Collision damage
	Deadly& deadly = registry.deadlys.emplace(entity);
	deadly.damage = 1;

	// Animation
	EntityAnimation enemy_ani;
	enemy_ani.spritesheet_scale = { 1.f / 4.f, 1.f / 4.f };
	enemy_ani.render_pos = { 1.f / 4.f, 1.f / 4.f };
	registry.animation.insert(entity, enemy_ani);

	// HP
	HP& hp = registry.hps.emplace(entity);

	// Boss
	Boss& boss = registry.bosses.emplace(entity);
	boss.boss_id = boss_id;
	boss.duration = 10000; // duration for each pattern
	boss.phase_change_time = 1500;

	if (boss_id == BOSS_ID::CIRNO) {
		hp.max_hp = 5;
		hp.curr_hp = hp.max_hp;

		boss.health_phase_thresholds = { 5, 4, 3, 2, -1 }; // -1 for end of phase
		//boss.health_phase_thresholds = { 5000, 3750, 2500, 1250, -1 }; // -1 for end of phase

		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::BOSS_CIRNO,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE });
	}
	else if (boss_id == BOSS_ID::FLANDRE) {
		hp.max_hp = 10020;
		hp.curr_hp = hp.max_hp;

		boss.health_phase_thresholds = { 10000, 7500, 5000, 2500, -1 }; // -1 for end of phase

		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::BOSS_FLANDRE,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE });
	}


	registry.colors.insert(entity, { 1,1,1 });
	// Boss health bar ui
	Entity ui_entity = createBossHealthBarUI(renderer, entity, boss_name, name_color);
	registry.bossHealthBarLink.emplace(entity, ui_entity);

	// Add invulnerability
	registry.invulnerableTimers.emplace(entity).invulnerable_counter_ms = 3600000;

	// Decision tree ai
	AiTimer& ai_timer = registry.aitimers.emplace(entity);
	ai_timer.update_base = 1000;

	return entity;
}

Entity createBeeEnemy(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.position = position;
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT });

	auto& kinematic = registry.kinematics.emplace(entity);
	kinematic.speed_base = 100.f;
	kinematic.speed_modified = 1.f * kinematic.speed_base;
	kinematic.direction = { 0, 0 };

	// Set the collision box
	auto& collidable = registry.collidables.emplace(entity);
	collidable.size = abs(motion.scale / 2.f);

	// HP
	HP& hp = registry.hps.emplace(entity);
	hp.max_hp = static_cast<int>(80 * combo_mode.combo_meter);
	hp.curr_hp = hp.max_hp;

	// Collision damage
	Deadly& deadly = registry.deadlys.emplace(entity);
	deadly.damage = 1;
	registry.beeEnemies.emplace(entity);
	EntityAnimation enemy_ani;
	enemy_ani.spritesheet_scale = { 0.166, 0.125 };
	enemy_ani.render_pos = { 0.166, 0.125 };
	registry.animation.insert(entity, enemy_ani);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::ENEMY_BEE,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE });

	registry.idleMoveActions.emplace(entity);

	BulletSpawner bs;
	bs.fire_rate = 14 * 1.f/combo_mode.combo_meter;
	bs.is_firing = false;
	bs.bullet_initial_speed = 200;

	registry.bulletSpawners.insert(entity, bs);
	registry.colors.insert(entity, { 1,1,1 });

	registry.aitimers.emplace(entity);

	return entity;
}

Entity createText(vec2 pos, vec2 scale, std::string text_content, vec3 color, bool is_perm, bool in_world) {
	auto entity = Entity();
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.scale = scale; // Only x is used for scaling both x & y
	//registry.kinematics.emplace(entity);
	if (is_perm) {
		if (in_world) {
			registry.textsPermWorld.emplace(entity).content = text_content;
		}
		else {
			registry.textsPerm.emplace(entity).content = text_content;
		}
	}
	else {
		if (in_world) {
			registry.textsWorld.emplace(entity).content = text_content;
		}
		else {
			registry.texts.emplace(entity).content = text_content;
		}
	}
	registry.colors.emplace(entity) = color;
	return entity;
}

Entity createBomberEnemy(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.position = position;
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT });

	auto& kinematic = registry.kinematics.emplace(entity);
	kinematic.speed_base = 240.f * combo_mode.combo_meter;
	kinematic.speed_modified = 1.f * kinematic.speed_base;
	kinematic.direction = { 0, 0 };

	// Set the collision box
	auto& collidable = registry.collidables.emplace(entity);
	collidable.size = motion.scale / 2.f;

	// HP
	HP& hp = registry.hps.emplace(entity);
	hp.max_hp = static_cast<int>(20 * combo_mode.combo_meter);
	hp.curr_hp = hp.max_hp;

	// Collision damage
	Deadly& deadly = registry.deadlys.emplace(entity);
	deadly.damage = static_cast<int>(1 * combo_mode.combo_meter);

	registry.bomberEnemies.emplace(entity);
	EntityAnimation enemy_ani;
	enemy_ani.spritesheet_scale = { 1.f / 6.f, 1.f / 12.f };
	enemy_ani.render_pos = { 1.f / 6.f, 1.f / 12.f };
	registry.animation.insert(entity, enemy_ani);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::ENEMY_BOMBER,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE });

	registry.idleMoveActions.emplace(entity);
	registry.colors.insert(entity, { 1,1,1 });

	AiTimer& aitimer = registry.aitimers.emplace(entity);
	aitimer.update_base = 1000; // updates decision tree every second
	aitimer.update_timer_ms = 1000;

	return entity;
}

Entity createWolfEnemy(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.position = position;
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT });

	auto& kinematic = registry.kinematics.emplace(entity);
	kinematic.speed_base = 100.f;
	kinematic.speed_modified = 1.f * kinematic.speed_base;
	kinematic.direction = { 0, 0 };

	// Set the collision box
	auto& collidable = registry.collidables.emplace(entity);
	collidable.size = motion.scale / 2.f;

	// HP
	HP& hp = registry.hps.emplace(entity);
	hp.max_hp = static_cast<int>(40 * combo_mode.combo_meter);
	hp.curr_hp = hp.max_hp;

	// Collision damage
	Deadly& deadly = registry.deadlys.emplace(entity);
	deadly.damage = 1;

	registry.wolfEnemies.emplace(entity);
	EntityAnimation enemy_ani;
	enemy_ani.spritesheet_scale = { 1.f / 6.f, 1.f / 12.f };
	enemy_ani.render_pos = { 1.f / 6.f, 5.f / 12.f };
	registry.animation.insert(entity, enemy_ani);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::ENEMY_WOLF,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE });

	registry.idleMoveActions.emplace(entity);

	BulletSpawner bs;
	bs.fire_rate = 40 * 1/combo_mode.combo_meter;
	bs.is_firing = false;
	bs.bullet_initial_speed = 160;
	bs.bullets_per_array = static_cast<int>(3 * combo_mode.combo_meter);
	bs.spread_within_array = 30;

	registry.bulletSpawners.insert(entity, bs);
	registry.colors.insert(entity, { 1,1,1 });

	AiTimer& aitimer = registry.aitimers.emplace(entity);
	aitimer.update_base = 500; // updates decision tree every second
	aitimer.update_timer_ms = 500;

	return entity;
}

Entity createSubmachineGunEnemy(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.position = position;
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT });

	auto& kinematic = registry.kinematics.emplace(entity);
	kinematic.speed_base = 100.f;
	kinematic.speed_modified = 1.f * kinematic.speed_base;
	kinematic.direction = { 0, 0 };

	// Set the collision box
	auto& collidable = registry.collidables.emplace(entity);
	collidable.size = motion.scale;

	// HP
	HP& hp = registry.hps.emplace(entity);
	hp.max_hp = 6;
	hp.curr_hp = hp.max_hp;

	// Collision damage
	Deadly& deadly = registry.deadlys.emplace(entity);
	deadly.damage = 1;

	registry.submachineGunEnemies.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::ENEMY_BEE,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE });

	registry.idleMoveActions.emplace(entity);
	BulletSpawner enemy_bullet_rate;
	enemy_bullet_rate.fire_rate = 5;
	enemy_bullet_rate.is_firing = true;
	registry.bulletSpawners.insert(entity, enemy_bullet_rate);
	registry.colors.insert(entity, { 1,1,1 });

	return entity;
}

std::vector<Entity> createFloor(RenderSystem* renderer, vec2 position, std::vector<TEXTURE_ASSET_ID> textureIDs) {
	std::vector<Entity> entities;
	for (int i = 0; i < textureIDs.size(); i++) {
		auto entity = Entity();

		// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
		Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
		registry.meshPtrs.emplace(entity, &mesh);

		// Initialize the motion
		auto& motion = registry.motions.emplace(entity);
		motion.angle = 0.f;
		motion.position = position;
		motion.scale = vec2(world_tile_size, world_tile_size);

		// TODO: remove this, used for testing ai can see player
		if (textureIDs[i] == TEXTURE_ASSET_ID::PILLAR_TOP) {
			registry.floors.emplace(entity);
			registry.renderRequestsForeground.insert(
				entity,
				{ textureIDs[i],
				 EFFECT_ASSET_ID::TEXTURED,
				 GEOMETRY_BUFFER_ID::SPRITE });
			entities.push_back(entity);
			continue;
		}

		// Create and (empty) Tile component to be able to refer to all decoration tiles
		registry.floors.emplace(entity);
		registry.renderRequests.insert( // TODO Change to ground texture
			entity,
			{ textureIDs[i],
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE });
		entities.push_back(entity);
	}
	return entities;
}

std::vector<Entity> createWall(RenderSystem* renderer, vec2 position, std::vector<TEXTURE_ASSET_ID> textureIDs) {
	std::vector<Entity> entities;
	for (int i = 0; i < textureIDs.size(); i++) {
		auto entity = Entity();

		// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
		Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
		registry.meshPtrs.emplace(entity, &mesh);

		// Initialize the motion
		auto& motion = registry.motions.emplace(entity);
		motion.angle = 0.f;
		motion.position = position;
		motion.scale = vec2(world_tile_size, world_tile_size);

		// Set the collision box
		auto& collidable = registry.collidables.emplace(entity);
		collidable.size = { motion.scale.x, motion.scale.y };
		collidable.shift = { 0, 0 };

		//if (textureIDs[i] == TEXTURE_ASSET_ID::LEFT_WALL) {
		//	collidable.size = { motion.scale.x, motion.scale.y };
		//	collidable.shift = { 0, 0 };
		//}
		//else if (textureIDs[i] == TEXTURE_ASSET_ID::RIGHT_WALL) {
		//	collidable.size = { motion.scale.x, motion.scale.y };
		//	collidable.shift = { 0, 0 };
		//}
		//else if (textureIDs[i] == TEXTURE_ASSET_ID::BOTTOM_WALL) {
		//	collidable.size = { motion.scale.x, motion.scale.y };
		//	collidable.shift = { 0, 0 };
		//}
		//else if (textureIDs[i] == TEXTURE_ASSET_ID::WALL_SURFACE) {
		//	collidable.size = { motion.scale.x, motion.scale.y };
		//	collidable.shift = { 0, 0 };
		//}
		//else if (textureIDs[i] == TEXTURE_ASSET_ID::LEFT_TOP_CORNER_WALL ||
		//	textureIDs[i] == TEXTURE_ASSET_ID::LEFT_BOTTOM_CORNER_WALL ||
		//	textureIDs[i] == TEXTURE_ASSET_ID::RIGHT_TOP_CORNER_WALL ||
		//	textureIDs[i] == TEXTURE_ASSET_ID::RIGHT_BOTTOM_CORNER_WALL) {
		//	collidable.size = { motion.scale.x, motion.scale.y };
		//	collidable.shift = { 0, 0 };
		//}
		//else
		//	// TODO: remove this, used for testing ai can see player
		//	if (textureIDs[i] == TEXTURE_ASSET_ID::PILLAR_BOTTOM) {
		//		collidable.size = { motion.scale.x, motion.scale.y / 2 };
		//		collidable.shift = { 0, -motion.scale.y / 4 };
		//	}
		//	else {
		//		// Temporary
		//		// TODO: Maybe change/refactor this since it's adding floors when its in createWall
		//		registry.collidables.remove(entity);
		//		registry.floors.emplace(entity);
		//		registry.renderRequests.insert(
		//			entity,
		//			{ textureIDs[i],
		//			 EFFECT_ASSET_ID::TEXTURED,
		//			 GEOMETRY_BUFFER_ID::SPRITE });
		//		entities.push_back(entity);
		//		continue;
		//	}

		// Create and (empty) Tile component to be able to refer to all physical tiles
		registry.walls.emplace(entity);
		registry.renderRequests.insert(
			entity,
			{ textureIDs[i],
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE });

		entities.push_back(entity);
	}
	return entities;
}

Entity createRock(RenderSystem* renderer, vec2 grid_position) {
	auto entity = Entity();

	// Initializes the motion
	auto& motion = registry.motions.emplace(entity);
	motion.position = convert_grid_to_world(grid_position);
	motion.scale = vec2(world_tile_size, world_tile_size);

	// Rocks are collidable
	auto& collidable = registry.collidables.emplace(entity);
	collidable.size = { motion.scale.x, motion.scale.y };
	collidable.shift = { 0, 0 };


	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> distrib(0, 1);
	double number = distrib(gen);

	// Placeholder texure
	if (number < 0.2) {
		world_map[grid_position.y][grid_position.x] = (int)TILE_TYPE::WALL;
		registry.walls.emplace(entity);

		registry.renderRequests.insert(
			entity,
			{
			TEXTURE_ASSET_ID::ROCK,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE });
	}
	else if (number < 0.4) {
		registry.renderRequests.insert(
			entity,
			{
			TEXTURE_ASSET_ID::SKELETON,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE });
	}
	else if (number < 0.6) {
		world_map[grid_position.y][grid_position.x] = (int)TILE_TYPE::WALL;
		registry.walls.emplace(entity);

		registry.renderRequests.insert(
			entity,
			{
			TEXTURE_ASSET_ID::POTTERY,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE });
	}
	else if (number < 0.8) {
		world_map[grid_position.y][grid_position.x] = (int)TILE_TYPE::WALL;
		registry.walls.emplace(entity);

		registry.renderRequests.insert(
			entity,
			{
			TEXTURE_ASSET_ID::BARREL,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE });
	}
	else {
		world_map[grid_position.y][grid_position.x] = (int)TILE_TYPE::WALL;
		registry.walls.emplace(entity);

		EntityAnimation& animation = registry.alwaysplayAni.emplace(entity);
		animation.spritesheet_scale = { 1 / 8.f, 1.0f };
		animation.render_pos = { 1 / 8.f, 1.0f };
		animation.full_rate_ms = 100.f;
		animation.frame_rate_ms = 100.f;

		registry.renderRequests.insert(
			entity,
			{
			TEXTURE_ASSET_ID::FIREPLACE,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE });
	}

	return entity;
}



// IMPORTANT: createDoor takes in grid coordinates
Entity createDoor(RenderSystem* renderer, vec2 grid_position, DIRECTION dir, int room_index) {
	auto entity = Entity();

	// Initializes the motion
	auto& motion = registry.motions.emplace(entity);
	motion.position = convert_grid_to_world(grid_position);
	motion.scale = vec2(world_tile_size, world_tile_size);

	// Creates door
	auto& door = registry.doors.emplace(entity);
	door.dir = dir;
	door.room_index = room_index;

	game_info.room_index[room_index].doors.push_back(entity);
	game_info.room_index[room_index].door_locations.push_back(grid_position);

	// Locked doors are collidable
	auto& collidable = registry.collidables.emplace(entity);
	collidable.size = { motion.scale.x, motion.scale.y };
	collidable.shift = { 0, 0 };

	if (dir == DIRECTION::LEFT || dir == DIRECTION::RIGHT) {
		door.top_texture = createDoorUpTexture(renderer, grid_position + vec2(0, -1));
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::DOOR_VERTICAL_CLOSE_DOWN,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE });
	}
	else {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::DOOR_HORIZONTAL_CLOSE,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE });
	}

	return entity;
}

// for vertical doors, aesthetic effect
Entity createDoorUpTexture(RenderSystem* renderer, vec2 grid_position) {
	auto entity = Entity();

	// Initializes the motion
	auto& motion = registry.motions.emplace(entity);
	motion.position = convert_grid_to_world(grid_position);
	motion.scale = vec2(world_tile_size, world_tile_size);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::DOOR_VERTICAL_CLOSE_UP,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createTile(RenderSystem* renderer, VisibilitySystem* visibility_system, vec2 grid_position, TILE_NAME tile_name, bool is_wall) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	//Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	//registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.position = convert_grid_to_world(grid_position);
	motion.scale = vec2(world_tile_size, world_tile_size);

	// Create wall or floor entity for physics collision
	if (is_wall) {
		registry.walls.emplace(entity);
		// Set the collision box
		auto& collidable = registry.collidables.emplace(entity);
		collidable.size = { motion.scale.x, motion.scale.y };
		collidable.shift = { 0, 0 };
	}
	else {
		registry.floors.emplace(entity);
	}

	// Set up instance data
	Transform t;
	t.translate(motion.position);
	t.scale(motion.scale);
	registry.tileInstanceData.emplace(entity) = {
		renderer->get_spriteloc(tile_name),
		t.mat
	};

	// Add visibility tile
	// We do it here bcause we have already calculated the transform matrix
	/*
	This entity has:

	TEXTURE_ASSET_ID::TEXTURE_COUNT,
	EFFECT_ASSET_ID::EGG,
	GEOMETRY_BUFFER_ID::DEBUG_LINE2
	*/
	if (map_info.level == MAP_LEVEL::TUTORIAL) return entity;

	auto entity2 = Entity();
	//registry.visibilityTiles.emplace(entity2); // TODO
	registry.visibilityTileInstanceData.emplace(entity2) = {
		t.mat,
		1.0
	};
	// add reference to entity in 2d array
	// when removing visibility tile entities, we set it's corresponding grid position in reference map to -1
	visibility_system->reference_map[grid_position.y][grid_position.x] = entity2;

	return entity;
}


// IMPORTANT: creates pillar using grid coordinates, NOT world coorindates
// textureIDs[0] == bottom, textureIDs[1] == top
std::vector<Entity> createPillar(RenderSystem* renderer, vec2 grid_position, std::vector<TEXTURE_ASSET_ID> textureIDs) {
	assert(textureIDs.size() == 2 && "textureIDs do not have size 2");
	assert((!is_valid_cell(grid_position.x, grid_position.y) ||
		!(grid_position.y < 0 ||
			grid_position.x < 0 ||
			grid_position.y >= world_height ||
			grid_position.x >= world_width)) && "Pillar position not valid");

	std::vector<Entity> entities;
	auto bottom_entity = Entity();
	auto top_entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& bottom_mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(bottom_entity, &bottom_mesh);
	Mesh& top_mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(top_entity, &top_mesh);

	// Initialize the motion
	vec2 bottom_world_position = convert_grid_to_world(grid_position);

	auto& bottom_motion = registry.motions.emplace(bottom_entity);
	bottom_motion.position = bottom_world_position;
	bottom_motion.scale = vec2(world_tile_size, world_tile_size);
	auto& top_motion = registry.motions.emplace(top_entity);
	top_motion.position = bottom_world_position + vec2{ 0, -world_tile_size };
	top_motion.scale = vec2(world_tile_size, world_tile_size);

	// Set the collision box
	auto& bottom_collidable = registry.collidables.emplace(bottom_entity);
	bottom_collidable.size = { bottom_motion.scale.x, bottom_motion.scale.y };
	bottom_collidable.shift = { 0, 0 };

	world_map[grid_position.y][grid_position.x] = (int)TILE_TYPE::WALL;

	registry.walls.emplace(bottom_entity);
	registry.floors.emplace(top_entity); // TODO: maybe foreground.emplace

	registry.renderRequests.insert(
		bottom_entity,
		{ textureIDs[0],
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE });
	registry.renderRequestsForeground.insert(
		top_entity,
		{ textureIDs[1],
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	entities.push_back(top_entity);
	entities.push_back(bottom_entity);

	return entities;
}

Entity createLine(vec2 position, vec2 scale)
{
	Entity entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 EFFECT_ASSET_ID::EGG,
		 GEOMETRY_BUFFER_ID::DEBUG_LINE });

	// Create motion
	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.position = position;
	motion.scale = scale;

	registry.debugComponents.emplace(entity);
	return entity;
}

Entity createEgg(vec2 pos, vec2 size)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.scale = size;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::EGG,
			GEOMETRY_BUFFER_ID::EGG });

	registry.debugComponents.emplace(entity);
	return entity;
}

Entity createNPC(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.scale = vec2({ PLAYER_BB_WIDTH, PLAYER_BB_HEIGHT });

	// Set the collision box
	auto& collidable = registry.collidables.emplace(entity);
	// Set player collision box at the feet of the player
	collidable.size = { motion.scale.x * 5, motion.scale.y * 5 };
	collidable.shift = { 0, 0 };

	registry.npcs.emplace(entity);
	EntityAnimation npc_ani;
	npc_ani.isCursor = true;
	npc_ani.spritesheet_scale = { 0.25, 1.0 };
	npc_ani.render_pos = { 0.25, 1.0 };
	registry.alwaysplayAni.insert(entity, npc_ani);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::NPC_MARISA, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });
	registry.colors.insert(entity, { 1,1,1 });

	return entity;
}

Entity createButton(RenderSystem* renderer, vec2 pos, float scale,
	MENU_STATE menu_state, std::string button_text, float text_scale, std::function<void()> func) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.scale = scale * vec2(BUTTON_HOVER_WIDTH, BUTTON_HOVER_HEIGHT);

	registry.colors.emplace(entity, vec3(0.f));

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BUTTON,
			EFFECT_ASSET_ID::UI,
			GEOMETRY_BUFFER_ID::SPRITE });

	Button& button = registry.buttons.emplace(entity);
	button.state = menu_state;
	button.text = button_text;
	button.text_scale = text_scale;
	button.func = func;

	return entity;
}

Entity createMainMenu(RenderSystem* renderer, vec2 title_pos, float title_scale, vec2 background_pos, float background_scale) {
	// Main menu background
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = { 0, 0 };
	// background will always be largest size and centered
	vec2 background_size = { MENU_BACKGROUND_WIDTH, MENU_BACKGROUND_HEIGHT };
	vec2 ratio = vec2(window_width_px, window_height_px) / background_size;
	motion.scale = (ratio.x + ratio.y) / 2.f * background_size;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::MENU_BACKGROUND,
			EFFECT_ASSET_ID::UI,
			GEOMETRY_BUFFER_ID::SPRITE });
	registry.mainMenus.emplace(entity);

	// Main menu title 
	auto entity2 = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh2 = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity2, &mesh2);

	Motion& motion2 = registry.motions.emplace(entity2);
	motion2.position = title_pos;
	motion2.scale = title_scale * vec2(MENU_TITLE_WIDTH, MENU_TITLE_HEIGHT);

	registry.renderRequests.insert(
		entity2,
		{ TEXTURE_ASSET_ID::MENU_TITLE,
			EFFECT_ASSET_ID::UI,
			GEOMETRY_BUFFER_ID::SPRITE });
	registry.mainMenus.emplace(entity2);

	// Paper on buttons for visibility
	auto entity3 = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh3 = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity3, &mesh3);

	Motion& motion3 = registry.motions.emplace(entity3);
	motion3.position = background_pos;
	motion3.scale = background_scale * vec2(PAUSE_BACKGROUND_WIDTH, PAUSE_BACKGROUND_HEIGHT);

	registry.renderRequests.insert(
		entity3,
		{ TEXTURE_ASSET_ID::PAUSE_BACKGROUND,
			EFFECT_ASSET_ID::UI,
			GEOMETRY_BUFFER_ID::SPRITE });
	registry.mainMenus.emplace(entity3);

	return entity;
}

Entity createPauseMenu(RenderSystem* renderer, vec2 background_pos, float background_scale) {
	// Pause menu background
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = background_pos;
	motion.scale = background_scale * vec2(PAUSE_BACKGROUND_WIDTH, PAUSE_BACKGROUND_HEIGHT);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PAUSE_BACKGROUND,
			EFFECT_ASSET_ID::UI,
			GEOMETRY_BUFFER_ID::SPRITE });
	registry.pauseMenus.emplace(entity);

	return entity;
}

Entity createTeleporter(RenderSystem* renderer, vec2 pos, float scale) {
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.scale = scale * vec2(TELEPORTER_WIDTH, TELEPORTER_HEIGHT);

	Collidable& collidable = registry.collidables.emplace(entity);
	collidable.size = motion.scale / 6.f;

	EntityAnimation key_ani;
	key_ani.isCursor = false;
	key_ani.spritesheet_scale = { 1 / 39.f, 1 };
	key_ani.render_pos = { 1 / 39.f, 1 };
	key_ani.frame_rate_ms = 100;
	key_ani.full_rate_ms = 100;
	key_ani.is_active = false;
	registry.alwaysplayAni.insert(entity, key_ani);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TELEPORTER,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE });
	registry.teleporters.emplace(entity);

	return entity;
}
