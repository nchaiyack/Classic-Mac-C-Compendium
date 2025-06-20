/*
**	include for titles & lightnings, etc.
**	//Jaf (c) 1991
*/
typedef	struct
{
					short	ref;
		unsigned	short	red;
		unsigned	short	green;
		unsigned	short	blue;
}		VAColorInfoElement;

typedef	VAColorInfoElement	VAColorInfo[10];	

enum { NOEVENT,LIGHTNING,LOGOSTART,LOGOLIVING,LOGOREMOVEY,LOGOREMOVEX,ZOOMSTART,DELAY};

void MainTitle();
void Text();
void Erase(int,int,int,int);
void ClearText();
void Lightning();
void DrawLightning();
void LightningEffect();
void FlashEffect();
void HitBlast();
/* void Logo(); */
void StartLogo();
void DrawLogo();
void LogoAway();
void RemoveLogoX();
void RemoveLogoY();
void DrawStormLogo(int,int);
void DrawStormLogoStatic(int,int,int);
void InitZoomer();
void DrawBox();
void Highscores();