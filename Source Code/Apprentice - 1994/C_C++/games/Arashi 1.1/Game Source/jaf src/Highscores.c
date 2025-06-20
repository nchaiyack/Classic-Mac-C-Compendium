/*
**	Game over, Highscore handling		//Jaf � copyright 1991
*/

#include "Palettes.h"
#include "VA.h"
#include "STORM.h"
#include "Highscores.h"
#include "NewTitleEffect.h"
#include "PlayOptions.h"

#define GAMEOVERDELAY 60

extern  int				MainStage;
extern  int				SpecialEvent;
extern  Initials		*HiScores;

Highscores(score)
long	score;
{
	int	i,ranking,ranky;
	Handle	ScoreHand;

	VASetColors(GetResource('CLOT',1002));
		
	
	if (PlayOptions->restart == 0)
		ScoreHand=GetResource('SCOR',128);
	else 
		ScoreHand=GetResource('SCOR',129);			/* Practice game scores */
	HLock(ScoreHand);
	HiScores=(Initials *)*ScoreHand;
	ranking = Ranking(score);
	VAStep();
	PmBackColor(BGC);
	EraseRect(&VA.frame);
	
	/* GameOver(); */
	titlemain(GAMEOVERPICT, VERTICAL);
	
	EraseRect(&VA.frame);
	
	if(ranking<11)
		WriteCongrats();
	else if(ranking<51)
		WriteCongrats2();
	else {
		WriteNoCongrats(score);
		VAEraseBuffer();
		return;
	}
	
	ranky = ScrollScores(ranking,score);
	GetInitials(ranking,ranky);
	SaveScores(ScoreHand);
}

int Ranking(score)
long	score;
{
	int	i,pos,j;
	
	pos=51;	
	/* get ranking */
	for(i=50;i>0;i--) {
		if (score>HiScores[i].score)
			pos = i;
	}
	
	if(pos==51)
		return pos;
		
	/* move lowerthan scores down one step*/
	for(i=50;i>pos;i--) {
		HiScores[i].score = HiScores[i-1].score;
		for(j=0;j<6;j++)
			HiScores[i].name[j] = HiScores[i-1].name[j];
	}
	
	HiScores[pos].score=score;
	for(j=0;j<6;j++)
		HiScores[pos].name[j] = ' ';
	
	return pos;
}

#define	OWNRANKINGCOLOR (2*9)

int ScrollScores(rank,score)
int		rank;
long	score;
{
	int i,x,y,del,sy,done,doneloop;
	long j;
	int NumScale,TextScale;
	int	returny;
	extern EventRecord	Event;

	FlushEvents(everyEvent,0);
	
	TextScale=Getfontscale();
	NumScale=Getfontscale()*2;

	del = 20;
	done=0;
	doneloop = -(rank-4);
	if(doneloop>0)
		doneloop=0;
	
	sy=VA.frame.bottom;
	
	do
	{
		VA.segmscale=NumScale;
		sy-=(VA.segmscale*4+4);
		if(sy<VA.frame.bottom/4) {
			doneloop++;
		}
		if(doneloop>0)
			done=1;
		if(sy<VA.frame.bottom/4+(VA.segmscale*4+4) && rank<4)
			done=1;
		PlayA(Bonk,1);
		del--;
		if (del<0)
			del = 0;

		Delay(del,&j);
		VACatchUp();
		VAStep();
		GameEvent();
		if(Event.what)
			del = 0;

		for(i=1;(i<=rank+2) && (i<51) ;i++) {   /* extra check on high score (mz) */
			y=sy+(i-1)*(VA.segmscale*4+4);
			if( y >= VA.frame.bottom/4 && y < (VA.frame.bottom-(VA.segmscale*4+4))) {				
				x = VA.frame.right/2;
				VA.segmscale=TextScale;
				VA.color=4*9;
				VADrawNumber(i,VA.frame.right/3,y-1);
				VAMoveTo(x+6*NumScale,y);
				if(i==rank) {
					VA.color=OWNRANKINGCOLOR;
					VADrawText(".....",0,5);
					VA.segmscale=NumScale;
					VADrawNumber(1000000,VA.frame.right/2,y);
					VADrawNumber(0,VA.frame.right/2-6*(VA.segmscale+7),y);
					VADrawNumber(score,VA.frame.right/2,y);
					returny=y;
				} 
				else {
					VA.color=BG2;
					VADrawText(HiScores[i].name,0,5);
					VA.segmscale=NumScale;
					VADrawNumber(1000000,VA.frame.right/2,y);
					VADrawNumber(0,VA.frame.right/2-6*(VA.segmscale+7),y);
					VADrawNumber(HiScores[i].score,VA.frame.right/2,y);
				}
				if((i==rank+2) || (i==50)) {  /* erase all 50's (mz) */
					Erase(VA.frame.right/4,y+1,VA.frame.right,y+VA.segmscale*4+5);
				}	
			}
		}
	} while (!done);
	
	return returny;	
}

void WriteCongrats()
{
	int		x,y,i;
	int 	len;
	Handle	TextHand;
	char	*str,*Text;
	
	if (PlayOptions->restart == 0)
	{
		TextHand=GetResource('TEXT',1000);
		HLock(TextHand);
		x = VA.frame.right/3.5;
		y = VA.frame.bottom/8;
	}
	else
	{
		TextHand=GetResource('TEXT',1010);
		HLock(TextHand);
		x = VA.frame.right/4;
		y = VA.frame.bottom/8;
	}
	

	VA.segmscale = Getfontscale();

	Text=(char *)*TextHand;
	len=GetHandleSize(TextHand);
	
	str = Text;
	for(i=0;i<len;i++) {
		if(*(str++) == 13) {
			if(Text<str-1) {
				VA.color=BG2;
				VAMoveTo(x,y);
				VADrawText(Text,0,str-Text-1);
				VA.color=4*9;
				VAMoveTo(x+1,y);
				VADrawText(Text,0,str-Text-1);
			}
			y+=VA.segmscale * 10;
			Text=str;
		}
	}
}
void WriteCongrats2()
{
	int		x,y,i;
	int 	len;
	Handle	TextHand;
	char	*str,*Text;
	
	if (PlayOptions->restart == 0)
	{
		TextHand=GetResource('TEXT',132);
		HLock(TextHand);
		x = VA.frame.right/3.5;
		y = VA.frame.bottom/8;
	}
	else
	{
		TextHand=GetResource('TEXT',136);
		HLock(TextHand);
		x = VA.frame.right/4;
		y = VA.frame.bottom/8;
	}
	

	VA.segmscale = Getfontscale();

	Text=(char *)*TextHand;
	len=GetHandleSize(TextHand);
	
	str = Text;
	for(i=0;i<len;i++) {
		if(*(str++) == 13) {
			if(Text<str-1) {
				VA.color=BG2;
				VAMoveTo(x,y);
				VADrawText(Text,0,str-Text-1);
				VA.color=4*9;
				VAMoveTo(x+1,y);
				VADrawText(Text,0,str-Text-1);
			}
			y+=VA.segmscale * 10;
			Text=str;
		}
	}
}
void WriteNoCongrats(score)
long	score;
{
	int		x,y,i;
	int 	len;
	Handle	TextHand;
	char	*str,*Text;
	
	TextHand=GetResource('TEXT',133);
	HLock(TextHand);
	x = VA.frame.right/4;
	y = VA.frame.bottom/4;

	VA.segmscale = Getfontscale();

	Text=(char *)*TextHand;
	len=GetHandleSize(TextHand);
	
	str = Text;
	for(i=0;i<len;i++) {
		if(*(str++) == 13) {
			if(Text<str-1) {
				VA.color=BG2;
				VAMoveTo(x,y);
				VADrawText(Text,0,str-Text-1);
				VA.color=4*9;
				VAMoveTo(x+1,y);
				VADrawText(Text,0,str-Text-1);
			}
			y+=VA.segmscale * 10;
			Text=str;
		}
	}
	VA.segmscale=Getfontscale();
	x=VA.frame.right/2;
	y=VA.frame.bottom/2;
	VAMoveTo(VA.frame.right/4,y);
	VA.color=2*9;
	VADrawText("score:",0,5);
	VA.segmscale=Getfontscale()*2;
	VADrawNumber(1000000,VA.frame.right/2,y);
	VADrawNumber(0,VA.frame.right/2-6*(VA.segmscale+7),y);
	VADrawNumber(score,VA.frame.right/2,y);

	for(i=0;i<GAMEOVERDELAY;i++) {
		if(!(((unsigned int)Random()) % 5))
			Firework();
		GameEvent();
		if((1<<Event.what) & (mUpMask+keyDownMask+keyUpMask))
			i=GAMEOVERDELAY;
		VAStep();
	}		
}


void GameOver()
{
	int		x,y,i;
	int 	len;
	Handle	TextHand;
	char	*str,*Text;
	
	TextHand=GetResource('TEXT',134);
	HLock(TextHand);
	x = VA.frame.right/6;
	y = VA.frame.bottom/2;

	VA.segmscale = Getfontscale()*6;

	Text=(char *)*TextHand;
	len=GetHandleSize(TextHand);
	
	str = Text;
	for(i=0;i<len;i++) {
		if(*(str++) == 13) {
			if(Text<str-1) {
				VA.color=BG2;
				VAMoveTo(x,y);
				VADrawText(Text,0,str-Text-1);
				VA.color=BG2;
				VAMoveTo(x+1,y);
				VADrawText(Text,0,str-Text-1);
				VA.color=BG2;
				VAMoveTo(x+2,y);
				VADrawText(Text,0,str-Text-1);
			}
			y+=VA.segmscale * 10;
			Text=str;
		}
	}
	for(i=0;i<GAMEOVERDELAY;i++) {
		if(!(((unsigned int)Random()) % 5))
			Firework();
		GameEvent();
		if((1<<Event.what) & (mUpMask+keyDownMask+keyUpMask))
			i=GAMEOVERDELAY;
		VAStep();
	}		
}

void	GetInitials(rank,ranky)
int rank,ranky;
{
			long			bigrotho;
			int				rotho=0,oldrotho=1,ang;
			int				x,y,xs,ys,i;
			int				vang,vx,vy,vxs,vys;
			char			bar;
			Point			mouse,oldmouse,center;
			char			name[6],currentchar;
			int				index;
	extern 	EventRecord		Event;
			int				Done;
			int				rankx;
			int				TextScale,TextScale2;
			int				delta,history,olddirection;
			char			thechar;
	
	VA.FrameSpeed = 3;
	bigrotho = 0;
	history = 0;	/* No previous mouse rotation.	*/
	olddirection = 1;
	
	TextScale=Getfontscale();
	TextScale2=Getfontscale()*2;

	rankx= VA.frame.right/2+6*TextScale2;

	VA.segmscale=TextScale2;
	VA.offset=1;
	VA.field=1;
	VA.color=2;

	vx=VA.segmscale*3+3+12;
	vy=VA.segmscale*4+4+14;
	x=VA.frame.right/2-vx/2-4;
	y=VA.frame.bottom/5+(150*(long)(VA.frame.right))/((Sins[90]/2)+500)-VA.segmscale*4-5;
	
	VAStaticLine(x,y-7,x+vx,y-7);
	VAStaticLine(x,y-6,x,y-7);
	VAStaticLine(x+vx,y-6,x+vx,y-7);	

 	VAStaticLine(x,y+vy-7,x+vx,y+vy-7);
	VAStaticLine(x,y+vy-7,x,y+vy-8);
	VAStaticLine(x+vx,y+vy-8,x+vx,y+vy-7);
		
	center.h=(VA.DisplayFrame.right+VA.DisplayFrame.left)/2;
	center.v=(VA.DisplayFrame.bottom+VA.DisplayFrame.top)/2;
	
	mouse=center;
	oldmouse=mouse;

	Done=0;
	index=0;
	rotho=90;
	currentchar='@';
	
	do {
		vang = rotho;

		/* mouse events... */
		GameEvent();
		
		if(Event.what==mouseUp) {
			PlayA(Springy,1);
			HiScores[rank].name[index]=currentchar;
			VAMoveTo(rankx+(TextScale*3+3)*index,ranky);
			VA.segmscale=TextScale;
			VADrawText(" ",0,1);
			VAMoveTo(rankx+(TextScale*3+3)*index,ranky);
			VA.color=OWNRANKINGCOLOR;
			VADrawText(&currentchar,0,1);
			VA.color=BG2;
			VA.segmscale=TextScale2;
			index++;
			if(index==5)
				Done=1;
		}
		
		if(Event.what==keyDown || Event.what==autoKey) {
			thechar=(Event.message & 0xFF);
			if(thechar>127)
				thechar-=127;
			/* return */
			if(thechar==13)
				Done=1;
			/* delete (all controlchars) */
			else if(thechar<32) {
				if(index>0) {
					PlayB(Whiz,1);
					index--;
					HiScores[rank].name[index]=' ';
					VAMoveTo(rankx+(TextScale*3+3)*index,ranky);
					VA.segmscale=TextScale;
					VADrawText(" ",0,1);
					VAMoveTo(rankx+(TextScale*3+3)*index,ranky);
					VA.color=OWNRANKINGCOLOR;
					VADrawText(".",0,1);
					VA.color=BG2;
					VA.segmscale=TextScale2;
				}
			}
			/* other keys */
			else if(thechar>=32) {
				PlayA(Bonk,1);
				HiScores[rank].name[index]=thechar;
				VAMoveTo(rankx+(TextScale*3+3)*index,ranky);
				VA.segmscale=TextScale;
				VADrawText(" ",0,1);
				VAMoveTo(rankx+(TextScale*3+3)*index,ranky);
				VA.color=OWNRANKINGCOLOR;
				VADrawText(&thechar,0,1);
				VA.color=BG2;
				VA.segmscale=TextScale2;
				index++;
				if(index==5)
					Done=1;
			}
		}
			
		mouse=center;
		GetSetMouse(&mouse);
		
		delta=mouse.h+history;
		history = delta;

#define	MAXROTLETTER	20
		if (delta>MAXROTLETTER)
			delta = MAXROTLETTER;
		if (delta<-MAXROTLETTER)
			delta = -MAXROTLETTER;
		
		history -= delta;
		history = history * 3 / 4;

#define	SUBROTATION	(5)

		if(delta)
		{	if(delta > 0)	olddirection = SUBROTATION;
			else			olddirection = -SUBROTATION;
			bigrotho += delta;
		}
		else
		{	if(((rotho+ANGLES) % 4) != 1)
			{	bigrotho += olddirection;
			}
		}

		bigrotho %= SUBROTATION * ANGLES;
		rotho = bigrotho / SUBROTATION;
		
		if(rotho>=ANGLES) rotho-=ANGLES;
		if(rotho<0) rotho+=ANGLES;
		
		if(rotho != oldrotho)
		{	int		direction;
			int		rotstep;
			
			if(oldrotho < rotho )
			{	rotstep = -4;
				direction = -1;
			}
			else
			{	rotstep = 4;
				direction = 1;
			}
			
			oldrotho = rotho;
			ang= rotho;
			for(i=0;i<ANGLES;i+=4) {

				if(ang>=ANGLES) ang-=ANGLES;
				if(ang<0) ang+=ANGLES;

				if(vang>=ANGLES) vang-=ANGLES;
				if(vang<0) vang+=ANGLES;

				x=Cosins[ang]/2;
				y=(Sins[ang]/2)+500;
				vx=Cosins[vang]/2;
				vy=(Sins[vang]/2)+500;
	
				xs=VA.frame.right/2+(x*(long)(VA.frame.right))/y;
				ys=VA.frame.bottom/5+(150*(long)(VA.frame.right))/y;
				vxs=VA.frame.right/2+(vx*(long)(VA.frame.right))/vy;
				vys=VA.frame.bottom/5+(150*(long)(VA.frame.right))/vy;
				if(ang>87&&ang<92) {
					VA.color=2*9;
					currentchar=bar;
				}
				else
					VA.color=BG2;
	
				if(vy<500)			VA.segmscale=TextScale2+1;
				else				VA.segmscale=TextScale2;
	
				VAMoveTo(vxs,vys);
				VADrawText(" ",0,1);
	
				if(y<500) 			VA.segmscale=TextScale2+1;
				else				VA.segmscale=TextScale2;
	
				VAMoveTo(xs,ys);
				VADrawText(&bar,0,1);

				bar+=direction;
				if(bar<'@')	bar = ']';
				if(bar>']') bar = '@';
				
				ang+=rotstep;
				vang+=rotstep;
			}
		}
		VAStep();
	} while(!Done);
	
	VACatchUp();
}

void SaveScores(ScoreHandle)
Handle	ScoreHandle;
{
	ChangedResource(ScoreHandle);
	WriteResource(ScoreHandle);
	DetachResource(ScoreHandle);
}