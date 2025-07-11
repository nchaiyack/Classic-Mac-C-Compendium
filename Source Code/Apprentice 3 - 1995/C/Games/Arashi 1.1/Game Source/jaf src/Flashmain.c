/*
**  Titleanimation && Highscorelists. Main program
**  Flashing effects, etc.		//Jaf � copyright 1991
**
*/


#include "Palettes.h"
#include "VA.h"
#include "STORM.h"
#include "Shuddup.h"
#include "Flash.h"
#include "Retrace.h"
#include "VAInternal.h"
#include "PlayOptions.h"
#include "GamePause.h"
#include "NewTitleEffect.h"

#include <GestaltEqu.h>

#define MAXINTENSITY 65535
#define NEWINTENSITY 32767
#define	DELAYTIME 110 
#define LONGDELAY 300

/* not checked if this is the max */
#define LIGHTNINGMALLOC 200

/* GLOBALS */

	
extern int Characters[];

int		MainStage;
int		SpecialEvent;

int		MaxScale;

long	intensity;
int		lightninghit;
long 	loop;
long 	Phaze;
int		BoxActive;
int		FireworksActive;

Rect	Zoom;

int		*px;
int		*py;
int		*ls;
int		*le;

int		maxsegments;
int		block,blockspeed;
int		discharge;

Handle			ColorHandle;
VAColorInfo		*Col;

extern	EventRecord		Event;

long	PlayGame(long HighScore,int Options);

void	DoCompleteGame()
{
	long			score;

	Phaze = 0;
	score = PlayGame(0,NormalPlay);
	if((score != -1)) 
	{	Highscores(score);
		MainStage = 16;
		SpecialEvent = DELAY;
		loop = -DELAYTIME;
		VAEraseBuffer();
		HighMain();
	}
	else
	{
		SpecialEvent = 0;
		MainStage = 0;
		loop = 0;
	}
}

void main() 
{

	long			endtime;
	int				Done;
	
	
	if(StormStart()){
		ResetGestalt(); /* remove gestalt selectors */
 		ReplaceASHI();
		return;
	}
	
	ThisLevel.lvNext = 1;
	STLoadLevel();
	
	InitScale();

	randSeed=Ticks;

	ColorHandle=GetResource('CLOT',1002);
	DetachResource(ColorHandle);
	HLock(ColorHandle);

	Col=(VAColorInfo *) *ColorHandle;
		(*Col)[8].red=65535;
		(*Col)[8].blue=0;
		(*Col)[8].green=0;
	VASetColors(ColorHandle);

	VA.FrameSpeed=3;
	Done = 0;
	SpecialEvent = 0;
	MainStage = 0;
	loop = 0;
	
	do {
		GameEvent();
		switch(Event.what) {
			case keyDown:
			case autoKey:
					switch(Event.message & 0xFF)
					{	case 'q':
						case 'Q':
						case 27:
						case 13: 	Done = 1;
									break;
						case 'p':
						case 'P':
									GamePause(TitlesRunning);
									break;
#define TEST_DEMO_GAMEx
#ifdef TEST_DEMO_GAME
						case ',':
									RecordDemoGame();
									break;
						case '.':
									PlayDemoGame();
									break;
#endif
						default:
									DoCompleteGame();
									break;
					}
					break;
			case mouseUp:
					DoCompleteGame();
					break;
		}
		MainTitle();
		VAStep();
	} while(!Done);
	
	VACatchUp();
	VACloseFractalLines();
	VAStep();
	CloseSoundKit();	
	VAClose();
	
 	ResetGestalt(); 
 	ReplaceASHI();
}
	
void MainTitle()
{
	switch(SpecialEvent) {
		case DELAY:
			loop=loop+5;
			if(loop>=DELAYTIME) {
				loop = 0;
				SpecialEvent=NOEVENT;
			}
			if(FireworksActive) {
				loop=loop-4;
		    	if(!(((unsigned int)Random()) % 9))
		    		Firework();
		    }
			if(BoxActive)
				DrawBox(); 
			break;
		case LIGHTNING:
			DrawLightning();
			break;
		case LOGOSTART:
			titlemain(ARASHIPICT, HORIZONTAL);  /*  */
			VASetColors(ColorHandle);
			SpecialEvent = NOEVENT;
			break;
		case ZOOMSTART:
			DrawZoomer(); 
			break;
			
		case NOEVENT:
		default:
			switch(MainStage) {
				case 0:
					BoxActive=0;
					FireworksActive=0;
					Erase(VA.frame.left,VA.frame.top,VA.frame.right,VA.frame.bottom);
					SpecialEvent = DELAY;
					break;		
				case 1:
					Lightning();
					break;
				case 2:
					HitBlast();
					titlemain(ARASHIPICT, HORIZONTAL);  /*  */
					VASetColors(ColorHandle);
					SpecialEvent = DELAY;
					break;
				case 4:
					Lightning();
					break;
				case 5:
					HitBlast();
					PlayDemoGame();
					VAEraseBuffer();
					VASetColors(GetResource('CLOT',1002));
					VAStep();
					break;					
				case 6:
					/* initial Rect */
					Zoom.left=(VA.frame.left+VA.frame.right/12);
					Zoom.right=(VA.frame.right-VA.frame.right/12);
					Zoom.top=(VA.frame.top+VA.frame.bottom/12);
					Zoom.bottom=(VA.frame.bottom-VA.frame.bottom/12);
					InitZoomer();
					break;
				case 7:
					Text(0,BG2);
					SpecialEvent = DELAY;
					break;
				case 8:
					Text(0,255);
					InitZoomer();
					break;
				case 9:
					Text(1,BG2);
					SpecialEvent = DELAY;
					break;
				case 10:
					Text(1,255);	
					InitZoomer();
					break;	
				case 11:
					Text(2,BG2);
					SpecialEvent = DELAY;
					break;	
				case 12:
					Text(2,255);
					InitZoomer();
					break;
				case 13:
					Text(3,BG2);
					SpecialEvent = DELAY;
					break;
				case 14:
					Text(3,255);
					InitZoomer();
					break;
				case 15:
					HighMain();
					SpecialEvent = DELAY;
					FireworksActive = 1;					
					break;
				case 16:
					BoxActive=0;
					FireworksActive=0;
					Erase(VA.frame.left,VA.frame.top,VA.frame.right,VA.frame.bottom);
					break;
			}
			MainStage++;
			if(MainStage==17)
				MainStage=0; 
			break;
	}
}

void InitZoomer() 
{
    SpecialEvent=ZOOMSTART;
 	BoxActive=1;
    DrawZoomer();					/* in RectZoom.c */
}	

/* gets the maximum scale to print the STORM logo */
InitScale()
{
	int scale;

	scale = 50;
	while((100+scale*41)>VA.frame.right || (100+scale*8)>VA.frame.bottom)
		scale--;
	MaxScale = scale;
}

int Getfontscale()
{
	return MaxScale/6;
}

void Erase(x1,y1,x2,y2)
int x1,y1,x2,y2;
{
	Rect eraser;
	eraser.top = y1;
	eraser.bottom = y2;
	eraser.left = x1;
	eraser.right = x2;
	PmBackColor(BGC);
	EraseRect(&eraser);
}

#define	RETURN_CHAR	13
void Text(number, color)
int	number;
int	color;
{
	int		x,y,i;
	int 	len;
	Handle	TextHand;
	char	*str,*Text;
	
	TextHand=GetResource('TEXT',128+number);
	HLock(TextHand);
	x = Zoom.left+MaxScale;
	y = Zoom.top+2*MaxScale;

	VA.segmscale = Getfontscale();
	VA.color=color;

	Text=(char *)*TextHand;
	len=GetHandleSize(TextHand);
	
	str = Text;
	for(i=0;i<len;i++) {
		if(*(str++) == RETURN_CHAR) {
			if(Text<str-1) {
				VAMoveTo(x,y);
				VADrawText(Text,0,str-Text-1);
				if(VA.segmscale>=2) {
					VAMoveTo(x+1,y);
					VADrawText(Text,0,str-Text-1);
				}
			}
			y+=VA.segmscale * 10;
			Text=str;
		}
	}
}

void ClearText() {
	Erase(VA.frame.left,VA.frame.top ,VA.frame.right,VA.frame.bottom);
}

void InitializeLightning()
{	
	int		segment,current;
	int		hitground,forkedlast,forkprob;
	int		i;

	px = (int *) NewPtr(sizeof(int) * LIGHTNINGMALLOC);
	py = (int *) NewPtr(sizeof(int) * LIGHTNINGMALLOC);
	ls = (int *) NewPtr(sizeof(int) * LIGHTNINGMALLOC);
	le = (int *) NewPtr(sizeof(int) * LIGHTNINGMALLOC);
	
	for(i=0;i<LIGHTNINGMALLOC;i++) {
		px[i]=0;py[i]=0;
		ls[i]=0;le[i]=0;
	}
	segment=1;
	current=0;
	px[0]=VA.frame.right/2;
	py[0]=-20;
	ls[0]=0;le[0]=0;
	hitground=0;forkedlast=1;
	forkprob=15;
	while(!hitground) {
		ls[segment]=current;
		le[segment]=segment;
		/* fork? */
		if(Random()%forkprob==0 && !forkedlast) {
			current--;
			forkedlast = 1;
			forkprob+=150;
		}
		else {
			forkedlast = 0;
			forkprob-=1;
			if(forkprob<5)
				forkprob=5;
		}
		if(Random()%2) {
			/* right-side */
			px[ls[segment]]=px[ls[current]]+2+((unsigned int)Random())%30;
			py[ls[segment]]=py[ls[current]]+20+((unsigned int)Random())%10;
			if(py[ls[segment]]>VA.frame.bottom)
				hitground=1;
		}
		else {
			/* left-side */
			px[ls[segment]]=px[ls[current]]-2-((unsigned int)Random())%30;
			py[ls[segment]]=py[ls[current]]+20+((unsigned int)Random())%10;
			if(py[ls[segment]]>VA.frame.bottom)
				hitground=1;
		}
		segment++;
		current++;
	}
	ls[segment]=0;
	le[segment]=0;
	maxsegments=segment;
}

void Lightning()
{			
	InitializeLightning();
	SpecialEvent = LIGHTNING;
	discharge = 0;
	lightninghit = 0;
	intensity = 0;
	block = 10;
	blockspeed = maxsegments/8+1;
	DrawLightning();
}

void DrawLightning()
{
	if(intensity || !lightninghit) {
		if(!lightninghit) {
			block+=blockspeed;
			}
		else {
			if(discharge<maxsegments) {
				discharge+=blockspeed;
			}
		}
		LightningEffect();
		FlashEffect();
	}
	else 
		SpecialEvent = 0;
}

void LightningEffect()
{
	int i;

	VA.color=2;
	
	for(i=discharge;i<block;i++) {
		if (i<maxsegments && px[le[i]] !=0 && px[ls[i]] !=10 && py[le[i]]!=0) {
			VAMoveTo(px[ls[i]],py[ls[i]]);
			VASafeLineTo(px[le[i]],py[le[i]]);
			}
		else {
			if(!lightninghit) 
				intensity+=MAXINTENSITY;
			lightninghit=1;
			}
		}

	for(i=discharge;i<block;i++) {
		if(px[ls[i]]!=0) {
			px[ls[i]]=px[ls[i]]+Random()%4;
			py[ls[i]]=py[ls[i]]+Random()%4;
		}
	}
}
	
void FlashEffect()
{
	if(intensity > MAXINTENSITY)
		intensity=MAXINTENSITY;

	(*Col)[9].red=intensity/2;
	(*Col)[9].blue=intensity;
	(*Col)[9].green=intensity/2;
		
	intensity= (intensity * 2)/3;

	VASetColors(ColorHandle);
}

void HitBlast()
{
	PlayA(Blast,999);
	if(!PlayOptions->noLoudSounds)
		PlayB(Blast,999);

	DisposPtr(px);
	DisposPtr(py);
	DisposPtr(ls);
	DisposPtr(le);
}

void DrawBox()
{
	VA.color=2;
	VALine(Zoom.left,Zoom.top,Zoom.right,Zoom.top);
	VALine(Zoom.left,Zoom.top,Zoom.left,Zoom.bottom);
	VALine(Zoom.right,Zoom.bottom,Zoom.right,Zoom.top);
	VALine(Zoom.right,Zoom.bottom,Zoom.left,Zoom.bottom);
}
