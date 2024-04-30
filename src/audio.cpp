#include "audio.hpp"

void Audio::step(float elapsed) {
	if (menu.state == MENU_STATE::MAIN_MENU || menu.state == MENU_STATE::WIN || menu.state == MENU_STATE::LOSE) {
		if (abackground_music.is_playing ||
			aboss_music.is_playing) {
			aboss_music.pause();
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
			if (boss_alive && (
				(map_info.level == MAP_LEVEL::LEVEL1 && boss_info.has_cirno_talked) ||
				(map_info.level == MAP_LEVEL::LEVEL2 && boss_info.has_flandre_talked) ||
				(map_info.level == MAP_LEVEL::LEVEL3 && boss_info.has_sakuya_talked) ||
				(map_info.level == MAP_LEVEL::LEVEL4 && boss_info.has_remilia_talked)
				)) {
				abackground_music.pause();
				aboss_music.play();
			}
		}
	}
	else if (menu.state == MENU_STATE::PAUSE || menu.state == MENU_STATE::INFOGRAPHIC) {
		if (abackground_music.is_playing ||
			aboss_music.is_playing) {
			aboss_music.pause();
			abackground_music.pause();
			amenu_music.play();
		}
	}
	else if (aboss_music.is_playing) {
		bool boss_alive = registry.bosses.size() == 1;
		if (!boss_alive) {
			aboss_music.pause();
			abackground_music.play();
		}
	}
}

void Audio::restart_audio_level() {
	Mix_PlayChannel(1, menu_music, -1);
	restart_audio_boss();
	restart_audio_background();

	amenu_music.pause();
	abackground_music.play();
	aboss_music.pause();
}

void Audio::restart_audio_boss() {
	if (map_info.level == MAP_LEVEL::LEVEL1) {
		Mix_PlayChannel(aboss_music.channel, cirno_boss_music, -1);
	}
	else if (map_info.level == MAP_LEVEL::LEVEL2) {
		Mix_PlayChannel(aboss_music.channel, flandre_boss_music, -1);
	}
	else if (map_info.level == MAP_LEVEL::LEVEL3) {
		Mix_PlayChannel(aboss_music.channel, sakuya_boss_music, -1);
	}
	else {
		Mix_PlayChannel(aboss_music.channel, remilia_boss_music, -1);
	}

	aboss_music.pause();
}

void Audio::restart_audio_background() {
	if (map_info.level == MAP_LEVEL::LEVEL1) {
		Mix_PlayChannel(abackground_music.channel, level1_background_music, -1);
	}
	else if (map_info.level == MAP_LEVEL::LEVEL2) {
		Mix_PlayChannel(abackground_music.channel, level2_background_music, -1);
	}
	else if (map_info.level == MAP_LEVEL::LEVEL3) {
		Mix_PlayChannel(abackground_music.channel, level3_background_music, -1);
	}
	else {
		Mix_PlayChannel(abackground_music.channel, level4_background_music, -1);
	}

	abackground_music.pause();
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

	level1_background_music = Mix_LoadWAV(audio_path("level1_background_music.wav").c_str());
	level2_background_music = Mix_LoadWAV(audio_path("level2_background_music.wav").c_str());
	level3_background_music = Mix_LoadWAV(audio_path("level3_background_music.wav").c_str());
	level4_background_music = Mix_LoadWAV(audio_path("level4_background_music.wav").c_str());
	menu_music = Mix_LoadWAV(audio_path("main_menu_bgm.wav").c_str());
	cirno_boss_music = Mix_LoadWAV(audio_path("cirno_boss_fight.wav").c_str());
	game_ending_sound = Mix_LoadWAV(audio_path("game_ending_sound.wav").c_str());
	firing_sound = Mix_LoadWAV(audio_path("spell_sound.wav").c_str());
	damage_sound = Mix_LoadWAV(audio_path("damage_sound.wav").c_str());
	hit_spell = Mix_LoadWAV(audio_path("hit_spell.wav").c_str());
	pause_menu_sound = Mix_LoadWAV(audio_path("pause_menu_sound.wav").c_str());
	open_gate_sound = Mix_LoadWAV(audio_path("open_gate_sound.wav").c_str());
	flandre_boss_music = Mix_LoadWAV(audio_path("flandre_boss_fight.wav").c_str());
	remilia_boss_music = Mix_LoadWAV(audio_path("remilia_boss_fight.wav").c_str());
	sakuya_boss_music = Mix_LoadWAV(audio_path("sakuya_boss_fight.wav").c_str());

	Mix_PlayChannel(1, menu_music, -1);
	Mix_PlayChannel(2, level1_background_music, -1);
	Mix_PlayChannel(4, cirno_boss_music, -1);

	amenu_music.channel = 1;
	abackground_music.channel = 2;
	aboss_music.channel = 4;

	amenu_music.play();
	abackground_music.pause();
	aboss_music.pause();

	// Set the music volume
	Mix_VolumeMusic(15);
	Mix_Volume(-1, 20);
	Mix_Volume(amenu_music.channel, 30);
	Mix_Volume(abackground_music.channel, 40);
	Mix_Volume(aboss_music.channel, 30);

	if (level1_background_music == nullptr ||
		level2_background_music == nullptr ||
		level3_background_music == nullptr ||
		level4_background_music == nullptr ||
		menu_music == nullptr ||
		cirno_boss_music == nullptr ||
		firing_sound == nullptr ||
		damage_sound == nullptr ||
		hit_spell == nullptr ||
		pause_menu_sound == nullptr ||
		flandre_boss_music == nullptr ||
		sakuya_boss_music == nullptr ||
		remilia_boss_music == nullptr ||
		open_gate_sound == nullptr ||
		game_ending_sound == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("level1_background_music.wav").c_str(),
			audio_path("level2_background_music.wav").c_str(),
			audio_path("level3_background_music.wav").c_str(),
			audio_path("level4_background_music.wav").c_str(),
			audio_path("main_menu_bgm.wav").c_str(),
			audio_path("boss_fight_bgm.wav").c_str(),
			audio_path("game_ending_sound.wav").c_str(),
			audio_path("spell_sound.wav").c_str(),
			audio_path("damage_sound.wav").c_str(),
			audio_path("hit_spell.wav").c_str(),
			audio_path("pause_menu_sound.wav").c_str(),
			audio_path("open_gate_sound.wav").c_str(),
			audio_path("flandre_boss_fight.wav").c_str(),
			audio_path("remilia_boss_fight.wav").c_str(),
			audio_path("sakuya_boss_fight.wav").c_str()
		);

		assert(false && "Failed to load sounds, make sure the data directory is present");
	}
}

Audio::~Audio() {
	// Destroy music components
	if (level1_background_music != nullptr)
		Mix_FreeChunk(level1_background_music);
	if (level2_background_music != nullptr)
		Mix_FreeChunk(level2_background_music);
	if (level3_background_music != nullptr)
		Mix_FreeChunk(level3_background_music);
	if (level4_background_music != nullptr)
		Mix_FreeChunk(level4_background_music);
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
	if (cirno_boss_music != nullptr)
		Mix_FreeChunk(cirno_boss_music);
	if (pause_menu_sound != nullptr)
		Mix_FreeChunk(pause_menu_sound);
	if (open_gate_sound != nullptr)
		Mix_FreeChunk(open_gate_sound);
	if (flandre_boss_music != nullptr)
		Mix_FreeChunk(flandre_boss_music);
	if (sakuya_boss_music != nullptr)
		Mix_FreeChunk(sakuya_boss_music);
	if (remilia_boss_music != nullptr)
		Mix_FreeChunk(remilia_boss_music);

	Mix_CloseAudio();
}