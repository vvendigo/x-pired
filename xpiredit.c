#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#ifdef UNIX

#include"xpired_unix.h"

#endif

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_gfxPrimitives.h>

#define	TURBO 1
#define	APPNAME "X-pir-Ed-It"
#define VERSION "1.2"

#ifdef UNIX
  char *SHARE_DIR=SHARE_PREFIX;
#else
  char *SHARE_DIR=".";
#endif;

char GAME_HOME[255];

Uint8	BPP=8;
Uint8	Shades=128;

SDL_Surface *screen,*Sh=NULL,*SLevel,*SLives,*STime,*SNumbers;

char Quit=0,Left=0,Right=0,Up=0,Down=0,Fire=0,Alt=0,Ctrl=0,Shift=0,FULLSCR;
int  Level=-1,MX,MY,MB,MBlock;
int  WIDTH=440,HEIGHT=410;
char LFile[256]="./xpired.lvl";
Uint16 UC;

SDL_Event event;

int MVolume=64;
int SVolume=64;

typedef struct T_Spr{
	SDL_Surface *img,*sha;
}T_Spr;

T_Spr Spr[256];


typedef struct T_LElem{
	unsigned char FSpr,FTyp,BSpr,BTyp;
	char x,y,px,py,f,txt;
}T_LElem;	

typedef struct T_Lev{
	T_LElem M[20][20];
	char Name[100],BgFile[256],Pw[11];
	unsigned int DL;
	SDL_Surface *Bg;
	char Text[10][51];
}T_Lev;

T_Lev Lvl[100];

// menucka...
typedef struct T_E_Sel_E{
	unsigned char Spr,Typ;
	char Desc[50];
}T_E_Sel_E;

typedef struct T_E_Sel{
	T_E_Sel_E E[256];
	int X,L,Top,Akt;
	char Name[10];
}T_E_Sel;

T_E_Sel M_Fg,M_Bg;

char M_Txt[10][20],M_Msg[10][20];

char ConfFName[255];

//--------------------------------------


void VIDEO_INIT(){
Uint32 videoflags;
const SDL_VideoInfo *info;

	info = SDL_GetVideoInfo();
	BPP=info->vfmt->BitsPerPixel;

	videoflags = SDL_SRCALPHA; // | SDL_RESIZABLE;
	if(FULLSCR){
		videoflags = SDL_SRCALPHA | SDL_FULLSCREEN | SDL_SWSURFACE;
	}else{
		videoflags = SDL_SRCALPHA;
		if (TURBO) videoflags=videoflags | SDL_HWSURFACE | SDL_HWPALETTE;
		else videoflags=videoflags | SDL_SWSURFACE;
	}

	if ( (screen=SDL_SetVideoMode(WIDTH,HEIGHT,BPP,videoflags)) == NULL ) {
		fprintf(stderr, "Couldn't set %ix%i video mode: %s\n",WIDTH,HEIGHT,SDL_GetError());
		exit(2);
	}

//	SDL_SetAlpha(screen, SDL_SRCALPHA, 0);
//	vscreen=SDL_CreateRGBSurface(SDL_SWSURFACE,WIDTH,HEIGHT,BPP,screen->format->Rmask,screen->format->Gmask,screen->format->Bmask,screen->format->Amask);
//	if (vscreen==NULL) printf("VSCREEN neni!!\n");

//	fprintf(stdout,"VIDEO SET: W=%d H=%d BPP=%d TURBO=%d\n",WIDTH,HEIGHT,BPP,TURBO);
}


void ClearSurface(SDL_Surface *Co){
Uint32 color;
SDL_Rect clip;

	clip.x = 0;
	clip.y = 0;
	clip.w = Co->w;
	clip.h = Co->h;

	/* Setup clear color color */
	color=SDL_MapRGBA(Co->format, 255,0,255,0);

	/* Clear the screen */
//	SDL_SetClipRect(Co, &clip);
	SDL_FillRect (Co, &clip, color);
}


int E_FILTER(const SDL_Event *event) {
char *Str;
	if ( event->type == SDL_VIDEORESIZE ) {
		WIDTH=event->resize.w;
		HEIGHT=event->resize.h;
		VIDEO_INIT();
		return 0;
	}

	if (event->type==SDL_KEYDOWN){
		if (event->key.keysym.sym==SDLK_RALT) Fire=1;
		if (event->key.keysym.sym==SDLK_RETURN) Fire=1;
		if (event->key.keysym.sym==SDLK_ESCAPE) Quit=1;
		if (event->key.keysym.sym==SDLK_LALT) Alt=1;
		if (event->key.keysym.sym==SDLK_LCTRL) Ctrl=1;
		if (event->key.keysym.sym==SDLK_q) Quit=2;
		if ((event->key.keysym.sym==SDLK_c)&&(Ctrl)) Quit=2;
		if ((event->key.keysym.sym==SDLK_F4)&&(Alt)) Quit=2;

		if (event->key.keysym.sym==SDLK_LEFT) Left=1;
		if (event->key.keysym.sym==SDLK_RIGHT) Right=1;
		if (event->key.keysym.sym==SDLK_UP) Up=1;
		if (event->key.keysym.sym==SDLK_DOWN) Down=1;
		if (event->key.keysym.sym==SDLK_LSHIFT)
			Shift=1;
		UC=event->key.keysym.unicode;
		return(0);
	}

	if (event->type==SDL_KEYUP){
		if (event->key.keysym.sym==SDLK_LEFT) Left=0;
		if (event->key.keysym.sym==SDLK_RIGHT) Right=0;
		if (event->key.keysym.sym==SDLK_UP) Up=0;
		if (event->key.keysym.sym==SDLK_DOWN) Down=0;
		if (event->key.keysym.sym==SDLK_RETURN) Fire=0;
		if (event->key.keysym.sym==SDLK_RALT) Fire=0;
		if (event->key.keysym.sym==SDLK_ESCAPE) Quit=0;
		if (event->key.keysym.sym==SDLK_LALT) Alt=0;
		if (event->key.keysym.sym==SDLK_LCTRL) Ctrl=0;
		if (event->key.keysym.sym==SDLK_LSHIFT) Shift=0;
		return 0;
	}

	if ( event->type == SDL_MOUSEBUTTONDOWN ) {
		if (event->button.button==SDL_BUTTON_LEFT)
			MB=1;
		if (event->button.button==SDL_BUTTON_RIGHT)
			MB=2;
		return 0;
	}

	if ( event->type == SDL_MOUSEBUTTONUP ) {
		MB=0;
		MBlock=0;
		return 0;
	}

	if ( event->type == SDL_MOUSEMOTION ) {
        MX=event->motion.x;
		MY=event->motion.y;
		return(0);
    }

	if (event->type==SDL_QUIT) Quit=2;
	
	return(1);
}


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


//---------------- EXEC ------------------


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
int I;

	for (I=0;I<256;I++){
		Spr[I].img=NULL;
		Spr[I].sha=NULL;
	}

#ifdef UNIX

  chdir(SHARE_DIR);

#endif
	
	LoadSprite(&SLevel,"./img/level.bmp",0);
	LoadSprite(&SLives,"./img/lives.bmp",0);
	LoadSprite(&STime,"./img/time.bmp",0);
	LoadSprite(&SNumbers,"./img/numbers.bmp",0);
	LoadSprite(&(Spr['*']).img,"./img/start.bmp",128);
	LoadSprite(&(Spr['+']).img,"./img/exit.bmp",128);
	LoadSprite(&(Spr['%']).img,"./img/retard.bmp",96);
	LoadSprite(&(Spr['#']).img,"./img/ice.bmp",128);
	LoadSprite(&(Spr['&']).img,"./img/flamable.bmp",200);
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

	LoadSprite(&(Spr[1]).img,"./img/expl3.bmp",0);
	Spr[1].sha=NULL;
	LoadSprite(&(Spr[2]).img,"./img/expl2.bmp",0);
	Spr[2].sha=NULL;
	LoadSprite(&(Spr[3]).img,"./img/expl1.bmp",0);
	LoadSprite(&(Spr[3]).sha,"./img/shade-expl1.bmp",0);

	LoadSprite(&(Spr[4]).img,"./img/bo-flamable.bmp",0);
	Spr[3].sha=NULL;

	LoadSprite(&(Spr[5]).img,"./img/fire1.bmp",128);
	Spr[5].sha=NULL;
	LoadSprite(&(Spr[6]).img,"./img/fire2.bmp",128);
	Spr[6].sha=NULL;
	LoadSprite(&(Spr[7]).img,"./img/fire3.bmp",128);
	Spr[7].sha=NULL;
	LoadSprite(&(Spr[8]).img,"./img/fire2.bmp",128);
	Spr[8].sha=NULL;

	LoadSprite(&(Spr['B']).img,"./img/bem1.bmp",0);
	LoadSprite(&(Spr['B']).sha,"./img/shade-bem1.bmp",0);


}


int chartoint(char Co){
	if (Co=='0') return 0;
	if (Co=='1') return 1;
	if (Co=='2') return 2;
	if (Co=='3') return 3;
	if (Co=='4') return 4;
	if (Co=='5') return 5;
	if (Co=='6') return 6;
	if (Co=='7') return 7;
	if (Co=='8') return 8;
	if (Co=='9') return 9;
	return -1;
}


int myatoi(char Co[]){
int I=strlen(Co)-1,Exp=1,Res=0;

while ((Exp!=-1)&&(I>=0)&&(Co[I]!='-')){
	Res+=Exp*chartoint(Co[I]);
	if (Exp<10000)
		Exp*=10;
	else
		Exp=-1;
	I--;
}
if ((I>=0)&&(Co[I]=='-')) Res*=-1;
return Res;
}


char LoadConfig(char *Soubor){
	FILE *F;
	char S[256],T[10];
	int I,J;

	if ((F=fopen(Soubor,"r"))==NULL)
		return 0;
	while (!feof(F)){
		fgets (S,255,F);
		while ((strlen(S)>0)&&(S[strlen(S)-1]<' '))
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
				if (!strncmp(S,"fullscreen=",11))
					FULLSCR=chartoint(S[11]);

		}
	}
	fclose (F);
	return 1;
}


char LoadLevels(char *Soubor){
	FILE *F;
	char S[256],T[10];
	int I=0,J,L=-1;
	
	for (J=0;J<100;J++)
		Lvl[J].Name[0]='\0';

	if ((F=fopen(Soubor,"r"))==NULL)
		return 0;
	while ((!feof(F))&&(fgets(S,255,F)!=NULL)){
		while ((strlen(S)>0)&&(S[strlen(S)-1]<' '))
			S[strlen(S)-1]='\0';
		if ((S[0]!='\0')&&(S[0]!='#')){
//			printf("%s\n",S);
			if ((S[0]=='>')&&(S[strlen(S)-1]=='-')){
				L++;
				Lvl[L].Name[0]='\0';
				Lvl[L].Pw[0]='\0';
				Lvl[L].BgFile[0]='\0';
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
//				fprintf(stdout,"Level(%d) loaded...\n",L);
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
					Lvl[L].DL=myatoi(T);
				}
				else
				if (!strncmp(S,"background=",5)){
					for (J=11;J<=strlen(S);J++)
						T[J-11]=S[J];
					strcpy(Lvl[L].BgFile,T);
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


void DrawGame(void){
int I,J;
char Str[20];
SDL_Rect Rct;

	SDL_BlitSurface(Lvl[Level].Bg,NULL,screen,NULL);
	for (I=0;I<20;I++)
		for (J=0;J<20;J++)
			DrawSpr(Spr[Lvl[Level].M[I][J].BSpr].img,screen,I*20+Lvl[Level].M[I][J].x,J*20+Lvl[Level].M[I][J].y);

	if (Shades){
//	ClearSurface(Sh);
		boxRGBA(Sh,0,0,399,14,0,0,0,0xFF);
		boxRGBA(Sh,0,15,14,399,0,0,0,0xFF);
		boxRGBA(Sh,15,15,399,399,255,0,255,0xFF);

		for (I=0;I<20;I++)
			for (J=0;J<20;J++)
				DrawSpr(Spr[Lvl[Level].M[I][J].FSpr].sha,Sh,I*20+Lvl[Level].M[I][J].x,J*20+Lvl[Level].M[I][J].y);

		SDL_BlitSurface(Sh,NULL,screen,NULL);
	}


	for (I=0;I<20;I++)
		for (J=0;J<20;J++){
			DrawSpr(Spr[Lvl[Level].M[I][J].FSpr].img,screen,I*20+Lvl[Level].M[I][J].x,J*20+Lvl[Level].M[I][J].y);
			if (Lvl[Level].M[I][J].txt>=0){
				sprintf(Str,"T%c",'0'+Lvl[Level].M[I][J].txt);
				stringRGBA(screen,I*20+2,J*20+6,Str,0xE0,0xE0,0xE0,150);
			}
		}

	// Stat: Level---
		DrawSpr(SLevel,screen,1,400-16);
		R_Blit(SNumbers,screen,(Level/10)*11,0,11,15,15,400-16);
		R_Blit(SNumbers,screen,(Level%10)*11,0,11,15,26,400-16);
	// Stat: DeadLine---
	if (Lvl[Level].DL){
		DrawSpr(STime,screen,200-23,400-17);
		R_Blit(SNumbers,screen,(Lvl[Level].DL/100)*11,0,11,15,200-9,400-16);
		R_Blit(SNumbers,screen,((Lvl[Level].DL%100)/10)*11,0,11,15,200+2,400-16);
		R_Blit(SNumbers,screen,(Lvl[Level].DL%10)*11,0,11,15,200+13,400-16);
	}

	SDL_UpdateRect(screen,0,0,400,400);
}


int sgn(int X){
	if (X<0) return -1;
	if (X>0) return 1;
	return 0;
}


void SetCaption(void){
char Caption[100];

	if (Lvl[Level].Pw[0]=='\0')
		sprintf(Caption,"%s: %s",APPNAME,Lvl[Level].Name);
	else
		sprintf(Caption,"%s: %s [%s]",APPNAME,Lvl[Level].Name,Lvl[Level].Pw);
	SDL_WM_SetCaption(Caption,Caption);
}


void SetLevel(char Lev){
int I,J;
char Name[50];
SDL_Surface *Img;
SDL_Rect Rct;
	Level=Lev;
	
	SetCaption();

	if (Lvl[Level].Name[0]=='\0'){
		sprintf(Name,"Level %d entered.",Level);
		strcpy(Lvl[Level].Name,Name);
		Lvl[Level].BgFile[0]='\0';
		Lvl[Level].Bg=NULL;
		Lvl[Level].DL=0;
		for (I=0;I<20;I++)
			for (J=0;J<20;J++){
				Lvl[Level].M[I][J].FSpr=' ';
				Lvl[Level].M[I][J].FTyp=' ';
				Lvl[Level].M[I][J].BSpr=' ';
				Lvl[Level].M[I][J].BTyp=' ';
				Lvl[Level].M[I][J].x=0;
				Lvl[Level].M[I][J].y=0;
				Lvl[Level].M[I][J].px=0;
				Lvl[Level].M[I][J].py=0;
				Lvl[Level].M[I][J].f=0;
				Lvl[Level].M[I][J].txt=-1;
			}
		for (I=0;I<10;I++)
			Lvl[Level].Text[I][0]='\0';
	}

	if (Lvl[Level].Bg==NULL)
		Lvl[Level].Bg=SDL_DisplayFormat(SDL_CreateRGBSurface(SDL_HWSURFACE,400,400,BPP,screen->format->Rmask,screen->format->Gmask,screen->format->Bmask,screen->format->Amask));
	if (Lvl[Level].Bg==NULL) fprintf(stderr,"ALvl.BG neni!!\n");

	Img=IMG_Load(Lvl[Level].BgFile);
	if (Img!=NULL){
		Rct.w=Img->w;
		Rct.h=Img->h;
		for (I=0;I<400;I+=Rct.w)
			for (J=0;J<400;J+=Rct.h){
				Rct.x=I;
				Rct.y=J;
				SDL_BlitSurface(Img,NULL,Lvl[Level].Bg,&Rct);
			}
		SDL_FreeSurface(Img);
	}else
		boxRGBA(Lvl[Level].Bg,0,0,399,399,0x40,0x40,0x40,0xFF);
}


void DrawMsg(char Msg[]){
	boxRGBA(screen,350,400,439,409,16,16,16,0xFF);
	rectangleRGBA(screen,350,400,439,409,32,32,32,0xFF);
	if (Msg[strlen(Msg)-1]!='!')
		stringRGBA(screen,351,401,Msg,200,200,200,0xFF);
	else
		stringRGBA(screen,351,401,Msg,250,50,50,0xFF);
	SDL_UpdateRect(screen,350,400,90,10);
}

void DrawMenu(T_E_Sel Mnu){
int I;
	boxRGBA(screen,Mnu.X,0,Mnu.X+19,399,64,64,64,0xFF);
	boxRGBA(screen,Mnu.X+19,0,Mnu.X+19,399,32,32,32,0xFF);

	rectangleRGBA(screen,Mnu.X,0,Mnu.X+19,19,32,32,32,0xFF);
	lineRGBA(screen,Mnu.X+5,5,Mnu.X+10,0,200,200,200,0xFF);
	lineRGBA(screen,Mnu.X+10,0,Mnu.X+15,5,200,200,200,0xFF);
	stringRGBA(screen,Mnu.X+1,10,Mnu.Name,200,200,200,0xFF);

	rectangleRGBA(screen,Mnu.X,380,Mnu.X+19,399,32,32,32,0xFF);
	lineRGBA(screen,Mnu.X+5,395,Mnu.X+10,399,200,200,200,0xFF);
	lineRGBA(screen,Mnu.X+10,399,Mnu.X+15,395,200,200,200,0xFF);
	
	for (I=0;(I<18)&&(Mnu.Top+I<Mnu.L);I++)
		DrawSpr(Spr[Mnu.E[Mnu.Top+I].Spr].img,screen,Mnu.X,20+I*20);

	if ((Mnu.Akt>=Mnu.Top)&&(Mnu.Akt<Mnu.Top+18)){
		rectangleRGBA(screen,Mnu.X+1,21+(Mnu.Akt-Mnu.Top)*20,Mnu.X+18,19+(Mnu.Akt-Mnu.Top)*20+19,0,0,0,128);
		rectangleRGBA(screen,Mnu.X,20+(Mnu.Akt-Mnu.Top)*20,Mnu.X+19,20+(Mnu.Akt-Mnu.Top)*20+19,255,255,255,128);
	}

	SDL_UpdateRect(screen,Mnu.X,0,20,400);
}

void OperMenu(T_E_Sel *Mnu){
int Y;
	if ((MX>=Mnu->X)&&(MX<Mnu->X+20)){
		Y=MY/20;
		if ((!MBlock)&&(MB)&&(Y==0)&&(Mnu->Top)) {Mnu->Top--;DrawMenu(*Mnu);MBlock=1;}
		if ((!MBlock)&&(MB)&&(Y==19)&&(Mnu->Top<Mnu->L-1)) {Mnu->Top++;DrawMenu(*Mnu);MBlock=1;}
		if ((Y>0)&&(Y<19)&&(Mnu->Top+Y-1<Mnu->L)){
			if ((!MBlock)&&(MB)){Mnu->Akt=Mnu->Top+Y-1;DrawMenu(*Mnu);MBlock=1;}
			else DrawMsg(Mnu->E[Mnu->Top+Y-1].Desc);
		}
	}
}

unsigned char GetMenu(T_E_Sel Mnu){
	return Mnu.E[Mnu.Akt].Spr;
}


void DrawTxtMenu(char Mnu[10][20]){
int I,L=0;
	boxRGBA(screen,0,400,419,409,64,64,64,0xFF);
	rectangleRGBA(screen,0,400,419,409,32,32,32,0xFF);
	for (I=0;(I<10)&&(Mnu[I][0]!='\0');I++){
		stringRGBA(screen,2+L*8,402,Mnu[I],250,250,250,0xFF);
		L+=strlen(Mnu[I]);
	}
	SDL_UpdateRect(screen,0,400,400,10);
}

char OperTxtMenu(char Mnu[10][20]){
int I,L=2;
	if ((!MBlock)&&(MB)&&(MY>400)){
		for (I=0;(I<10)&&(Mnu[I][0]!='\0');I++)
			if ((L+=strlen(Mnu[I])*8)>=MX) return I+1;
	}
	return 0;
}


void SelectBgImg(void){
FILE *F;
SDL_Surface *Img;
SDL_Rect Rct;
char Fls[256][256];
int I,J,C=1,Akt=0;
	Fls[0][0]='\0';
	F=fopen("./bgimages.txt","r");
	while ((!feof(F))&&(fgets(Fls[C],255,F)!=NULL)){
		while ((strlen(Fls[C]))&&(Fls[C][strlen(Fls[C])-1]<' '))
			Fls[C][strlen(Fls[C])-1]='\0';
		if (Fls[C][0]!='\0'){
//			fprintf(stdout,">%s\n",Fls[C]);
			if (!strcmp(Fls[C],Lvl[Level].BgFile)) Akt=C;
			C++;
		}
	}
	fclose(F);

	while ((!Fire)&&(!Quit)){
		strcpy(Lvl[Level].BgFile,Fls[Akt]);
		Img=IMG_Load(Lvl[Level].BgFile);
		if (Img!=NULL){
			Rct.w=Img->w;
			Rct.h=Img->h;
			for (I=0;I<400;I+=Rct.w)
				for (J=0;J<400;J+=Rct.h){
					Rct.x=I;
					Rct.y=J;
					SDL_BlitSurface(Img,NULL,Lvl[Level].Bg,&Rct);
				}
			SDL_FreeSurface(Img);
		}else
			boxRGBA(Lvl[Level].Bg,0,0,399,399,0x40,0x40,0x40,0xFF);
		DrawGame();
		boxRGBA(screen,0,0,200,200,32,32,32,0xFF);
		boxRGBA(screen,0,100,200,110,64,64,64,0xFF);
		stringRGBA(screen,1,1,"Select BgImage:",255,255,255,0xFF);
		for (I=0;I<19;I++)
			if ((Akt-9+I>=0)&&(Akt-9+I<C))
				stringRGBA(screen,1,10+I*10,Fls[Akt-9+I],200,200,200,0xFF);
		SDL_UpdateRect(screen,0,0,200,200);
		while ((Fire)||(Up)||(Down)) SDL_PollEvent(&event);
		while ((!Fire)&&(!Up)&&(!Down)&&(!Quit)) SDL_PollEvent(&event);
		if ((Up)&&(Akt>0)) Akt--;
		if ((Down)&&(Akt<C-1)) Akt++;
	}
	DrawGame();
}


char Save(char FileName[]){
FILE *F;
int I,J,K;
char P,S[100];

if ((F=fopen(FileName,"w"))==NULL)
	return 0;
fprintf(F,"#	Level Definition File for X-pired (xpired.ldx)\n");
fprintf(F,"#\n");
fprintf(F,"#	Level record:\n");
fprintf(F,"#	>---------		- level begin (/^>-+$/)\n");
fprintf(F,"#	name=Name		- level name (max 100 chars)		(optional)\n");
fprintf(F,"#	password=psw	- level password (max 10 chars)		(optional)\n");
fprintf(F,"#	deadline=X		- level deadline (in secs 0-999)	(optional)\n");
fprintf(F,"#	background=file	- level background (would be tiled)	(optional)\n");
fprintf(F,"#	X=some text		- text message definition (X-text ID (0-9), Text length must by less than 50 chars.)\n");
fprintf(F,"#	- Follows 20 lines consisting of 20 chunks ',FfBbXT' (level matrix 20x20) (F-foreground type, f-foreground sprite, B-background type, b-background sprite, X-effector, T-message ID).\n");
fprintf(F,"#	- Unrecognized and empty lines or lines beginning with # are ignored.\n");
fprintf(F,"#\n");
fprintf(F,"#	Level elems:\n");
fprintf(F,"#	*...Start position\n");
fprintf(F,"#	+...Exit position\n");
fprintf(F,"#	c...Crate\n");
fprintf(F,"#	o...Barrel\n");
fprintf(F,"#	O...HiExplosive\n");
fprintf(F,"#	X...Wall\n");
fprintf(F,"#	x...Tiny wall\n");
fprintf(F,"#	B...BEM\n");
fprintf(F,"#	%%...Retarder\n");
fprintf(F,"#	&...Flamable\n");
fprintf(F,"#	@...Hot\n");
fprintf(F,"#	#...Ice\n");
fprintf(F,"#	T...Teleport\n");
fprintf(F,"\n");

fprintf(F,"\n");
for (I=0;(I<100)&&(Lvl[I].Name[0]!='\0');I++){
	fprintf(F,">--------------------\n");
	fprintf(F,"name=%s\n",Lvl[I].Name);
	if (Lvl[I].Pw[0]!='\0')
		fprintf(F,"password=%s\n",Lvl[I].Pw);
	if (Lvl[I].BgFile[0]!='\0')
		fprintf(F,"background=%s\n",Lvl[I].BgFile);
	if (Lvl[I].DL)
		fprintf(F,"deadline=%d\n",Lvl[I].DL);
	for (J=0;J<20;J++){
		for (K=0;K<20;K++){
			if (Lvl[I].M[K][J].txt<0)
				P=' ';
			else
				P='0'+Lvl[I].M[K][J].txt;
			fprintf(F,",%c%c%c%c%d%c",Lvl[I].M[K][J].FTyp,Lvl[I].M[K][J].FSpr,Lvl[I].M[K][J].BTyp,Lvl[I].M[K][J].BSpr,Lvl[I].M[K][J].f,P);
		}
		fprintf(F,"\n");
	}
	for (J=0;J<10;J++)
		if (Lvl[I].Text[J][0]!='\0')
			fprintf(F,"%d=%s\n",J,Lvl[I].Text[J]);
}

fclose(F);
return 1;
}


char GetString(char *Dest,int MaxLen,int X, int Y){
int P=strlen(Dest);
char *Bck;
	
	Bck=(char *)malloc(strlen(Dest)+1);
	strcpy(Bck,Dest);

	do{
		boxRGBA(screen,X,Y,X+((MaxLen<50)?MaxLen:50)*8,Y+10,100,55,55,255);
		if (strlen(Dest)<=50)
			stringRGBA(screen,X+1,Y+1,Dest,250,250,250,255);
		else
			stringRGBA(screen,X+1,Y+1,Dest+(strlen(Dest)-50),250,250,250,255);
		SDL_UpdateRect(screen,X,Y,((MaxLen<50)?MaxLen:50)*8,10);
		UC=0;
		do {SDL_PollEvent(&event);}
		while (!UC);
//		fprintf(stdout,"'%c'\n",Key);
		if (UC==8){
			if (P>0){
				P--;
				Dest[P]='\0';
			}
		}
		else
		if ((!Fire)&&(!Quit)&&(P<MaxLen)){
			Dest[P]=UC;
			P++;
			Dest[P]='\0';
		}
	}while ((!Fire)&&(!Quit)&&(UC!=13));

	if (Quit){
		strcpy(Dest,Bck);
		free(Bck);
		return 0;
	}

	free(Bck);
	return 1;
}


void InputScr(char *Title){
	boxRGBA(screen,0,150,399,250,55,55,55,255);
	stringRGBA(screen,1,151,Title,255,255,255,255);
	SDL_UpdateRect(screen,0,150,400,100);
}


void DrawMessages(void){
char Str[55],I;
	boxRGBA(screen,0,0,399,100,100,100,100,255);
	for (I=0;I<10;I++){
		sprintf(Str,"%d:%s",I,Lvl[Level].Text[I]);
		stringRGBA(screen,0,I*10,Str,200,200,200,255);
	}
	SDL_UpdateRect(screen,0,0,400,100);
}

void SetMessages(void){
char M;
	DrawTxtMenu(M_Msg);
	DrawMsg("...Esc");
	DrawMessages();
	while (MB)
		SDL_PollEvent(&event);
	do{
		SDL_PollEvent(&event);
		if (M=OperTxtMenu(M_Msg)){
			InputScr("Enter message:");
			GetString(Lvl[Level].Text[M-1],50,1,195);
			DrawGame();
			DrawMessages();
		}
	} while (!Quit);
	DrawGame();
	DrawTxtMenu(M_Txt);
	DrawMsg("");
}

// --------------------
void Edit(void){
char M,Str[256];
int I,LX,LY;
SetLevel(0);
DrawGame();
DrawMenu(M_Fg);
DrawMenu(M_Bg);
DrawTxtMenu(M_Txt);
DrawMsg("");
while (1){
	LX=MX;
	LY=MY;
	UC=0;
	if (!MB)
		SDL_Delay(200);
	else
		SDL_Delay(50);
	SDL_PollEvent(&event);
	OperMenu(&M_Fg);
	OperMenu(&M_Bg);
	if ((UC>='0')&&(UC<='9')){
		Lvl[Level].M[MX/20][MY/20].txt=UC-'0';
		DrawGame();
	}
	if (Shift){
		Lvl[Level].M[MX/20][MY/20].txt=-1;
		DrawGame();
	}
	if ((MB==1)&&(MX<400)&&(MY<400)){
		if ((LX/20!=MX/20)||(LY/2!=MY/20))
			MBlock=0;
		if (!MBlock){
			Lvl[Level].M[MX/20][MY/20].FSpr=M_Fg.E[M_Fg.Akt].Spr;
			Lvl[Level].M[MX/20][MY/20].FTyp=M_Fg.E[M_Fg.Akt].Typ;
			Lvl[Level].M[MX/20][MY/20].BSpr=M_Bg.E[M_Bg.Akt].Spr;
			Lvl[Level].M[MX/20][MY/20].BTyp=M_Bg.E[M_Bg.Akt].Typ;
			MBlock=1;
			DrawGame();
		}
	}
	if ((MB==2)&&(MX<400)&&(MY<400)){
		for (I=0;I<M_Fg.L;I++)
			if ((Lvl[Level].M[MX/20][MY/20].FSpr==M_Fg.E[I].Spr)&&(Lvl[Level].M[MX/20][MY/20].FTyp==M_Fg.E[I].Typ)){
				M_Fg.Top=M_Fg.Akt=I;
				DrawMenu(M_Fg);
			}
		for (I=0;I<M_Bg.L;I++)
			if ((Lvl[Level].M[MX/20][MY/20].BSpr==M_Bg.E[I].Spr)&&(Lvl[Level].M[MX/20][MY/20].BTyp==M_Bg.E[I].Typ)){
				M_Bg.Top=M_Bg.Akt=I;
				DrawMenu(M_Bg);
			}
		MBlock=1;
		DrawGame();
	}
	if (M=OperTxtMenu(M_Txt)){
		if (M==1){			//name
			InputScr("Enter level name:");
			GetString(Lvl[Level].Name,50,1,195);
			SetCaption();
			DrawGame();
		}
		else if (M==2){			//password
			InputScr("Enter level password:");
			GetString(Lvl[Level].Pw,10,1,195);
			SetCaption();
			DrawGame();
		}
		else if (M==3){		//timer
			InputScr("Enter time limit (0-999 secs):");
			sprintf(Str,"%d",Lvl[Level].DL);
			GetString(Str,3,1,195);
			Lvl[Level].DL=atoi(Str);
			DrawGame();
		}
		else if (M==4)		//bgimg
			SelectBgImg();
		else if (M==5)		//txt
			SetMessages();
		else if (M==6){		//go to
			InputScr("Enter level no. (0-99):");
			sprintf(Str,"%d",Level);
			GetString(Str,2,1,195);
			Level=atoi(Str);
			SetLevel(Level);
			DrawGame();
		}
		else if (M==7){		//save
			InputScr("Enter filename:");
			if (GetString(LFile,255,1,195)){
				if (Save(LFile))
					DrawMsg("Saved...");
				else
					DrawMsg("Can't save!");
			}else
				DrawMsg("Aborted...");
			DrawGame();
		}
	}
	if (Up&&(Level<99)){
		SetLevel(Level+1);
		DrawGame();
		SDL_Delay(200);
	}
	if ((Down)&&(Level>0)){
		SetLevel(Level-1);
		DrawGame();
		SDL_Delay(200);
	}
	
	if (Quit==2)
		return;
}
}



// MAIN ---------------------------------
int main(int argc, char *argv[]){
int I;
char buf[255];

#ifdef UNIX
  strcpy(GAME_HOME,getenv("HOME"));
  strcat(GAME_HOME,"/.xpired");
#else
  strcpy(GAME_HOME,".");
#endif

  sprintf(ConfFName,"%s/xpired.cfg",GAME_HOME);

  if(!LoadConfig(ConfFName))
  {
    sprintf(buf,"%s/xpired.cfg",SHARE_DIR);
    LoadConfig(buf);
  }

  sprintf(LFile,"%s/xpired.lvl",SHARE_DIR);

  for (I=1;I<argc;I++){
    if ((!strcmp("--help",argv[I]))||(!strcmp("-h",argv[I]))){
		fprintf(stdout,"Example of usage: %s --nosound -l levelfile.lvl -d demo.dmo\n",argv[0]);
		fprintf(stdout," Switches:\n");
		fprintf(stdout,"   -f, --fullscreen    Toggle fullscreen/windowed\n");
		fprintf(stdout,"   -l  <file>          Load level from <file>\n");
		fprintf(stdout,"   -h, --help          Display this help screen\n");
		fprintf(stdout,"   --version           Display version no.\n");
		return 0;
    }else if (!strcmp("-l",argv[I])){
      strcpy(LFile,argv[++I]);
    }else if (!strcmp("-f",argv[I]) || !strcmp("--fullscreen",argv[I])){
      FULLSCR=!FULLSCR;
    }else if (!strcmp("--version",argv[I])){
		fprintf(stdout,"%s ver. %s \n",APPNAME,VERSION);
		return 0;
	}
  }

	if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);

	SDL_WM_SetCaption(APPNAME,APPNAME);

  sprintf(buf,"%s/img/icon.bmp",SHARE_DIR);
  
	SDL_WM_SetIcon(IMG_Load(buf), NULL);
	VIDEO_INIT();
	SDL_ShowCursor(SDL_ENABLE);
	SDL_SetEventFilter(E_FILTER);
	SDL_EnableKeyRepeat(0,0);
	SDL_EnableUNICODE(1);

	LoadSprites();
	LoadLevels(LFile);

	if (Shades){
		Sh=SDL_CreateRGBSurface(SDL_SRCALPHA,400,400,BPP,screen->format->Rmask,screen->format->Gmask,screen->format->Bmask,screen->format->Amask);
		if (Sh==NULL) fprintf(stderr,"SH neni!!\n");
		SDL_SetColorKey(Sh,SDL_SRCCOLORKEY,SDL_MapRGB(Sh->format,255,0,255));
		SDL_SetAlpha(Sh,SDL_SRCALPHA,Shades);
	}

//--- MNU DEFS
	M_Fg.X=400;
	M_Fg.Top=0;
	M_Fg.Akt=0;
	M_Fg.L=7;
	strcpy(M_Fg.Name,"Fg");
	M_Fg.E[0].Spr=' ';
	M_Fg.E[0].Typ=' ';
	strcpy(M_Fg.E[0].Desc,"None");
	M_Fg.E[1].Spr='X';
	M_Fg.E[1].Typ='X';
	strcpy(M_Fg.E[1].Desc,"Wall");
	M_Fg.E[2].Spr='x';
	M_Fg.E[2].Typ='x';
	strcpy(M_Fg.E[2].Desc,"TinyWall");
	M_Fg.E[3].Spr='c';
	M_Fg.E[3].Typ='c';
	strcpy(M_Fg.E[3].Desc,"Crate");
	M_Fg.E[4].Spr='o';
	M_Fg.E[4].Typ='o';
	strcpy(M_Fg.E[4].Desc,"Barrel");
	M_Fg.E[5].Spr='O';
	M_Fg.E[5].Typ='O';
	strcpy(M_Fg.E[5].Desc,"HiExplosive");
	M_Fg.E[6].Spr='B';
	M_Fg.E[6].Typ='B';
	strcpy(M_Fg.E[6].Desc,"BEM");

	M_Bg.X=420;
	M_Bg.Top=0;
	M_Bg.Akt=0;
	M_Bg.L=8;
	strcpy(M_Bg.Name,"Bg");
	M_Bg.E[0].Spr=' ';
	M_Bg.E[0].Typ=' ';
	strcpy(M_Bg.E[0].Desc,"None");
	M_Bg.E[1].Spr='*';
	M_Bg.E[1].Typ='*';
	strcpy(M_Bg.E[1].Desc,"Start");
	M_Bg.E[2].Spr='+';
	M_Bg.E[2].Typ='+';
	strcpy(M_Bg.E[2].Desc,"Exit");
	M_Bg.E[3].Spr='%';
	M_Bg.E[3].Typ='%';
	strcpy(M_Bg.E[3].Desc,"Retarder");
	M_Bg.E[4].Spr='#';
	M_Bg.E[4].Typ='#';
	strcpy(M_Bg.E[4].Desc,"Ice");
	M_Bg.E[5].Spr='&';
	M_Bg.E[5].Typ='&';
	strcpy(M_Bg.E[5].Desc,"Flamable");
	M_Bg.E[6].Spr='@';
	M_Bg.E[6].Typ='@';
	strcpy(M_Bg.E[6].Desc,"Hot! ");
	M_Bg.E[7].Spr='T';
	M_Bg.E[7].Typ='T';
	strcpy(M_Bg.E[7].Desc,"Teleport");

	strcpy(M_Txt[0],"Name|");
	strcpy(M_Txt[1],"Pw|");
	strcpy(M_Txt[2],"Timer|");
	strcpy(M_Txt[3],"BgImg|");
	strcpy(M_Txt[4],"Msg|");
	strcpy(M_Txt[5],"GoTo|");
	strcpy(M_Txt[6],"Save|");
	M_Txt[7][0]='\0';

	strcpy(M_Msg[0],"T0|");
	strcpy(M_Msg[1],"T1|");
	strcpy(M_Msg[2],"T2|");
	strcpy(M_Msg[3],"T3|");
	strcpy(M_Msg[4],"T4|");
	strcpy(M_Msg[5],"T5|");
	strcpy(M_Msg[6],"T6|");
	strcpy(M_Msg[7],"T7|");
	strcpy(M_Msg[8],"T8|");
	strcpy(M_Msg[9],"T9|");

	Edit();

	SDL_Quit();
	return 0;
}
