#pragma once

#include <array>
#include <utility>
#include <iostream>
#include <map>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>

#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs.hpp"
#include "camera.hpp"
#include "ui.hpp"
#include "tiny_ecs_registry.hpp"

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem {
	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count> texture_dimensions;

	// Make sure these paths remain in sync with the associated enumerators.
	// Associated id with .obj path
	const std::vector < std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths =
	{
		// specify meshes of other assets here
		 std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::REIMU_FRONT, mesh_path("Reimu-Mesh-Front.obj")),
		 std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::REIMU_LEFT, mesh_path("Reimu-Mesh-Left.obj")),
		 std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::REIMU_RIGHT, mesh_path("Reimu-Mesh-Right.obj"))
	};

	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, texture_count> texture_paths = {
			textures_path("Reimu-Bullet.png"),
			textures_path("Bee-Figure.png"),
			textures_path("Reimu-Figure.png"),
			textures_path("Wolf-Figure.png"),
			textures_path("Bomber-Figure.png"),
			textures_path("Enemy-Bullet.png"),
			textures_path("Rock_1.png"),
			textures_path("Reimu_HealthBar.png"),
			textures_path("Reimu_Head.png"),
			textures_path("Health+1.png"),
			textures_path("Health+2.png"),
			textures_path("RegenerateHealth.png"),
			textures_path("Cirno-Figure.png"),
			textures_path("BossHealthBar.png"),
			textures_path("Reimu-Bullet-Disappear.png"),
			textures_path("FocusDot.png"),
			textures_path("keyboard.png"),
			textures_path("Coins.png"),
			textures_path("AttackDemage.png"),
			textures_path("AttackSpeed.png"),
			textures_path("CriticalChance.png"),
			textures_path("CriticalDemage.png"),
			textures_path("Bomb.png"),
			textures_path("CriticalHitIcon.png"),
			textures_path("FocusBar.png"),
			textures_path("Coins-Static.png"),
			textures_path("TileAtlasSandstone.png"),
			textures_path("TileAtlasRuins.png"),
			textures_path("Button.png"),
			textures_path("ButtonHover.png"),
			textures_path("None.png"),
			textures_path("MainMenuTitle.png"),
			textures_path("MainMenuBackground.png"),
			textures_path("PauseMenuBackground.png"),
			textures_path("C.png"),
			textures_path("B.png"),
			textures_path("A.png"),
			textures_path("S.png"),
			textures_path("item_red.png"),
			textures_path("item_green.png"),
			textures_path("item_blue.png"),
			textures_path("item_yellow.png"),
			textures_path("item_purple.png"),
			textures_path("DoorHorizontalOpen.png"),
			textures_path("DoorHorizontalClose.png"),
			textures_path("DoorVerticalOpenDown.png"),
			textures_path("DoorVerticalOpenUp.png"),
			textures_path("DoorVerticalCloseDown.png"),
			textures_path("DoorVerticalcloseUp.png"),
			textures_path("reimu_portrait.png"),
			textures_path("cirno_portrait.png"),
			textures_path("flandre_portrait.png"),
			textures_path("marisa_portrait.png"),
			textures_path("remilia_portrait.png"),
			textures_path("sakuya_portrait.png"),
			textures_path("DialogueBox.png"),
			textures_path("Teleporter.png"),
			textures_path("WinDeathMenu.png"),
			textures_path("Flandre-Figure.png"),
			textures_path("Flandre-Bullet.png"),
			textures_path("Infographic.png"),
			textures_path("Marisa-Figure.png"),
			textures_path("Lizard-Figure.png"),
			textures_path("Worm-Figure.png"),
			textures_path("Bee2-Figure.png"),
			textures_path("Gargoyle-Figure.png"),
			textures_path("AimbotCursor.png"),
			textures_path("explosions/AOEAmmoExplosion.png"),
			textures_path("AoeAmmoBullet.png"),
			textures_path("AimbotAmmoBullet.png"),
			textures_path("AoeAmmoBulletDisappear.png"),
			textures_path("AimbotAmmoBulletDisappear.png"),
			textures_path("HitSpark.png"),
			textures_path("ChestOpened.png"),
			textures_path("ChestClosed.png"),
			textures_path("Skeleton.png"),
			textures_path("Barrel.png"),
			textures_path("Pottery.png"),
			textures_path("Fireplace.png"),
			textures_path("AimbotAmmoItem.png"),
			textures_path("AoeAmmoItem.png"),
			textures_path("TripleAmmoItem.png"),
			textures_path("Aimbot1BulletAmmoItem.png"),
			textures_path("CirnoAura.png"),
			textures_path("FlandreAura.png"),
			textures_path("TeleporterSmall.png"),
			textures_path("NormalSign.png"),
			textures_path("BossSign.png"),
			textures_path("StartSign.png"),
			textures_path("ShopSign.png"),
			textures_path("Parralex.png"),
			textures_path("TileAtlasWater.png"),
			textures_path("TileAtlasSky.png"),
			textures_path("Sakuya-Figure.png"),
			textures_path("Remilia-Figure.png"),
			textures_path("Cloud.png"),
			textures_path("SakuyaAura.png"),
			textures_path("RemiliaAura.png"),
			textures_path("Sakuya-Bullet.png"),
			textures_path("Remilia-Bullet.png"),
			textures_path("ReimuCry.png"),
			textures_path("Turtle-Figure.png"),
			textures_path("Skeleton-Figure.png"),
			textures_path("Seagull-Figure.png"),
			textures_path("TombstoneBroken.png"),
			textures_path("Cylinder1.png"),
			textures_path("Cylinder2.png"),
			textures_path("PillarTopBroken.png"),
			textures_path("PillarTop.png"),
			textures_path("PillarMiddle.png"),
			textures_path("PillarBottom.png"),
			textures_path("SkyTree0.png"),
			textures_path("SkyTree1.png"),
			textures_path("SkyTree2.png"),
			textures_path("SkyTree3.png"),
			textures_path("SkyTree4.png"),
			textures_path("SkyTree5.png"),
			textures_path("SkyTree6.png"),
			textures_path("SkyTree7.png"),
			textures_path("SkyTree8.png"),
			textures_path("Log.png"),
			textures_path("LogMushroom.png"),
			textures_path("RockMoss0.png"),
			textures_path("RockMoss1.png"),
			textures_path("RockMoss2.png"),
			textures_path("Crates.png"),
			textures_path("CratesSmall.png"),
			textures_path("RuinsPillarTop.png"),
			textures_path("RuinsPillarBottom.png"),
			textures_path("RuinsPillarLeft.png"),
			textures_path("RuinsPillarRight.png"),
	};

	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = {
		shader_path("coloured"),
		shader_path("egg"),
		shader_path("player"),
		shader_path("textured"),
		shader_path("wind"),
		shader_path("ui"),
		shader_path("font"),
		shader_path("playerhealthbar"),
		shader_path("bosshealthbar"),
		shader_path("combo"),
		shader_path("grey"),
	};

	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;

public:
	// Initialize the window
	bool init(GLFWwindow* window);

	template <class T>
	void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

	void initializeGlTextures();

	void initializeGlEffects();

	void initializeGlMeshes();
	Mesh& getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

	void initializeGlGeometryBuffers();
	// Initialize the screen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the wind
	// shader
	bool initScreenTexture();

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	void render_buttons(glm::mat3& projection_2D, glm::mat3& view_2D, glm::mat3& view_2D_ui, MENU_STATE state);

	// Draw all entities
	void draw();

	mat3 createProjectionMatrix();

	// Camera for managing view matrix
	Camera camera;
	UI ui;

	// font initialization
	bool initFont(GLFWwindow* window, const std::string& font_filename, unsigned int font_default_size);

	// extra parameter in_world specifies whether or not text should be world or screen coordinate
	void renderText(const std::string& text, float x, float y, float scale, const glm::vec3& color, const glm::mat3& trans, bool in_world = false, float transparency_rate = 1);

	// tiles instancing
	// called once when generating new map
	void set_tiles_instance_buffer();
	// called when visibility map is decreased
	void set_visibility_tiles_instance_buffer();
	// called once when generating new map
	void set_visibility_tiles_instance_buffer_max();
	// get sprite location of tile name sandstone atlas
	vec4 get_spriteloc(TILE_NAME tile_name);
	// if is_close true, switch to closed texture, otherwise open texture
	void switch_door_texture(Entity door_entity, bool is_close);
private:
	// Internal drawing functions for each entity type
	void drawTexturedMesh(Entity entity, const mat3& projection, const mat3& view, const mat3& view_ui);
	void drawBulletsInstanced(const std::vector<Entity>& entities, const glm::mat3& projection, const glm::mat3& view);
	void drawTilesInstanced(const glm::mat3& projection, const glm::mat3& view);
	void drawVisibilityTilesInstanced(const glm::mat3& projection, const glm::mat3& view);
	void drawToScreen();

	// Window handle
	GLFWwindow* window;

	// Screen texture handles
	GLuint frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;

	// Enemy bullet instancing
	void initializeEnemyBulletInstance();
	GLuint enemy_bullet_instance_program;
	GLuint enemy_bullet_instance_VAO;
	GLuint enemy_bullet_instance_VBO;

	// Tile instancing
	void initializeTileInstance();
	GLuint tile_instance_program;
	GLuint tiles_instance_VAO;
	GLuint tiles_instance_VBO;

	// Visibility tile instancing
	void initializeVisibilityTileInstance();
	GLuint visibility_tile_instance_program;
	GLuint visibility_tile_instance_VAO;
	GLuint visibility_tile_instance_VBO;

	// Fonts
	std::map<char, Character> m_ftCharacters;
	GLuint m_font_shaderProgram;
	GLuint m_font_VAO;
	GLuint m_font_VBO;
	GLuint dummyVAO;

	glm::mat4 trans = glm::mat4(1.0f);

	const char* fontVertexShaderSource =
		"#version 330 core\n"
		"layout(location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>\n"
		"out vec2 TexCoords; \n"
		"\n"
		"uniform mat3 projection; \n"
		"uniform mat3 transform;\n"
		"uniform mat3 view;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    gl_Position = vec4(projection * view * transform * vec3(vertex.xy, 1.0), 1.0); \n"
		"    TexCoords = vertex.zw; \n"
		"}\0";

	const char* fontFragmentShaderSource =
		"#version 330 core\n"
		"in vec2 TexCoords;\n"
		"out vec4 color;\n"
		"\n"
		"uniform sampler2D text;\n"
		"uniform vec3 textColor;\n"
		"uniform float transparency;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);\n"
		"    vec4 finalColor = vec4(textColor, 1.0) * sampled;\n"
		"    finalColor.a *= transparency; // Adjust alpha with transparency\n"
		"    color = finalColor;\n"
		"}\n";

	Entity screen_state_entity;

	// Utilities/Helper functions
	void get_strings_delim(const std::string& input, char delim, std::vector<std::string>& output);
	void render_text_newline(const std::string& text, float x, float y, float scale, const glm::vec3& color, const glm::mat3& trans, bool in_world, float padding_y = 25.f, float transparency = 1.f);
};

bool loadEffectFromFile(
	const std::string& vs_path, const std::string& fs_path, GLuint& out_program);
