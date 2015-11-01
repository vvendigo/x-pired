#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#ifdef UNIX

#include"xpired_unix.h"

#endif

#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_image.h>
#include <SDL_gfxPrimitives.h>

#define	ShowFPS 0
#define	GCycle 33
#define	TextDelay 100
#define	DeadDelay 30
#define	TURBO 1
#define	APPNAME "X-pired"
#define VERSION "1.22"

// pridame konstanty...
#define MaxLevel 100
#define MaxLvlSize 20
#define MaxSpr 256
#define MaxText 10
#define MaxStrLenShort 51
#define MaxStrLenLong 256
#define MaxStrLenXLong 1024

#define MaxRecLength 5120

#ifdef UNIX

	char *SHARE_DIR=SHARE_PREFIX;

#else
	
	char *SHARE_DIR=".";

#endif

char GAME_HOME[MaxStrLenXLong];


Uint8	BPP=8;
char	FULLSCR=0;
Uint8	Shades=128;

SDL_Surface *screen,*swapscreen,*TitleS,*Sh=NULL,*PlayerV,*PlayerH,*Player[3][4][20],*SLevel,*SLives,*STime,*SNumbers;

char Sound=1,Quit=0,PAlive=20,Left=0,Right=0,Up=0,Down=0,Fire=0,Alt=0,Ctrl=0,Space=0,Setup=0,Passwd[11]="",BEM;
int  Level,Lives,PX,PY,PPX,PPY,PF=1,LM=1,DRX1,DRX2,DRY1,DRY2,DRT,DRA=0,DRB[3][4];
int  WIDTH=400,HEIGHT=400;
char Timed;
int Sec;
Uint16 UC=0;

SDL_Event event;
SDL_Joystick *js; // added by Largos for joysticks

Mix_Chunk *Snd_Expl,*Snd_Push,*Snd_Burn,*Snd_Fall,*Snd_Beep,*Snd_GameOver,*Snd_Teleport;
Mix_Music *Music;
int MVolume=64;
int SVolume=128;

typedef struct T_Txt{
	int x,y,a;
	char t[MaxStrLenShort];
}T_Txt;
T_Txt Text[MaxText];


typedef struct T_Spr{
	SDL_Surface *img,*sha;
}T_Spr;

T_Spr Spr[MaxSpr];


typedef struct T_LElem{
	unsigned char	FSpr,FTyp,BSpr,BTyp;
	char			x,y,px,py,f,txt;
}T_LElem;	

typedef struct T_Lev{
	T_LElem			M[MaxLvlSize][MaxLvlSize];
	char			Name[MaxStrLenShort],Pw[MaxStrLenShort];
	unsigned int	DL;
	SDL_Surface		*Bg;
	char			Text[MaxText][MaxStrLenShort];
}T_Lev;

T_Lev Lvl[MaxLevel];

T_Lev ALvl;

typedef struct T_RecKey{
	Uint32	Tick;
	unsigned char	Left:1;
	unsigned char	Right:1;
	unsigned char	Up:1;
	unsigned char	Down:1;
	unsigned char	Fire:1;
	unsigned char	Quit:3;
}T_RecKey;

T_RecKey Rec[MaxRecLength];

char ConfFName[255];

//-END DEF-------------------------------------

// --- Music ---

void MusicLoopback(void){
//	printf("Module finished...\n");
	Mix_PlayMusic(Music,0);
	Mix_VolumeMusic(MVolume);
}

void PlaySample(Mix_Chunk *Snd){
	int Ch;
	if (Sound)
		if (Mix_PlayChannel(-1,Snd,0)<0){
			Ch=Mix_GroupOldest(-1);
			Mix_HaltChannel(Ch);
			Mix_PlayChannel(Ch,Snd,0);
		}
}

// --- Video ---

void VIDEO_INIT(){
Uint32 videoflags;
const SDL_VideoInfo *info;

	info = SDL_GetVideoInfo();
	BPP=info->vfmt->BitsPerPixel;

	if(FULLSCR){
		videoflags = SDL_SRCALPHA | SDL_FULLSCREEN | SDL_SWSURFACE;
	}else{
		videoflags = SDL_SRCALPHA;
		if (TURBO) videoflags=videoflags | SDL_HWSURFACE | SDL_HWPALETTE;
		else videoflags=videoflags | SDL_SWSURFACE;
	}


	videoflags=videoflags ;

	if ( (screen=SDL_SetVideoMode(WIDTH,HEIGHT,BPP,videoflags)) == NULL ) {
		fprintf(stderr, "Couldn't set %ix%i video mode: %s\n",WIDTH,HEIGHT,SDL_GetError());
		exit(2);
	}

	SDL_ShowCursor(FULLSCR?SDL_DISABLE:SDL_ENABLE);
//	SDL_ShowCursor(SDL_ENABLE);
}


void ClearSurface(SDL_Surface *Co){
Uint32 color;
SDL_Rect clip;

	clip.x = 0;
	clip.y = 0;
	clip.w = Co->w;
	clip.h = Co->h;

	color=SDL_MapRGBA(Co->format, 255,0,255,0);

	SDL_FillRect (Co, &clip, color);
}


// --- Event filter ---

int E_FILTER(const SDL_Event *event) {

	switch( event->type ){
	case SDL_KEYDOWN:
		if (event->key.keysym.sym==SDLK_LEFT) Left=1;
		if (event->key.keysym.sym==SDLK_RIGHT) Right=1;
		if (event->key.keysym.sym==SDLK_UP) Up=1;
		if (event->key.keysym.sym==SDLK_DOWN) Down=1;
		if (event->key.keysym.sym==SDLK_RCTRL) Fire=1;
		if (event->key.keysym.sym==SDLK_LCTRL) Fire=1;
		if (event->key.keysym.sym==SDLK_LCTRL) Ctrl=1;
		if (event->key.keysym.sym==SDLK_SPACE) Space=1;
		if (event->key.keysym.sym==SDLK_ESCAPE) Quit=1;
		if (event->key.keysym.sym==SDLK_LALT) Alt=1;
		if (event->key.keysym.sym==SDLK_RALT) Alt=1;
		if (event->key.keysym.sym==SDLK_x && Alt) Quit=2;
		if ((event->key.keysym.sym==SDLK_s) && Alt) Setup=1;
		if ((event->key.keysym.sym==SDLK_q) && Alt) Quit=2;
		if ((event->key.keysym.sym==SDLK_F4) && Alt) Quit=2;
		if (Alt&&((event->key.keysym.sym==SDLK_RETURN)||(event->key.keysym.sym==SDLK_f))){
			SDL_BlitSurface(screen,NULL,swapscreen,NULL);
			FULLSCR=!FULLSCR;
			VIDEO_INIT();
			SDL_BlitSurface(swapscreen,NULL,screen,NULL);
			SDL_UpdateRect(screen,0,0,0,0);
		}else
			UC=event->key.keysym.unicode;
		break;

	case SDL_KEYUP:
		if (event->key.keysym.sym==SDLK_LEFT) Left=0;
		if (event->key.keysym.sym==SDLK_RIGHT) Right=0;
		if (event->key.keysym.sym==SDLK_UP) Up=0;
		if (event->key.keysym.sym==SDLK_DOWN) Down=0;
		if (event->key.keysym.sym==SDLK_RCTRL) Fire=0;
		if (event->key.keysym.sym==SDLK_LCTRL) Fire=0;
		if (event->key.keysym.sym==SDLK_LCTRL) Ctrl=0;
		if (event->key.keysym.sym==SDLK_SPACE) Space=0;
		if (event->key.keysym.sym==SDLK_ESCAPE) Quit=0;
		if (event->key.keysym.sym==SDLK_LALT) Alt=0;
		if (event->key.keysym.sym==SDLK_RALT) Alt=0;
		break;
	
	case SDL_JOYAXISMOTION:
		if( event->jaxis.axis == 0 ){
		// motion was on the horizontal axis
		if( event->jaxis.value > 0 ){
		// moved left
		Left =0;
		Right=1;
		Up   =0;
		Down =0;
		}else if( event->jaxis.value < 0 ){
		// moved right
		Left =1;
		Right=0;
		Up   =0;
		Down =0;
		}else{
		// released ( the stick is centered )
		Left =0;
		Right=0;
		Up   =0;
		Down =0;
		}

		}else{
		// motion was on the vertical axis
		if( event->jaxis.value > 0 ){
		// moved down
		Left =0;
		Right=0;
		Up   =0;
		Down =1;
		}else if( event->jaxis.value < 0 ){
		// moved up
		Left =0;
		Right=0;
		Up   =1;
		Down =0;
		}else{
		// released ( the stick is centered )
		Left =0;
		Right=0;
		Up   =0;
		Down =0;
		}
		}
		break;

	case SDL_JOYBUTTONUP:
		if( event->jbutton.button == 0 ){
			Fire=0;
		}else if( event->jbutton.button == 0 ){
			Quit=0;
		}
	break;

	case SDL_JOYBUTTONDOWN:
		if( event->jbutton.button == 0 ){
			Fire=1;
		}else if( event->jbutton.button == 1 ){
			Quit=1;
		}
		break;
	// add new key event handlers here.


	case SDL_QUIT:	//this case needs to drop through to
		Quit=2;		// default or the incorrect value will be returned.
	
	default:
		return(1);
	}
	return(0);
}



int chartoint(char C){
	return (C>='0'&&C<='9')?(C-'0'):-1;
}


// --- misc. load/save ---

void LoadSprite(SDL_Surface **Co, char *Soubor, Uint8 sh){
	*Co=IMG_Load(Soubor);
	if (*Co==NULL) fprintf(stderr,"Error loading %s!\n",Soubor);
	else{
		SDL_SetColorKey(*Co,SDL_SRCCOLORKEY|SDL_RLEACCEL,SDL_MapRGB((*Co)->format,255,0,255));
		if (sh)
			SDL_SetAlpha(*Co,SDL_SRCALPHA,sh);
		*Co=SDL_DisplayFormat(*Co);
	}
}


void LoadSprites(void){
int I,J;
SDL_Rect Src;

	for (I=0;I<MaxSpr;I++){
		Spr[I].img=NULL;
		Spr[I].sha=NULL;
	}
	
#ifdef UNIX

  chdir(SHARE_DIR);

#endif
  
	TitleS=IMG_Load("./img/title.jpg");
	stringRGBA(TitleS,399-strlen(strchr(VERSION,'.'))*8,2,strchr(VERSION,'.'),0x60,0xC0,0x60,128);
	
	LoadSprite(&SLevel,"./img/level.bmp",0);
	LoadSprite(&SLives,"./img/lives.bmp",0);
	LoadSprite(&STime,"./img/time.bmp",0);
	LoadSprite(&SNumbers,"./img/numbers.bmp",0);
	LoadSprite(&(Spr['*']).img,"./img/start.bmp",128);
	LoadSprite(&(Spr['*']).sha,"./img/shade-player.bmp",0);
	LoadSprite(&(Spr['+']).img,"./img/exit.bmp",128);
	LoadSprite(&(Spr['+']).sha,"./img/exit-denied.bmp",128);
	LoadSprite(&(Spr['%']).img,"./img/retard.bmp",96);
	LoadSprite(&(Spr['#']).img,"./img/ice.bmp",128);
	LoadSprite(&(Spr['&']).img,"./img/flamable.bmp",150);
	LoadSprite(&(Spr['@']).img,"./img/hot.bmp",200);
	LoadSprite(&(Spr['T']).img,"./img/teleport.bmp",200);

	LoadSprite(&(Spr['X']).img,"./img/wall.bmp",0);
	LoadSprite(&(Spr['X']).sha,"./img/shade-square.bmp",0);
	LoadSprite(&(Spr['x']).img,"./img/tinywall.bmp",0);
	LoadSprite(&(Spr['x']).sha,"./img/shade-square.bmp",0);
	LoadSprite(&(Spr['c']).img,"./img/crate.bmp",0);
	LoadSprite(&(Spr['c']).sha,"./img/shade-crate.bmp",0);
	LoadSprite(&(Spr['o']).img,"./img/barel.bmp",0);
	LoadSprite(&(Spr['o']).sha,"./img/shade-round.bmp",0);
	LoadSprite(&(Spr['O']).img,"./img/explosive.bmp",0);
	LoadSprite(&(Spr['O']).sha,"./img/shade-round.bmp",0);

	LoadSprite(&PlayerV,"./img/player-v-legs.bmp",0);
	LoadSprite(&PlayerH,"./img/player-h-legs.bmp",0);
	for (I=0;I<4;I++)
		for (J=0;J<20;J++){
			Player[0][I][J]=SDL_CreateRGBSurface(SDL_SRCALPHA,20,20,BPP,screen->format->Rmask,screen->format->Gmask,screen->format->Bmask,screen->format->Amask);
			if (Player[0][I][J]==NULL) printf("Error alocating player!!\n");
			ClearSurface(Player[0][I][J]);
			SDL_SetColorKey(Player[0][I][J],SDL_SRCCOLORKEY|SDL_RLEACCEL,SDL_MapRGB(Player[0][I][J]->format,255,0,255));
		}
	Src.w=20;
	Src.h=20;
	for (I=0;I<11;I++){
		Src.x=0;
		Src.y=I*40;
		SDL_BlitSurface(PlayerV,&Src,Player[0][0][(15+I)%20],NULL);
		SDL_BlitSurface(PlayerV,&Src,Player[0][0][15-I],NULL);
		Src.x=40;
		SDL_BlitSurface(PlayerV,&Src,Player[0][1][(15+I)%20],NULL);
		SDL_BlitSurface(PlayerV,&Src,Player[0][1][15-I],NULL);
	}
	for (I=0;I<11;I++){
		Src.x=I*40;
		Src.y=0;
		SDL_BlitSurface(PlayerH,&Src,Player[0][2][(15+I)%20],NULL);
		SDL_BlitSurface(PlayerH,&Src,Player[0][2][15-I],NULL);
		Src.y=40;
		SDL_BlitSurface(PlayerH,&Src,Player[0][3][(15+I)%20],NULL);
		SDL_BlitSurface(PlayerH,&Src,Player[0][3][15-I],NULL);
	}
	SDL_FreeSurface(PlayerV);
	SDL_FreeSurface(PlayerH);

	LoadSprite(&PlayerV,"./img/player-v-body.bmp",0);
	LoadSprite(&PlayerH,"./img/player-h-body.bmp",0);
	for (I=0;I<4;I++)
		for (J=0;J<20;J++){
			Player[1][I][J]=SDL_CreateRGBSurface(SDL_SRCALPHA,20,20,BPP,screen->format->Rmask,screen->format->Gmask,screen->format->Bmask,screen->format->Amask);
			if (Player[1][I][J]==NULL) printf("Error alocating player!!\n");
			ClearSurface(Player[1][I][J]);
			SDL_SetColorKey(Player[1][I][J],SDL_SRCCOLORKEY|SDL_RLEACCEL,SDL_MapRGB(Player[1][I][J]->format,255,0,255));
		}
	Src.w=20;
	Src.h=20;
	for (I=0;I<11;I++){
		Src.x=0;
		Src.y=I*40;
		SDL_BlitSurface(PlayerV,&Src,Player[1][0][(15+I)%20],NULL);
		SDL_BlitSurface(PlayerV,&Src,Player[1][0][15-I],NULL);
		Src.x=40;
		SDL_BlitSurface(PlayerV,&Src,Player[1][1][(15+I)%20],NULL);
		SDL_BlitSurface(PlayerV,&Src,Player[1][1][15-I],NULL);
	}
	for (I=0;I<11;I++){
		Src.x=I*40;
		Src.y=0;
		SDL_BlitSurface(PlayerH,&Src,Player[1][2][(15+I)%20],NULL);
		SDL_BlitSurface(PlayerH,&Src,Player[1][2][15-I],NULL);
		Src.y=40;
		SDL_BlitSurface(PlayerH,&Src,Player[1][3][(15+I)%20],NULL);
		SDL_BlitSurface(PlayerH,&Src,Player[1][3][15-I],NULL);
	}
	SDL_FreeSurface(PlayerV);
	SDL_FreeSurface(PlayerH);

	LoadSprite(&PlayerV,"./img/player-v-push.bmp",0);
	LoadSprite(&PlayerH,"./img/player-h-push.bmp",0);
	for (I=0;I<4;I++)
		for (J=0;J<20;J++){
			Player[2][I][J]=SDL_CreateRGBSurface(SDL_SRCALPHA,20,20,BPP,screen->format->Rmask,screen->format->Gmask,screen->format->Bmask,screen->format->Amask);
			if (Player[2][I][J]==NULL) printf("Error alocating player!!\n");
			ClearSurface(Player[2][I][J]);
			SDL_SetColorKey(Player[2][I][J],SDL_SRCCOLORKEY|SDL_RLEACCEL,SDL_MapRGB(Player[2][I][J]->format,255,0,255));
		}
	Src.w=20;
	Src.h=20;
	for (I=0;I<11;I++){
		Src.x=0;
		Src.y=I*40;
		SDL_BlitSurface(PlayerV,&Src,Player[2][0][(15+I)%20],NULL);
		SDL_BlitSurface(PlayerV,&Src,Player[2][0][15-I],NULL);
		Src.x=40;
		SDL_BlitSurface(PlayerV,&Src,Player[2][1][(15+I)%20],NULL);
		SDL_BlitSurface(PlayerV,&Src,Player[2][1][15-I],NULL);
	}
	for (I=0;I<11;I++){
		Src.x=I*40;
		Src.y=0;
		SDL_BlitSurface(PlayerH,&Src,Player[2][2][(15+I)%20],NULL);
		SDL_BlitSurface(PlayerH,&Src,Player[2][2][15-I],NULL);
		Src.y=40;
		SDL_BlitSurface(PlayerH,&Src,Player[2][3][(15+I)%20],NULL);
		SDL_BlitSurface(PlayerH,&Src,Player[2][3][15-I],NULL);
	}
	SDL_FreeSurface(PlayerV);
	SDL_FreeSurface(PlayerH);


	LoadSprite(&(Spr[1]).img,"./img/expl3.bmp",0);
	Spr[1].sha=NULL;
	LoadSprite(&(Spr[2]).img,"./img/expl2.bmp",0);
	Spr[2].sha=NULL;
	LoadSprite(&(Spr[3]).img,"./img/expl1.bmp",0);
	LoadSprite(&(Spr[3]).sha,"./img/shade-expl1.bmp",0);

	LoadSprite(&(Spr[4]).img,"./img/bo-flamable.bmp",0);
	Spr[3].sha=NULL;

	LoadSprite(&(Spr[5]).img,"./img/fire1.bmp",150);
	Spr[5].sha=NULL;
	LoadSprite(&(Spr[6]).img,"./img/fire2.bmp",150);
	Spr[6].sha=NULL;
	LoadSprite(&(Spr[7]).img,"./img/fire3.bmp",150);
	Spr[7].sha=NULL;
	LoadSprite(&(Spr[8]).img,"./img/fire2.bmp",150);
	Spr[8].sha=NULL;

	LoadSprite(&(Spr[9]).img,"./img/telee3.bmp",0);
	LoadSprite(&(Spr[9]).sha,"./img/shade-t3.bmp",0);
	LoadSprite(&(Spr[10]).img,"./img/telee2.bmp",0);
	LoadSprite(&(Spr[10]).sha,"./img/shade-t2.bmp",0);
	LoadSprite(&(Spr[11]).img,"./img/telee1.bmp",0);
	LoadSprite(&(Spr[11]).sha,"./img/shade-t1.bmp",0);

	LoadSprite(&(Spr[12]).img,"./img/player-death1.bmp",0);
	Spr[12].sha=NULL;
	LoadSprite(&(Spr[13]).img,"./img/player-death2.bmp",0);
	Spr[13].sha=NULL;
	LoadSprite(&(Spr[14]).img,"./img/player-death3.bmp",0);
	Spr[14].sha=NULL;
	LoadSprite(&(Spr[15]).img,"./img/player-death4.bmp",0);
	Spr[15].sha=NULL;
	
	LoadSprite(&(Spr['B']).img,"./img/bem1.bmp",0);
	LoadSprite(&(Spr['B']).sha,"./img/shade-bem1.bmp",0);
	LoadSprite(&(Spr['C']).img,"./img/bem2.bmp",0);
	LoadSprite(&(Spr['C']).sha,"./img/shade-bem2.bmp",0);
	LoadSprite(&(Spr['D']).img,"./img/bem3.bmp",0);
	LoadSprite(&(Spr['D']).sha,"./img/shade-bem3.bmp",0);
	LoadSprite(&(Spr['E']).img,"./img/bem4.bmp",0);
	LoadSprite(&(Spr['E']).sha,"./img/shade-bem4.bmp",0);



	Snd_Expl=Mix_LoadWAV("./snd/bombex.wav");
	Snd_Push=Mix_LoadWAV("./snd/slide.wav");
	Snd_Burn=Mix_LoadWAV("./snd/burn.wav");
	Snd_Fall=Mix_LoadWAV("./snd/fall.wav");
	Snd_Beep=Mix_LoadWAV("./snd/beep.wav");
	Snd_Teleport=Mix_LoadWAV("./snd/teleport.wav");
	Snd_GameOver=Mix_LoadWAV("./snd/gameover.wav");
	Music = Mix_LoadMUS("./snd/smashem.s3m");
}


char LoadDemo(char *Soubor){
	FILE *F;
	int I=0;
	char S[MaxStrLenLong],*SP;

	if ((F=fopen(Soubor,"r"))==NULL)
		return 0;
	while ((!feof(F))&&(I<MaxRecLength)){
		fgets(S,MaxStrLenLong-1,F);
		if ((SP=strchr(S,'	'))!=NULL){
			SP[0]='\0';
			Rec[I].Tick=atoi(S);
			Rec[I].Up=chartoint(SP[1]);
			Rec[I].Down=chartoint(SP[3]);
			Rec[I].Left=chartoint(SP[5]);
			Rec[I].Right=chartoint(SP[7]);
			Rec[I].Fire=chartoint(SP[9]);
			Rec[I].Quit=chartoint(SP[11]);
			I++;
		}
	}

	fclose (F);
	return 1;
}


char SaveDemo(char *Soubor){
	FILE *F;
	int I=0;

	if ((F=fopen(Soubor,"w"))==NULL)
		return 0;

	while ((I<MaxRecLength)&&((I<1)||(Rec[I-1].Quit!=2))){
		fprintf(F,"%d	%d,%d,%d,%d,%d,%d\n",Rec[I].Tick,Rec[I].Up,Rec[I].Down,Rec[I].Left,Rec[I].Right,Rec[I].Fire,Rec[I].Quit);
		I++;
	}

	fclose (F);
	return 1;
}


char LoadConfig(char *Soubor){
	FILE *F;
	char S[MaxStrLenXLong];

	if ((F=fopen(Soubor,"r"))==NULL)
		return 0;
	while (!feof(F)){
		fgets (S,MaxStrLenXLong-1,F);
		while((strlen(S))&&(S[strlen(S)-1]<' '))
			S[strlen(S)-1]='\0';
		if ((S[0]!='\0')&&(S[0]!='#')){
//			printf("%s\n",S);
				if (!strcmp(S,"shades=none"))
					Shades=0;
				else
				if (!strcmp(S,"shades=solid"))
					Shades=255;
				else
				if (!strcmp(S,"shades=transparent"))
					Shades=128;
				else
				if (!strncmp(S,"m_volume=",9))
					MVolume=((float)chartoint(S[9])/9.0)*128;
				else
				if (!strncmp(S,"s_volume=",9))
					SVolume=((float)chartoint(S[9])/9.0)*128;
				else
				if (!strncmp(S,"fullscreen=",11))
					FULLSCR=chartoint(S[11]);

		}
	}
	fclose (F);
	return 1;
}


void SaveConfig(char FileName[]){
FILE *F;

F=fopen(FileName,"w");
fprintf(F,"#	X-pired Configuration File\n");
fprintf(F,"\n");
fprintf(F,"#	shades: none, transparent, solid (set 'solid' or 'none' to increase performance)\n");
if (Shades==0)
	fprintf(F,"shades=none\n");
if (Shades==128)
	fprintf(F,"shades=transparent\n");
if (Shades==255)
	fprintf(F,"shades=solid\n");
fprintf(F,"\n");
fprintf(F,"#	music volume: 0-9\n");
fprintf(F,"m_volume=%i\n",(int)(((float)MVolume/128.0)*9.0)+1);
fprintf(F,"\n");
fprintf(F,"#	sound volume: 0-9\n");
fprintf(F,"s_volume=%i\n",(int)(((float)SVolume/128.0)*9.0)+1);
fprintf(F,"\n");
fprintf(F,"#	Fullscreen mode: 0/1\n");
fprintf(F,"fullscreen=%i\n",FULLSCR);
fprintf(F,"\n");
fclose(F);
}


char LoadLevels(char *Soubor){
	FILE *F;
	char S[MaxStrLenLong],T[MaxStrLenShort];
	int I,J,L=-1;

	for (I=0;I<100;I++)
		Lvl[I].Pw[0]='\0';

	I=0;

	if ((F=fopen(Soubor,"r"))==NULL)
		return 0;
	while (!feof(F)){
		fgets (S,MaxStrLenLong-1,F);
		while ((strlen(S)>0)&&(S[strlen(S)-1]<' '))
			S[strlen(S)-1]='\0';
		if ((S[0]!='\0')&&(S[0]!='#')){
//			printf("%s\n",S);
			if ((S[0]=='>')&&(S[strlen(S)-1]=='-')){
				L++;
				sprintf(Lvl[L].Name,"Level no. %d",L);
				Lvl[L].Pw[0]='\0';
				Lvl[L].Bg=NULL;
				Lvl[L].DL=0;
				for (I=0;I<20;I++)
					for (J=0;J<20;J++){
						Lvl[L].M[I][J].FSpr=' ';
						Lvl[L].M[I][J].FTyp=' ';
						Lvl[L].M[I][J].BSpr=' ';
						Lvl[L].M[I][J].BTyp=' ';
						Lvl[L].M[I][J].x=0;
						Lvl[L].M[I][J].y=0;
						Lvl[L].M[I][J].px=0;
						Lvl[L].M[I][J].py=0;
						Lvl[L].M[I][J].f=0;
						Lvl[L].M[I][J].txt=-1;
					}
				for (I=0;I<10;I++)
					Lvl[L].Text[I][0]='\0';

				I=0;
//				printf("Level(%d) loaded...\n",L);
			}else{// LEVEL
				if ((S[1]=='=')&&(chartoint(S[0])>=0))
					for (J=2;J<=strlen(S);J++)
						Lvl[L].Text[chartoint(S[0])][J-2]=S[J];
				else
				if (!strncmp(S,"name=",5))
					for (J=5;J<=strlen(S);J++)
						Lvl[L].Name[J-5]=S[J];
				else
				if (!strncmp(S,"password=",9))
					for (J=9;J<=strlen(S);J++)
						Lvl[L].Pw[J-9]=S[J];
				else
				if (!strncmp(S,"deadline=",5)){
					for (J=9;J<=strlen(S);J++)
						T[J-9]=S[J];
					Lvl[L].DL=atoi(T);
				}
				else
				if (!strncmp(S,"background=",5)){
					for (J=11;J<=strlen(S);J++)
						T[J-11]=S[J];
					Lvl[L].Bg=SDL_DisplayFormat(IMG_Load(T));
				}
				else
				if ((I<20)&&(strlen(S)>=20*7)&&(S[0]==',')){
					for (J=0;J<20*7;J+=7){
							Lvl[L].M[J/7][I].FTyp=S[J+1];
							Lvl[L].M[J/7][I].FSpr=S[J+2];
							Lvl[L].M[J/7][I].BTyp=S[J+3];
							Lvl[L].M[J/7][I].BSpr=S[J+4];
							Lvl[L].M[J/7][I].x=0;
							Lvl[L].M[J/7][I].y=0;
							Lvl[L].M[J/7][I].px=0;
							Lvl[L].M[J/7][I].py=0;
							Lvl[L].M[J/7][I].f=chartoint(S[J+5]);
							Lvl[L].M[J/7][I].txt=chartoint(S[J+6]);
					}
					I++;
				}
			}

		}
	}
	fclose (F);
	return 1;
}


// --- Dirty rectangle ---

void DrawSpr(SDL_Surface *Co,SDL_Surface *Kam,int X,int Y){
SDL_Rect Dest;
	if (Co==NULL) return;
	Dest.x=X;
	Dest.y=Y;
	Dest.w=Co->w;
	Dest.h=Co->h;
	SDL_BlitSurface(Co,NULL,Kam,&Dest);
}


void R_Blit(SDL_Surface *Co,SDL_Surface *Kam,int SX, int SY, int SW, int SH, int X, int Y){
SDL_Rect Src,Dest;
	if (Co==NULL) return;
	Src.x=SX;
	Src.y=SY;
	Src.w=SW;
	Src.h=SH;
	Dest.x=X;
	Dest.y=Y;
	Dest.w=SW;
	Dest.h=SH;
	SDL_BlitSurface(Co,&Src,Kam,&Dest);
}


void DR_RCT(int x1, int y1, int x2, int y2){
	if ((x1<DRX1)||(DRX1==-1000)) DRX1=x1;
	if ((y1<DRY1)||(DRY1==-1000)) DRY1=y1;
	if ((x2>DRX2)||(DRX2==-1000)) DRX2=x2;
	if ((y2>DRY2)||(DRY2==-1000)) DRY2=y2;
	DRT=1;
}


void DR_SPR(int x, int y, char Sprite){
	if (Spr[Sprite].img!=NULL)
		DR_RCT(x,y,x+Spr[Sprite].img->w,y+Spr[Sprite].img->h);
//	else
//		fprintf(stderr,"%c\n",Sprite);
	if (Spr[Sprite].sha!=NULL)
		DR_RCT(x,y,x+Spr[Sprite].sha->w,y+Spr[Sprite].sha->h);
}


void DRNULL(void){
	DRX1=-1000;
	DRY1=-1000;
	DRX2=-1000;
	DRY2=-1000;
	DRT=0;
}


// --- Messages ---

void TextReset(void){
int I;
	for (I=0;I<MaxText;I++)
		Text[I].t[0]='\0';
}


void TextDel(void){
int I;
	for (I=1;(I<MaxText)&&(Text[I-1].t[0]!='\0');I++){
		strcpy(Text[I-1].t,Text[I].t);
		Text[I-1].x=Text[I].x;
		Text[I-1].y=Text[I].y;
		Text[I-1].a=Text[I].a;
	}
	if (I==MaxText)
		Text[MaxText-1].t[0]='\0';
}

void TextAdd(int x,int y,char Txt[]){
int I;
	for (I=0;(I<MaxText)&&(Text[I].t[0]!='\0');I++);
	if (I==MaxText){
		TextDel();
		I=MaxText-1;
	}
	strcpy(Text[I].t,Txt);
	Text[I].x=x-strlen(Txt)*4;
	if (Text[I].x<0)
		Text[I].x=0;
	if (Text[I].x+strlen(Txt)*8>WIDTH)
		Text[I].x=WIDTH-strlen(Txt)*8;
	Text[I].y=y-4;
	Text[I].a=TextDelay;
}

void TextBehave(void){
int I;
for (I=0;(I<MaxText)&&(Text[I].t[0]!='\0');I++){
	DR_RCT(Text[I].x,Text[I].y,Text[I].x+strlen(Text[I].t)*8,Text[I].y+9);
	if (!(--Text[I].a)){
		TextDel();
		I--;
	}else{
		Text[I].y-=TextDelay/Text[I].a;
		DR_RCT(Text[I].x,Text[I].y,Text[I].x+strlen(Text[I].t)*8,Text[I].y+9);
	}
}
}	

void TextDisplay(SDL_Surface *Kam){
int I;
	for (I=0;(I<MaxText)&&(Text[I].t[0]!='\0');I++){
		stringRGBA(Kam,Text[I].x+1,Text[I].y+1,Text[I].t,0x00,0x00,0x00,64+191*((float)(Text[I].a)/(float)(TextDelay)));
		stringRGBA(Kam,Text[I].x,Text[I].y,Text[I].t,0x00,0xFF,0x00,64+191*((float)(Text[I].a)/(float)(TextDelay)));
	}
}


int sgn(int X){
	if (X<0) return -1;
	if (X>0) return 1;
	return 0;
}


char shift(int X,int Y){
	if ((ALvl.M[X][Y].FTyp=='c')||(ALvl.M[X][Y].FTyp=='o')||(ALvl.M[X][Y].FTyp=='O')) return 1;
	return 0;
}


void DrawGame(void){
int I,J,X,Y,W,H;
SDL_Rect Rct;

//	DR MAGIC....
	if (!DRT)
		return;

	DRX1/=20;
	DRY1/=20;
	DRX2/=20;
	DRY2/=20;
	DRX1*=(DRX1>0);
	DRY1*=(DRY1>0);
	DRX2*=(DRX2>0);
	DRY2*=(DRY2>0);
	DRX1=(DRX1>19)?19:DRX1;
	DRY1=(DRY1>19)?19:DRY1;
	DRX2=(DRX2>19)?19:DRX2;
	DRY2=(DRY2>19)?19:DRY2;
	X=DRX1;
	Y=DRY1;
	W=DRX2-DRX1+1;
	H=DRY2-DRY1+1;
	Rct.x=X*20;
	Rct.y=Y*20;
	Rct.w=W*20;
	Rct.h=H*20;
	X--;
	Y--;
	W+=2;
	H+=2;
	X*=(X>0);
	Y*=(Y>0);
	if (X+W>20) W=20-X;
	if (Y+H>20) H=20-Y;

	SDL_BlitSurface(ALvl.Bg,&Rct,screen,&Rct);

	if (Shades){
		boxRGBA(Sh,Rct.x,Rct.y,Rct.x+Rct.w-1,Rct.y+Rct.h-1,255,0,255,0xFF);
		boxRGBA(Sh,0,0,399,14,0,0,0,0xFF);
		boxRGBA(Sh,0,15,14,399,0,0,0,0xFF);

		for (I=X;I<X+W;I++)
			for (J=Y;J<Y+H;J++)
				DrawSpr(Spr[ALvl.M[I][J].FSpr].sha,Sh,I*20+ALvl.M[I][J].x,J*20+ALvl.M[I][J].y);
		if ((PX>=X)&&(PX<=X+W)&&(PY>=Y)&&(PY<=Y+H))
			if ((PAlive<=DeadDelay)||(PAlive%2))
				DrawSpr(Spr['*'].sha,Sh,PX*20+PPX,PY*20+PPY);

		SDL_BlitSurface(Sh,&Rct,screen,&Rct);
	}


	for (I=X;I<X+W;I++)
		for (J=Y;J<Y+H;J++)
			DrawSpr(Spr[ALvl.M[I][J].FSpr].img,screen,I*20+ALvl.M[I][J].x,J*20+ALvl.M[I][J].y);

	if ((PX>=X)&&(PX<=X+W)&&(PY>=Y)&&(PY<=Y+H)){
		if ((PAlive==DeadDelay)||((PAlive>DeadDelay)&&(PAlive%2))){
			DrawSpr(Player[0][(((!PPX)&&(!PPY))*LM)+(PPY>0)+(PPX>0)*2+(PPX<0)*3][(abs(PPX)+abs(PPY))],screen,PX*20+PPX,PY*20+PPY);
			if (shift(PX+sgn(PPX),PY+sgn(PPY)))
				DrawSpr(Player[2][(((!PPX)&&(!PPY))*LM)+(PPY>0)+(PPX>0)*2+(PPX<0)*3][(abs(PPX)+abs(PPY))],screen,PX*20+PPX,PY*20+PPY);
			else
				DrawSpr(Player[1][(((!PPX)&&(!PPY))*LM)+(PPY>0)+(PPX>0)*2+(PPX<0)*3][(abs(PPX)+abs(PPY))],screen,PX*20+PPX,PY*20+PPY);
		}else
		if (PAlive<DeadDelay)
			DrawSpr(Spr[15-PAlive/(DeadDelay/4)].img,screen,PX*20+PPX,PY*20+PPY);
	}
	// Stat: Level---
	if ((X<2)&&(Y+H>=19)){
		DrawSpr(SLevel,screen,1,400-16);
		R_Blit(SNumbers,screen,(Level/10)*11,0,11,15,15,400-16);
		R_Blit(SNumbers,screen,(Level%10)*11,0,11,15,26,400-16);
	}
	// Stat: Lives---
	if ((18<=X+W)&&(19<=Y+H)){
		DrawSpr(SLives,screen,400-16,400-16);
		R_Blit(SNumbers,screen,(Lives/10)*11,0,11,15,400-37,400-16);
		R_Blit(SNumbers,screen,(Lives%10)*11,0,11,15,400-26,400-16);
	}
	// Stat: DeadLine---
	if ((ALvl.DL)&&(((9>=X)&&(9<=X+W))||((12<=X+W)&&(12>=X)))&&(19<=Y+H)){
		DrawSpr(STime,screen,200-23,400-17);
		R_Blit(SNumbers,screen,(ALvl.DL/100)*11,0,11,15,200-9,400-16);
		R_Blit(SNumbers,screen,((ALvl.DL%100)/10)*11,0,11,15,200+2,400-16);
		R_Blit(SNumbers,screen,(ALvl.DL%10)*11,0,11,15,200+13,400-16);
	}

	TextDisplay(screen);

//	Dirty rectangle visualization:
//	rectangleRGBA(screen,Rct.x,Rct.y,Rct.x+Rct.w-1,Rct.y+Rct.h-1,255,255,255,128);
	
	SDL_UpdateRect(screen,Rct.x,Rct.y,Rct.w,Rct.h);
	DRNULL();
}


char bem(int X,int Y){
	if ((X<0)||(Y<0)||(X>19)||(Y>19))	return 0;
	if (ALvl.M[X][Y].FTyp=='B')			return 1;
	return 0;
}


char hard(int X,int Y){
	if ((X<0)||(Y<0)||(X>19)||(Y>19)
		||(ALvl.M[X][Y].FTyp=='X')||(ALvl.M[X][Y].FTyp=='x')||(ALvl.M[X][Y].FTyp=='B')) return 1;
	if ((X==PX)&&(Y==PY)) return 1;
	return 0;
}


char fobj(int X,int Y){
	if ((X<0)||(Y<0)||(X>19)||(Y>19))
		return 0;
	if (bem(X,Y)||hard(X,Y)||shift(X,Y))
		return 1;
	return 0;
}


char collision(int X, int Y, int pX, int pY){
	if ((X+pX<0)||(Y+pY<0)||(X+pX>19)||(Y+pY>19))
		return 1;
	if (fobj(X+pX,Y+pY)&&((sgn(ALvl.M[X+pX][Y+pY].px)!=pX)||(sgn(ALvl.M[X+pX][Y+pY].py)!=pY)))
		return 1;
	if ((pX<=0)&&(fobj(X+pX-1,Y+pY))&&(sgn(ALvl.M[X+pX-1][Y+pY].px)>0))
		return 1;
	if ((pX>=0)&&(fobj(X+pX+1,Y+pY))&&(sgn(ALvl.M[X+pX+1][Y+pY].px)<0))
		return 1;
	if ((pY<=0)&&(fobj(X+pX,Y+pY-1))&&(sgn(ALvl.M[X+pX][Y+pY-1].py)>0))
		return 1;
	if ((pY>=0)&&(fobj(X+pX,Y+pY+1))&&(sgn(ALvl.M[X+pX][Y+pY+1].py)<0))
		return 1;
	return 0;
}


char push(int PPX,int PPY, int F){
	if ((ALvl.M[PX+PPX][PY+PPY].x)||(ALvl.M[PX+PPX][PY+PPY].y)||(!shift(PX+PPX,PY+PPY))
		||(shift(PX+2*PPX,PY+2*PPY))||(hard(PX+2*PPX,PY+2*PPY)))
		return 0;

	if (ALvl.M[PX+PPX][PY+PPY].FTyp=='c'){
		PF=1;
		F=1;
	}

	ALvl.M[PX+PPX][PY+PPY].px=PPX;
	ALvl.M[PX+PPX][PY+PPY].py=PPY;
	ALvl.M[PX+PPX][PY+PPY].x=PPX*F;
	ALvl.M[PX+PPX][PY+PPY].y=PPY*F;
	ALvl.M[PX+PPX][PY+PPY].f=F;
	if (F>1)
		PlaySample(Snd_Push);
	return 1;
}


char is_free(int PPX,int PPY){
	if (hard(PX+PPX,PY+PPY)) return 0;
	if (shift(PX+PPX,PY+PPY)&&(sgn(ALvl.M[PX+PPX][PY+PPY].px)==sgn(PPX))&&(sgn(ALvl.M[PX+PPX][PY+PPY].py)==sgn(PPY))) return 1;
	if (shift(PX+PPX,PY+PPY)) return push(PPX,PPY,PF);
	return 1;
}


char GetTeleport(int *X,int *Y){
int I,J;

	for (I=0;I<=20;I++)
		for (J=(*Y+1)*(I==0);J<20;J++)
			if ((ALvl.M[(*X+I)%20][J].BTyp=='T')&&((ALvl.M[(*X+I)%20][J].FTyp==' ')||(((*X+I)%20==*X)&&(J==*Y)))&&(((*X+I)%20!=PX)||(J!=PY))){
				if (((*X+I)%20==*X)&&(J==*Y))
					return 0;
				*X=(*X+I)%20;
				*Y=J;
				return 1;
			}
	return 0;
}

void TeleportEffect(int X, int Y){
	PlaySample(Snd_Teleport);
	ALvl.M[X][Y].FSpr=11;
	ALvl.M[X][Y].FTyp=' ';
	ALvl.M[X][Y].f=9;
	ALvl.M[X][Y].x=0;
	ALvl.M[X][Y].y=0;
	ALvl.M[X][Y].px=3;
	ALvl.M[X][Y].py=3;
}


void MoveObjects(void){
int I,J,K,L,P;
char BEM2=0;

	for (I=0;I<MaxLvlSize;I++)
			for (J=0;J<MaxLvlSize;J++) if (ALvl.M[I][J].f)
				if (ALvl.M[I][J].f<3){// SHIFT/SWIFT
					DR_SPR(I*20+ALvl.M[I][J].x,J*20+ALvl.M[I][J].y,ALvl.M[I][J].FSpr);
					if ((!ALvl.M[I][J].x)&&(!ALvl.M[I][J].y)){
						if ((ALvl.M[I][J].f==1)&&(ALvl.M[I][J].BTyp!='T'))
							ALvl.M[I][J].f=0;

						if (ALvl.M[I][J].BTyp=='@'){
							PlaySample(Snd_Burn);
							DrawSpr(Spr[4].img,ALvl.Bg,I*20,J*20);
							ALvl.M[I][J].BTyp=' ';
							ALvl.M[I][J].FTyp=' ';
							ALvl.M[I][J].f=6;
							ALvl.M[I][J].px=0;
							ALvl.M[I][J].py=0;
						}else
						if (ALvl.M[I][J].BTyp=='%'){
							ALvl.M[I][J].f=0;
							ALvl.M[I][J].px=0;
							ALvl.M[I][J].py=0;
						}else
						if ((ALvl.M[I][J].px||ALvl.M[I][J].py)&&(collision(I,J,sgn(ALvl.M[I][J].px),sgn(ALvl.M[I][J].py)))){
							if ((ALvl.M[I][J].FTyp!='c')&&(ALvl.M[I][J].f>1))
								ALvl.M[I][J].f=4+(ALvl.M[I][J].FTyp=='O');
							ALvl.M[I][J].px=0;
							ALvl.M[I][J].py=0;
						}
					}
					if ((ALvl.M[I][J].px||ALvl.M[I][J].py)&&((abs(ALvl.M[I][J].x+=ALvl.M[I][J].px*ALvl.M[I][J].f)>=20)||(abs(ALvl.M[I][J].y+=ALvl.M[I][J].py*ALvl.M[I][J].f)>=20))){
						if (ALvl.M[I+ALvl.M[I][J].px][J+ALvl.M[I][J].py].BTyp=='T'){// T e l e p o r t
							K=I+ALvl.M[I][J].px;
							L=J+ALvl.M[I][J].py;
							P=GetTeleport(&K,&L);

							ALvl.M[K][L].FSpr=ALvl.M[I][J].FSpr;
							ALvl.M[K][L].FTyp=ALvl.M[I][J].FTyp;
							ALvl.M[K][L].x=0;
							ALvl.M[K][L].y=0;

							ALvl.M[K][L].px=ALvl.M[I][J].px;
							ALvl.M[K][L].py=ALvl.M[I][J].py;
							ALvl.M[K][L].f=ALvl.M[I][J].f;

							if (P)
								TeleportEffect(I+ALvl.M[I][J].px,J+ALvl.M[I][J].py);
						}else{
							ALvl.M[I+ALvl.M[I][J].px][J+ALvl.M[I][J].py].FSpr=ALvl.M[I][J].FSpr;
							ALvl.M[I+ALvl.M[I][J].px][J+ALvl.M[I][J].py].FTyp=ALvl.M[I][J].FTyp;
							ALvl.M[I+ALvl.M[I][J].px][J+ALvl.M[I][J].py].x=0;
							ALvl.M[I+ALvl.M[I][J].px][J+ALvl.M[I][J].py].y=0;
							if (ALvl.M[I][J].f<2){
								ALvl.M[I+ALvl.M[I][J].px][J+ALvl.M[I][J].py].px=0;
								ALvl.M[I+ALvl.M[I][J].px][J+ALvl.M[I][J].py].py=0;
								ALvl.M[I+ALvl.M[I][J].px][J+ALvl.M[I][J].py].f=1;
							}else{
								ALvl.M[I+ALvl.M[I][J].px][J+ALvl.M[I][J].py].px=ALvl.M[I][J].px;
								ALvl.M[I+ALvl.M[I][J].px][J+ALvl.M[I][J].py].py=ALvl.M[I][J].py;
								ALvl.M[I+ALvl.M[I][J].px][J+ALvl.M[I][J].py].f=2;
							}
						}
						ALvl.M[I][J].FSpr=' ';
						ALvl.M[I][J].FTyp=' ';
						ALvl.M[I][J].f=0;
						ALvl.M[I][J].x=0;
						ALvl.M[I][J].y=0;
						ALvl.M[I][J].px=0;
						ALvl.M[I][J].py=0;
					}
					DR_SPR(I*20+ALvl.M[I][J].x,J*20+ALvl.M[I][J].y,ALvl.M[I][J].FSpr);
				}else// EXPLODE/IMPLODE
				if ((ALvl.M[I][J].f>=4)&&(ALvl.M[I][J].f<=6)){
					DR_SPR(I*20+ALvl.M[I][J].x,J*20+ALvl.M[I][J].y,ALvl.M[I][J].FSpr);
					if ((!ALvl.M[I][J].px)&&(!ALvl.M[I][J].py)){
						ALvl.M[I][J].px=4;
						ALvl.M[I][J].py=6;
					}else
					if (!(--ALvl.M[I][J].py)){
						ALvl.M[I][J].py=6;
						if ((ALvl.M[I][J].px==3)&&(ALvl.M[I][J].f<6)){
							PlaySample(Snd_Expl);
							for (K=-1;K<=1;K++)
								for (L=-1;L<=1;L++)
									if ((K||L)&&(I+K>=0)&&(I+K<MaxLvlSize)&&(J+L>=0)&&(J+L<MaxLvlSize))
										if (((ALvl.M[I+K][J+L].FTyp=='o')||(ALvl.M[I+K][J+L].FTyp=='O'))&&((ALvl.M[I+K][J+L].f<4)||(ALvl.M[I+K][J+L].f>6))){
											ALvl.M[I+K][J+L].f=4+(ALvl.M[I+K][J+L].FTyp=='O');
											ALvl.M[I+K][J+L].x+=K;
											ALvl.M[I+K][J+L].y+=L;
											ALvl.M[I+K][J+L].px=0;
											ALvl.M[I+K][J+L].py=0;
										}else
										if ((ALvl.M[I+K][J+L].FTyp=='x')||(ALvl.M[I+K][J+L].FTyp=='c')){
											PlaySample(Snd_Fall);
											ALvl.M[I+K][J+L].f=6;
											ALvl.M[I+K][J+L].x+=K;
											ALvl.M[I+K][J+L].y+=L;
											ALvl.M[I+K][J+L].px=0;
											ALvl.M[I+K][J+L].py=0;
										}else
										if ((ALvl.M[I+K][J+L].FTyp==' ')||(ALvl.M[I+K][J+L].FTyp=='B')){
											if (ALvl.M[I+K][J+L].f==4);
											else
											if (ALvl.M[I+K][J+L].f==5);
											else
											//if (ALvl.M[I+K][J+L].f==6)
												{
												ALvl.M[I+K][J+L].f=6-2*(ALvl.M[I][J].f==5);
												ALvl.M[I+K][J+L].px=0;
												ALvl.M[I+K][J+L].py=0;
												ALvl.M[I+K][J+L].x=0;
												ALvl.M[I+K][J+L].y=0;
												}
										}
						}else
						if (ALvl.M[I][J].px==2){
							for (K=10;K<=20;K+=ALvl.M[I][J].f)
								filledCircleRGBA(ALvl.Bg,I*20+7+rand()%6,J*20+7+rand()%6,K,0,0,0,20-(K*(rand()%2))/2);
							DR_RCT(I*20-5,J*20-5,I*20+19+5,J*20+19+5);
						}
						if (!(ALvl.M[I][J].FSpr=--ALvl.M[I][J].px)){
							ALvl.M[I][J].f=0;
							ALvl.M[I][J].FTyp=' ';
							ALvl.M[I][J].FSpr=' ';
							ALvl.M[I][J].px=0;
							ALvl.M[I][J].py=0;
							ALvl.M[I][J].x=0;
							ALvl.M[I][J].y=0;
							if (ALvl.M[I][J].BTyp=='&')
								ALvl.M[I][J].f=8;
						}
					}
					DR_SPR(I*20+ALvl.M[I][J].x,J*20+ALvl.M[I][J].y,ALvl.M[I][J].FSpr);
				}else// FLAMABLE
				if (ALvl.M[I][J].f==8){
					DR_SPR(I*20,J*20,ALvl.M[I][J].FSpr);
					if (!ALvl.M[I][J].px){
						if ((ALvl.M[I][J].FTyp=='O')){
							ALvl.M[I][J].f=5;
							ALvl.M[I][J].px=0;
							ALvl.M[I][J].py=0;
						}else if ((ALvl.M[I][J].FTyp=='o')){
							ALvl.M[I][J].f=4;
							ALvl.M[I][J].px=0;
							ALvl.M[I][J].py=0;
						}else{
							ALvl.M[I][J].FSpr=6;
							ALvl.M[I][J].px=40;
						}
						PlaySample(Snd_Burn);
					}else
					if (--ALvl.M[I][J].px){
						ALvl.M[I][J].FSpr=5+ALvl.M[I][J].px%4;
						if (ALvl.M[I][J].px==20)
							for (K=-1;K<=1;K++)
								for (L=-1;L<=1;L++)
									if ((I+K>=0)&&(I+K<MaxLvlSize)&&(J+L>=0)&&(J+L<MaxLvlSize))
										if ((ALvl.M[I+K][J+L].BTyp=='&')&&(ALvl.M[I+K][J+L].f!=8)&&((ALvl.M[I+K][J+L].f<4)||(ALvl.M[I+K][J+L].f>6))){
											ALvl.M[I+K][J+L].f=8;
											ALvl.M[I+K][J+L].px=0;
										}
					}else{
						ALvl.M[I][J].FTyp=' ';
						ALvl.M[I][J].FSpr=' ';
						ALvl.M[I][J].BTyp=' ';
						ALvl.M[I][J].BSpr=' ';
						ALvl.M[I][J].f=0;
						ALvl.M[I][J].px=0;
						ALvl.M[I][J].py=0;
						DrawSpr(Spr[4].img,ALvl.Bg,I*20,J*20);
						filledCircleRGBA(ALvl.Bg,I*20+7+rand()%6,J*20+7+rand()%6,13,0,0,0,64);
						DR_RCT(I*20-3,J*20-3,I*20+19+3,J*20+19+3);
					}
					DR_SPR(I*20,J*20,ALvl.M[I][J].FSpr);
//					fprintf(stderr,"at(%d,%d)-t%d-pxy(%d,%d)-xy(%d,%d)\n",I,J,ALvl.M[I][J].f,ALvl.M[I][J].px,ALvl.M[I][J].py,ALvl.M[I][J].x,ALvl.M[I][J].y);
				}else// PARTICLES
				if (ALvl.M[I][J].f==9){
					DR_SPR(I*20,J*20,ALvl.M[I][J].FSpr);
					if (!(ALvl.M[I][J].py-=1)){
						if (!(ALvl.M[I][J].px-=1)){
							ALvl.M[I][J].FSpr=' ';
							ALvl.M[I][J].f=0;
						}else{
							ALvl.M[I][J].FSpr--;
							ALvl.M[I][J].py=3;
						}
					}
					DR_SPR(I*20,J*20,ALvl.M[I][J].FSpr);
				}else// BEM
				if (ALvl.M[I][J].f==10){
					BEM2=1;
					if (!(rand()%50)){
						DR_SPR(I*20,J*20,ALvl.M[I][J].FSpr);
						ALvl.M[I][J].FSpr='B'+rand()%4;
						DR_SPR(I*20,J*20,ALvl.M[I][J].FSpr);
					}
				}else{// WEIRD!!!
					fprintf(stderr,"?! %i at %i,%i\n",ALvl.M[I][J].f,I,J);
					ALvl.M[I][J].f--;
				}

	if (BEM&&(!BEM2)){
		for (I=0;I<MaxLvlSize;I++)
			for (J=0;J<MaxLvlSize;J++)
				if (ALvl.M[I][J].BTyp=='+'){
					DrawSpr(Spr['+'].img,ALvl.Bg,I*20+ALvl.M[I][J].x,J*20+ALvl.M[I][J].y);
					DR_SPR(I*20,J*20,ALvl.M[I][J].BSpr);
				}
		BEM=BEM2;
	}
}


void SetLevel(char Lev,char DemoMode){
int I,J;
char Caption[MaxStrLenLong];
SDL_Rect Rct;
	Level=Lev;
	PPX=0;
	PPY=0;

	if ((Lvl[Lev].Pw[0]=='\0')||(DemoMode))
		sprintf(Caption,"%s: %s",APPNAME,Lvl[Lev].Name);
	else{
		strcpy(Passwd,Lvl[Lev].Pw);
		sprintf(Caption,"%s: %s [%s]",APPNAME,Lvl[Lev].Name,Lvl[Lev].Pw);
	}

	SDL_WM_SetCaption(Caption,Caption);

	BEM=0;
	for (I=0;I<MaxLvlSize;I++)
		for (J=0;J<MaxLvlSize;J++)
			if (Lvl[Lev].M[I][J].FTyp=='B')
				BEM=1;

	ALvl.DL=Lvl[Lev].DL;
	strcpy(ALvl.Name,Lvl[Lev].Name);
	strcpy(ALvl.Pw,Lvl[Lev].Pw);
	for (I=0;I<MaxLvlSize;I++)
		for (J=0;J<MaxLvlSize;J++){
			if (Lvl[Lev].M[I][J].BTyp=='*'){
				PX=I;
				PY=J;
			}
			ALvl.M[I][J].FTyp=Lvl[Lev].M[I][J].FTyp;
			ALvl.M[I][J].FSpr=Lvl[Lev].M[I][J].FSpr;
			ALvl.M[I][J].BTyp=Lvl[Lev].M[I][J].BTyp;
			ALvl.M[I][J].BSpr=Lvl[Lev].M[I][J].BSpr;
			ALvl.M[I][J].f=Lvl[Lev].M[I][J].f;
			ALvl.M[I][J].txt=Lvl[Lev].M[I][J].txt;
			ALvl.M[I][J].x=0;
			ALvl.M[I][J].y=0;
			ALvl.M[I][J].px=0;
			ALvl.M[I][J].py=0;
			if (ALvl.M[I][J].FTyp=='B')
				ALvl.M[I][J].f=10;
		}
	for (I=0;I<MaxText;I++)
		strcpy(ALvl.Text[I],Lvl[Lev].Text[I]);

	if (Lvl[Level].Bg!=NULL){
		Rct.w=Lvl[Level].Bg->w;
		Rct.h=Lvl[Level].Bg->h;
		for (I=0;I<WIDTH;I+=Rct.w)
			for (J=0;J<HEIGHT;J+=Rct.h){
				Rct.x=I;
				Rct.y=J;
				SDL_BlitSurface(Lvl[Level].Bg,NULL,ALvl.Bg,&Rct);
			}
	}else
		boxRGBA(ALvl.Bg,0,0,WIDTH-1,HEIGHT-1,0x40,0x40,0x40,0xFF);

	for (I=0;I<MaxLvlSize;I++)
		for (J=0;J<MaxLvlSize;J++)
			if (!BEM||(ALvl.M[I][J].BTyp!='+'))
				DrawSpr(Spr[ALvl.M[I][J].BSpr].img,ALvl.Bg,I*20+ALvl.M[I][J].x,J*20+ALvl.M[I][J].y);
			else
				DrawSpr(Spr['+'].sha,ALvl.Bg,I*20+ALvl.M[I][J].x,J*20+ALvl.M[I][J].y);
	
	ALvl.Bg=SDL_DisplayFormat(ALvl.Bg);

	TextAdd(PX*20+10,PY*20+10,ALvl.Name);
	DRT=0;
	DRNULL();
	DR_RCT(0,0,399,399);
	Timed=(ALvl.DL>0);
	Sec=1000/GCycle;
}


void ExplodeLevel(void){
int I,J;

	for (I=1;I<MaxLvlSize;I+=3)
		for (J=1;J<MaxLvlSize;J+=3)
			ALvl.M[I][J].f=4;
}


void SystemSetup(void){
char Menu[3][100]={"Shadows:","Music volume:","Sound volume:"};
char M=0,I;

	SDL_BlitSurface(screen,NULL,swapscreen,NULL);
	boxRGBA(screen,30,150,369,249,0,128,0,128);
	rectangleRGBA(screen,30,150,369,249,0,190,0,128);
	stringRGBA(screen,32,152,"X-pired 1.0 ------------------------ SetUp",0,255,0,255);

	while ((!Quit)&&(!Fire)){
		for (I=0;I<3;I++){
			stringRGBA(screen,40,175+30*I,Menu[I],0,200+50*(I==M),0,255);
			boxRGBA(screen,160,175+30*I,359,175+30*I+8,0,64,0,255);
		}
			boxRGBA(screen,160,175+30*0+1,160+((float)Shades/255.0)*200,175+30*0+7,0,128,0,255);
			boxRGBA(screen,160,175+30*1+1,160+((float)MVolume/128.0)*200,175+30*1+7,0,128,0,255);
			boxRGBA(screen,160,175+30*2+1,160+((float)SVolume/128.0)*200,175+30*2+7,0,128,0,255);

		SDL_UpdateRect(screen,0,0,0,0);
		SDL_Delay(500);

		do {SDL_PollEvent(&event);} while ((!Fire)&&(!Quit)&&(!Up)&&(!Down)&&(!Left)&&(!Right));

		if (Up)		if ((--M)<0) M=2;
		if (Down)	if ((++M)>2) M=0;
		if (Right){
			if (M==0){
				if (Shades==0) Shades=128;
				else
				if (Shades==128) Shades=255;
				SDL_FreeSurface(Sh);
				if (Shades){
					Sh=SDL_CreateRGBSurface(SDL_SRCALPHA | SDL_HWSURFACE,WIDTH,HEIGHT,BPP,screen->format->Rmask,screen->format->Gmask,screen->format->Bmask,screen->format->Amask);
					if (Sh==NULL) fprintf(stderr,"SH neni!!\n");
					SDL_SetColorKey(Sh,SDL_SRCCOLORKEY,SDL_MapRGB(Sh->format,255,0,255));
					SDL_SetAlpha(Sh,SDL_SRCALPHA,Shades);
				}
			}
			if ((M==1)&&(MVolume<128)){
				if ((MVolume+=128/9)>128)
					MVolume=128;
				Mix_VolumeMusic(MVolume);
			}
			if ((M==2)&&(SVolume<128)){
				if ((SVolume+=128/9)>128)
					SVolume=128;
				Mix_Volume(-1,SVolume);
				PlaySample(Snd_Push);
			}
		}
		if (Left){
			if (M==0){
				if (Shades==255) Shades=128;
				else
				if (Shades==128) Shades=0;
				SDL_FreeSurface(Sh);
				if (Shades){
					Sh=SDL_CreateRGBSurface(SDL_SRCALPHA | SDL_HWSURFACE,WIDTH,HEIGHT,BPP,screen->format->Rmask,screen->format->Gmask,screen->format->Bmask,screen->format->Amask);
					if (Sh==NULL) fprintf(stderr,"SH neni!!\n");
					SDL_SetColorKey(Sh,SDL_SRCCOLORKEY,SDL_MapRGB(Sh->format,255,0,255));
					SDL_SetAlpha(Sh,SDL_SRCALPHA,Shades);
				}
			}
			if ((M==1)&&(MVolume>0)){
				if ((MVolume-=128/9)<0)
					MVolume=0;
				Mix_VolumeMusic(MVolume);
			}
			if ((M==2)&&(SVolume>0)){
				if ((SVolume-=128/9)<0)
					SVolume=0;
				Mix_Volume(-1,SVolume);
				PlaySample(Snd_Push);
			}
		}
	
	}

	Setup=0;
	Fire=0;
	Quit=0;
	UC=0;


	//SaveConfig("./xpired.cfg");

	SDL_BlitSurface(swapscreen,NULL,screen,NULL);
	SDL_UpdateRect(screen,0,0,0,0);
}


char Gameplay(char DemoMode){
Uint32 Ticks,OldTicks,WT,FR=0,RecTick=0,RecState=1;
int PX2,PY2;
char Sw=0,LLeft=0,LRight=0,LUp=0,LDown=0,LFire=0,LQuit=0;

	if (DemoMode)
		Level=Rec[0].Tick;
	else
		Rec[0].Tick=Level;

	Lives=3;
	TextReset();

do{	
	SetLevel(Level,DemoMode);
	Sec=1000/GCycle;
	PAlive=1.5*DeadDelay;
	OldTicks=WT=SDL_GetTicks();

	do {

// BOF vykresleni
		DrawGame();
		FR++;
// EOF vykresleni

		Ticks=SDL_GetTicks();
		if ((ShowFPS)&&(Ticks-OldTicks>=1000)){
			fprintf(stdout,"%d FPS\n",FR);
			FR=0;
			OldTicks=Ticks;
		}

		if ((WT+GCycle)>Ticks)
			SDL_Delay((WT+GCycle)-Ticks);
		Ticks=SDL_GetTicks();
		
// GameState smycka

		while ((WT+GCycle)<=Ticks){
			RecTick++;
			SDL_PollEvent(&event);
			if (DemoMode){ // Playing
				if ((Quit!=LQuit)||(Left!=LLeft)||(Right!=LRight)||(Up!=LUp)||(Down!=LDown)||(Fire!=LFire))
					Quit=2;
				else
				if (Rec[RecState].Tick==RecTick){
					LLeft=Left=Rec[RecState].Left;
					LRight=Right=Rec[RecState].Right;
					LUp=Up=Rec[RecState].Up;
					LDown=Down=Rec[RecState].Down;
					LFire=Fire=Rec[RecState].Fire;
					LQuit=Quit=Rec[RecState].Quit;
					if (RecState<MaxRecLength-1)
						RecState++;
				}
			}else // Recording
				if ((Left!=LLeft)||(Right!=LRight)||(Up!=LUp)||(Down!=LDown)||(Fire!=LFire)||(Quit!=LQuit)){
					Rec[RecState].Tick=RecTick;
					LLeft=Rec[RecState].Left=Left;
					LRight=Rec[RecState].Right=Right;
					LUp=Rec[RecState].Up=Up;
					LDown=Rec[RecState].Down=Down;
					LFire=Rec[RecState].Fire=Fire;
					LQuit=Rec[RecState].Quit=Quit;
					if (RecState<MaxRecLength-1)
						RecState++;
				}
			
			if (Setup && !DemoMode){
				SystemSetup();
				DR_RCT(0,0,399,399);
			}

			if ((abs(PPX)==19)||(abs(PPY)==19)){
				DR_SPR(PX*20+PPX,PY*20+PPY,'*');
				PX+=sgn(PPX);
				PY+=sgn(PPY);
//				PF=1;
				if ((ALvl.M[PX][PY].BTyp=='T')&&(PPX||PPY)){
					DR_SPR(PX*20,PY*20,'*');
					PX2=PX;
					PY2=PY;
					if (GetTeleport(&PX2,&PY2)){
						TeleportEffect(PX,PY);
						PX=PX2;
						PY=PY2;
					}
					PPX=sgn(PPX);
					PPY=sgn(PPY);
					if (!is_free(PPX,PPY)){
						PPX=0;
						PPY=0;
					}
				}else{
					PPX=0;
					PPY=0;
				}
				DR_SPR(PX*20+PPX,PY*20+PPY,'*');
			}else
			if (PPX||PPY){
				DR_SPR(PX*20+PPX,PY*20+PPY,'*');
				PPX+=PF*sgn(PPX);
				PPY+=PF*sgn(PPY);
				LM=(PPY>0)+(PPX>0)*2+(PPX<0)*3;
				DR_SPR(PX*20+PPX,PY*20+PPY,'*');
			}

			MoveObjects();

			if ((PPX==0)&&(PPY==0)&&(PAlive==DeadDelay)){
				if (Fire)
					PF=2;
				else
					PF=1;
				if (ALvl.M[PX][PY].BTyp=='#'){
					PF=2;
					if ((!Left)&&(!Right)&&(!Up)&&(!Down)){
						PPX=-1*(LM==3)+(LM==2);
						PPY=-1*(LM==0)+(LM==1);
						DR_SPR(PX*20,PY*20,'*');
					}
					if (!is_free(PPX,PPY)){
						PPX=0;
						PPY=0;
					}
				}
				if ((Left&&!Right&&!Up&&!Down)||(Left&&Down&&Sw)||(Left&&Up&&!Sw))		if (is_free(-1,0))		{Sw=!Sw;PPX=-1;PPY=0;DR_SPR(PX*20,PY*20,'*');} else Sw=!Sw;
				else
				if ((Right&&!Left&&!Up&&!Down)||(Right&&Down&&!Sw)||(Right&&Up&&Sw))	if (is_free(+1,0))		{Sw=!Sw;PPX=+1;PPY=0;DR_SPR(PX*20,PY*20,'*');} else Sw=!Sw;
				else
				if ((Up&&!Down&&!Left&&!Right)||(Up&&Left&&Sw)||(Up&&Right&&!Sw))		if (is_free(0,-1))		{Sw=!Sw;PPY=-1;PPX=0;DR_SPR(PX*20,PY*20,'*');} else Sw=!Sw;
				else
				if ((Down&&!Up&&!Left&&!Right)||(Down&&Left&&!Sw)||(Down&&Right&&Sw))	if (is_free(0,+1))		{Sw=!Sw;PPY=+1;PPX=0;DR_SPR(PX*20,PY*20,'*');} else Sw=!Sw;
			}

			if (!BEM&&(ALvl.M[PX][PY].BTyp=='+')){
				Lives+=(Level%2);
				SetLevel(Level+1,DemoMode);
			}
			if ((ALvl.M[PX][PY].txt>=0)&&(ALvl.Text[ALvl.M[PX][PY].txt][0]!='\0')){
				TextAdd(PX*20+10,PY*20+10,ALvl.Text[ALvl.M[PX][PY].txt]);
				ALvl.Text[ALvl.M[PX][PY].txt][0]='\0';
				ALvl.M[PX][PY].txt=-1;
			}

			//--- SMRT
			// --- SMRT NA PRANI ---
			if (Quit){
				if (PAlive>=DeadDelay){
					PAlive=DeadDelay-1;
					if (Quit==2)
						Lives=0;
					TextAdd(PX*20+10,PY*20+10,"Why?!");
				}
				Quit=0;
			}
			//--- EO SMRT ON DEMAND ---

			if (PAlive==DeadDelay){
				if ((abs(PPX)>9)||(abs(PPY>9))){
					if ((ALvl.M[PX+sgn(PPX)][PY+sgn(PPY)].f==4)||(ALvl.M[PX+sgn(PPX)][PY+sgn(PPY)].f==6)||(ALvl.M[PX+sgn(PPX)][PY+sgn(PPY)].f==8)){
						PAlive=DeadDelay-1;
						TextAdd(PX*20+10,PY*20+10,"AAAAaaaaaeghr!");
					}
				}else
				if ((ALvl.M[PX][PY].f==4)||(ALvl.M[PX][PY].f==6)||(ALvl.M[PX][PY].f==8)){
						PAlive=DeadDelay-1;
						TextAdd(PX*20+10,PY*20+10,"AAAAaaaaaeghr!");
				}else
				if (ALvl.M[PX][PY].BTyp=='@'){
						PAlive=DeadDelay-1;
						TextAdd(PX*20+10,PY*20+10,"AAAAaaaaaeghr!");
				}else
				if (bem(PX-1,PY-1)||bem(PX,PY-1)||bem(PX+1,PY-1)||bem(PX-1,PY)||bem(PX+1,PY)||bem(PX-1,PY+1)||bem(PX,PY+1)||bem(PX+1,PY+1)){
						PAlive=DeadDelay-1;
						TextAdd(PX*20+10,PY*20+10,"AAAAaaaaaeghr!");
				}
			}
			//--- EO SMRT

			if ((PAlive!=DeadDelay)&&(PAlive)){
				PAlive--;
				DR_SPR(PX*20+PPX,PY*20+PPY,'*');
			}

			if (Timed){
				if (!--Sec){
					Sec=1000/GCycle;
					if (--ALvl.DL<=0){
						ALvl.DL=0;
						PAlive--;
						DR_SPR(PX*20+PPX,PY*20+PPY,'*');
						Timed=0;
						ExplodeLevel();
					}
					PlaySample(Snd_Beep);
					DR_RCT(200-26,400-17,200+26,399);
				}
			}
			// ---EOSmrt

			TextBehave();
			
			WT+=GCycle;
		}
	} while (PAlive);
	
	Lives--;

} while (Lives>0);

	if (DemoMode)
		Left=Right=Up=Down=Fire=Quit=0;
	else
		if (RecState<MaxRecLength-1)
			Rec[RecState+1].Quit=2;

	PlaySample(Snd_GameOver);
	return 0;
}


char Title(void){
char Return=-1,M=5,Caption[MaxStrLenLong];
SDL_Rect Src;
Uint32 OldTicks;

	Src.h=400;
	Src.w=400;
	Src.x=0;
	Src.y=0;

	sprintf(Caption,"%s %s",APPNAME,VERSION);
	SDL_WM_SetCaption(Caption,APPNAME);

	OldTicks=SDL_GetTicks();
	SDL_BlitSurface(TitleS,NULL,screen,NULL);
	if (Passwd[0]!='\0'){
		boxRGBA(screen,0,390,399,399,0,255,0,64);
		stringRGBA(screen,200-strlen(Passwd)*4,391,Passwd,10,250,10,128);
	}
	SDL_UpdateRect(screen, 0, 0, 0, 0);

	while ((Fire)||(Quit==1))
		SDL_PollEvent(&event);

	SDL_EnableUNICODE(1);

	while ((Return==-1)){
		UC=0;
		SDL_PollEvent(&event);

		if (Setup)
			SystemSetup();
		
		if ((UC)&&(!Fire)&&(!Quit)){
			if ((UC!=8)&&(UC!=13)&&(strlen(Passwd)<10)){
				Passwd[strlen(Passwd)+1]='\0';
				Passwd[strlen(Passwd)]=UC;
			}else
			if ((UC==8)&&(strlen(Passwd)>0))
				Passwd[strlen(Passwd)-1]='\0';
			if (UC==13)
				Return=1;
				SDL_BlitSurface(TitleS,&Src,screen,NULL);
				if (Passwd[0]!='\0'){
					boxRGBA(screen,0,390,399,399,0,255,0,64);
					stringRGBA(screen,200-strlen(Passwd)*4,391,Passwd,10,250,10,128);
				}
				SDL_UpdateRect(screen, 0, 390, 400, 10);
		}
		if (SDL_GetTicks()-OldTicks>10000){
			Src.y+=M;
			SDL_BlitSurface(TitleS,&Src,screen,NULL);
			if (Passwd[0]!='\0'){
				boxRGBA(screen,0,390,399,399,0,255,0,64);
				stringRGBA(screen,200-strlen(Passwd)*4,391,Passwd,10,250,10,128);
			}
			SDL_UpdateRect(screen, 0, 0, 0, 0);
			if (Src.y==400*(M>0)){
				OldTicks=SDL_GetTicks();
				M*=-1;
			}
		}
		if (Fire)
			Return=1;
		if (Quit)
			Return=0;
	}

	SDL_EnableUNICODE(0);
	
	while (Fire)
		SDL_PollEvent(&event);

	return Return;
}


int GetLevelByPasswd(char *Pswd){
int I;

	if (Pswd[0]=='\0')
		return 0;
	for (I=MaxLevel-1;(I>0)&&(strcmp(Lvl[I].Pw,Pswd));I--);

	return I;
}


// --------------------
void Game(void){
	while (1)
		if (Title()){
			Level=GetLevelByPasswd(Passwd);
			Gameplay(!strcmp(Passwd,"demo"));
		}
		else
			return;
}



// MAIN ---------------------------------
int main(int argc, char *argv[]){
  int I;
  Uint32 InitFlags;
  char buf[255];
  char LevelF[255];
  char joystick=0;

#ifdef UNIX

  strcpy(GAME_HOME,getenv("HOME"));
  strcat(GAME_HOME,"/.xpired");

#else

  strcpy(GAME_HOME,".");

#endif

  sprintf(LevelF,"%s/xpired.lvl",SHARE_DIR);

// init demo record
  Rec[0].Tick=0;
  Rec[1].Tick=1;
  Rec[1].Quit=2;

// load demo
  sprintf(buf,"%s/xpired.dmo",GAME_HOME);

  if(!LoadDemo(buf)){
    sprintf(buf,"%s/xpired.dmo",SHARE_DIR);
    LoadDemo(buf);
  }

// load config
  sprintf(ConfFName,"%s/xpired.cfg",GAME_HOME);
  if(!LoadConfig(ConfFName))
    {
      sprintf(buf,"%s/xpired.cfg",SHARE_DIR);
      LoadConfig(buf);
#ifdef UNIX
      if(mkdir(GAME_HOME,0750))
	fprintf(stderr,"Cannot create %s!\n",buf);
      else
	SaveConfig(ConfFName);
#endif
    }

 
  for (I=1;I<argc;I++){
    if ((!strcmp("--nosound",argv[I]))||(!strcmp("-n",argv[I]))){
      Sound=0;
    }else if ((!strcmp("--help",argv[I]))||(!strcmp("-h",argv[I]))){
		fprintf(stdout,"Example of usage: %s --nosound -l levelfile.lvl -d demo.dmo\n",argv[0]);
		fprintf(stdout," Switches:\n");
		fprintf(stdout,"   -j, --joystick      Enable Joystick 0\n");
		fprintf(stdout,"   -f, --fullscreen    Toggle fullscreen/windowed\n");
		fprintf(stdout,"   -n, --nosound       Disable sound\n");
		fprintf(stdout,"   -l  <file>          Load level from <file>\n");
		fprintf(stdout,"   -d  <file>          Use demo record from <file>\n");
		fprintf(stdout,"   --version           Display version no.\n");
		fprintf(stdout,"   -h, --help          Display this help screen\n");
		return 0;
    }else if (!strcmp("-l",argv[I])){
      strcpy(LevelF,argv[++I]);
    }else if (!strcmp("-d",argv[I])){
      if (!LoadDemo(argv[++I]))
	fprintf(stderr,"Error loading demo record...\n");
    }else if( !strcmp("-f",argv[I]) || !strcmp("--fullscreen",argv[I])){
      FULLSCR=!FULLSCR;
    }else if (!strcmp("--version",argv[I])){
		fprintf(stdout,"%s ver. %s \n",APPNAME,VERSION);
		return 0;
	}
  }
  InitFlags=SDL_INIT_VIDEO;
  if (Sound)
    InitFlags|=SDL_INIT_AUDIO;
  if (joystick)
	InitFlags |= SDL_INIT_JOYSTICK;

  if ( SDL_Init(InitFlags) < 0 ) {
    fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
    exit(1);
  }
  atexit(SDL_Quit);

  SDL_WM_SetCaption("X-pired says: A moment, please...","X-pired at work");

  if (Sound)
    if ( Mix_OpenAudio(22050, AUDIO_S16, 1, 1024) < 0 ){
      fprintf(stderr,"Warning: Couldn't set 22050 Hz 16-bit audio\n- Reason: %s\n",SDL_GetError());
      Sound=0;
    }
  if (joystick)
	js = SDL_JoystickOpen(0); // open the first joystick

  //	fprintf(stdout,"SDL awaken!\n");

  sprintf(buf,"%s/img/icon.bmp",SHARE_DIR);
  SDL_WM_SetIcon(SDL_LoadBMP(buf), NULL);


  VIDEO_INIT();
  SDL_SetEventFilter(E_FILTER);
  SDL_EnableKeyRepeat(0,0);

  LoadSprites();
  if (!LoadLevels(LevelF))
    fprintf(stderr,"Error loading level file...\n");


  //	printf("Shades=%d\n",Shades);
  //	printf("MVolume=%d\n",MVolume);

  ALvl.Bg=SDL_DisplayFormat(SDL_CreateRGBSurface(SDL_HWSURFACE,WIDTH,HEIGHT,BPP,screen->format->Rmask,screen->format->Gmask,screen->format->Bmask,screen->format->Amask));
  if (ALvl.Bg==NULL) fprintf(stderr,"Can't allocate ALvl.BG!\n");

  if (Shades){
    Sh=SDL_CreateRGBSurface(SDL_SRCALPHA | SDL_HWSURFACE,WIDTH,HEIGHT,BPP,screen->format->Rmask,screen->format->Gmask,screen->format->Bmask,screen->format->Amask);
    if (Sh==NULL) fprintf(stderr,"Can't allocate SH!\n");
    SDL_SetColorKey(Sh,SDL_SRCCOLORKEY,SDL_MapRGB(Sh->format,255,0,255));
    SDL_SetAlpha(Sh,SDL_SRCALPHA,Shades);
  }

  swapscreen=SDL_CreateRGBSurface(SDL_SRCALPHA | SDL_HWSURFACE,WIDTH,HEIGHT,BPP,screen->format->Rmask,screen->format->Gmask,screen->format->Bmask,screen->format->Amask);
  if (swapscreen==NULL) fprintf(stderr,"Can't allocate SwapScreen!\n");

  if (Sound){
    Mix_Volume(-1,SVolume);
    Mix_HookMusicFinished(&MusicLoopback);
    Mix_VolumeMusic(MVolume);
    Mix_FadeInMusic(Music,0,3000);
  }

  Game();

  SaveConfig(ConfFName);
  sprintf(buf,"%s/xpired.dmo",GAME_HOME);
  SaveDemo(buf);

  if (Sound){
    Mix_FadeOutMusic(1000);
    SDL_Delay(1000);
  }
  if( joystick )
	SDL_JoystickClose(js);

  SDL_Quit();
  //	fprintf(stdout,"SDL shot down.\n\n");
  return 0;
}
