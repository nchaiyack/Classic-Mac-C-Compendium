/*/
     Project Arashi: STInterface.c
     Major release: Version 1.1, 7/22/92

     Last modification: Tuesday, March 2, 1993, 19:05
     Created: Tuesday, January 8, 1991, 8:38

     Copyright � 1991-1993, Juri Munkki
/*/

#include "VA.h"
#include "STORM.h"
#include "PlayOptions.h"

/*
>>	Called at program startup to allocate storage.
*/
void	STAllocate()
{
	AllocParser();		/*	Allocate & init memory for game compiler.		*/
	AllocCracks();
	AllocGrids();		/*	Allocate memory for grid structures.			*/
	AllocCStars();		/*	Allocate memory for center star structures.		*/
	AllocFlippers();	/*	Allocate memory for flippers.					*/
	AllocPulsars();		/*	Allocate memory for pulsars.					*/
	AllocSpikers();		/*	Allocate memory for spikers & spikes & -shots.	*/
	AllocFuseBalls();	/*	Allocate memory for fuseballs.					*/
	AllocTankers();		/*	Allocate memory for tankers.					*/
	AllocFloatingScores(); /* Allocate memory for floating scores (mz)		*/
}
/*
>>	Called at level startup to initialize variables.
*/
void	STInitialize()
{
	InitCStars();
	InitPlayer();
	InitSpikes();
	InitShots();
	InitPlayer();
	InitFlippers();
	InitPulsars();
	InitFuseBalls();
	InitTankers();
	InitCracks();
	InitFloatingScores();	/* mz */
}
/*
>>	Called each time through event/animation loop.
*/
void	STUpdate()
{
	UpdateCStars();

	ThisLevel.edgeCount=0;	/*	Some routines below must update this variable!	*/
	ThisLevel.starCount=0;	/*	Same as above, if you need baby stars.			*/

	UpdatePlayer();
	UpdateFuseBalls(); /* */
	UpdatePulsars();
	UpdateFlippers(); /* */
	UpdateShots();
	UpdateSpikes();
	UpdateTankers();
	UpdateCracks();
	if(PlayOptions->showfscores)
		UpdateFloatingScores();	/* mz */
}