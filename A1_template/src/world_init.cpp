#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"
#include <glm/trigonometric.hpp>


Entity createBullet(RenderSystem* renderer, float entity_speed, vec2 entity_position, float rotation_angle, vec2 direction, bool is_player_bullet)
{
	auto entity = Entity();
	
	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = rotation_angle;
	motion.speed_base = 200.f;
	motion.speed_modified = 1.f * motion.speed_base + entity_speed; // bullet speed takes into account of entity's speed
	motion.direction = direction;
	motion.position = entity_position; // bullet spawns from entity's center position

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ -BUG_BB_WIDTH, BUG_BB_HEIGHT });
	
	// Create and (empty) bullet component to be able to refer to all bullets
	if (is_player_bullet) {
		registry.bullets.emplace(entity);
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::BUG,
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

// Note, BUG corresponds to texture Bullet; EAGLE corresponds to texture Enemy; CHICKEN corresponds to texture Reimu

Entity createChicken(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.speed_base = 100.f;
	motion.speed_modified = 1.f * motion.speed_base;
	motion.direction = { 0, 0 };
	motion.scale = vec2({ -CHICKEN_BB_WIDTH, CHICKEN_BB_HEIGHT });
	motion.scale.x = -motion.scale.x;

	HP& hp = registry.hps.emplace(entity);
	hp.max_hp = 6;
	hp.curr_hp = hp.max_hp;

	// Create and (empty) Chicken component to be able to refer to all eagles
	registry.players.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::CHICKEN, // TEXTURE_COUNT indicates that no txture is needed
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

Entity createBug(RenderSystem* renderer, vec2 position)
{
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.speed_base = 50.f;
	motion.speed_modified = 1.f * motion.speed_base;
	motion.direction = { 0, 1 };
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ -BUG_BB_WIDTH, BUG_BB_HEIGHT });

	// Create an (empty) Bug component to be able to refer to all bug
	registry.eatables.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BUG,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createEagle(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.speed_base = 100.f;
	motion.speed_modified = 1.f * motion.speed_base;
	motion.direction = { 0, 0 };
	motion.position = position;

	HP& hp = registry.hps.emplace(entity);
	hp.max_hp = 6;
	hp.curr_hp = hp.max_hp;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ -EAGLE_BB_WIDTH, EAGLE_BB_HEIGHT });

	// Create and (empty) Eagle component to be able to refer to all eagles
	registry.deadlys.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::EAGLE,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE });

	registry.idleMoveActions.emplace(entity);
	BulletFireRate enemy_bullet_rate;
	enemy_bullet_rate.fire_rate = 1;
	enemy_bullet_rate.is_firing = true;
	registry.bulletFireRates.insert(entity,enemy_bullet_rate);
	registry.colors.insert(entity, { 1,1,1 });

	return entity;
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
	motion.direction = { 0, 0 };
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
	motion.direction = { 0.f, 0.f };
	motion.scale = size;

	// Create and (empty) Chicken component to be able to refer to all eagles
	registry.deadlys.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::EGG,
			GEOMETRY_BUFFER_ID::EGG });

	return entity;
}