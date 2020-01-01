//#########################
//Pekka Kana 2
//by Janne Kivilahti from Piste Gamez (2003)
//#########################
#pragma once

#include "engine/platform.hpp"
#include "episode.hpp"

const int MAX_SAVES = 10;

struct PK2SAVE_V1 {

	int   jakso;
	char  episodi[PE_PATH_SIZE];
	char  nimi[20];
	bool  kaytossa;
	bool  jakso_cleared[EPISODI_MAX_LEVELS];
	DWORD pisteet;

};

struct PK2SAVE {

	bool  empty;
	DWORD level;
	char  episode[PE_PATH_SIZE];
	char  name[20];
	DWORD score;
	bool  level_cleared[EPISODI_MAX_LEVELS];
	bool  all_apples[EPISODI_MAX_LEVELS];
	
};

extern PK2SAVE saves_list[MAX_SAVES];

int Load_SaveFile();
int Save_Records(int i);