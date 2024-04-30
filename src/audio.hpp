#pragma once

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

// internal
#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"

struct audio {
	int channel = -1;
	bool is_playing = false;

	void play() {
		is_playing = true;
		Mix_Resume(channel);
	}

	void pause() {
		is_playing = false;
		Mix_Pause(channel);
	}
};

class Audio {
public:
	Audio();
	~Audio();

	// restart audio playing
	void restart_audio_level();
	void restart_audio_boss();

	void step(float elapsed);

	// music references
	Mix_Chunk* background_music;
	Mix_Chunk* menu_music;
	Mix_Chunk* open_gate_sound;
	Mix_Chunk* cirno_boss_music;
	Mix_Chunk* pause_menu_sound;
	Mix_Chunk* game_ending_sound;
	Mix_Chunk* firing_sound;
	Mix_Chunk* damage_sound;
	Mix_Chunk* hit_spell;
	Mix_Chunk* flandre_boss_music;
	Mix_Chunk* remilia_boss_music;
	Mix_Chunk* sakuya_boss_music;

	audio abackground_music;
	audio amenu_music;
	audio aboss_music;
};