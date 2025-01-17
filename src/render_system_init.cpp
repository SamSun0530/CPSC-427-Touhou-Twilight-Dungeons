// internal
#include "render_system.hpp"

#include <array>
#include <fstream>

#include "../ext/stb_image/stb_image.h"

// This creates circular header inclusion, that is quite bad.
#include "tiny_ecs_registry.hpp"

// stlib
#include <iostream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#include<algorithm>
#include<iterator>

// World initialization
bool RenderSystem::init(GLFWwindow* window_arg)
{
	this->window = window_arg;

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // vsync

	// Load OpenGL function pointers
	const int is_fine = gl3w_init();
	assert(is_fine == 0);

	// Create a frame buffer
	frame_buffer = 0;
	glGenFramebuffers(1, &frame_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();

	// For some high DPI displays (ex. Retina Display on Macbooks)
	// https://stackoverflow.com/questions/36672935/why-retina-screen-coordinate-value-is-twice-the-value-of-pixel-value
	int frame_buffer_width_px, frame_buffer_height_px;
	glfwGetFramebufferSize(window, &frame_buffer_width_px, &frame_buffer_height_px);  // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	if (frame_buffer_width_px != window_width_px)
	{
		printf("WARNING: retina display! https://stackoverflow.com/questions/36672935/why-retina-screen-coordinate-value-is-twice-the-value-of-pixel-value\n");
		printf("glfwGetFramebufferSize = %d,%d\n", frame_buffer_width_px, frame_buffer_height_px);
		printf("window width_height = %d,%d\n", window_width_px, window_height_px);
	}

	// Hint: Ask your TA for how to setup pretty OpenGL error callbacks. 
	// This can not be done in mac os, so do not enable
	// it unless you are on Linux or Windows. You will need to change the window creation
	// code to use OpenGL 4.3 (not suported on mac) and add additional .h and .cpp
	// glDebugMessageCallback((GLDEBUGPROC)errorCallback, nullptr);

	// We are not really using VAO's but without at least one bound we will crash in
	// some systems.
	glGenVertexArrays(1, &dummyVAO);
	glBindVertexArray(dummyVAO);

	gl_has_errors();

	initScreenTexture();
	initializeGlTextures();
	initializeGlEffects();
	initializeGlGeometryBuffers();

	initializeEnemyBulletInstance();
	initializeTileInstance();
	initializeVisibilityTileInstance();

	return true;
}

// Adapted from: https://learnopengl.com/Advanced-OpenGL/Instancing
void RenderSystem::initializeEnemyBulletInstance() {
	enemy_bullet_instance_program = glCreateProgram();

	// Load shaders
	std::string path = shader_path("textured_instance");

	const std::string vertex_shader_name = path + ".vs.glsl";
	const std::string fragment_shader_name = path + ".fs.glsl";

	bool is_valid = loadEffectFromFile(vertex_shader_name, fragment_shader_name, enemy_bullet_instance_program);
	assert(is_valid && (GLuint)enemy_bullet_instance_program != 0);

	glUseProgram(enemy_bullet_instance_program);
	gl_has_errors();

	glGenBuffers(1, &enemy_bullet_instance_VBO);
	glGenVertexArrays(1, &enemy_bullet_instance_VAO);
	gl_has_errors();

	// Instance vbo
	glBindBuffer(GL_ARRAY_BUFFER, enemy_bullet_instance_VBO);
	gl_has_errors();

	// Instance vao
	glBindVertexArray(enemy_bullet_instance_VAO);
	gl_has_errors();

	int loc = glGetAttribLocation(enemy_bullet_instance_program, "in_transform");
	assert(loc >= 0);
	gl_has_errors();
	int loc1 = loc;
	int loc2 = loc + 1;
	int loc3 = loc + 2;
	glEnableVertexAttribArray(loc1);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, sizeof(mat3), (void*)(0));
	gl_has_errors();
	glEnableVertexAttribArray(loc2);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, sizeof(mat3), (void*)(sizeof(vec3)));
	gl_has_errors();
	glEnableVertexAttribArray(loc3);
	glVertexAttribPointer(loc3, 3, GL_FLOAT, GL_FALSE, sizeof(mat3), (void*)(sizeof(vec3) * 2));
	gl_has_errors();
	glVertexAttribDivisor(loc1, 1);
	glVertexAttribDivisor(loc2, 1);
	glVertexAttribDivisor(loc3, 1);
	gl_has_errors();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void RenderSystem::initializeGlTextures()
{
	glGenTextures((GLsizei)texture_gl_handles.size(), texture_gl_handles.data());

	for (uint i = 0; i < texture_paths.size(); i++)
	{
		const std::string& path = texture_paths[i];
		ivec2& dimensions = texture_dimensions[i];

		stbi_uc* data;
		data = stbi_load(path.c_str(), &dimensions.x, &dimensions.y, NULL, 4);

		if (data == NULL)
		{
			const std::string message = "Could not load the file " + path + ".";
			fprintf(stderr, "%s", message.c_str());
			assert(false);
		}
		glBindTexture(GL_TEXTURE_2D, texture_gl_handles[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dimensions.x, dimensions.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		gl_has_errors();
		stbi_image_free(data);
	}
	gl_has_errors();
}

void RenderSystem::initializeGlEffects()
{
	for (uint i = 0; i < effect_paths.size(); i++)
	{
		const std::string vertex_shader_name = effect_paths[i] + ".vs.glsl";
		const std::string fragment_shader_name = effect_paths[i] + ".fs.glsl";

		bool is_valid = loadEffectFromFile(vertex_shader_name, fragment_shader_name, effects[i]);
		assert(is_valid && (GLuint)effects[i] != 0);
	}
}



// One could merge the following two functions as a template function...
template <class T>
void RenderSystem::bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices)
{
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(uint)gid]);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(vertices[0]) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	gl_has_errors();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffers[(uint)gid]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		sizeof(indices[0]) * indices.size(), indices.data(), GL_STATIC_DRAW);
	gl_has_errors();
}

bool is_leftturn(const vec3& p, const vec3& q, const vec3& r) {
	float val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
	return (val > 0);
}

float square_dist(const vec3& p1, const vec3& p2) {
	return (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y);
}


vec3 p0;
bool compare_angle(const ColoredVertex& v1, const ColoredVertex& v2) {
	vec3 p1 = v1.position;
	vec3 p2 = v2.position;
	bool isLeft = is_leftturn(p0, p1, p2);
	if (isLeft) {
		return true;
	}
	else if (!isLeft && !is_leftturn(p0, p2, p1)) {
		return square_dist(p0, p1) < square_dist(p0, p2);
	}
	return false;
}

void RenderSystem::initializeGlMeshes()
{
	for (uint i = 0; i < mesh_paths.size(); i++)
	{
		// Initialize meshes
		GEOMETRY_BUFFER_ID geom_index = mesh_paths[i].first;
		std::string name = mesh_paths[i].second;
		Mesh::loadFromOBJFile(name,
			meshes[(int)geom_index].vertices,
			meshes[(int)geom_index].vertex_indices,
			meshes[(int)geom_index].original_size);

		bindVBOandIBO(geom_index,
			meshes[(int)geom_index].vertices,
			meshes[(int)geom_index].vertex_indices);

		std::vector<ColoredVertex> colored_vertices = meshes[(int)geom_index].vertices;
		std::vector<ColoredVertex> ordered_vertices;
		std::copy(colored_vertices.begin(), colored_vertices.end(), std::back_inserter(ordered_vertices));

		// Graham's Scan algorithm for finding convex hulls in counter-clockwise order. Reference CPSC 420: Advanced Algorithms
		int n = ordered_vertices.size(), lowest_y = 0;
		for (int i = 1; i < n; i++) {
			if ((ordered_vertices[i].position.y < ordered_vertices[lowest_y].position.y) || (ordered_vertices[i].position.y == ordered_vertices[lowest_y].position.y && ordered_vertices[i].position.x < ordered_vertices[lowest_y].position.x))
				lowest_y = i;
		}
		std::swap(ordered_vertices[0], ordered_vertices[lowest_y]);

		p0 = ordered_vertices[0].position;
		std::sort(ordered_vertices.begin() + 1, ordered_vertices.end(), compare_angle);
		std::vector<vec3> stack;
		stack.push_back(ordered_vertices[0].position);
		stack.push_back(ordered_vertices[1].position);
		stack.push_back(ordered_vertices[2].position);

		for (int i = 3; i < n; i++) {
			while (stack.size() > 1 && !is_leftturn(*(stack.end() - 2), stack.back(), ordered_vertices[i].position))
				stack.pop_back();
			stack.push_back(ordered_vertices[i].position);
		}
		meshes[(int)geom_index].ordered_vertices = stack;
		//for (vec3& v : stack) {
		//	std::cout << v.x << "   " << v.y << std::endl;
		//}
	}
}

void RenderSystem::initializeGlGeometryBuffers()
{
	// Vertex Buffer creation.
	glGenBuffers((GLsizei)vertex_buffers.size(), vertex_buffers.data());
	// Index Buffer creation.
	glGenBuffers((GLsizei)index_buffers.size(), index_buffers.data());

	// Index and Vertex buffer data initialization.
	initializeGlMeshes();

	//////////////////////////
	// Initialize sprite
	// The position corresponds to the center of the texture.
	std::vector<TexturedVertex> textured_vertices(4);
	textured_vertices[0].position = { -1.f / 2, +1.f / 2, 0.f };
	textured_vertices[1].position = { +1.f / 2, +1.f / 2, 0.f };
	textured_vertices[2].position = { +1.f / 2, -1.f / 2, 0.f };
	textured_vertices[3].position = { -1.f / 2, -1.f / 2, 0.f };
	textured_vertices[0].texcoord = { 0.f, 1.f };
	textured_vertices[1].texcoord = { 1.f, 1.f };
	textured_vertices[2].texcoord = { 1.f, 0.f };
	textured_vertices[3].texcoord = { 0.f, 0.f };

	// Counterclockwise as it's the default opengl front winding direction.
	const std::vector<uint16_t> textured_indices = { 0, 3, 1, 1, 3, 2 };
	bindVBOandIBO(GEOMETRY_BUFFER_ID::SPRITE, textured_vertices, textured_indices);

	////////////////////////
	// Initialize egg
	std::vector<ColoredVertex> egg_vertices;
	std::vector<uint16_t> egg_indices;
	constexpr float z = -0.1f;
	constexpr int NUM_TRIANGLES = 62;

	for (int i = 0; i < NUM_TRIANGLES; i++) {
		const float t = float(i) * M_PI * 2.f / float(NUM_TRIANGLES - 1);
		egg_vertices.push_back({});
		egg_vertices.back().position = { 0.5 * cos(t), 0.5 * sin(t), z };
		egg_vertices.back().color = { 0.0, 0.8, 0.0 };
	}
	egg_vertices.push_back({});
	egg_vertices.back().position = { 0, 0, 0 };
	egg_vertices.back().color = { 1, 1, 1 };
	for (int i = 0; i < NUM_TRIANGLES; i++) {
		egg_indices.push_back((uint16_t)i);
		egg_indices.push_back((uint16_t)((i + 1) % NUM_TRIANGLES));
		egg_indices.push_back((uint16_t)NUM_TRIANGLES);
	}
	int geom_index = (int)GEOMETRY_BUFFER_ID::EGG;
	meshes[geom_index].vertices = egg_vertices;
	meshes[geom_index].vertex_indices = egg_indices;
	bindVBOandIBO(GEOMETRY_BUFFER_ID::EGG, meshes[geom_index].vertices, meshes[geom_index].vertex_indices);

	//////////////////////////////////
	// Initialize debug line
	std::vector<ColoredVertex> line_vertices;
	std::vector<uint16_t> line_indices;

	constexpr float depth = 0.5f;
	constexpr vec3 red = { 0.8,0.1,0.1 };
	constexpr vec3 white = { 1.0,1.0,1.0 };

	// Corner points
	//line_vertices = {
	//	{{-0.5,-0.5, depth}, red},
	//	{{-0.5, 0.5, depth}, red},
	//	{{ 0.5, 0.5, depth}, red},
	//	{{ 0.5,-0.5, depth}, red},
	//};	
	line_vertices = {
		{{-0.5,-0.5, depth}, white},
		{{-0.5, 0.5, depth}, white},
		{{ 0.5, 0.5, depth}, white},
		{{ 0.5,-0.5, depth}, white},
	};

	// Two triangles
	line_indices = { 0, 1, 3, 1, 2, 3 };

	geom_index = (int)GEOMETRY_BUFFER_ID::DEBUG_LINE;
	meshes[geom_index].vertices = line_vertices;
	meshes[geom_index].vertex_indices = line_indices;
	bindVBOandIBO(GEOMETRY_BUFFER_ID::DEBUG_LINE, line_vertices, line_indices);

	///////////////////////////////////////////////////////
	// Initialize screen triangle (yes, triangle, not quad; its more efficient).
	std::vector<vec3> screen_vertices(3);
	screen_vertices[0] = { -1, -6, 0.f };
	screen_vertices[1] = { 6, -1, 0.f };
	screen_vertices[2] = { -1, 6, 0.f };

	// Counterclockwise as it's the default opengl front winding direction.
	const std::vector<uint16_t> screen_indices = { 0, 1, 2 };
	bindVBOandIBO(GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE, screen_vertices, screen_indices);
}

RenderSystem::~RenderSystem()
{
	// Don't need to free gl resources since they last for as long as the program,
	// but it's polite to clean after yourself.
	glDeleteBuffers((GLsizei)vertex_buffers.size(), vertex_buffers.data());
	glDeleteBuffers((GLsizei)index_buffers.size(), index_buffers.data());
	glDeleteTextures((GLsizei)texture_gl_handles.size(), texture_gl_handles.data());
	glDeleteTextures(1, &off_screen_render_buffer_color);
	glDeleteRenderbuffers(1, &off_screen_render_buffer_depth);
	gl_has_errors();

	for (uint i = 0; i < effect_count; i++) {
		glDeleteProgram(effects[i]);
	}
	// delete allocated resources
	glDeleteFramebuffers(1, &frame_buffer);
	gl_has_errors();

	// remove all entities created by the render system
	while (registry.renderRequests.entities.size() > 0)
		registry.remove_all_components_of(registry.renderRequests.entities.back());
}

// Initialize the screen texture from a standard sprite
bool RenderSystem::initScreenTexture()
{
	registry.screenStates.emplace(screen_state_entity);

	int framebuffer_width, framebuffer_height;
	glfwGetFramebufferSize(const_cast<GLFWwindow*>(window), &framebuffer_width, &framebuffer_height);  // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

	glGenTextures(1, &off_screen_render_buffer_color);
	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, framebuffer_width, framebuffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl_has_errors();

	glGenRenderbuffers(1, &off_screen_render_buffer_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, off_screen_render_buffer_depth);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, off_screen_render_buffer_color, 0);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, framebuffer_width, framebuffer_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, off_screen_render_buffer_depth);
	gl_has_errors();

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	return true;
}

// Font initiation
// This is adapted from lecture material (Wednesday Feb 28th 2024)
bool RenderSystem::initFont(GLFWwindow* window, const std::string& font_filename, unsigned int font_default_size) {
	// font buffer setup
	glGenVertexArrays(1, &m_font_VAO);
	glGenBuffers(1, &m_font_VBO);

	// font vertex shader
	unsigned int font_vertexShader;
	font_vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(font_vertexShader, 1, &fontVertexShaderSource, NULL);
	glCompileShader(font_vertexShader);

	// font fragement shader
	unsigned int font_fragmentShader;
	font_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(font_fragmentShader, 1, &fontFragmentShaderSource, NULL);
	glCompileShader(font_fragmentShader);

	// font shader program
	m_font_shaderProgram = glCreateProgram();
	glAttachShader(m_font_shaderProgram, font_vertexShader);
	glAttachShader(m_font_shaderProgram, font_fragmentShader);
	glLinkProgram(m_font_shaderProgram);

	// clean up shaders
	glDeleteShader(font_vertexShader);
	glDeleteShader(font_fragmentShader);

	// use our new shader
	glUseProgram(m_font_shaderProgram);

	// apply projection matrix for font
	glm::mat3 projection = createProjectionMatrix();
	GLint project_location = glGetUniformLocation(m_font_shaderProgram, "projection");
	assert(project_location > -1);
	glUniformMatrix3fv(project_location, 1, GL_FALSE, glm::value_ptr(projection));

	// init FreeType fonts
	FT_Library ft;
	if (FT_Init_FreeType(&ft))
	{
		std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
		return false;
	}

	FT_Face face;
	if (FT_New_Face(ft, font_filename.c_str(), 0, &face))
	{
		std::cerr << "ERROR::FREETYPE: Failed to load font: " << font_filename << std::endl;
		return false;
	}

	// extract a default size
	FT_Set_Pixel_Sizes(face, 0, font_default_size);

	// disable byte-alignment restriction in OpenGL
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// load each of the chars - note only first 128 ASCII chars
	for (unsigned char c = 0; c < 128; c++)
	{
		// load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			std::cerr << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}

		// generate texture
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		// std::cout << "texture: " << c << " = " << texture << std::endl;

		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);

		// set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// now store character for later use
		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			static_cast<unsigned int>(face->glyph->advance.x),
			c
		};
		m_ftCharacters.insert(std::pair<char, Character>(c, character));
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	// clean up
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	// bind buffers
	glBindVertexArray(m_font_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_font_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

	// release buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return true;
}

bool gl_compile_shader(GLuint shader)
{
	glCompileShader(shader);
	gl_has_errors();
	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint log_len;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
		std::vector<char> log(log_len);
		glGetShaderInfoLog(shader, log_len, &log_len, log.data());
		glDeleteShader(shader);

		gl_has_errors();

		fprintf(stderr, "GLSL: %s", log.data());
		return false;
	}

	return true;
}

bool loadEffectFromFile(
	const std::string& vs_path, const std::string& fs_path, GLuint& out_program)
{
	// Opening files
	std::ifstream vs_is(vs_path);
	std::ifstream fs_is(fs_path);
	if (!vs_is.good() || !fs_is.good())
	{
		fprintf(stderr, "Failed to load shader files %s, %s", vs_path.c_str(), fs_path.c_str());
		assert(false);
		return false;
	}

	// Reading sources
	std::stringstream vs_ss, fs_ss;
	vs_ss << vs_is.rdbuf();
	fs_ss << fs_is.rdbuf();
	std::string vs_str = vs_ss.str();
	std::string fs_str = fs_ss.str();
	const char* vs_src = vs_str.c_str();
	const char* fs_src = fs_str.c_str();
	GLsizei vs_len = (GLsizei)vs_str.size();
	GLsizei fs_len = (GLsizei)fs_str.size();

	GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vs_src, &vs_len);
	gl_has_errors();
	GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fs_src, &fs_len);
	gl_has_errors();

	// Compiling
	if (!gl_compile_shader(vertex))
	{
		fprintf(stderr, "Vertex compilation failed");
		assert(false);
		return false;
	}
	if (!gl_compile_shader(fragment))
	{
		fprintf(stderr, "Fragment compilation failed");
		assert(false);
		return false;
	}

	// Linking
	out_program = glCreateProgram();
	gl_has_errors();
	glAttachShader(out_program, vertex);
	gl_has_errors();
	glAttachShader(out_program, fragment);
	gl_has_errors();
	glLinkProgram(out_program);
	gl_has_errors();

	{
		GLint is_linked = GL_FALSE;
		glGetProgramiv(out_program, GL_LINK_STATUS, &is_linked);
		gl_has_errors();
		if (is_linked == GL_FALSE)
		{
			GLint log_len;
			glGetProgramiv(out_program, GL_INFO_LOG_LENGTH, &log_len);
			gl_has_errors();
			std::vector<char> log(log_len);
			glGetProgramInfoLog(out_program, log_len, &log_len, log.data());
			gl_has_errors();

			fprintf(stderr, "Link error: %s", log.data());
			assert(false);
			return false;
		}
	}

	// No need to carry this around. Keeping these objects is only useful if we recycle
	// the same shaders over and over, which we don't, so no need and this is simpler.
	glDetachShader(out_program, vertex);
	gl_has_errors();
	glDetachShader(out_program, fragment);
	gl_has_errors();
	glDeleteShader(vertex);
	gl_has_errors();
	glDeleteShader(fragment);
	gl_has_errors();

	return true;
}

void RenderSystem::initializeTileInstance() {
	tile_instance_program = glCreateProgram();

	// Load shaders
	std::string path = shader_path("tiles_instance");

	const std::string vertex_shader_name = path + ".vs.glsl";
	const std::string fragment_shader_name = path + ".fs.glsl";

	bool is_valid = loadEffectFromFile(vertex_shader_name, fragment_shader_name, tile_instance_program);
	assert(is_valid && (GLuint)tile_instance_program != 0);

	glUseProgram(tile_instance_program);
	gl_has_errors();

	glGenBuffers(1, &tiles_instance_VBO);
	glGenVertexArrays(1, &tiles_instance_VAO);
	gl_has_errors();

	glBindVertexArray(tiles_instance_VAO);
	gl_has_errors();

	// bind vbo for textured vertex
	const GLuint vbo = vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SPRITE];
	const GLuint ibo = index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SPRITE];

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	GLint in_position_loc = glGetAttribLocation(tile_instance_program, "in_position");
	GLint in_texcoord_loc = glGetAttribLocation(tile_instance_program, "in_texcoord");
	assert(in_position_loc >= 0);
	assert(in_texcoord_loc >= 0);

	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)0);
	gl_has_errors();

	glEnableVertexAttribArray(in_texcoord_loc);
	// note the stride to skip the preceeding vertex position
	glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)sizeof(vec3));

	// bind tiles instance vbo for instance rendering
	glBindBuffer(GL_ARRAY_BUFFER, tiles_instance_VBO);
	gl_has_errors();

	// sprite location data
	int sprite_loc = glGetAttribLocation(tile_instance_program, "in_sprite_loc");
	assert(sprite_loc >= 0);
	gl_has_errors();
	glEnableVertexAttribArray(sprite_loc);
	glVertexAttribPointer(sprite_loc, 4, GL_FLOAT, GL_FALSE, sizeof(TileInstanceData), (void*)(0));
	gl_has_errors();
	glVertexAttribDivisor(sprite_loc, 1);
	gl_has_errors();

	// transform data
	int loc = glGetAttribLocation(tile_instance_program, "in_transform");
	assert(loc >= 0);
	gl_has_errors();
	int loc1 = loc;
	int loc2 = loc + 1;
	int loc3 = loc + 2;
	glEnableVertexAttribArray(loc1);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, sizeof(TileInstanceData), (void*)(sizeof(vec4)));
	gl_has_errors();
	glEnableVertexAttribArray(loc2);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, sizeof(TileInstanceData), (void*)(sizeof(vec4) + sizeof(vec3)));
	gl_has_errors();
	glEnableVertexAttribArray(loc3);
	glVertexAttribPointer(loc3, 3, GL_FLOAT, GL_FALSE, sizeof(TileInstanceData), (void*)(sizeof(vec4) + sizeof(vec3) * 2));
	gl_has_errors();
	glVertexAttribDivisor(loc1, 1);
	glVertexAttribDivisor(loc2, 1);
	glVertexAttribDivisor(loc3, 1);
	gl_has_errors();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	gl_has_errors();
}

void RenderSystem::initializeVisibilityTileInstance() {
	visibility_tile_instance_program = glCreateProgram();

	// Load shaders
	std::string path = shader_path("visibilitytiles_instance");

	const std::string vertex_shader_name = path + ".vs.glsl";
	const std::string fragment_shader_name = path + ".fs.glsl";

	bool is_valid = loadEffectFromFile(vertex_shader_name, fragment_shader_name, visibility_tile_instance_program);
	assert(is_valid && (GLuint)visibility_tile_instance_program != 0);

	glUseProgram(visibility_tile_instance_program);
	gl_has_errors();

	glGenBuffers(1, &visibility_tile_instance_VBO);
	glGenVertexArrays(1, &visibility_tile_instance_VAO);
	gl_has_errors();

	glBindVertexArray(visibility_tile_instance_VAO);
	gl_has_errors();

	// bind vbo for colored vertex
	const GLuint vbo = vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::DEBUG_LINE];
	const GLuint ibo = index_buffers[(GLuint)GEOMETRY_BUFFER_ID::DEBUG_LINE];

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	// Input data location as in the vertex buffer
	GLint in_position_loc = glGetAttribLocation(visibility_tile_instance_program, "in_position");
	GLint in_color_loc = glGetAttribLocation(visibility_tile_instance_program, "in_color");
	gl_has_errors();
	assert(in_position_loc >= 0);
	assert(in_color_loc >= 0);

	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (void*)0);
	gl_has_errors();

	glEnableVertexAttribArray(in_color_loc);
	glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (void*)sizeof(vec3));
	gl_has_errors();

	// bind visbility tiles instance vbo for instance rendering
	glBindBuffer(GL_ARRAY_BUFFER, visibility_tile_instance_VBO);
	gl_has_errors();

	// transform data
	int loc = glGetAttribLocation(visibility_tile_instance_program, "in_transform");
	assert(loc >= 0);
	gl_has_errors();
	int loc1 = loc;
	int loc2 = loc + 1;
	int loc3 = loc + 2;
	glEnableVertexAttribArray(loc1);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, sizeof(VisibilityTileInstanceData), (void*)(0));
	gl_has_errors();
	glEnableVertexAttribArray(loc2);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, sizeof(VisibilityTileInstanceData), (void*)(sizeof(vec3)));
	gl_has_errors();
	glEnableVertexAttribArray(loc3);
	glVertexAttribPointer(loc3, 3, GL_FLOAT, GL_FALSE, sizeof(VisibilityTileInstanceData), (void*)(sizeof(vec3) * 2));
	gl_has_errors();
	glVertexAttribDivisor(loc1, 1);
	glVertexAttribDivisor(loc2, 1);
	glVertexAttribDivisor(loc3, 1);
	gl_has_errors();

	// neighbors data
	GLint in_alpha_loc = glGetAttribLocation(visibility_tile_instance_program, "in_alpha");
	gl_has_errors();
	assert(in_alpha_loc >= 0);
	glEnableVertexAttribArray(in_alpha_loc);
	glVertexAttribPointer(in_alpha_loc, 1, GL_FLOAT, GL_FALSE, sizeof(VisibilityTileInstanceData), (void*)(sizeof(vec3) * 3));
	gl_has_errors();
	glVertexAttribDivisor(in_alpha_loc, 1);
	gl_has_errors();


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	gl_has_errors();
}

