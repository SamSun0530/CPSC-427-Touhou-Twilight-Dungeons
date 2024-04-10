#pragma once

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

// internal
#include "common.hpp"
#include "components.hpp"

class Audio {
public:
	Audio();
	~Audio();

	void step(float elapsed);

	// music references
	Mix_Chunk* background_music;
	Mix_Chunk* menu_music;
	Mix_Music* boss_music;
	Mix_Chunk* game_ending_sound;
	Mix_Chunk* firing_sound;
	Mix_Chunk* damage_sound;
	Mix_Chunk* hit_spell;
};