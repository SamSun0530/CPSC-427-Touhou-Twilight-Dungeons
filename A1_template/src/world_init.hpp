#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"
#include "tiny_ecs_registry.hpp"
#include <glm/trigonometric.hpp>
#include "world_system.hpp"
#include <random>
#include "global.hpp"


// These are ahrd coded to the dimensions of the entity texture
const float BULLET_BB_WIDTH = 1.0f * 27.f;
const float BULLET_BB_HEIGHT = 1.0f * 27.f;
const float PLAYER_BB_WIDTH = 0.5f * 128.f;
const float PLAYER_BB_HEIGHT = 0.5f * 128.f;
const float ENEMY_BB_WIDTH = 1.f * 80.f;
const float ENEMY_BB_HEIGHT = 1.f * 90.f;
const float HP_BB_WIDTH = 1.3f * 194.f;
const float HP_BB_HEIGHT = 1.3f * 36.f;
const float VP_BB_WIDTH = 1.5f * 218.f;
const float VP_BB_HEIGHT = 1.5f * 16.f;
const float HEALTH_WIDTH = 0.3f * 128.f;
const float HEALTH_HEIGHT = 0.3f * 80.f;
const float BOSS_HEALTH_BAR_WIDTH = 0.6f * 931.f;
const float BOSS_HEALTH_BAR_HEIGHT = 0.6f * 116.f /*232.f*/;
const float BUTTON_HOVER_WIDTH = 1.f * 284.f;
const float BUTTON_HOVER_HEIGHT = 1.f * 104.f;
const float MENU_TITLE_WIDTH = 1.f * 1608.f;
const float MENU_TITLE_HEIGHT = 1.f * 538.f;
const float MENU_BACKGROUND_WIDTH = 1.f * 1792.f;
const float MENU_BACKGROUND_HEIGHT = 1.f * 1008.f;
const float PAUSE_BACKGROUND_WIDTH = 1.f * 416.f;
const float PAUSE_BACKGROUND_HEIGHT = 1.f * 448.f;

// the bullet, takes into account entity's speed and position
Entity createBullet(RenderSystem* renderer, float entity_speed, vec2 entity_position, float rotation_angle, vec2 direction, float bullet_speed = 100.f, bool is_player_bullet = false, BulletPattern* bullet_pattern = nullptr);
Entity createBulletDisappear(RenderSystem* renderer, vec2 entity_position, float rotation_angle, bool is_player_bullet);

Entity createText(vec2 pos, vec2 scale, std::string text_content, vec3 color, bool is_perm, bool in_world = false);

Entity createCombo(RenderSystem* renderer);

void createDialogue(CHARACTER character, std::string sentence, CHARACTER talk_2);

Entity createCriHit(RenderSystem* renderer, vec2 pos);
std::vector<Entity> createAttributeUI(RenderSystem* renderer);
Entity createHealthUI(RenderSystem*);
Entity createFood(RenderSystem* renderer, vec2 position);
Entity createBossHealthBarUI(RenderSystem* renderer, Entity boss, std::string boss_name);
// ui key
Entity createKey(vec2 pos, vec2 size, KEYS key, bool is_on_ui = true, bool is_active = true, float frame_rate = 500.f);
// focus mode dot
Entity createFocusDot(RenderSystem* renderer, vec2 pos, vec2 size);
// the player
Entity createPlayer(RenderSystem* renderer, vec2 pos);
// the coin
Entity createCoin(RenderSystem* renderer, vec2 position, int value);
// max hp increase
Entity createMaxHPIncrease(RenderSystem* renderer, vec2 position);
// the attack up
Entity createAttackUp(RenderSystem* renderer, vec2 position);
// the chest
Entity createChest(RenderSystem* renderer, vec2 position);
// the key
Entity createKey(RenderSystem* renderer, vec2 position);
// the enemy
Entity createBeeEnemy(RenderSystem* renderer, vec2 position);
// the suicide enemy
Entity createBomberEnemy(RenderSystem* renderer, vec2 position);
// the shotgun enemy
Entity createWolfEnemy(RenderSystem* renderer, vec2 position);
// the grenade launcher enemy
Entity createSubmachineGunEnemy(RenderSystem* renderer, vec2 position);
// boss enemy
Entity createBoss(RenderSystem* renderer, vec2 position, std::string boss_name);
// dummy enemy for tutorial
Entity createDummyEnemy(RenderSystem* renderer, vec2 position);
Entity createDummyEnemySpawner(RenderSystem* renderer, vec2 position);
// invisible entity with only position
Entity createInvisible(RenderSystem* renderer, vec2 position);
// Non interactable tile
std::vector<Entity> createFloor(RenderSystem* renderer, vec2 position, std::vector<TEXTURE_ASSET_ID> textureIDs);
// Interactable Tile
std::vector<Entity> createWall(RenderSystem* renderer, vec2 position, std::vector<TEXTURE_ASSET_ID> textureIDs);
// Pillar tile
std::vector<Entity> createPillar(RenderSystem* renderer, vec2 grid_position, std::vector<TEXTURE_ASSET_ID> textureIDs);
// Tile for instance rendering
Entity createTile(RenderSystem* renderer, vec2 position, TILE_NAME_SANDSTONE tile_name, bool is_wall);

// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);
// a egg
Entity createEgg(vec2 pos, vec2 size);

// Menu button
Entity createButton(RenderSystem* renderer, vec2 pos, float scale,
	MENU_STATE menu_state, std::string button_text, float button_scale, std::function<void()> func);
// Main menu title and background
Entity createMainMenu(RenderSystem* renderer, vec2 title_pos = { 0, 0 }, float title_scale = 1.f, vec2 background_pos = { 0, 0 }, float background_scale = 1.f);

// Pause menu background
Entity createPauseMenu(RenderSystem* renderer, vec2 background_pos = { 0,0 }, float background_scale = 1.f);
