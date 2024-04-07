#pragma once

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

// internal
#include "common.hpp"

class Audio {
public:
	Audio();
	~Audio();

	// music references
	Mix_Music* background_music;
	Mix_Music* menu_music;
	Mix_Chunk* game_ending_sound;
	Mix_Chunk* firing_sound;
	Mix_Chunk* damage_sound;
	Mix_Chunk* hit_spell;
};