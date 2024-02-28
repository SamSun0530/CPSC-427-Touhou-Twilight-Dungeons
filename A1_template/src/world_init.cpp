#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"
#include <glm/trigonometric.hpp>
#include <iostream>


Entity createBullet(RenderSystem* renderer, float entity_speed, vec2 entity_position, float rotation_angle, vec2 direction, bool is_player_bullet)
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
	motion.scale = vec2({ -BULLET_BB_WIDTH, BULLET_BB_HEIGHT });

	auto& realmotion = registry.realMotions.emplace(entity);
	realmotion.speed_base = 200.f;
	realmotion.speed_modified = 1.f * realmotion.speed_base + entity_speed; // bullet speed takes into account of entity's speed
	realmotion.direction = direction;

	// Set the collision box
	auto& collidable = registry.collidables.emplace(entity);
	collidable.size = motion.scale;

	// Create and (empty) bullet component to be able to refer to all bullets
	if (is_player_bullet) {
		registry.playerBullets.emplace(entity);
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::BULLET,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE });
	}
	else {
		registry.enemyBullets.emplace(entity);
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::ENEMY_BULLET,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE });
	}

	return entity;
}

Entity createPlayer(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.scale = vec2({ -PLAYER_BB_WIDTH, PLAYER_BB_HEIGHT });
	motion.scale.x = -motion.scale.x;

	auto& realmotion = registry.realMotions.emplace(entity);
	realmotion.speed_base = 100.f;
	realmotion.speed_modified = 3.f * realmotion.speed_base;
	realmotion.direction = { 0, 0 };

	// Set the collision box
	auto& collidable = registry.collidables.emplace(entity);
	collidable.size = motion.scale;

	// Set player collision box at the feet of the player
	//collidable.size = { motion.scale.x, motion.scale.y / 4.f };
	//collidable.shift = { 0, motion.scale.y / 4.f };

	HP & hp = registry.hps.emplace(entity);
	hp.max_hp = 6;
	hp.curr_hp = hp.max_hp;

	registry.players.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLAYER, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	registry.bulletFireRates.emplace(entity);
	registry.colors.insert(entity, { 1,1,1 });

	return entity;
}

std::vector<Entity> createUI(RenderSystem* renderer, int max_hp)
{
	std::vector<Entity> hp_entities;
	for (int i = 0; i < max_hp; i++) {
		auto entity = Entity();

		// Store a reference to the potentially re-used mesh object
		Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
		registry.meshPtrs.emplace(entity, &mesh);

		// Setting initial motion values
		Motion& motion = registry.motions.emplace(entity);

		motion.scale = vec2({ -HP_BB_WIDTH, HP_BB_HEIGHT });
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::FULL_HEART, // TEXTURE_COUNT indicates that no txture is needed
				EFFECT_ASSET_ID::UI,
				GEOMETRY_BUFFER_ID::SPRITE });
		registry.colors.insert(entity, { 1,1,1 });
		hp_entities.push_back(entity);
	}

	return hp_entities;
}

Entity createCoin(RenderSystem* renderer, vec2 position)
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
	motion.scale = vec2({ -BULLET_BB_WIDTH, BULLET_BB_HEIGHT });

	auto& realmotion = registry.realMotions.emplace(entity);
	realmotion.speed_base = 50.f;
	realmotion.speed_modified = 1.f * realmotion.speed_base;
	realmotion.direction = { 0, 1 };

	registry.pickupables.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BULLET,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createEnemy(RenderSystem* renderer, vec2 position)
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
	motion.scale = vec2({ -ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT });

	auto& realmotion = registry.realMotions.emplace(entity);
	realmotion.speed_base = 100.f;
	realmotion.speed_modified = 1.f * realmotion.speed_base;
	realmotion.direction = { 0, 0 };

	// Set the collision box
	auto& collidable = registry.collidables.emplace(entity);
	collidable.size = motion.scale;

	HP& hp = registry.hps.emplace(entity);
	hp.max_hp = 6;
	hp.curr_hp = hp.max_hp;

	registry.deadlys.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::ENEMY,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE });

	registry.idleMoveActions.emplace(entity);
	BulletFireRate enemy_bullet_rate;
	enemy_bullet_rate.fire_rate = 3;
	enemy_bullet_rate.is_firing = true;
	registry.bulletFireRates.insert(entity, enemy_bullet_rate);
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
		collidable.size = motion.scale;

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

// TEMPORARY EGG (remove/refactor later)
Entity createEgg(vec2 pos, vec2 size)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.scale = size;

	registry.deadlys.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::EGG,
			GEOMETRY_BUFFER_ID::EGG });

	return entity;
}