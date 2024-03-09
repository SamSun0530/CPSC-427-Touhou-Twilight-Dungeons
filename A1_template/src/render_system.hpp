#pragma once

#include <array>
#include <utility>

#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs.hpp"
#include "camera.hpp"
#include "ui.hpp"
//#include "world_system.hpp"
#
#include <map>

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
			textures_path("NormalTile1.png"),
			textures_path("NormalTile2.png"),
			textures_path("InnerWall.png"),
			textures_path("TopWall.png"),
			textures_path("Door.png"),
			textures_path("DoorOpen.png"),
			textures_path("LeftWall.png"),
			textures_path("RightWall.png"),
			textures_path("LeftTopCorner.png"),
			textures_path("LeftBottomCorner.png"),
			textures_path("RightTopCorner.png"),
			textures_path("RightBottomCorner.png"),
			textures_path("FullHeart.png"),
			textures_path("HalfHeart.png"),
			textures_path("EmptyHeart.png"),
			textures_path("BottomWall.png"),
			textures_path("WallEdge.png"),
			textures_path("WallSurface.png"),
			textures_path("Pillar-Top.png"),
			textures_path("Pillar-Bottom.png"),
			textures_path("Health+1.png"),
			textures_path("Health+2.png"),
			textures_path("RegenerateHealth.png")
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
		shader_path("font")
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

	// Draw all entities
	void draw();

	mat3 createProjectionMatrix();

	// Camera for managing view matrix
	Camera camera;
	UI ui;

	// font initialization
	bool initFont(GLFWwindow* window, const std::string& font_filename, unsigned int font_default_size);

	GLuint getDummyVAO() const {
		return dummyVAO;
	}

	void renderText(const std::string& text, float x, float y, float scale, const glm::vec3& color, const glm::mat4& trans);
private:
	// Internal drawing functions for each entity type
	void drawTexturedMesh(Entity entity, const mat3& projection, const mat3& view, const mat3& view_ui);
	void drawToScreen();

	// Window handle
	GLFWwindow* window;

	// Screen texture handles
	GLuint frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;

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
		"uniform mat4 projection; \n"
		"uniform mat4 transform;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    gl_Position = projection * transform * vec4(vertex.xy, 0.0, 1.0); \n"
		"    TexCoords = vertex.zw; \n"
		"}\0";

	const char* fontFragmentShaderSource =
		"#version 330 core\n"
		"in vec2 TexCoords; \n"
		"out vec4 color; \n"
		"\n"
		"uniform sampler2D text; \n"
		"uniform vec3 textColor; \n"
		"\n"
		"void main()\n"
		"{\n"
		"    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);\n"
		"    color = vec4(textColor, 1.0) * sampled;\n"
		"}\0";

	Entity screen_state_entity;
};

bool loadEffectFromFile(
	const std::string& vs_path, const std::string& fs_path, GLuint& out_program);
