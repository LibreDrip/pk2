//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#include "game/mapclass.hpp"

#include "episode/episodeclass.hpp"
#include "game/sprites.hpp"
#include "game/game.hpp"
#include "gfx/effect.hpp"
#include "language.hpp"
#include "system.hpp"

#include "engine/PDraw.hpp"
#include "engine/PUtils.hpp"
#include "engine/PInput.hpp"
#include "engine/PLog.hpp"

#include <cinttypes>
#include <cstring>
#include <cmath>

int aste,
	vesiaste = 0,
	animaatio,
	ajastin1,
	ajastin2,
	ajastin3,
	avaimet;

struct PK2KARTTA{ // Vanha versio 0.1
	char		versio[8];
	char		nimi[40];
	u8		taustakuva;
	u8		musiikki;
	u8		kartta [640*224];
	u8		palikat[320*256];
	u8		extrat [640*480];
};

void MapClass_Animoi(int degree, int anim, int aika1, int aika2, int aika3, bool keys){
	aste = degree;
	animaatio = anim;
	ajastin1 = aika1;
	ajastin2 = aika2;
	ajastin3 = aika3;
	avaimet  = keys;
}

MapClass::MapClass(){

	this->tiles_buffer = -1;
	this->bg_tiles_buffer = -1;
	this->background_buffer = -1;
	this->water_buffer = -1;
	this->bg_water_buffer = -1;

	strcpy(this->versio, PK2MAP_LAST_VERSION);
	strcpy(this->palikka_bmp,"blox.bmp");
	strcpy(this->taustakuva, "default.bmp");
	strcpy(this->musiikki,   "default.xm");

	strcpy(this->nimi,  "untitled");
	strcpy(this->tekija,"unknown");

	this->jakso		= 0;
	this->ilma		= ILMA_NORMAALI;
	this->kytkin1_aika = SWITCH_INITIAL_VALUE;
	this->kytkin2_aika = SWITCH_INITIAL_VALUE;
	this->kytkin3_aika = SWITCH_INITIAL_VALUE;
	this->pelaaja_sprite = 0;
	this->aika		= 0;
	this->extra		= PK2KARTTA_EXTRA_EI;
	this->tausta	= TAUSTA_STAATTINEN;

	this->x = 0;
	this->y = 0;
	this->icon = 0;

	memset(this->taustat, 255, sizeof(taustat));
	memset(this->seinat , 255, sizeof(seinat));
	memset(this->spritet, 255, sizeof(spritet));
	memset(this->reunat,  0,   sizeof(reunat));

	for (u32 i=0; i<PK2MAP_MAP_MAX_PROTOTYPES; i++)
		strcpy(this->protot[i], "");

}

MapClass::MapClass(const MapClass &kartta){

	this->Kopioi(kartta);

}

MapClass::~MapClass(){
	PDraw::image_delete(this->tiles_buffer);
	PDraw::image_delete(this->bg_tiles_buffer);
	PDraw::image_delete(this->background_buffer);
	PDraw::image_delete(this->water_buffer);
	PDraw::image_delete(this->bg_water_buffer);
}

MAP_RECT MapClass::LaskeTallennusAlue(u8 *lahde, u8 *&kohde){

	u32 x, y;
	u32 kartan_vasen = PK2MAP_MAP_WIDTH,//PK2MAP_MAP_WIDTH/2,
		kartan_oikea = 0,
		kartan_yla	 = PK2MAP_MAP_HEIGHT,//PK2MAP_MAP_HEIGHT/2,
		kartan_ala	 = 0,
		kartan_korkeus = 0,
		kartan_leveys = 0;

	MAP_RECT rajat = {0,0,0,0};

	// tutkitaan kartan reunimmaiset tilet ja asetetaan reunat niiden mukaan
	for (y=0; y<PK2MAP_MAP_HEIGHT; y++) {
		for (x=0; x<PK2MAP_MAP_WIDTH; x++)	{
			if (lahde[x+y*PK2MAP_MAP_WIDTH] != 255) {
				if (x < kartan_vasen)
					kartan_vasen = x;
				if (y < kartan_yla)
					kartan_yla = y;
				if (x > kartan_oikea)
					kartan_oikea = x;
				if (y > kartan_ala)
					kartan_ala = y;
			}
		}
	}

	kartan_leveys = kartan_oikea - kartan_vasen;
	kartan_korkeus = kartan_ala - kartan_yla;

	// onko kartta tyhja?
	if (kartan_leveys < 0 || kartan_korkeus < 0) {
		kartan_vasen = kartan_yla = 0;
		kartan_ala = kartan_oikea = 1;
		kartan_leveys = kartan_oikea - kartan_vasen;
		kartan_korkeus = kartan_ala - kartan_yla;
	}

	kohde = new u8[kartan_leveys*kartan_korkeus];
	u8 tile;

	for (y=0; y<kartan_korkeus; y++){
		for (x=0; x<kartan_leveys; x++){
			tile = lahde[(x+kartan_vasen)+(y+kartan_yla)*PK2MAP_MAP_WIDTH];
			kohde[x+y*kartan_leveys] = tile;
			if (x==0 || y==0 || x==kartan_leveys-1 || y==kartan_korkeus-1)
				lahde[(x+kartan_vasen)+(y+kartan_yla)*PK2MAP_MAP_WIDTH] = 104;
		}
	}

	rajat.left = kartan_vasen;
	rajat.top  = kartan_yla;
	rajat.right = kartan_oikea;
	rajat.bottom= kartan_ala;

	return rajat;
}

MAP_RECT MapClass::LaskeTallennusAlue(u8 *alue){

	u32 x,y;
	u32 kartan_vasen		= PK2MAP_MAP_WIDTH,
		  kartan_oikea		= 0,
		  kartan_yla		= PK2MAP_MAP_HEIGHT,
		  kartan_ala		= 0;

	MAP_RECT rajat = {0,0,0,0};

	// tutkitaan kartan reunimmaiset tilet ja asetetaan reunat niiden mukaan
	for (y=0; y<PK2MAP_MAP_HEIGHT; y++) {
		for (x=0; x<PK2MAP_MAP_WIDTH; x++)	{
			if (alue[x+y*PK2MAP_MAP_WIDTH] != 255) {

				if (x < kartan_vasen) {
					kartan_vasen = x;
					//alue[x+y*PK2MAP_MAP_WIDTH] = 101;
				}
				if (y < kartan_yla) {
					kartan_yla = y;
					//alue[x+y*PK2MAP_MAP_WIDTH] = 102;
				}
				if (x > kartan_oikea) {
					kartan_oikea = x;
					//alue[x+y*PK2MAP_MAP_WIDTH] = 103;
				}
				if (y > kartan_ala) {
					kartan_ala = y;
					//alue[x+y*PK2MAP_MAP_WIDTH] = 104;
				}
			}
		}
	}

	// onko kartta tyhja?
	if (kartan_oikea < kartan_vasen || kartan_ala < kartan_yla) {
		kartan_vasen = 0;
		kartan_yla	 = 0;
		kartan_ala   = 1;
		kartan_oikea = 1;
	}

	rajat.left = kartan_vasen;
	rajat.top  = kartan_yla;
	rajat.right = kartan_oikea;
	rajat.bottom= kartan_ala;
	return rajat;
}

void MapClass::LueTallennusAlue(u8 *lahde, MAP_RECT alue, int kohde){

	int x,y;
	int kartan_vasen   = alue.left,
		kartan_oikea   = alue.right,
		kartan_yla     = alue.top,
		kartan_ala     = alue.bottom,
		kartan_korkeus = kartan_oikea - kartan_vasen,
		kartan_leveys  = kartan_ala - kartan_yla;

	u8 tile;
	if (lahde != NULL && kohde != 0)	{
		for (y=0;y<kartan_korkeus;y++) {
			for (x=0;x<kartan_leveys;x++) {
				tile = lahde[x+y*kartan_leveys];
				if (kohde == 1)
					taustat[(x+kartan_vasen)+(y+kartan_yla)*PK2MAP_MAP_WIDTH] = tile;
				if (kohde == 2)
					seinat[(x+kartan_vasen)+(y+kartan_yla)*PK2MAP_MAP_WIDTH] = tile;
				if (kohde == 3)
					spritet[(x+kartan_vasen)+(y+kartan_yla)*PK2MAP_MAP_WIDTH] = tile;
			}
		}
	}
}

int MapClass::Load(PFile::Path path){
	
	char versio[8];

	PFile::RW* file = path.GetRW("r");
	if (file == nullptr){
		return 1;
	}

	file->read(versio, sizeof(versio));
	file->close();

	int ok = 2;

	PLog::Write(PLog::DEBUG, "PK2", "Loading %s, version %s", path.c_str(), versio);

	if (strcmp(versio,"1.3")==0) {
		this->LoadVersion13(path);
		ok = 0;
	}
	if (strcmp(versio,"1.2")==0) {
		this->LoadVersion12(path);
		ok = 0;
	}
	if (strcmp(versio,"1.1")==0) {
		this->LoadVersion11(path);
		ok = 0;
	}
	if (strcmp(versio,"1.0")==0) {
		this->LoadVersion10(path);
		ok = 0;
	}
	if (strcmp(versio,"0.1")==0) {
		this->LoadVersion01(path);
		ok = 0;
	}
	
	path.SetFile(this->palikka_bmp);
	Load_TilesImage(path);

	path.SetFile(this->taustakuva);
	Load_BG(path);

	return ok;
}

int MapClass::Load_Plain_Data(PFile::Path path) {
	
	char versio[8];

	PFile::RW* file = path.GetRW("r");
	if (file == nullptr){
		return 1;
	}

	file->read(versio, sizeof(versio));
	file->close();

	if (strcmp(versio,"1.3")==0)
		this->LoadVersion13(path);

	if (strcmp(versio,"1.2")==0)
		this->LoadVersion12(path);

	if (strcmp(versio,"1.1")==0)
		this->LoadVersion11(path);

	if (strcmp(versio,"1.0")==0)
		this->LoadVersion10(path);

	if (strcmp(versio,"0.1")==0)
		this->LoadVersion01(path);

	return(0);
}

int MapClass::LoadVersion01(PFile::Path path){

	PK2KARTTA kartta;

	PFile::RW* file = path.GetRW("r");
	if (file == nullptr) {
		return 1;
	}

	file->read(&kartta, sizeof(PK2KARTTA));
	file->close();

	strcpy(this->versio, PK2MAP_LAST_VERSION);
	strcpy(this->palikka_bmp,"blox.bmp");
	strcpy(this->taustakuva, "default.bmp");
	strcpy(this->musiikki,   "default.xm");

	strcpy(this->nimi,  "v01");
	strcpy(this->tekija,"unknown");

	this->aika		= 0;
	this->extra		= 0;
	this->tausta	= kartta.taustakuva;

	for (u32 i=0;i<PK2MAP_MAP_SIZE;i++)
		this->seinat[i] = kartta.kartta[i%PK2MAP_MAP_WIDTH + (i/PK2MAP_MAP_WIDTH) * 640];

	memset(this->taustat,255, sizeof(taustat));

	memset(this->spritet,255, sizeof(spritet));

	return(0);
}
int MapClass::LoadVersion10(PFile::Path path){
	
	MapClass *kartta = new MapClass();

	PFile::RW* file = path.GetRW("r");
	if (file == nullptr) {
		return 1;
	}

	file->read(&kartta, sizeof(PK2KARTTA));
	file->close();

	strcpy(this->versio,		kartta->versio);
	strcpy(this->palikka_bmp,	kartta->palikka_bmp);
	strcpy(this->taustakuva,	kartta->taustakuva);
	strcpy(this->musiikki,		kartta->musiikki);

	strcpy(this->nimi,			kartta->nimi);
	strcpy(this->tekija,		kartta->tekija);

	this->aika			= kartta->aika;
	this->extra			= kartta->extra;
	this->tausta		= kartta->tausta;

	for (u32 i=0; i<PK2MAP_MAP_SIZE; i++)
		this->taustat[i] = kartta->taustat[i];

	for (u32 i=0; i<PK2MAP_MAP_SIZE;i++)
		this->seinat[i] = kartta->seinat[i];

	for (u32 i=0; i<PK2MAP_MAP_SIZE; i++)
		this->spritet[i] = kartta->spritet[i];


	//Load_TilesImage(kartta->palikka_bmp);
	//Load_BG(kartta->taustakuva);

	//delete kartta;

	return(0);
}
int MapClass::LoadVersion11(PFile::Path path){
	int virhe = 0;

	PFile::RW* file = path.GetRW("r");
	if (file == nullptr) {
		return 1;
	}

	memset(this->taustat, 255, sizeof(this->taustat));
	memset(this->seinat , 255, sizeof(this->seinat));
	memset(this->spritet, 255, sizeof(this->spritet));

	file->read(this->versio,      sizeof(char) * 5);
	file->read(this->palikka_bmp, sizeof(char) * 13);
	file->read(this->taustakuva,  sizeof(char) * 13);
	file->read(this->musiikki,    sizeof(char) * 13);
	file->read(this->nimi,        sizeof(char) * 40);
	file->read(this->tekija,      sizeof(char) * 40);
	file->read(&this->aika,       sizeof(int));
	file->read(&this->extra,      sizeof(u8));
	file->read(&this->tausta,     sizeof(u8));
	file->read(this->taustat,     sizeof(taustat));
	if (file->read(this->seinat,  sizeof(seinat)) != PK2MAP_MAP_SIZE)
		virhe = 2;
	file->read(this->spritet,     sizeof(spritet));

	file->close();

	for (u32 i=0;i<PK2MAP_MAP_SIZE;i++)
		if (seinat[i] != 255)
			seinat[i] -= 50;

	for (u32 i=0;i<PK2MAP_MAP_SIZE;i++)
		if (taustat[i] != 255)
			taustat[i] -= 50;

	for (u32 i=0;i<PK2MAP_MAP_SIZE;i++)
		if (spritet[i] != 255)
			spritet[i] -= 50;

	//Load_TilesImage(this->palikka_bmp);
	//Load_BG(this->taustakuva);

	return (virhe);
}
int MapClass::LoadVersion12(PFile::Path path){

	char luku[8];
	
	PFile::RW* file = path.GetRW("r");
	if (file == nullptr) {
		return 1;
	}

	memset(this->taustat, 255, sizeof(this->taustat));
	memset(this->seinat , 255, sizeof(this->seinat));
	memset(this->spritet, 255, sizeof(this->spritet));

	for (u32 i=0; i<PK2MAP_MAP_MAX_PROTOTYPES; i++)
		strcpy(this->protot[i],"");

	//tiedosto->read ((char *)this, sizeof (*this));
	file->read(versio,      sizeof(versio));
	file->read(palikka_bmp, sizeof(palikka_bmp));
	file->read(taustakuva,  sizeof(taustakuva));
	file->read(musiikki,    sizeof(musiikki));
	file->read(nimi,        sizeof(nimi));
	file->read(tekija,      sizeof(tekija));

	file->read(luku, sizeof(luku));
	this->jakso = atoi(luku);

	file->read(luku, sizeof(luku));
	this->ilma = atoi(luku);

	file->read(luku, sizeof(luku));
	this->kytkin1_aika = atoi(luku);

	file->read(luku, sizeof(luku));
	this->kytkin2_aika = atoi(luku);

	file->read(luku, sizeof(luku));
	this->kytkin3_aika = atoi(luku);

	file->read(luku, sizeof(luku));
	this->aika = atoi(luku);

	file->read(luku, sizeof(luku));
	this->extra = atoi(luku);

	file->read(luku, sizeof(luku));
	this->tausta = atoi(luku);

	file->read(luku, sizeof(luku));
	this->pelaaja_sprite = atoi(luku);

	file->read(taustat, sizeof(taustat));
	file->read(seinat,  sizeof(seinat));
	file->read(spritet, sizeof(spritet));

	file->read(protot, sizeof(protot[0]) * PK2MAP_MAP_MAX_PROTOTYPES);

	file->close();

	//Load_TilesImage(this->palikka_bmp);
	//Load_BG(this->taustakuva);

	return 0;
}
int MapClass::LoadVersion13(PFile::Path path){

	char luku[8];
	u32 i;

	PFile::RW* file = path.GetRW("r");
	if (file == nullptr) {
		return 1;
	}

	memset(this->taustat, 255, sizeof(this->taustat));
	memset(this->seinat , 255, sizeof(this->seinat));
	memset(this->spritet, 255, sizeof(this->spritet));

	for (i=0;i<PK2MAP_MAP_MAX_PROTOTYPES;i++)
		strcpy(this->protot[i],"");

	file->read(versio,      sizeof(versio));
	file->read(palikka_bmp, sizeof(palikka_bmp));
	file->read(taustakuva,  sizeof(taustakuva));
	file->read(musiikki,    sizeof(musiikki));
	file->read(nimi,        sizeof(nimi));

	for (int i = 38; i > 0 && (nimi[i] == (char)0xCD); i--)
		nimi[i] = 0;
	
	file->read(tekija,      sizeof(tekija));

	file->read(luku, sizeof(luku));
	this->jakso = atoi(luku);
	memset(luku, 0, sizeof(luku));

	file->read(luku, sizeof(luku));
	this->ilma = atoi(luku);
	memset(luku, 0, sizeof(luku));

	file->read(luku, sizeof(luku));
	this->kytkin1_aika = atoi(luku);
	memset(luku, 0, sizeof(luku));

	file->read(luku, sizeof(luku));
	this->kytkin2_aika = atoi(luku);
	memset(luku, 0, sizeof(luku));

	file->read(luku, sizeof(luku));
	this->kytkin3_aika = atoi(luku);
	memset(luku, 0, sizeof(luku));

	file->read(luku, sizeof(luku));
	this->aika = atoi(luku);
	memset(luku, 0, sizeof(luku));

	file->read(luku, sizeof(luku));
	this->extra = atoi(luku);
	memset(luku, 0, sizeof(luku));

	file->read(luku, sizeof(luku));
	this->tausta = atoi(luku);
	memset(luku, 0, sizeof(luku));

	file->read(luku, sizeof(luku));
	this->pelaaja_sprite = atoi(luku);
	memset(luku, 0, sizeof(luku));

	file->read(luku, sizeof(luku));
	this->x = atoi(luku);
	memset(luku, 0, sizeof(luku));

	file->read(luku, sizeof(luku));
	this->y = atoi(luku);
	memset(luku, 0, sizeof(luku));

	file->read(luku, sizeof(luku));
	this->icon = atoi(luku);
	memset(luku, 0, sizeof(luku));

	u32 lkm;
	file->read(luku, sizeof(luku));
	lkm = (int)atoi(luku);

	file->read(protot, sizeof(protot[0]) * lkm);

	u32 leveys, korkeus;
	u32 offset_x, offset_y;

	// taustat
	file->read(luku, sizeof(luku)); offset_x = atol(luku); memset(luku, 0, sizeof(luku));
	file->read(luku, sizeof(luku)); offset_y = atol(luku); memset(luku, 0, sizeof(luku));
	file->read(luku, sizeof(luku)); leveys   = atol(luku); memset(luku, 0, sizeof(luku));
	file->read(luku, sizeof(luku)); korkeus  = atol(luku); memset(luku, 0, sizeof(luku));
	for (u32 y = offset_y; y <= offset_y + korkeus; y++) {
		u32 x_start = offset_x + y * PK2MAP_MAP_WIDTH;
		file->read(&taustat[x_start], leveys + 1);
	}

	// seinat
	file->read(luku, sizeof(luku)); offset_x = atol(luku); memset(luku, 0, sizeof(luku));
	file->read(luku, sizeof(luku)); offset_y = atol(luku); memset(luku, 0, sizeof(luku));
	file->read(luku, sizeof(luku)); leveys   = atol(luku); memset(luku, 0, sizeof(luku));
	file->read(luku, sizeof(luku)); korkeus  = atol(luku); memset(luku, 0, sizeof(luku));
	for (u32 y = offset_y; y <= offset_y + korkeus; y++) {
		u32 x_start = offset_x + y * PK2MAP_MAP_WIDTH;
		file->read(&seinat[x_start], leveys + 1);
	}

	//spritet
	file->read(luku, sizeof(luku)); offset_x = atol(luku); memset(luku, 0, sizeof(luku));
	file->read(luku, sizeof(luku)); offset_y = atol(luku); memset(luku, 0, sizeof(luku));
	file->read(luku, sizeof(luku)); leveys   = atol(luku); memset(luku, 0, sizeof(luku));
	file->read(luku, sizeof(luku)); korkeus  = atol(luku); memset(luku, 0, sizeof(luku));
	for (u32 y = offset_y; y <= offset_y + korkeus; y++) {
		u32 x_start = offset_x + y * PK2MAP_MAP_WIDTH;
		file->read(&spritet[x_start], leveys + 1);
	}

	file->close();

	return 0;
}

void MapClass::Tyhjenna(){
	strcpy(this->versio, PK2MAP_LAST_VERSION);
	strcpy(this->palikka_bmp,"blox.bmp");
	strcpy(this->taustakuva, "default.bmp");
	strcpy(this->musiikki,   "default.xm");

	strcpy(this->nimi,  "untitled");
	strcpy(this->tekija,"unknown");

	this->jakso			= 0;
	this->ilma			= ILMA_NORMAALI;
	this->kytkin1_aika	= SWITCH_INITIAL_VALUE;
	this->kytkin2_aika	= SWITCH_INITIAL_VALUE;
	this->kytkin3_aika	= SWITCH_INITIAL_VALUE;
	this->pelaaja_sprite = 0;
	this->aika		= 0;
	this->extra		= PK2KARTTA_EXTRA_EI;
	this->tausta	= PK2KARTTA_TAUSTAKUVA_EI;

	this->x = 0;
	this->y = 0;
	this->icon = 0;

	memset(this->taustat, 255, sizeof(taustat));
	memset(this->seinat,  255, sizeof(seinat));
	memset(this->spritet, 255, sizeof(spritet));

	for (u32 i=0;i<PK2MAP_MAP_MAX_PROTOTYPES;i++)
		strcpy(this->protot[i],"");

	//PDraw::image_fill(this->tiles_buffer,255);
	//PDraw::image_fill(this->background_buffer,255);
}

MapClass &MapClass::operator = (const MapClass &kartta){

	this->Kopioi(kartta);
	return *this;
}

int MapClass::Load_BG(PFile::Path path){
	
	if (!FindAsset(&path, "gfx" PE_SEP "scenery" PE_SEP))
		return 1;

	PDraw::image_load(this->background_buffer, path, true);
	if (this->background_buffer == -1)
		return -2;

	//strcpy(this->taustakuva, filename.c_str());

	/*
	u8 *buffer = NULL;
	u32 leveys;

	PDraw::drawimage_start(background_buffer, buffer, leveys);

	for ( int x = 0; x < 640; x++)
		for ( int y = 0; y < 480; y++) {

			if (buffer[x+y*leveys] == 255)
				buffer[x+y*leveys] = 254;
			
		}

	PDraw::drawimage_end(background_buffer);
	*/

	return 0;
}

int MapClass::Load_TilesImage(PFile::Path path){
	
	PFile::Path bkp = path;

	if (!FindAsset(&path, "gfx" PE_SEP "tiles" PE_SEP))
		return 1;

	PDraw::image_load(this->tiles_buffer, path, false);
	if (this->tiles_buffer == -1)
		return 2;

	PDraw::image_delete(this->water_buffer); //Delete last water buffer
	this->water_buffer = PDraw::image_cut(this->tiles_buffer,0,416,320,32);

	// load bg buffer
	{
		// transform tiles01.bmp to tiles01_bg.bmp
		path = bkp;
		std::string filename = path.GetFileName();
		size_t i = filename.find_last_of('.');
		filename = filename.substr(0, i) + "_bg" + filename.substr(i, std::string::npos);
		path.SetFile(filename);
		if (FindAsset(&path, "gfx" PE_SEP "tiles" PE_SEP)) {
			PDraw::image_load(this->bg_tiles_buffer, path, false);
			if (this->bg_tiles_buffer >= 0) {
				PDraw::image_delete(this->bg_water_buffer); //Delete last water buffer
				this->bg_water_buffer = PDraw::image_cut(this->bg_tiles_buffer,0,416,320,32);
			}
		}
	}

	return 0;

}

int MapClass::Load_BGSfx(PFile::Path path){

	return 0;
}

void MapClass::Kopioi(const MapClass &kartta){
	if (this != &kartta){
		strcpy(this->versio,		kartta.versio);
		strcpy(this->palikka_bmp,	kartta.palikka_bmp);
		strcpy(this->taustakuva,	kartta.taustakuva);
		strcpy(this->musiikki,		kartta.musiikki);

		strcpy(this->nimi,			kartta.nimi);
		strcpy(this->tekija,		kartta.tekija);

		this->jakso			= kartta.jakso;
		this->ilma			= kartta.ilma;
		this->kytkin1_aika	= kartta.kytkin1_aika;
		this->kytkin2_aika	= kartta.kytkin2_aika;
		this->kytkin3_aika	= kartta.kytkin3_aika;
		this->pelaaja_sprite = kartta.pelaaja_sprite;
		this->aika		= kartta.aika;
		this->extra		= kartta.extra;
		this->tausta	= kartta.tausta;

		for (u32 i=0; i<PK2MAP_MAP_SIZE; i++)
			this->seinat[i] = kartta.seinat[i];

		for (u32 i=0; i<PK2MAP_MAP_SIZE; i++)
			this->taustat[i] = kartta.taustat[i];

		for (u32 i=0; i<PK2MAP_MAP_SIZE; i++)
			this->spritet[i] = kartta.spritet[i];

		for (u32 i=0;i<PK2MAP_MAP_MAX_PROTOTYPES;i++)
			strcpy(this->protot[i],kartta.protot[i]);

		PDraw::image_delete(this->background_buffer);
		PDraw::image_delete(this->tiles_buffer);
		PDraw::image_delete(this->water_buffer);
		PDraw::image_delete(this->bg_water_buffer);

		this->background_buffer = PDraw::image_copy(kartta.background_buffer);
		this->tiles_buffer = PDraw::image_copy(kartta.tiles_buffer);
		this->water_buffer = PDraw::image_copy(kartta.water_buffer);
		this->bg_water_buffer = PDraw::image_copy(kartta.bg_water_buffer);
	}
}

/* Pekka Kana 2 funcitons ---------------------------------------------------------------*/


// Put in GameClass
void MapClass::Place_Sprites() {

	Sprites_clear();
	Sprites_add(this->pelaaja_sprite, 1, 0, 0, MAX_SPRITEJA, false);

	for (u32 x = 0; x < PK2MAP_MAP_WIDTH; x++) {
		for (u32 y = 0; y < PK2MAP_MAP_HEIGHT; y++) {

			int sprite = this->spritet[x+y*PK2MAP_MAP_WIDTH];

			if (sprite != 255 && Prototypes_List[sprite].korkeus > 0) {

				char* name = Prototypes_List[sprite].nimi;
				if (!Episode->ignore_collectable && strncmp(name, Episode->collectable_name.c_str(), Episode->collectable_name.size()) == 0)
					Game->apples_count++;

				Sprites_add(sprite, 0, x*32, y*32 - Prototypes_List[sprite].korkeus+32, MAX_SPRITEJA, false);
				
			}
		}
	}

	Sprites_sort_bg();

}

void MapClass::Select_Start() {
	double  pos_x = 320,
			pos_y = 196;
	uint starts_count = 0, alku = 0;
	uint x, y;

	for (x=0; x<PK2MAP_MAP_SIZE; x++)
		if (this->seinat[x] == BLOCK_ALOITUS)
			starts_count++;

	if (starts_count > 0){
		alku = rand()%starts_count + 1;
		starts_count = 1;

		for (x=0; x < PK2MAP_MAP_WIDTH; x++)
			for (y=0; y < PK2MAP_MAP_HEIGHT; y++)
				if (this->seinat[x+y*PK2MAP_MAP_WIDTH] == BLOCK_ALOITUS){
					if (starts_count == alku){
						pos_x = x*32;
						pos_y = y*32;
					}

					starts_count ++;
				}
	}

	Player_Sprite->x = pos_x + Player_Sprite->tyyppi->leveys/2;
	Player_Sprite->y = pos_y - Player_Sprite->tyyppi->korkeus/2;

	Game->camera_x = (int)Player_Sprite->x;
	Game->camera_y = (int)Player_Sprite->y;
	Game->dcamera_x = Game->camera_x;
	Game->dcamera_y = Game->camera_y;

}


int MapClass::Count_Keys() {
	u8 sprite;
	u32 x;

	int keys = 0;

	for (x=0; x < PK2MAP_MAP_SIZE; x++){
		sprite = this->spritet[x];
		if (sprite != 255)
			if (Prototypes_List[sprite].avain && 
				Prototypes_List[sprite].tuhoutuminen != FX_DESTRUCT_EI_TUHOUDU)

				keys++;
	}

	return keys;
}

void MapClass::Change_SkullBlocks() {
	u8 front, back;
	u32 x,y;

	for (x=0; x<PK2MAP_MAP_WIDTH; x++)
		for (y=0; y<PK2MAP_MAP_HEIGHT; y++){
			front = this->seinat[x+y*PK2MAP_MAP_WIDTH];
			back  = this->taustat[x+y*PK2MAP_MAP_WIDTH];

			if (front == BLOCK_KALLOSEINA){
				this->seinat[x+y*PK2MAP_MAP_WIDTH] = 255;
				if (back != BLOCK_KALLOSEINA)
					Effect_SmokeClouds(x*32+24,y*32+6);

			}

			if (back == BLOCK_KALLOTAUSTA && front == 255)
				this->seinat[x+y*PK2MAP_MAP_WIDTH] = BLOCK_KALLOSEINA;
		}

	//Put in game
	Game->vibration = 90;//60
	PInput::Vibrate(1000);

	//Game->Show_Info(tekstit->Get_Text(PK_txt.game_locksopen));

	Calculate_Edges();
}

void MapClass::Open_Locks() {
	u8 palikka;
	u32 x,y;

	for (x=0; x < PK2MAP_MAP_WIDTH; x++)
		for (y=0; y < PK2MAP_MAP_HEIGHT; y++){
			palikka = this->seinat[x+y*PK2MAP_MAP_WIDTH];
			if (palikka == BLOCK_LUKKO){
				this->seinat[x+y*PK2MAP_MAP_WIDTH] = 255;
				Effect_SmokeClouds(x*32+6,y*32+6);
			}
		}

	//Put in game
	Game->vibration = 90;//60
	PInput::Vibrate(1000);

	Game->Show_Info(tekstit->Get_Text(PK_txt.game_locksopen));

	Calculate_Edges();

}

void MapClass::Calculate_Edges(){
	u8 tile1, tile2, tile3;
	bool edge = false;

	memset(this->reunat, false, sizeof(this->reunat));

	for (u32 x=1; x<PK2MAP_MAP_WIDTH-1; x++)
		for (u32 y=0; y<PK2MAP_MAP_HEIGHT-1; y++){
			edge = false;

			tile1 = this->seinat[x+y*PK2MAP_MAP_WIDTH];

			if (tile1 > BLOCK_LOPETUS)
				this->seinat[x+y*PK2MAP_MAP_WIDTH] = 255;

			tile2 = this->seinat[x+(y+1)*PK2MAP_MAP_WIDTH];

			if (tile1 > 79 || tile1 == BLOCK_ESTO_ALAS) tile1 = 1; else tile1 = 0;
			if (tile2 > 79) tile2 = 1; else tile2 = 0;

			if (tile1 == 1 && tile2 == 1){
				tile1 = this->seinat[x+1+(y+1)*PK2MAP_MAP_WIDTH];
				tile2 = this->seinat[x-1+(y+1)*PK2MAP_MAP_WIDTH];

				if (tile1 < 80  && !(tile1 < 60 && tile1 > 49)) tile1 = 1; else tile1 = 0;
				if (tile2 < 80  && !(tile2 < 60 && tile2 > 49)) tile2 = 1; else tile2 = 0;

				if (tile1 == 1){
					tile3 = this->seinat[x+1+y*PK2MAP_MAP_WIDTH];
					if (tile3 > 79 || (tile3 < 60 && tile3 > 49) || tile3 == BLOCK_ESTO_ALAS)
						edge = true;
				}

				if (tile2 == 1){
					tile3 = this->seinat[x-1+y*PK2MAP_MAP_WIDTH];
					if (tile3 > 79 || (tile3 < 60 && tile3 > 49) || tile3 == BLOCK_ESTO_ALAS)
						edge = true;
				}

				if (edge){
					this->reunat[x+y*PK2MAP_MAP_WIDTH] = true;
					//this->taustat[x+y*PK2MAP_MAP_WIDTH] = 49; //Debug
				}
			}
		}
}

/* Kartanpiirtorutiineja ----------------------------------------------------------------*/

void MapClass::Animate_Fire(int tiles){
	u8 *buffer = NULL;
	u32 leveys;
	int x,y;
	int color;

	PDraw::drawimage_start(tiles, buffer, leveys);

	for (x=128;x<160;x++)
		for (y=448;y<479;y++)
		{
			color = buffer[x+(y+1)*leveys];

			if (color != 255)
			{
				color %= 32;
				color = color - rand()%4;

				if (color < 0)
					color = 255;
				else
				{
					if (color > 21)
						color += 128;
					else
						color += 64;
				}
			}

			buffer[x+y*leveys] = color;
		}

	if (ajastin1 < 20)
	{
		for (x=128;x<160;x++)
			buffer[x+479*leveys] = rand()%15+144;
	}
	else
		for (x=128;x<160;x++)
			buffer[x+479*leveys] = 255;

	PDraw::drawimage_end(tiles);
}

void MapClass::Animate_Waterfall(int tiles){
	u8 *buffer = NULL;
	u32 leveys;
	int x,y,plus;
	int color,color2;

	u8 temp[32*32];

	PDraw::drawimage_start(tiles, buffer, leveys);

	for (x=32;x<64;x++)
		for (y=416;y<448;y++)
			temp[x-32+(y-416)*32] = buffer[x+y*leveys];

	color2 = (temp[0]/32)*32;	// mahdollistaa erivriset vesiputoukset

	for (x=32;x<64;x++)
	{
		plus = rand()%2+2;//...+1
		for (y=416;y<448;y++)
		{
			color = temp[x-32+(y-416)*32];

			if (color != 255)	// mahdollistaa eri leveyksiset vesiputoukset
			{
				color %= 32;
				if (color > 10)//20
					color--;
				if (rand()%40 == 1)
					color = 11+rand()%11;//15+rand()%8;//9+rand()%5;
				if (rand()%160 == 1)
					color = 30;
				buffer[x + (416+(y+plus)%32)*leveys] = color+color2;
			}
			else
				buffer[x + (416+(y+plus)%32)*leveys] = color;
		}
	}

	PDraw::drawimage_end(tiles);
}

void MapClass::Animate_WaterSurface(int tiles){
	u8 *buffer = NULL;
	u32 leveys;
	int x,y;

	u8 temp[32];

	PDraw::drawimage_start(tiles, buffer, leveys);

	for (y=416;y<448;y++)
		temp[y-416] = buffer[y*leveys];

	for (y=416;y<448;y++)
	{
		for (x=0;x<31;x++)
		{
			buffer[x+y*leveys] = buffer[x+1+y*leveys];
		}
	}

	for (y=416;y<448;y++)
		buffer[31+y*leveys] = temp[y-416];

	PDraw::drawimage_end(tiles);
}

void MapClass::Animate_Water(int tiles, int water_tiles){
	u8 *buffer_lahde = NULL, *buffer_kohde = NULL;
	u32 leveys_lahde, leveys_kohde;
	int x, y, color1, color2,
		d1 = vesiaste / 2, d2;
	int sini, cosi;
	int vx,vy;
	int i;


	PDraw::drawimage_start(tiles, buffer_kohde, leveys_kohde);
	PDraw::drawimage_start(water_tiles, buffer_lahde, leveys_lahde);

	for (y=0;y<32;y++){
		d2 = d1;

		for (x=0;x<32;x++){
			sini = int((y+d2/2)*11.25)%360;
			cosi = int((x+d1/2)*11.25)%360;
			sini = (int)sin_table[sini];
			cosi = (int)cos_table[cosi];

			vy = (y+sini/11)%32;
			vx = (x+cosi/11)%32;

			if (vy < 0){
				vy = -vy;
				vy = 31-(vy%32);
			}

			if (vx < 0){
				vx= -vx;
				vx = 31-(vx%32);
			}

			color1 = buffer_lahde[64+vx+vy*leveys_lahde];
			buffer_lahde[32+x+y*leveys_lahde] = color1;
			d2 = 1 + d2 % 360;
		}

		d1 = 1 + d1 % 360;
	}

	int vy2;

	for (int p=2;p<5;p++){
		i = p*32;

		for (y=0;y<32;y++){
			//d2 = d1;
			vy = y*leveys_lahde;
			vy2 = (y+416)*leveys_kohde;

			for (x=0;x<32;x++){
				vx = x+vy;
				color1 = buffer_lahde[32+vx];
				color2 = buffer_lahde[ i+vx];
				buffer_kohde[i+x+vy2] = (color1 + color2*2) / 3;
			}
		}
	}
	PDraw::drawimage_end(tiles);
	PDraw::drawimage_end(water_tiles);
}

void MapClass::Animate_RollUp(int tiles){
	u8 *buffer = NULL;
	u32 leveys;
	int y;

	u8 temp[32];

	PDraw::drawimage_start(tiles, buffer, leveys);

	memcpy(temp, 64 + buffer + 448*leveys, 32);

	for (y=448; y<479; y++)
		memcpy(buffer + 64 + y*leveys, buffer + 64 + (y+1)*leveys, 32);

	memcpy(buffer + 64 + 479*leveys, temp, 32);

	PDraw::drawimage_end(tiles);
}

int MapClass::Piirra_Taustat(int kamera_x, int kamera_y, bool editor){
	
	int kartta_x = kamera_x/32;
	int kartta_y = kamera_y/32;

	int tiles_w = screen_width/32 + 1;
	int tiles_h = screen_height/32 + 1;

	int buffer = bg_tiles_buffer;
	if (buffer < 0)
		buffer = tiles_buffer;

	for (int x = 0; x < tiles_w; x++){
		if (x + kartta_x < 0 || uint(x + kartta_x) > PK2MAP_MAP_WIDTH) continue;

		for (int y = 0; y < tiles_h; y++){
			if (y + kartta_y < 0 || uint(y + kartta_y) > PK2MAP_MAP_HEIGHT) continue;

			int i = x + kartta_x + (y + kartta_y) * PK2MAP_MAP_WIDTH;
			if( i < 0 || i >= int(sizeof(taustat)) ) continue; //Dont access a not allowed address

			int palikka = taustat[i];

			if (palikka != 255){
				int px = ((palikka%10)*32);
				int py = ((palikka/10)*32);

				if (palikka == BLOCK_ANIM1 || palikka == BLOCK_ANIM2 || palikka == BLOCK_ANIM3 || palikka == BLOCK_ANIM4)
					px += animaatio * 32;

				PDraw::image_cutclip(buffer, x*32-(kamera_x%32), y*32-(kamera_y%32), px, py, px+32, py+32);
			}
		}
	}

	return 0;
}

int MapClass::Piirra_Seinat(int kamera_x, int kamera_y, bool editor){
	int palikka;
	int px = 0,
		py = 0,
		ay = 0,
		ax = 0,
		kartta_x = kamera_x/32,
		kartta_y = kamera_y/32;

	int ajastin1_y = 0,
		ajastin2_y = 0,
		ajastin3_x = 0;

	int tiles_w = screen_width/32 + 1;
	int tiles_h = screen_height/32 + 1;

	if (ajastin1 > 0){
		ajastin1_y = 64;

		if (ajastin1 < 64)
			ajastin1_y = ajastin1;

		if (ajastin1 > SWITCH_INITIAL_VALUE-64)
			ajastin1_y = SWITCH_INITIAL_VALUE - ajastin1;
	}

	if (ajastin2 > 0){
		ajastin2_y = 64;

		if (ajastin2 < 64)
			ajastin2_y = ajastin2;

		if (ajastin2 > SWITCH_INITIAL_VALUE-64)
			ajastin2_y = SWITCH_INITIAL_VALUE - ajastin2;
	}

	if (ajastin3 > 0){
		ajastin3_x = 64;

		if (ajastin3 < 64)
			ajastin3_x = ajastin3;

		if (ajastin3 > SWITCH_INITIAL_VALUE-64)
			ajastin3_x = SWITCH_INITIAL_VALUE - ajastin3;
	}


	for (int x = -1; x < tiles_w + 1; x++) {
		if (x + kartta_x < 0 || uint(x + kartta_x) > PK2MAP_MAP_WIDTH) continue;

		for (int y = -1; y < tiles_h + 1; y++) {
			if (y + kartta_y < 0 || uint(y + kartta_y) > PK2MAP_MAP_HEIGHT) continue;

			int i = x + kartta_x + (y + kartta_y) * PK2MAP_MAP_WIDTH;
			if( i < 0 || i >= int(sizeof(seinat)) ) continue; //Dont access a not allowed address

			palikka = seinat[i];

			if (palikka != 255 && !(!editor && palikka == BLOCK_ESTO_ALAS)){
				px = ((palikka%10)*32);
				py = ((palikka/10)*32);
				ay = 0;
				ax = 0;

				if (!editor){
					if (palikka == BLOCK_HISSI_VERT)
						ay = floor(sin_table[aste%360]);

					if (palikka == BLOCK_HISSI_HORI)
						ax = floor(cos_table[aste%360]);

					if (palikka == BLOCK_KYTKIN1)
						ay = ajastin1_y/2;

					if (palikka == BLOCK_KYTKIN2_YLOS)
						ay = -ajastin2_y/2;

					if (palikka == BLOCK_KYTKIN2_ALAS)
						ay = ajastin2_y/2;

					if (palikka == BLOCK_KYTKIN2)
						ay = ajastin2_y/2;

					if (palikka == BLOCK_KYTKIN3_OIKEALLE)
						ax = ajastin3_x/2;

					if (palikka == BLOCK_KYTKIN3_VASEMMALLE)
						ax = -ajastin3_x/2;

					if (palikka == BLOCK_KYTKIN3)
						ay = ajastin3_x/2;
				}

				if (palikka == BLOCK_ANIM1 || palikka == BLOCK_ANIM2 || palikka == BLOCK_ANIM3 || palikka == BLOCK_ANIM4)
					px += animaatio * 32;

				PDraw::image_cutclip(tiles_buffer, x*32-(kamera_x%32)+ax, y*32-(kamera_y%32)+ay, px, py, px+32, py+32);
			}
		}
	}

	if (vesiaste%2 == 0)
	{
		Animate_Fire(this->tiles_buffer);
		Animate_Waterfall(this->tiles_buffer);
		Animate_RollUp(this->tiles_buffer);
		Animate_WaterSurface(this->tiles_buffer);

		if (this->bg_tiles_buffer >= 0) {
			Animate_Fire(this->bg_tiles_buffer);
			Animate_Waterfall(this->bg_tiles_buffer);
			Animate_RollUp(this->bg_tiles_buffer);
			Animate_WaterSurface(this->bg_tiles_buffer);
		}
	}

	if (vesiaste%4 == 0)
	{
		Animate_Water(this->tiles_buffer, this->water_buffer);
		if (this->bg_tiles_buffer >= 0)
			Animate_Water(this->bg_tiles_buffer, this->bg_water_buffer);

		PDraw::rotate_palette(224,239);
	}

	vesiaste = 1 + vesiaste % 320;

	return 0;
}
