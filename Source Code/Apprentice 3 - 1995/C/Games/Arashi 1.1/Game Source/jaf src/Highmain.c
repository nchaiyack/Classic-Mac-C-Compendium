/*
**	List Highscores			//Jaf � copyright 1991
*/

/* va paketin numerot - scrollaus jos >40. SCROLLAUS */
/* VA packet */
#include "VA.h"
#include "Palettes.h"
#include "Highscores.h"
#include "Flash.h"
#include "PlayOptions.h"

Initials	*HiScores;
extern int 	SpecialEvent;
extern int  FireworksActive;

HighMain() 
{
	Handle	ScoreHand;
		
	/* read scores from resources 
	ScoreHand=GetResource('SCOR',128); */
	if (PlayOptions->restart == 0)
		ScoreHand=GetResource('SCOR',128);
	else 
		ScoreHand=GetResource('SCOR',129);
	HLock(ScoreHand);
	HiScores=(Initials *)*ScoreHand;
	
	DisplayScores();
	SpecialEvent=DELAY;
	FireworksActive=0;					/*    */
}		

DisplayScores()
{
	char	Pstring[255];
	int		i,x,y,len;
	char	*str;
	Handle	TextHand;
	char	*Text;

	if (PlayOptions->restart == 0)
		TextHand=GetResource('TEXT',1001);
	else 
		TextHand=GetResource('TEXT',1011);
	
	HLock(TextHand);

	SetPort(VA.window);
	PmForeColor(BG1);
	PmBackColor(BGC);
	
	Text=(char *)*TextHand;
	len=GetHandleSize(TextHand);
	VA.segmscale=Getfontscale()+1;
	y = VA.frame.bottom/6;
	x = VA.frame.right/2 - (len*(VA.segmscale*3+3))/2;

	str = Text;
	for(i=0;i<len;i++) {
		if(*(str++) == 13) {
			if(Text<str-1) {
				VAMoveTo(x,y);
				VA.color= BG2;
				VADrawText(Text,0,str-Text-1);				
				VAMoveTo(x+1,y);
				VA.color= BG2;
				VADrawText(Text,0,str-Text-1);				
			}
			y+=VA.segmscale * 10;
			Text=str;
		}
	}

	for(i=1;i<11;i++) {
		y = VA.frame.bottom/4 + Getfontscale() * 10 * i;
		x = VA.frame.right/2;
		VA.segmscale=Getfontscale();
		VA.color=BG2;
		VAMoveTo(x,y);
		VADrawText(HiScores[i].name,0,5);
		VA.segmscale=Getfontscale()*2;
		VADrawNumber(HiScores[i].score,VA.frame.right/2.5,y);
	}
	return;
}

Firework() {
		VAExplosion(((unsigned int)Random()) %  VA.frame.right,
			((unsigned int)Random()) % VA.frame.bottom,2,
			((unsigned int)Random()) % 7);
	return 1;
}
