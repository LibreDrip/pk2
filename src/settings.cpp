//#########################
//Pekka Kana 2
//by Janne Kivilahti from Piste Gamez (2003)
//#########################
#include "settings.hpp"

#include "engine/PInput.hpp"
#include "engine/PUtils.hpp"

#include <SDL_rwops.h>
#include <cstring>

#define SETTINGS_PATH "data" PE_SEP "settings.ini"
#define SETTINGS_VERSION "1.2"

PK2SETTINGS Settings;

void settings_init() {

	strcpy(Settings.versio, SETTINGS_VERSION);
	Settings.ladattu = false;

	strcpy(Settings.kieli, "english.txt");

	Settings.draw_transparent = true;
	Settings.transparent_text = false;
	Settings.draw_weather = true;
	Settings.draw_itembar = true;
	Settings.bg_sprites = true;

	Settings.aanet = true;
	Settings.musiikki = true;

	Settings.control_left      = PI_LEFT;
	Settings.control_right     = PI_RIGHT;
	Settings.control_jump      = PI_UP;
	Settings.control_down      = PI_DOWN;
	Settings.control_walk_slow = PI_LALT;
	Settings.control_attack1   = PI_LCONTROL;
	Settings.control_attack2   = PI_LSHIFT;
	Settings.control_open_gift = PI_SPACE;

	Settings.isFiltered = true;
	Settings.isFit = true;
	Settings.isFullScreen = true;
	Settings.isWide = true;

	Settings.music_max_volume = 64;
	Settings.sfx_max_volume = 90;

}

int Settings_Open() {

	SDL_RWops *file = SDL_RWFromFile(SETTINGS_PATH, "rb");

	if (file == nullptr){
		settings_init();
		Settings_Save();
		return 1;
	}

	SDL_RWread(file, Settings.versio, 4, 1);
	
	if (strncmp(Settings.versio, SETTINGS_VERSION, 4) != 0) { 
		// If settings isn't in current version
		settings_init();
        Settings_Save();
		return 2;
	}
	
	SDL_RWread(file, &Settings, sizeof(PK2SETTINGS), 1);
	
	Settings.ladattu = true;

	SDL_RWclose(file);
	
	return 0;

}

int Settings_Save() {

    PUtils::CreateDir("data");
	
	SDL_RWops *file = SDL_RWFromFile(SETTINGS_PATH, "wb");

	if (file == nullptr) {
		printf("Error saving settings\n");
		return 1;
	}
	
	SDL_RWwrite(file, SETTINGS_VERSION, 1, 4);
	SDL_RWwrite(file, (char*)&Settings, 1, sizeof(PK2SETTINGS));
	
	SDL_RWclose(file);

	return 0;

}