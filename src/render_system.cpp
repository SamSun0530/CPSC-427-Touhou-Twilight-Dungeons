// internal
#include "render_system.hpp"
#include "world_system.hpp"
#include <SDL.h>

// Helper function to get vector of strings separated by delimiter of input string
// Adapted from: https://stackoverflow.com/a/10058725
void RenderSystem::get_strings_delim(const std::string& input, char delim, std::vector<std::string>& output) {
	std::stringstream ss(input);
	std::string segment;
	while (std::getline(ss, segment, delim))
	{
		output.push_back(segment);
	}
}

// Helper function to render text with new lines
void RenderSystem::render_text_newline(const std::string& text, float x, float y, float scale, const glm::vec3& color, const glm::mat3& trans, bool in_world, float padding_y, float transparency) {
	// Prevent having to split string if there are no new lines
	if (text.find('\n') == std::string::npos) {
		renderText(text, x, y, scale, color, trans, in_world, transparency);
		return;
	}
	std::vector<std::string> segments;
	get_strings_delim(text, '\n', segments);
	// Either use x, y OR Transform t. t.mat will not work together with x, y
	//Transform t;
	//t.mat = trans;
	//vec2 t_translate = { 0, scale * padding_y };
	int segments_size = segments.size();
	for (int i = 0; i < segments_size; ++i) {
		renderText(segments[i], x, y + i * scale * padding_y, scale, color, trans, in_world, transparency);
		//t.translate(t_translate);
	}
}

void RenderSystem::drawTexturedMesh(Entity entity,
	const mat3& projection,
	const mat3& view,
	const mat3& view_ui) {
	Motion& motion = registry.motions.get(entity);
	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	Transform transform;
	transform.translate(motion.position);
	transform.rotate(motion.angle);
	transform.scale(motion.scale);

	//assert(registry.renderRequests.has(entity));
	//const RenderRequest &render_request = registry.renderRequests.get(entity);
	RenderRequest* render_request;
	if (registry.renderRequests.has(entity)) {
		render_request = &registry.renderRequests.get(entity);
	}
	else if (registry.renderRequestsForeground.has(entity)) {
		render_request = &registry.renderRequestsForeground.get(entity);
	}
	else {
		assert(false && "Entity not contained in ECS registry");
	}

	const GLuint used_effect_enum = (GLuint)(*render_request).used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();

	assert((*render_request).used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)(*render_request).used_geometry];
	const GLuint ibo = index_buffers[(GLuint)(*render_request).used_geometry];

	// Setting vertex and index buffers
	glBindVertexArray(dummyVAO);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	if ((*render_request).used_effect == EFFECT_ASSET_ID::TEXTURED ||
		(*render_request).used_effect == EFFECT_ASSET_ID::UI ||
		(*render_request).used_effect == EFFECT_ASSET_ID::BOSSHEALTHBAR ||
		(*render_request).used_effect == EFFECT_ASSET_ID::PLAYER_HB ||
		(*render_request).used_effect == EFFECT_ASSET_ID::PLAYER ||
		(*render_request).used_effect == EFFECT_ASSET_ID::COMBO ||
		(*render_request).used_effect == EFFECT_ASSET_ID::GREY)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(TexturedVertex), (void*)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void*)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		//assert(registry.renderRequests.has(entity));
		//GLuint texture_id =
		//	texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];
		GLuint texture_id;
		if (registry.renderRequests.has(entity)) {
			texture_id = texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];
		}
		else if (registry.renderRequestsForeground.has(entity)) {
			texture_id = texture_gl_handles[(GLuint)registry.renderRequestsForeground.get(entity).used_texture];
		}
		else {
			assert(false && "Entity not contained in ECS registry");
		}

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();
		if ((*render_request).used_effect == EFFECT_ASSET_ID::PLAYER_HB && (*render_request).used_texture == TEXTURE_ASSET_ID::REIMU_HEALTH) {
			assert(registry.players.entities.size() == 1);
			GLint health_uloc = glGetUniformLocation(program, "health_percentage");
			const HP& player_hp = registry.hps.get(registry.players.entities[0]);
			float health_percentage = (float)player_hp.curr_hp / player_hp.max_hp;
			glUniform1f(health_uloc, health_percentage);
			gl_has_errors();
		}
		else if ((*render_request).used_effect == EFFECT_ASSET_ID::PLAYER_HB && (*render_request).used_texture == TEXTURE_ASSET_ID::FOCUS_BAR) {
			assert(registry.players.entities.size() == 1);
			GLint health_uloc = glGetUniformLocation(program, "health_percentage");
			//float health_percentage = registry.invulnerableTimers.has(registry.players.entities[0]) ? (float)registry.invulnerableTimers.get(registry.players.entities[0]).invulnerable_counter_ms / registry.players.components[0].invulnerability_time_ms : 1.f;
			float health_percentage = (float)focus_mode.counter_ms / focus_mode.max_counter_ms;
			glUniform1f(health_uloc, health_percentage);
			gl_has_errors();
		}
		else if ((*render_request).used_effect == EFFECT_ASSET_ID::BOSSHEALTHBAR && registry.bossHealthBarUIs.has(entity)) {
			BossHealthBarLink& link = registry.bossHealthBarLink.get(entity);
			GLint health_uloc = glGetUniformLocation(program, "health_percentage");
			const HP* boss_hp = registry.hps.has(link.other) ? &registry.hps.get(link.other) : nullptr;
			float health_percentage = boss_hp ? (float)boss_hp->curr_hp / boss_hp->max_hp : 0.f;
			glUniform1f(health_uloc, health_percentage);
			gl_has_errors();
		}
		else if ((*render_request).used_effect == EFFECT_ASSET_ID::PLAYER)
		{
			GLint focus_uloc = glGetUniformLocation(program, "focus_mode_alpha");
			// there will only be one focus dot
			float focus_mode_alpha = registry.focusdots.entities.size() > 0 ? 0.5f : 1.0f;
			glUniform1f(focus_uloc, focus_mode_alpha);
			gl_has_errors();

			// allow hit timer to change color before being transparent
			GLint invul_uloc = glGetUniformLocation(program, "invul_timer");
			float invul_timer = !registry.hitTimers.has(entity) && registry.invulnerableTimers.has(entity) ? registry.invulnerableTimers.get(entity).invulnerable_counter_ms : 0.f;
			glUniform1f(invul_uloc, invul_timer);
			gl_has_errors();
		}
		float strength = 0.2f;
		if ((*render_request).used_effect == EFFECT_ASSET_ID::COMBO && (*render_request).used_texture == TEXTURE_ASSET_ID::C) {
			strength = 0.2f;
		}
		else if ((*render_request).used_effect == EFFECT_ASSET_ID::COMBO && (*render_request).used_texture == TEXTURE_ASSET_ID::B) {
			strength = 0.3f;
		}
		else if ((*render_request).used_effect == EFFECT_ASSET_ID::COMBO && (*render_request).used_texture == TEXTURE_ASSET_ID::A) {
			strength = 0.4f;
		}
		else if ((*render_request).used_effect == EFFECT_ASSET_ID::COMBO && (*render_request).used_texture == TEXTURE_ASSET_ID::S) {
			strength = 0.5f;
		}
		GLint strength_uloc = glGetUniformLocation(program, "strength");
		glUniform1f(strength_uloc, strength);
		gl_has_errors();
	}
	else if ((*render_request).used_effect == EFFECT_ASSET_ID::EGG)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_color_loc = glGetAttribLocation(program, "in_color");
		gl_has_errors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(ColoredVertex), (void*)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(ColoredVertex), (void*)sizeof(vec3));
		gl_has_errors();
	}
	else
	{
		assert(false && "Type of render request not supported");
	}

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
	glUniform3fv(color_uloc, 1, (float*)&color);
	gl_has_errors();

	GLint end_pos_uloc = glGetUniformLocation(program, "end_pos");
	vec2 end_pos = vec2(1);
	if (registry.animation.has(entity)) {
		end_pos = registry.animation.get(entity).render_pos;
	}
	else if (registry.alwaysplayAni.has(entity)) {
		end_pos = registry.alwaysplayAni.get(entity).render_pos;
	}
	glUniform2fv(end_pos_uloc, 1, (float*)&end_pos);
	gl_has_errors();

	GLint scale_uloc = glGetUniformLocation(program, "scale");

	vec2 ani_scale = vec2(1);
	if (registry.animation.has(entity)) {
		ani_scale = registry.animation.get(entity).spritesheet_scale;
	}
	else if (registry.alwaysplayAni.has(entity)) {
		ani_scale = registry.alwaysplayAni.get(entity).spritesheet_scale;
	}
	glUniform2fv(scale_uloc, 1, (float*)&ani_scale);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float*)&transform.mat);
	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&projection);
	GLuint view_loc = glGetUniformLocation(currProgram, "view");
	glUniformMatrix3fv(view_loc, 1, GL_FALSE, (float*)&view);
	GLuint view_loc_ui = glGetUniformLocation(currProgram, "view_ui");
	glUniformMatrix3fv(view_loc_ui, 1, GL_FALSE, (float*)&view_ui);
	GLuint time_uloc = glGetUniformLocation(currProgram, "time");
	glUniform1f(time_uloc, (float)(glfwGetTime() * 10.0f));
	gl_has_errors();
	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
}

// draw the intermediate texture to the screen, with some distortion to simulate
// wind
void RenderSystem::drawToScreen()
{
	// Setting shaders
	// get the wind texture, sprite mesh, and program
	glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::WIND]);
	gl_has_errors();
	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(1.f, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();
	// Enabling alpha channel for textures
	glDisable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Draw the screen texture on the quad geometry
	glBindVertexArray(dummyVAO);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]); // Note, GL_ELEMENT_ARRAY_BUFFER associates
	// indices to the bound GL_ARRAY_BUFFER
	gl_has_errors();
	const GLuint wind_program = effects[(GLuint)EFFECT_ASSET_ID::WIND];
	// Set clock
	GLuint time_uloc = glGetUniformLocation(wind_program, "time");
	GLuint dead_timer_uloc = glGetUniformLocation(wind_program, "darken_screen_factor");
	glUniform1f(time_uloc, (float)(glfwGetTime() * 10.0f));
	ScreenState& screen = registry.screenStates.get(screen_state_entity);
	glUniform1f(dead_timer_uloc, screen.darken_screen_factor);
	gl_has_errors();
	// Set the vertex position and vertex texture coordinates (both stored in the
	// same VBO)
	GLint in_position_loc = glGetAttribLocation(wind_program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
	gl_has_errors();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	gl_has_errors();
	// Draw
	glDrawElements(
		GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
		nullptr); // one triangle = 3 vertices; nullptr indicates that there is
	// no offset from the bound index buffer
	gl_has_errors();
}

void RenderSystem::render_buttons(glm::mat3& projection_2D, glm::mat3& view_2D, glm::mat3& view_2D_ui, MENU_STATE state)
{
	ComponentContainer<Button>& button_container = registry.buttons;
	int button_container_size = button_container.size();
	for (int i = 0; i < button_container_size; ++i) {
		Button& button = button_container.components[i];
		if (button.state != state) continue; // don't render other buttons
		Entity& entity = button_container.entities[i];
		Motion& motion = registry.motions.get(entity);
		RenderRequest& rr = registry.renderRequests.get(entity);
		rr.used_texture = button.is_hovered ? TEXTURE_ASSET_ID::BUTTON_HOVERED : TEXTURE_ASSET_ID::BUTTON;

		drawTexturedMesh(entity, projection_2D, view_2D, view_2D_ui);
		renderText(button.text, motion.position.x, motion.position.y, button.text_scale, button.is_hovered ? vec3(0.03f) : vec3(0.5f), trans, false, 1.f);
	}
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw()
{
	gl_has_errors();
	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

	// First render to the custom framebuffer
	gl_has_errors();
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();
	// Clearing backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);
	//glClearColor(0.674, 0.847, 1.0, 1.0);
	if (map_info.level == MAP_LEVEL::LEVEL2) {
		glClearColor(131.f / 255.f, 164.f / 255.f, 72.f / 255.f, 1.f);
	}
	else {
		glClearColor(0, 0, 0, 1.0);
	}
	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
	// and alpha blending, one would have to sort
	// sprites back to front
	gl_has_errors();
	mat3 projection_2D = createProjectionMatrix();
	mat3 view_2D = camera.createViewMatrix();
	mat3 view_2D_ui = ui.createViewMatrix();

	camera.setCameraAABB();

	if (menu.state == MENU_STATE::PLAY ||
		menu.state == MENU_STATE::PAUSE ||
		menu.state == MENU_STATE::DIALOGUE ||
		menu.state == MENU_STATE::WIN ||
		menu.state == MENU_STATE::LOSE ||
		menu.state == MENU_STATE::INFOGRAPHIC) {

		// Draw all textured meshes that have a position and size component
		std::vector<Entity> boss_ui_entities;
		std::vector<Entity> uiux_world_entities;

		drawTilesInstanced(projection_2D, view_2D);

		for (Entity entity : registry.renderRequests.entities)
		{
			if (registry.renderRequests.get(entity).used_texture == TEXTURE_ASSET_ID::BOSS_HEALTH_BAR) {
				boss_ui_entities.push_back(entity);
				continue;
			}
			if (!registry.motions.has(entity) || !camera.isInCameraView(registry.motions.get(entity).position)) {
				continue;
			}
			if (registry.buttons.has(entity) || registry.mainMenus.has(entity) || registry.pauseMenus.has(entity)) continue;
			if (registry.UIUXWorld.has(entity)) {
				uiux_world_entities.push_back(entity);
				continue;
			}
			if (registry.focusdots.has(entity)) continue;
			if (registry.UIUX.has(entity)) continue;
			if (registry.players.has(entity)) continue;
			if (registry.dialogueMenus.has(entity)) continue;
			if (registry.winMenus.has(entity)) continue;
			if (registry.loseMenus.has(entity)) continue;
			if (registry.playerBullets.has(entity)) continue;
			if (registry.infographicsMenus.has(entity)) continue;

			// Note, its not very efficient to access elements indirectly via the entity
			// albeit iterating through all Sprites in sequence. A good point to optimize
			drawTexturedMesh(entity, projection_2D, view_2D, view_2D_ui);
		}

		// UIUX entities that in the world (e.g. tutorial keys that is before player)
		for (Entity entity : uiux_world_entities) {
			drawTexturedMesh(entity, projection_2D, view_2D, view_2D_ui);
		}

		// World texts:
		for (Entity entity : registry.textsWorld.entities) {
			Motion& text_motion = registry.motions.get(entity);
			if (!camera.isInCameraView(registry.motions.get(entity).position)) continue;
			vec3 text_color = registry.colors.get(entity);
			RenderTextWorld& text_cont = registry.textsWorld.get(entity);
			render_text_newline(text_cont.content, text_motion.position.x, text_motion.position.y, text_motion.scale.x, text_color, trans, true, 25.f, text_cont.transparency);
		}
		for (Entity entity : registry.textsPermWorld.entities) {
			Motion& text_motion = registry.motions.get(entity);
			if (!camera.isInCameraView(registry.motions.get(entity).position)) continue;
			vec3 text_color = registry.colors.get(entity);
			RenderTextPermanentWorld& text_cont = registry.textsPermWorld.get(entity);

			// Hardcoded only for item description
			if (registry.realDeathTimers.has(entity)) {
				DeathTimer& death_counter = registry.realDeathTimers.get(entity);
				Motion& motion = registry.motions.get(registry.players.entities[0]);
				text_cont.transparency = death_counter.death_counter_ms / 2000;
				registry.motions.get(entity).position = { motion.position.x, motion.position.y + ((text_cont.transparency - 1) * 35) - 40.f };
			}

			render_text_newline(text_cont.content, text_motion.position.x, text_motion.position.y, text_motion.scale.x, text_color, trans, true, 25.f, text_cont.transparency);
		}

		// Render player
		for (Entity entity : registry.players.entities) {
			drawTexturedMesh(entity, projection_2D, view_2D, view_2D_ui);
		}

		for (Entity entity : registry.playerBullets.entities) {
			if (!camera.isInCameraView(registry.motions.get(entity).position)) continue;
			drawTexturedMesh(entity, projection_2D, view_2D, view_2D_ui);
		}

		// Render instance of visible enemy bullets
		std::vector<Entity> enemy_bullets;
		for (Entity entity : registry.enemyBullets.entities) {
			if (registry.motions.has(entity) && camera.isInCameraView(registry.motions.get(entity).position)) {
				enemy_bullets.push_back(entity);
			}
		}
		drawBulletsInstanced(enemy_bullets, projection_2D, view_2D);

		// this will only have at most one focusdots
		// it will always be in camera view, and has motion
		for (Entity entity : registry.focusdots.entities) {
			drawTexturedMesh(entity, projection_2D, view_2D, view_2D_ui);
		}

		// Render foreground entities, these will be in front of things rendered before
		for (Entity entity : registry.renderRequestsForeground.entities) {
			if (!registry.motions.has(entity) || !camera.isInCameraView(registry.motions.get(entity).position)) {
				continue;
			}
			drawTexturedMesh(entity, projection_2D, view_2D, view_2D_ui);
		}

		for (Entity entity : registry.pickupables.entities) {
			if (!registry.motions.has(entity) || !camera.isInCameraView(registry.motions.get(entity).position)) continue;
			const Motion& motion = registry.motions.get(entity);
			const Pickupable& food = registry.pickupables.get(entity);

			renderText("HP Up+", motion.position.x, motion.position.y + 25, 0.5f, glm::vec3(0.0f, 0.8f, 0.0f), trans, true, 1.f);
		}

		if (registry.visibilityTileInstanceData.components.size() > 0) {
			drawVisibilityTilesInstanced(projection_2D, view_2D);
		}

		if (menu.state != MENU_STATE::DIALOGUE) {
			for (Entity entity : registry.UIUX.entities) {
				drawTexturedMesh(entity, projection_2D, view_2D, view_2D_ui);
			}

			for (Entity entity : boss_ui_entities) {
				BossHealthBarUI& bhp = registry.bossHealthBarUIs.get(entity);
				if (bhp.is_visible) {
					drawTexturedMesh(entity, projection_2D, view_2D, view_2D_ui);
				}
			}
		}

		// Render user guide on screen
		if (WorldSystem::getInstance().get_display_instruction() == true) {
			renderText("Press 'T' for tutorial", window_width_px / 3.3f, -window_height_px / 2.3f, 0.9f, glm::vec3(1), trans, false, 1.f);
		}

		if (WorldSystem::getInstance().get_show_fps() == true) {
			renderText("FPS:", window_width_px / 2.45f, -window_height_px / 2.2f, 1.0f, glm::vec3(0, 1, 0), trans, false, 1.f);
			renderText(WorldSystem::getInstance().get_fps_in_string(), window_width_px / 2.2f, -window_height_px / 2.2f, 1.0f, glm::vec3(0, 1, 0), trans, false, 1.f);
		}

		// On screen/ui texts:
		if (menu.state != MENU_STATE::DIALOGUE) {
			for (Entity entity : registry.texts.entities) {
				Motion& text_motion = registry.motions.get(entity);
				vec3 text_color = registry.colors.get(entity);
				RenderText& text_cont = registry.texts.get(entity);
				renderText(text_cont.content, text_motion.position.x, text_motion.position.y, text_motion.scale.x, text_color, trans, false, text_cont.transparency);
			}
			for (Entity entity : registry.textsPerm.entities) {
				if (registry.winMenus.has(entity)) continue;
				if (registry.loseMenus.has(entity)) continue;
				Motion& text_motion = registry.motions.get(entity);
				vec3 text_color = registry.colors.get(entity);
				RenderTextPermanent& text_cont = registry.textsPerm.get(entity);
				renderText(text_cont.content, text_motion.position.x, text_motion.position.y, text_motion.scale.x, text_color, trans, false, text_cont.transparency);
			}
		}
		if (menu.state == MENU_STATE::DIALOGUE) {
			for (Entity entity : registry.dialogueMenus.entities) {
				drawTexturedMesh(entity, projection_2D, view_2D, view_2D_ui);
			}
			for (Entity entity : registry.texts.entities) {
				Motion& text_motion = registry.motions.get(entity);
				vec3 text_color = registry.colors.get(entity);
				RenderText& text_cont = registry.texts.get(entity);
				render_text_newline(text_cont.content, text_motion.position.x, text_motion.position.y, text_motion.scale.x, text_color, trans, false, 50.f, text_cont.transparency);
			}
			for (Entity entity : registry.textsPerm.entities) {
				if (registry.winMenus.has(entity)) continue;
				if (registry.loseMenus.has(entity)) continue;
				Motion& text_motion = registry.motions.get(entity);
				vec3 text_color = registry.colors.get(entity);
				RenderTextPermanent& text_cont = registry.textsPerm.get(entity);
				render_text_newline(text_cont.content, text_motion.position.x, text_motion.position.y, text_motion.scale.x, text_color, trans, false, 50.f, text_cont.transparency);
			}
		}
		if (menu.state == MENU_STATE::INFOGRAPHIC) {
			for (Entity entity : registry.infographicsMenus.entities) {
				drawTexturedMesh(entity, projection_2D, view_2D, view_2D_ui);
			}

			render_buttons(projection_2D, view_2D, view_2D_ui, MENU_STATE::INFOGRAPHIC);
		}
		// Render this last, because it should be on top
		if (menu.state == MENU_STATE::PAUSE) {
			for (Entity entity : registry.pauseMenus.entities) {
				drawTexturedMesh(entity, projection_2D, view_2D, view_2D_ui);
			}

			render_buttons(projection_2D, view_2D, view_2D_ui, MENU_STATE::PAUSE);
		}
		if (menu.state == MENU_STATE::WIN) {
			for (Entity entity : registry.winMenus.entities) {
				if (registry.textsPerm.has(entity)) {
					Motion& text_motion = registry.motions.get(entity);
					vec3 text_color = registry.colors.get(entity);
					RenderTextPermanent& text_cont = registry.textsPerm.get(entity);
					renderText(text_cont.content, text_motion.position.x, text_motion.position.y, text_motion.scale.x, text_color, trans, false, text_cont.transparency);
				}
				else {
					drawTexturedMesh(entity, projection_2D, view_2D, view_2D_ui);
				}
			}
			render_buttons(projection_2D, view_2D, view_2D_ui, MENU_STATE::WIN);
		}
		if (menu.state == MENU_STATE::LOSE) {
			for (Entity entity : registry.loseMenus.entities) {
				if (registry.textsPerm.has(entity)) {
					Motion& text_motion = registry.motions.get(entity);
					vec3 text_color = registry.colors.get(entity);
					RenderTextPermanent& text_cont = registry.textsPerm.get(entity);
					renderText(text_cont.content, text_motion.position.x, text_motion.position.y, text_motion.scale.x, text_color, trans, false, text_cont.transparency);
				}
				else {
					drawTexturedMesh(entity, projection_2D, view_2D, view_2D_ui);
				}
			}
			render_buttons(projection_2D, view_2D, view_2D_ui, MENU_STATE::LOSE);
		}

	}
	else if (menu.state == MENU_STATE::MAIN_MENU) {
		for (Entity entity : registry.mainMenus.entities) {
			drawTexturedMesh(entity, projection_2D, view_2D, view_2D_ui);
		}

		render_buttons(projection_2D, view_2D, view_2D_ui, MENU_STATE::MAIN_MENU);
	}


	// Truely render to the screen
	drawToScreen();

	// flicker-free display with a double buffer
	glfwSwapBuffers(window);
	gl_has_errors();
}

// This adapted from lecture material (Wednesday Feb 28th 2024)
// fully transparent when transparency_rate = 0
void RenderSystem::renderText(const std::string& text, float x, float y, float scale, const glm::vec3& color, const glm::mat3& trans, bool in_world, float transparency_rate) {
	// activate the shaders!
	glUseProgram(m_font_shaderProgram);

	unsigned int textColor_location =
		glGetUniformLocation(
			m_font_shaderProgram,
			"textColor"
		);
	assert(textColor_location >= 0);
	glUniform3f(textColor_location, color.x, color.y, color.z);

	auto transparency_location = glGetUniformLocation(m_font_shaderProgram, "transparency");
	assert(transparency_location > -1);
	glUniform1f(transparency_location, transparency_rate);

	// flip both y axis so translations will match opengl
	y = -1 * y;
	Transform t;
	t.mat = trans;
	t.scale({ 1, -1 });

	auto transform_location = glGetUniformLocation(m_font_shaderProgram, "transform");
	assert(transform_location > -1);
	glUniformMatrix3fv(transform_location, 1, GL_FALSE, glm::value_ptr(t.mat));

	// apply view matrix, origin is now center of the screen
	// e.g. passing in x=0, y=0 will automatically translate to world_center
	glm::mat3 view = in_world ? camera.createViewMatrix() : ui.createViewMatrix();
	GLint view_location = glGetUniformLocation(m_font_shaderProgram, "view");
	assert(view_location > -1);
	glUniformMatrix3fv(view_location, 1, GL_FALSE, glm::value_ptr(view));

	glBindVertexArray(m_font_VAO);

	// https://gamedev.stackexchange.com/q/178035
	float offset_x = 0.f;
	float offset_y = 0.f;
	for (auto c = text.begin(); c != text.end(); c++) {
		Character ch = m_ftCharacters[*c];
		// advance is length of current glyph to next glyph
		offset_x += (ch.Advance >> 6) * scale;
		// size is entire glyph size, need to shift equally for different glyph heights
		offset_y = max(offset_y, ch.Size.y * scale);
	}
	offset_x /= 2.f;
	offset_y /= 2.f;

	// iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = m_ftCharacters[*c];

		float xpos = x + ch.Bearing.x * scale - offset_x;
		float ypos = y - (ch.Size.y - ch.Bearing.y) * scale - offset_y;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;

		// update VBO for each character
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};

		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// std::cout << "binding texture: " << ch.character << " = " << ch.TextureID << std::endl;

		// update text of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, m_font_VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}


mat3 RenderSystem::createProjectionMatrix()
{
	// Fake projection matrix, scales with respect to window coordinates
	float left = 0.f;
	float top = 0.f;

	gl_has_errors();
	float right = (float)window_width_px;
	float bottom = (float)window_height_px;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	return { {sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f} };
}

// Adapted from: https://learnopengl.com/Advanced-OpenGL/Instancing
void RenderSystem::drawBulletsInstanced(const std::vector<Entity>& entities, const glm::mat3& projection, const glm::mat3& view)
{
	int amount = entities.size();

	if (amount == 0) return; // nothing to draw

	mat3* instance_transforms = new mat3[amount];
	for (int i = 0; i < amount; ++i) {
		Motion& motion = registry.motions.get(entities[i]);
		Transform transform;
		transform.translate(motion.position);
		transform.rotate(motion.angle);
		transform.scale(motion.scale);
		instance_transforms[i] = transform.mat;
	}

	// Setting shaders
	glUseProgram(enemy_bullet_instance_program);
	gl_has_errors();

	const GLuint vbo = vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SPRITE];
	const GLuint ibo = index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SPRITE];

	// Setting vertex and index buffers
	glBindVertexArray(enemy_bullet_instance_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	GLint in_position_loc = glGetAttribLocation(enemy_bullet_instance_program, "in_position");
	GLint in_texcoord_loc = glGetAttribLocation(enemy_bullet_instance_program, "in_texcoord");
	GLint in_transform_loc = glGetAttribLocation(enemy_bullet_instance_program, "in_transform");
	gl_has_errors();
	assert(in_position_loc >= 0);
	assert(in_texcoord_loc >= 0);
	assert(in_transform_loc >= 0);

	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)0);
	gl_has_errors();

	glEnableVertexAttribArray(in_texcoord_loc);
	// note the stride to skip the preceeding vertex position
	glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)sizeof(vec3));

	// Enabling and binding texture to slot 0
	glActiveTexture(GL_TEXTURE0);
	gl_has_errors();

	GLuint texture_id;

	if (boss_info.should_use_flandre_bullet) {
		texture_id = texture_gl_handles[(GLuint)TEXTURE_ASSET_ID::FLANDRE_BULLET];
	}
	else {
		texture_id = texture_gl_handles[(GLuint)TEXTURE_ASSET_ID::ENEMY_BULLET];
	}

	glBindTexture(GL_TEXTURE_2D, texture_id);
	gl_has_errors();

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(enemy_bullet_instance_program, "fcolor");
	//const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
	const vec3 color = vec3(1);
	glUniform3fv(color_uloc, 1, (float*)&color);
	gl_has_errors();

	GLint end_pos_uloc = glGetUniformLocation(enemy_bullet_instance_program, "end_pos");
	//const vec2 end_pos = registry.animation.has(entity) ? registry.animation.get(entity).render_pos : vec2(1);
	const vec3 end_pos = vec3(1);
	glUniform2fv(end_pos_uloc, 1, (float*)&end_pos);
	gl_has_errors();

	GLint scale_uloc = glGetUniformLocation(enemy_bullet_instance_program, "scale");
	//const vec2 ani_scale = registry.animation.has(entity) ? registry.animation.get(entity).spritesheet_scale : vec2(1);
	const vec3 ani_scale = vec3(1);
	glUniform2fv(scale_uloc, 1, (float*)&ani_scale);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&projection);
	GLuint view_loc = glGetUniformLocation(currProgram, "view");
	glUniformMatrix3fv(view_loc, 1, GL_FALSE, (float*)&view);
	gl_has_errors();

	glBindBuffer(GL_ARRAY_BUFFER, enemy_bullet_instance_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mat3) * amount, instance_transforms, GL_DYNAMIC_DRAW);
	glDrawElementsInstanced(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, 0, amount);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	gl_has_errors();
	delete[] instance_transforms;
}

vec4 RenderSystem::get_spriteloc(TILE_NAME tile_name) {
	// Adapted from: https://gamedev.stackexchange.com/a/86356
	// spriteloc = { offset_x, offset_y, sprite_width, sprite_height }
	// Note:
	// spriteloc will be scaled by (x,y)=(sprite_width/atlas_width, sprite_height/atlas_height)
	// e.g. (1.f/2.f, 1.f/3.f) is the same as (32.f/64.f, 32.f/96.f) assuming each texture is 32x32. Also works for any texture size in atlas
	// then shifted by (x,y)=(offset_x/atlas_width, offset_y/atlas_height), works the same with above example
	const float ATLAS_WIDTH = 4.f;
	const float ATLAS_HEIGHT = 7.f;
	const vec4 DIVISOR = vec4(ATLAS_WIDTH, ATLAS_HEIGHT, ATLAS_WIDTH, ATLAS_HEIGHT);
	vec4 spriteloc = { -1.f, -1.f, -1.f, -1.f };
	// switch is faster than unordered_map in this case, no need to hash
	switch (tile_name) {
	case TILE_NAME::DEFAULT_FLOOR:
		spriteloc = { 0.f, 0.f, 1.0, 1.0 };
		break;
	case TILE_NAME::FLOOR_1_0:
		spriteloc = { 1.f, 0.f, 1.0, 1.0 };
		break;
	case TILE_NAME::FLOOR_2_0:
		spriteloc = { 2.f, 0.f, 1.0, 1.0 };
		break;
	case TILE_NAME::FLOOR_3_0:
		spriteloc = { 3.f, 0.f, 1.0, 1.0 };
		break;
	case TILE_NAME::FLOOR_0_1:
		spriteloc = { 0.f, 1.f, 1.0, 1.0 };
		break;
	case TILE_NAME::FLOOR_1_1:
		spriteloc = { 1.f, 1.f, 1.0, 1.0 };
		break;
	case TILE_NAME::FLOOR_2_1:
		spriteloc = { 2.f, 1.f, 1.0, 1.0 };
		break;
	case TILE_NAME::FLOOR_3_1:
		spriteloc = { 3.f, 1.f, 1.0, 1.0 };
		break;
	case TILE_NAME::LEFT_WALL:
		spriteloc = { 0.f, 2.f, 1.0, 1.0 };
		break;
	case TILE_NAME::TOP_WALL:
		spriteloc = { 1.f, 2.f, 1.0, 1.0 };
		break;
	case TILE_NAME::RIGHT_WALL:
		spriteloc = { 2.f, 2.f, 1.0, 1.0 };
		break;
	case TILE_NAME::CORRIDOR_BOTTOM_RIGHT:
		spriteloc = { 0.f, 5.f, 1.0, 1.0 };
		break;
	case TILE_NAME::CORRIDOR_BOTTOM_LEFT:
		spriteloc = { 2.f, 5.f, 1.0, 1.0 };
		break;
	case TILE_NAME::BOTTOM_LEFT:
		spriteloc = { 0.f, 6.f, 1.0, 1.0 };
		break;
	case TILE_NAME::BOTTOM_WALL:
		spriteloc = { 1.f, 6.f, 1.0, 1.0 };
		break;
	case TILE_NAME::BOTTOM_RIGHT:
		spriteloc = { 2.f, 6.f, 1.0, 1.0 };
		break;
	case TILE_NAME::CORRIDOR_BOTTOM_RIGHT_LIGHT:
		spriteloc = { 0.f, 3.f, 1.0, 1.0 };
		break;
	case TILE_NAME::CORRIDOR_BOTTOM_LEFT_LIGHT:
		spriteloc = { 2.f, 3.f, 1.0, 1.0 };
		break;
	case TILE_NAME::BOTTOM_LEFT_LIGHT:
		spriteloc = { 0.f, 4.f, 1.0, 1.0 };
		break;
	case TILE_NAME::BOTTOM_RIGHT_LIGHT:
		spriteloc = { 2.f, 4.f, 1.0, 1.0 };
		break;
	default:
		break;
	}
	//assert(spriteloc.x != -1.f && spriteloc.y != -1.f && spriteloc.z != -1.f && spriteloc.w != -1.f && "Tile name not found");
	return spriteloc / DIVISOR;
}

void RenderSystem::set_tiles_instance_buffer() {
	glUseProgram(tile_instance_program);
	glBindVertexArray(tiles_instance_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, tiles_instance_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TileInstanceData) * registry.tileInstanceData.size(), registry.tileInstanceData.components.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}


void RenderSystem::drawTilesInstanced(const glm::mat3& projection, const glm::mat3& view)
{
	const GLuint ibo = index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SPRITE];

	// Setting vertex and index buffers
	glUseProgram(tile_instance_program);
	glBindVertexArray(tiles_instance_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, tiles_instance_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	// Enabling and binding texture to slot 0
	glActiveTexture(GL_TEXTURE0);
	gl_has_errors();

	GLuint texture_id;
	if (map_info.level == MAP_LEVEL::LEVEL2) {
		texture_id = texture_gl_handles[(GLuint)TEXTURE_ASSET_ID::TILES_ATLAS_RUINS];
	}
	else {
		texture_id = texture_gl_handles[(GLuint)TEXTURE_ASSET_ID::TILES_ATLAS_SANDSTONE];
	}
	glBindTexture(GL_TEXTURE_2D, texture_id);
	gl_has_errors();

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(tile_instance_program, "fcolor");
	//const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
	const vec3 color = vec3(1);
	glUniform3fv(color_uloc, 1, (float*)&color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();
	GLsizei num_indices = size / sizeof(uint16_t);

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&projection);
	GLuint view_loc = glGetUniformLocation(currProgram, "view");
	glUniformMatrix3fv(view_loc, 1, GL_FALSE, (float*)&view);
	gl_has_errors();

	glDrawElementsInstanced(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, 0, registry.tileInstanceData.size());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	gl_has_errors();
}

// called once when generating new map
void RenderSystem::set_visibility_tiles_instance_buffer_max() {
	glUseProgram(visibility_tile_instance_program);
	glBindVertexArray(visibility_tile_instance_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, visibility_tile_instance_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VisibilityTileInstanceData) * registry.visibilityTileInstanceData.size(), registry.visibilityTileInstanceData.components.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

// called when visibility map is decreased, we do this instead of glBufferData so don't realloc every time
void RenderSystem::set_visibility_tiles_instance_buffer() {
	glUseProgram(visibility_tile_instance_program);
	glBindVertexArray(visibility_tile_instance_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, visibility_tile_instance_VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(VisibilityTileInstanceData) * registry.visibilityTileInstanceData.size(), registry.visibilityTileInstanceData.components.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void RenderSystem::drawVisibilityTilesInstanced(const glm::mat3& projection, const glm::mat3& view)
{
	const GLuint ibo = index_buffers[(GLuint)GEOMETRY_BUFFER_ID::DEBUG_LINE];

	// Setting vertex and index buffers
	glUseProgram(visibility_tile_instance_program);
	glBindVertexArray(visibility_tile_instance_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, visibility_tile_instance_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(visibility_tile_instance_program, "fcolor");
	//const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
	vec3 color;
	if (map_info.level == MAP_LEVEL::LEVEL2) {
		color = vec3(131.f / 255.f, 164.f / 255.f, 72.f / 255.f);
	}
	else {
		color = vec3(0);
	}
	glUniform3fv(color_uloc, 1, (float*)&color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();
	GLsizei num_indices = size / sizeof(uint16_t);

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&projection);
	GLuint view_loc = glGetUniformLocation(currProgram, "view");
	glUniformMatrix3fv(view_loc, 1, GL_FALSE, (float*)&view);
	gl_has_errors();

	glDrawElementsInstanced(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, 0, registry.visibilityTileInstanceData.size());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	gl_has_errors();
}

// if is_close true, switch to closed texture, otherwise open texture
void RenderSystem::switch_door_texture(Entity door_entity, bool is_close) {
	Door& door = registry.doors.get(door_entity);
	RenderRequest& request = registry.renderRequests.get(door_entity);
	if (door.dir == DIRECTION::RIGHT || door.dir == DIRECTION::LEFT) {
		RenderRequest& top_request = registry.renderRequests.get(door.top_texture);
		if (is_close) {
			top_request.used_texture = TEXTURE_ASSET_ID::DOOR_VERTICAL_CLOSE_UP;
			request.used_texture = TEXTURE_ASSET_ID::DOOR_VERTICAL_CLOSE_DOWN;
		}
		else {
			top_request.used_texture = TEXTURE_ASSET_ID::DOOR_VERTICAL_OPEN_UP;
			request.used_texture = TEXTURE_ASSET_ID::DOOR_VERTICAL_OPEN_DOWN;
		}
	}
	else {
		if (is_close) {
			request.used_texture = TEXTURE_ASSET_ID::DOOR_HORIZONTAL_CLOSE;
		}
		else {
			request.used_texture = TEXTURE_ASSET_ID::DOOR_HORIZONTAL_OPEN;
		}
	}
}