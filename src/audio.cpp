#include "audio.hpp"

void Audio::step(float elapsed) {
	if (menu.state == MENU_STATE::MAIN_MENU) {
		if (abackground_music.is_playing ||
			aboss_music.is_playing ||
			aflandre_boss_music.is_playing) {
			aboss_music.pause();
			aflandre_boss_music.pause();
			abackground_music.pause();
			amenu_music.play();
		}
	}
	else if (menu.state == MENU_STATE::PLAY) {
		if (amenu_music.is_playing) {
			amenu_music.pause();
			abackground_music.play();
		}
		if (abackground_music.is_playing) {
			bool boss_alive = registry.bosses.size() == 1;
			if (boss_alive &&
				boss_info.has_cirno_talked &&
				map_info.level == MAP_LEVEL::LEVEL1) {
				abackground_music.pause();
				aboss_music.play();
			}
			else if (boss_alive &&
				boss_info.has_flandre_talked &&
				map_info.level == MAP_LEVEL::LEVEL2) {
				abackground_music.pause();
				aflandre_boss_music.play();
			}
		}
	}
	else if (aboss_music.is_playing) {
		bool boss_alive = registry.bosses.size() == 1;
		if (!boss_alive ||
			map_info.level == MAP_LEVEL::LEVEL2) {
			aboss_music.pause();
			abackground_music.play();
		}
	}
	else if (aflandre_boss_music.is_playing) {
		bool boss_alive = registry.bosses.size() == 1;
		if (!boss_alive ||
			map_info.level == MAP_LEVEL::LEVEL1) {
			aflandre_boss_music.pause();
			abackground_music.play();
		}
	}
	else if (menu.state == MENU_STATE::PAUSE) {
		if (abackground_music.is_playing ||
			aboss_music.is_playing ||
			aflandre_boss_music.is_playing) {
			aboss_music.pause();
			aflandre_boss_music.pause();
			abackground_music.pause();
			amenu_music.play();
		}
	}
}

void Audio::restart_audio_level1() {
	Mix_PlayChannel(1, menu_music, -1);
	Mix_PlayChannel(2, background_music, -1);
	Mix_PlayChannel(4, boss_music, -1);
	Mix_PlayChannel(5, flandre_boss_music, -1);

	amenu_music.pause();
	abackground_music.play();
	aboss_music.pause();
	aflandre_boss_music.pause();
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
	boss_music = Mix_LoadWAV(audio_path("boss_fight_bgm.wav").c_str());
	game_ending_sound = Mix_LoadWAV(audio_path("game_ending_sound.wav").c_str());
	firing_sound = Mix_LoadWAV(audio_path("spell_sound.wav").c_str());
	damage_sound = Mix_LoadWAV(audio_path("damage_sound.wav").c_str());
	hit_spell = Mix_LoadWAV(audio_path("hit_spell.wav").c_str());
	pause_menu_sound = Mix_LoadWAV(audio_path("pause_menu_sound.wav").c_str());
	open_gate_sound = Mix_LoadWAV(audio_path("open_gate_sound.wav").c_str());
	flandre_boss_music = Mix_LoadWAV(audio_path("flandre_boss_fight.wav").c_str());

	Mix_PlayChannel(1, menu_music, -1);
	Mix_PlayChannel(2, background_music, -1);
	Mix_PlayChannel(4, boss_music, -1);
	Mix_PlayChannel(5, flandre_boss_music, -1);

	amenu_music.channel = 1;
	abackground_music.channel = 2;
	aboss_music.channel = 4;
	aflandre_boss_music.channel = 5;

	amenu_music.play();
	abackground_music.pause();
	aboss_music.pause();
	aflandre_boss_music.pause();

	// Set the music volume
	Mix_VolumeMusic(15);
	Mix_Volume(-1, 20);
	Mix_Volume(amenu_music.channel, 20);
	Mix_Volume(abackground_music.channel, 20);
	Mix_Volume(aboss_music.channel, 20);
	Mix_Volume(aflandre_boss_music.channel, 20);

	if (background_music == nullptr ||
		menu_music == nullptr ||
		boss_music == nullptr ||
		firing_sound == nullptr ||
		damage_sound == nullptr ||
		hit_spell == nullptr ||
		pause_menu_sound == nullptr ||
		flandre_boss_music == nullptr ||
		open_gate_sound == nullptr ||
		game_ending_sound == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("regular_room_bgm.wav").c_str(),
			audio_path("main_menu_bgm.wav").c_str(),
			audio_path("boss_fight_bgm.wav").c_str(),
			audio_path("game_ending_sound.wav").c_str(),
			audio_path("spell_sound.wav").c_str(),
			audio_path("damage_sound.wav").c_str(),
			audio_path("hit_spell.wav").c_str(),
			audio_path("pause_menu_sound.wav").c_str(),
			audio_path("open_gate_sound.wav").c_str(),
			audio_path("flandre_boss_fight.wav").c_str());

		assert(false && "Failed to load sounds, make sure the data directory is present");
	}
}

Audio::~Audio() {
	// Destroy music components
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
		Mix_FreeChunk(boss_music);
	if (pause_menu_sound != nullptr)
		Mix_FreeChunk(pause_menu_sound);
	if (open_gate_sound != nullptr)
		Mix_FreeChunk(open_gate_sound);
	if (flandre_boss_music != nullptr)
		Mix_FreeChunk(flandre_boss_music);

	Mix_CloseAudio();
}