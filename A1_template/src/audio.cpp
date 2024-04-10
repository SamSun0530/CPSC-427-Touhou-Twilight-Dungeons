#include "audio.hpp"

void Audio::step(float elapsed) {
	if (menu.state == MENU_STATE::MAIN_MENU) {
		if (Mix_Playing(2)) {
			Mix_HaltChannel(2);
			Mix_PlayChannel(1, menu_music, -1);
		}
		else if (Mix_Playing(1)) {

		}
	}
	else if (menu.state == MENU_STATE::PLAY) {
		if (Mix_Playing(2)) {

		}
		else if (Mix_Playing(1)) {
			Mix_HaltChannel(1);
			Mix_PlayChannel(2, background_music, -1);
		}
	}
}

Audio::Audio() {
	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		assert(false && "Failed to initialize SDL Audio");
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		assert(false && "Failed to open audio device");
	}

	background_music = Mix_LoadWAV(audio_path("regular_room_bgm.wav").c_str());
	menu_music = Mix_LoadWAV(audio_path("main_menu_bgm.wav").c_str());
	boss_music = Mix_LoadMUS(audio_path("boss_fight_bgm.wav").c_str());
	game_ending_sound = Mix_LoadWAV(audio_path("game_ending.wav").c_str());
	firing_sound = Mix_LoadWAV(audio_path("spell_sound.wav").c_str());
	damage_sound = Mix_LoadWAV(audio_path("damage_sound.wav").c_str());
	hit_spell = Mix_LoadWAV(audio_path("hit_spell.wav").c_str());

	// Set the music volume
	Mix_VolumeMusic(15);
	Mix_Volume(-1, 30);

	if (background_music == nullptr || game_ending_sound == nullptr || firing_sound == nullptr || damage_sound == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("backgroundmusic.wav").c_str(),
			audio_path("game_ending.wav").c_str(),
			audio_path("spell_sound.wav").c_str(),
			audio_path("damage_sound.wav").c_str(),
			audio_path("hit_spell.wav").c_str());

		assert(false && "Failed to load sounds, make sure the data directory is present");
	}
}

Audio::~Audio() {
	// Destroy music components
	if (background_music != nullptr)
		Mix_FreeChunk(background_music);
	if (game_ending_sound != nullptr)
		Mix_FreeChunk(game_ending_sound);
	if (firing_sound != nullptr)
		Mix_FreeChunk(firing_sound);
	if (damage_sound != nullptr)
		Mix_FreeChunk(damage_sound);
	if (hit_spell != nullptr)
		Mix_FreeChunk(hit_spell);
	if (menu_music != nullptr)
		Mix_FreeChunk(menu_music);
	if (boss_music != nullptr)
		Mix_FreeMusic(boss_music);

	Mix_CloseAudio();
}