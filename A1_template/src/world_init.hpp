#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

// These are ahrd coded to the dimensions of the entity texture
const float BUG_BB_WIDTH = 1.0f * 30.f;
const float BUG_BB_HEIGHT = 1.0f * 30.f;
const float CHICKEN_BB_WIDTH = 1.3f * 64.f;
const float CHICKEN_BB_HEIGHT = 1.3f * 80.f;
const float EAGLE_BB_WIDTH = 1.f * 80.f;
const float EAGLE_BB_HEIGHT = 1.f * 90.f;
const float HP_BB_WIDTH = 1.0f * 55.f;
const float HP_BB_HEIGHT = 1.0f * 55.f;

// the bullet, takes into account entity's speed and position
Entity createBullet(RenderSystem* renderer, float entity_speed, vec2 entity_position, float rotation_angle, vec2 direction, bool is_player_bullet = false);

std::vector<Entity> createUI(RenderSystem* , int max_hp);

// the player
Entity createChicken(RenderSystem* renderer, vec2 pos);
// the prey
Entity createBug(RenderSystem* renderer, vec2 position);
// the enemy
Entity createEagle(RenderSystem* renderer, vec2 position);
// Non interactable tile
std::vector<Entity> createDecoTile(RenderSystem* renderer, vec2 position, std::vector<TEXTURE_ASSET_ID> textureIDs);

// Interactable Tile
std::vector<Entity> createPhysTile(RenderSystem* renderer, vec2 position, std::vector<TEXTURE_ASSET_ID> textureIDs);

// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);
// a egg
Entity createEgg(vec2 pos, vec2 size);


