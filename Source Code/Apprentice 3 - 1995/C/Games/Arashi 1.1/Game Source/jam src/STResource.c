/*/
     Project Arashi: STResource.c
     Major release: Version 1.1, 7/22/92

     Last modification: Monday, March 15, 1993, 21:33
     Created: Sunday, March 3, 1991, 7:43

     Copyright � 1991-1993, Juri Munkki
/*/


#include "STORM.h"
#include "EditorTypes.h"
#include "Parser.h"
#include "InternalVars.h"
#include <VA.h>
#include "heroflags.h"

#define RANDOMLEVELS 224		/* start of random levels */
#define STARTGREEN 208

extern	Player	Hero;

void	STOldLoadLevel()
{
	register	int					i;
	register	EditorLevelType		*edp,**edh;
				int					randGreenLevel;
	static		int					highLvNum;
	
	/*	ThisLevel.lvType = 2;	*/
	
	/* Check to see if last green level completed. If so go random. (mz) 		*/
	/* The lvNext in LEVL 223 (level 96) points to a non-existant LEVL 224. 	*/
	/* Flag for that, and go into random levels.								*/
	
	if (ThisLevel.lvNext == RANDOMLEVELS)
	{
		ThisLevel.lvType = 4;
		highLvNum = 97;
	}
		
	/* Only cases 0,3,4 are currently used.  The other cases are for other 		*/
	/* possible level RSRC's which were never implemented.						*/
	
	switch(ThisLevel.lvType)
	{	case 0:
			ThisLevel= **(LevelInfo **)GetResource('LEVL',ThisLevel.lvNext);
			break;
		case 1:
			ThisLevel.lvField=		ThisLevel.lvNumber % 16 + 100;
			ThisLevel.lvNumber++;
			ThisLevel.lvColor=		1000;
			ThisLevel.lvBonus=		0;
			ThisLevel.lvNext=		0;
	
			ThisLevel.plMaxMove=	12;
			ThisLevel.plSuperZaps=	1;
			
			ThisLevel.shSpeed=		4;
			ThisLevel.shPower=		2;
			ThisLevel.shColor[0]=	1;
			ThisLevel.shColor[1]=	0;
			ThisLevel.shColor[2]=	6;
			ThisLevel.shColor[3]=	4;
			ThisLevel.shColor[4]=	5;
			
			ThisLevel.flPoints=		10;
			ThisLevel.flCount=		3;
			ThisLevel.flProb=		700;
			ThisLevel.flSpeed=		65536L;
			ThisLevel.flRot=		4;
			ThisLevel.flColor=		2;
	
			for(i=0;i<=TANKTYPES;i++)
			{	ThisLevel.tk[i].points=	25;
				ThisLevel.tk[i].count=	2;
				ThisLevel.tk[i].prob=	600;
				ThisLevel.tk[i].speed=	65536L;
				ThisLevel.tk[i].color=	4+i;
			}
			
			ThisLevel.fuPoints=		25;
			ThisLevel.fuBullseye=	100;
			ThisLevel.fuCount=		1;
			ThisLevel.fuProb=		600;
			ThisLevel.fuWarpP=		2;
			ThisLevel.fuPlayerPlus=	40;
			ThisLevel.fuColor[0]=	0;
			ThisLevel.fuColor[1]=	5;
			ThisLevel.fuColor[2]=	1;
			ThisLevel.fuColor[3]=	4;
			ThisLevel.fuColor[4]=	2;
			
			ThisLevel.puPoints=		50;
			ThisLevel.puCount=		3;
			ThisLevel.puProb=		600;
			ThisLevel.puSpeed=		32768L;
			ThisLevel.puRot=		5;
			ThisLevel.puColor=		5;
			ThisLevel.puTime=		20;
			ThisLevel.puPulsDepth=	DEPTH-DEPTH/5;
			
			ThisLevel.spPoints=		50;
			ThisLevel.spProb=		100;
			ThisLevel.spSpeed=		65536L;
			ThisLevel.spStart=		DEPTH;
			ThisLevel.spTop=		10;
			ThisLevel.spPlasma=		1;
			ThisLevel.spPlSpeed=	5L*65536L;
			ThisLevel.spPlPoints=	200;
			ThisLevel.spSpikePoints=1;
			ThisLevel.spColor[0]=	3;
			ThisLevel.spColor[1]=	0;
			ThisLevel.spColor[2]=	5;
			
			ThisLevel.boredomCount=	4;
			ThisLevel.boredProb=	2000;
			ThisLevel.endTimer=		60;
			
			break;
		case 2:
			edh = (void *) GetIndResource('LEVE',1);
			HLock(edh);
			edp = *edh;
			
			ThisLevel.lvField=		edp->fieldId;
			ThisLevel.lvNumber++;
			ThisLevel.lvColor=		1000 ;//edp->fieldColor;
			ThisLevel.lvBonus=		edp->levelBonus;
			ThisLevel.lvNext=		0;
	
			ThisLevel.plMaxMove=	edp->playerSpeed;
			ThisLevel.plSuperZaps=	edp->playerSuperZapperCount;
			
			ThisLevel.shSpeed=		edp->playerShotSpeed / 10;
			ThisLevel.shPower=		edp->spikeDurability;
			for(i=0;i<6;i++)
			{	ThisLevel.shColor[i] = edp->playerShotColors[i];
			}
			
			ThisLevel.flPoints=		edp->flipperPoints;
			ThisLevel.flCount=		edp->flipperCount;
			ThisLevel.flProb=		edp->flipperFreq * 65536L / 1000;
			ThisLevel.flSpeed=		edp->flipperSpeed * 65536L / 10;
			ThisLevel.flRot=		edp->flipperRotSpeed;
			ThisLevel.flColor=		edp->flipperColor;

			for(i=0;i<=TANKTYPES;i++)
			{	ThisLevel.tk[i].points=	edp->tanks[i].points;
				ThisLevel.tk[i].count=	edp->tanks[i].count;
				ThisLevel.tk[i].prob=	edp->tanks[i].freq * 65536L / 1000;
				ThisLevel.tk[i].speed=	edp->tanks[i].speed * 65536L / 10;
				ThisLevel.tk[i].color=	edp->tanks[i].color;
			}
			
			ThisLevel.fuPoints=		edp->fusePoints;
			ThisLevel.fuBullseye=	edp->fuseBullsEyePoints;
			ThisLevel.fuCount=		edp->fuseCount;
			ThisLevel.fuProb=		edp->fuseFreq * 65536L / 1000;
			ThisLevel.fuWarpP=		edp->fuseWarpProbability * 65536L / 1000;
			ThisLevel.fuPlayerPlus=	40;
			for(i=0;i<5;i++)
			{
				ThisLevel.fuColor[0]=	edp->fuseColors[i];
			}
			
			ThisLevel.puPoints=		edp->pulsarPoints;
			ThisLevel.puCount=		edp->pulsarCount;
			ThisLevel.puProb=		edp->pulsarFreq * 65536L / 1000;
			ThisLevel.puSpeed=		edp->pulsarSpeed * 65536L / 10;
			ThisLevel.puRot=		edp->pulsarRotSpeed;
			ThisLevel.puColor=		edp->pulsarColor;
			ThisLevel.puTime=		edp->pulsarPulseTime;
			ThisLevel.puPulsDepth=	DEPTH-DEPTH/5;
			
			ThisLevel.spPoints=		edp->spikerPoints;
			ThisLevel.spProb=		edp->spikerFreq * 65536L / 1000;
			ThisLevel.spSpeed=		edp->spikerUpSpeed * 65536L / 10;
			ThisLevel.spStart=		edp->spikeInitialLength/10;
			ThisLevel.spTop=		edp->spikeMaxLength/10;
			ThisLevel.spPlasma=		edp->spikerPlasmaFlags;
			ThisLevel.spPlSpeed=	edp->spikerPlasmaSpeed;
			ThisLevel.spPlPoints=	edp->spikerPlasmaPoints;
			ThisLevel.spSpikePoints=edp->spikerPoints;
			ThisLevel.spColor[0]=	edp->spikerPlasmaColors[0];
			ThisLevel.spColor[1]=	edp->spikerPlasmaColors[1];
			ThisLevel.spColor[2]=	edp->spikerPlasmaColors[2];
			
			ThisLevel.boredomCount=	4;
			ThisLevel.boredProb=	2000;
			ThisLevel.endTimer=		60;
			
			HUnlock(edh);
			ReleaseResource(edh);
			break;
		case 3: /* case for death on doomsday levels, dont increment */
		case 4: 
			randGreenLevel = STARTGREEN + ((unsigned int)(Random()) % 16);
			ThisLevel= **(LevelInfo **)GetResource('LEVL',randGreenLevel);
			ThisLevel.lvNumber = highLvNum;
			if (ThisLevel.lvType == 4)
				highLvNum++;
			ThisLevel.lvType = 4;
			break;
			
	}
	
	ThisLevel.totalCount=
			ThisLevel.flCount+
			ThisLevel.fuCount+
			ThisLevel.puCount;

	for(i=0;i<TANKTYPES;i++)
	{	ThisLevel.totalCount+=ThisLevel.tk[i].count;
	}
}

void	STLoadLevel()
{
	short	i;
	int		doomzaps = ThisLevel.plSuperZaps;

	LoadLevel(ThisLevel.lvNext);
	WriteVariable(kConstDEPTH, DEPTH);

	ThisLevel.lvNumber = ThisLevel.lvNext;
	ThisLevel.lvField= EvalVariable(kVarLevelField);
	ThisLevel.lvColor= EvalVariable(kVarLevelColor);
	ThisLevel.lvBonus= EvalVariable(kVarLevelBonus);
	ThisLevel.lvStBonus = EvalVariable(kVarLevelSTBonus);
	ThisLevel.lvNext= EvalVariable(kVarLevelNext);

	ThisLevel.plMaxMove= EvalVariable(kVarPlayerMaxMove);
	ThisLevel.plSuperZaps= EvalVariable(kVarPlayerSuperZaps);
	
	ThisLevel.shSpeed= EvalVariable(kVarShotSpeed);
	ThisLevel.shPower= EvalVariable(kVarShotPower);
	ThisLevel.shColor[0]= EvalVariable(kVarShotColor_0_);
	ThisLevel.shColor[1]= EvalVariable(kVarShotColor_1_);
	ThisLevel.shColor[2]= EvalVariable(kVarShotColor_2_);
	ThisLevel.shColor[3]= EvalVariable(kVarShotColor_3_);
	ThisLevel.shColor[4]= EvalVariable(kVarShotColor_4_);
	
	ThisLevel.flPoints= EvalVariable(kVarFlipperPoints);
	ThisLevel.flCount= EvalVariable(kVarFlipperCount);
	ThisLevel.flProb= EvalVariable(kVarFlipperProb);
	ThisLevel.flSpeed= EvalVariable(kVarFlipperSpeed);
	ThisLevel.flRot= EvalVariable(kVarFlipperRot);
	ThisLevel.flColor= EvalVariable(kVarFlipperColor);

	ThisLevel.tk[0].points= EvalVariable(kVarTank_0_points);
	ThisLevel.tk[0].count= EvalVariable(kVarTank_0_count);
	ThisLevel.tk[0].prob= EvalVariable(kVarTank_0_prob);
	ThisLevel.tk[0].speed= EvalVariable(kVarTank_0_speed);
	ThisLevel.tk[0].color= EvalVariable(kVarTank_0_color);

	ThisLevel.tk[1].points= EvalVariable(kVarTank_1_points);
	ThisLevel.tk[1].count= EvalVariable(kVarTank_1_count);
	ThisLevel.tk[1].prob= EvalVariable(kVarTank_1_prob);
	ThisLevel.tk[1].speed= EvalVariable(kVarTank_1_speed);
	ThisLevel.tk[1].color= EvalVariable(kVarTank_1_color);

	ThisLevel.tk[2].points= EvalVariable(kVarTank_2_points);
	ThisLevel.tk[2].count= EvalVariable(kVarTank_2_count);
	ThisLevel.tk[2].prob= EvalVariable(kVarTank_2_prob);
	ThisLevel.tk[2].speed= EvalVariable(kVarTank_2_speed);
	ThisLevel.tk[2].color= EvalVariable(kVarTank_2_color);

	
	ThisLevel.fuPoints= EvalVariable(kVarFusePoints);
	ThisLevel.fuBullseye= EvalVariable(kVarFuseBullseye);
	ThisLevel.fuCount= EvalVariable(kVarFuseCount);
	ThisLevel.fuProb= EvalVariable(kVarFuseProb);
	ThisLevel.fuWarpP= EvalVariable(kVarFuseWarpP);
	ThisLevel.fuPlayerPlus= EvalVariable(kVarFusePlayerPlus);
	ThisLevel.fuColor[0]= EvalVariable(kVarFuseColor_0_);
	ThisLevel.fuColor[1]= EvalVariable(kVarFuseColor_1_);
	ThisLevel.fuColor[2]= EvalVariable(kVarFuseColor_2_);
	ThisLevel.fuColor[3]= EvalVariable(kVarFuseColor_3_);
	ThisLevel.fuColor[4]= EvalVariable(kVarFuseColor_4_);
	
	ThisLevel.puPoints= EvalVariable(kVarPulsarPoints);
	ThisLevel.puCount= EvalVariable(kVarPulsarCount);
	ThisLevel.puProb= EvalVariable(kVarPulsarProb);
	ThisLevel.puSpeed= EvalVariable(kVarPulsarSpeed);
	ThisLevel.puRot= EvalVariable(kVarPulsarRot);
	ThisLevel.puColor= EvalVariable(kVarPulsarColor);
	ThisLevel.puTime= EvalVariable(kVarPulsarTime);
	ThisLevel.puPulsDepth= EvalVariable(kVarPulsarPulsDepth);
	
	ThisLevel.spPoints= EvalVariable(kVarSpikerPoints);
	ThisLevel.spProb= EvalVariable(kVarSpikerProb);
	ThisLevel.spSpeed= EvalVariable(kVarSpikerSpeed);
	ThisLevel.spStart= EvalVariable(kVarSpikerStart);
	ThisLevel.spTop= EvalVariable(kVarSpikerTop);
	ThisLevel.spPlasma= EvalVariable(kVarSpikerPlasma);
	ThisLevel.spPlSpeed= EvalVariable(kVarSpikerPlSpeed);
	ThisLevel.spPlPoints= EvalVariable(kVarSpikerPlPoints);
	ThisLevel.spSpikePoints= EvalVariable(kVarSpikerSpikePoints);
	ThisLevel.spColor[0]= EvalVariable(kVarSpikerColor_0_);
	ThisLevel.spColor[1]= EvalVariable(kVarSpikerColor_1_);
	ThisLevel.spColor[2]= EvalVariable(kVarSpikerColor_2_);
	
	ThisLevel.boredomCount= EvalVariable(kVarBoredomCount);
	ThisLevel.boredProb= EvalVariable(kVarBoredProb);
	ThisLevel.endTimer= EvalVariable(kVarEndTimer);
	
	ThisLevel.ShowSpikesMsg = EvalVariable(kVarShowSpikesMsg);
	ThisLevel.Doomsday = EvalVariable(kVarDoomsday);
	if(ThisLevel.Doomsday == 1){
		ThisLevel.lvField =  (( (unsigned)(VARandom()) ) % 16)+ 1; 
		if(Hero.Flags & SavedGameStartMask){
			Hero.Flags &= (FLAGSMASK - SavedGameStartMask);
			ThisLevel.plSuperZaps = doomzaps;
		}
	} 
	

	ThisLevel.totalCount=
			ThisLevel.flCount+
			ThisLevel.fuCount+
			ThisLevel.puCount;

	for(i=0;i<TANKTYPES;i++)
	{	ThisLevel.totalCount+=ThisLevel.tk[i].count;
	}

}

short	STLoadForSelect(int restartMode)
{
	LoadLevel(ThisLevel.lvNext);
	WriteVariable(kConstDEPTH, DEPTH);

	ThisLevel.lvNumber = ThisLevel.lvNext;
	ThisLevel.lvField= EvalVariable(kVarLevelField);
	ThisLevel.lvColor= EvalVariable(kVarLevelColor);
	ThisLevel.lvBonus= EvalVariable(kVarLevelBonus);
	ThisLevel.lvStBonus = EvalVariable(kVarLevelSTBonus);
	ThisLevel.lvNext= EvalVariable(kVarLevelNext);
	
	if(restartMode)
		return EvalVariable(kVarAllowPracticeRestart);
	else
		return EvalVariable(kVarAllowArcadeRestart);
}

void	DumpThisLevel()
{
	Handle	saved;
	
	saved = GetResource('LEVL', 999);
	SetHandleSize(saved, sizeof(ThisLevel));
	BlockMove(&ThisLevel, *saved, sizeof(ThisLevel));
	ChangedResource(saved);
	WriteResource(saved);
}