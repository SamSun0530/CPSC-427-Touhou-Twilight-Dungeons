#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"
#include "tiny_ecs_registry.hpp"
#include <glm/trigonometric.hpp>
#include "world_system.hpp"
#include <random>

// These are ahrd coded to the dimensions of the entity texture
const float BULLET_BB_WIDTH = 1.0f * 27.f;
const float BULLET_BB_HEIGHT = 1.0f * 27.f;
const float PLAYER_BB_WIDTH = 0.5f * 128.f;
const float PLAYER_BB_HEIGHT = 0.5f * 128.f;
const float ENEMY_BB_WIDTH = 1.f * 80.f;
const float ENEMY_BB_HEIGHT = 1.f * 90.f;
const float HP_BB_WIDTH = 0.2f * 283.f;
const float HP_BB_HEIGHT = 0.2f * 244.f;
const float HEALTH_WIDTH = 0.3f * 128.f;
const float HEALTH_HEIGHT = 0.3f * 80.f;

// the bullet, takes into account entity's speed and position
Entity createBullet(RenderSystem* renderer, float entity_speed, vec2 entity_position, float rotation_angle, vec2 direction, bool is_player_bullet = false);

std::vector<Entity> createUI(RenderSystem*, int max_hp);

Entity createHealth(RenderSystem* renderer, vec2 position);

// focus mode dot
Entity createFocusDot(RenderSystem* renderer, vec2 pos, vec2 size);
// the player
Entity createPlayer(RenderSystem* renderer, vec2 pos);
// the coin
Entity createCoin(RenderSystem* renderer, vec2 position);
// the enemy
Entity createBeeEnemy(RenderSystem* renderer, vec2 position);
// the suicide enemy
Entity createBomberEnemy(RenderSystem* renderer, vec2 position);
// the shotgun enemy
Entity createWolfEnemy(RenderSystem* renderer, vec2 position);
// the grenade launcher enemy
Entity createSubmachineGunEnemy(RenderSystem* renderer, vec2 position);
// Non interactable tile
std::vector<Entity> createFloor(RenderSystem* renderer, vec2 position, std::vector<TEXTURE_ASSET_ID> textureIDs);
// Interactable Tile
std::vector<Entity> createWall(RenderSystem* renderer, vec2 position, std::vector<TEXTURE_ASSET_ID> textureIDs);
// Pillar tile
std::vector<Entity> createPillar(RenderSystem* renderer, vec2 grid_position, std::vector<TEXTURE_ASSET_ID> textureIDs);

// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);
// a egg
Entity createEgg(vec2 pos, vec2 size);


