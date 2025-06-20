/********************************************************************************
         Copyright  �  N. Jonas Englund, 1993.    All Rights Reserved.
         
         WARNING: This file has been modified by Leo Breebaart. Don't hold
         Jonas responsible for my hacks. If you want his original code,
         e-mail me and I'll send you a copy.
 ********************************************************************************

 	PROJECT:	clut_fade.�
 	
 	FILE:		fade.c
 	
 	PURPOSE:	'clut' fading functions

 ********************************************************************************/

//================================= INCLUDES ====================================

#include "fade.h"

//================================= FUNCTIONS ===================================

void copy_cluts	 (GDHandle);
void calc_fade	 (short, FadeValues*);
void fade_out	 (short, FadeValues);
void fade_in	 (short, FadeValues);
void black_out	 (void);

//================================== GLOBALS ====================================

CTabHandle 	gFade,			// 'clut' to alter with fading functions
			gOrig;			// 'clut' to hold a copy of original
Handle		gTempH;			//  temporary handle to copy 'clut'

/********************************** fade_screen *********************************/
extern
void fade_screen(short speed, Boolean fadeOut)
{
	GDHandle 	oldDev, hGD;
	FadeValues	rgbs;
	
	oldDev = GetGDevice();
	hGD = GetMainDevice();
	SetGDevice(hGD);
	
	copy_cluts(hGD);
	calc_fade(speed, &rgbs);
 
	if (fadeOut)
		fade_out(speed, rgbs);
	else
		fade_in(speed, rgbs);

	restore_clut();
	SetGDevice(oldDev);

	DisposeHandle(gTempH);
}
/*** fade_screen ***/

extern
void setup_cluts()
{
	GDHandle 	oldDev, hGD;
	FadeValues	rgbs;
	
	oldDev = GetGDevice();
	hGD = GetMainDevice();
	SetGDevice(hGD);
	
	copy_cluts(hGD);
	SetGDevice(oldDev);
}

/********************************** copy_cluts **********************************/
static
void copy_cluts(GDHandle hGD)
{	
	gFade = (*(*hGD)->gdPMap)->pmTable;
	gTempH = (Handle) (*(*hGD)->gdPMap)->pmTable;
	HandToHand(&gTempH);
	gOrig = (CTabHandle) gTempH;
	
	HLock((Handle) gFade);
	HLock((Handle) gOrig);
}

/*********************************** calc_fade **********************************/
static
void calc_fade(short speed, FadeValues *rgbs)
{
	short	i;
	
	for (i = 0; i < MAXBYTE; i++)
	{
		rgbs->reds[i]   = (*gFade)->ctTable[i].rgb.red   / speed;
		rgbs->greens[i] = (*gFade)->ctTable[i].rgb.green / speed;
		rgbs->blues[i]  = (*gFade)->ctTable[i].rgb.blue  / speed;
	}
}

/*********************************** fade_out ***********************************/
static
void fade_out(short speed, FadeValues rgbs)
{
	short 	i, j;
	
	for (j = speed; j >= 0; j--)
	{
		for (i = 0; i < MAXBYTE; i++)
		{
			if ((*gFade)->ctTable[i].rgb.red   > rgbs.reds[i])
				(*gFade)->ctTable[i].rgb.red   -= rgbs.reds[i];
			if ((*gFade)->ctTable[i].rgb.green > rgbs.greens[i])
				(*gFade)->ctTable[i].rgb.green -= rgbs.greens[i];
			if ((*gFade)->ctTable[i].rgb.blue  > rgbs.blues[i])
				(*gFade)->ctTable[i].rgb.blue  -= rgbs.blues[i];
		}
		SetEntries(0,  MAXBYTE-1, (*gFade)->ctTable);
	}
	black_out();
}

/************************************ fade_in ***********************************/
static
void fade_in(short speed, FadeValues rgbs)
{
	short 	i, j;
	
	black_out();

	for (j = 0; j < speed; j++)
	{
		for (i = 0; i < MAXBYTE; i++)
		{
			if ((*gFade)->ctTable[i].rgb.red   < (*gOrig)->ctTable[i].rgb.red)
				(*gFade)->ctTable[i].rgb.red   += rgbs.reds[i];
			if ((*gFade)->ctTable[i].rgb.green < (*gOrig)->ctTable[i].rgb.green)
				(*gFade)->ctTable[i].rgb.green += rgbs.greens[i];
			if ((*gFade)->ctTable[i].rgb.blue  < (*gOrig)->ctTable[i].rgb.blue)
				(*gFade)->ctTable[i].rgb.blue  += rgbs.blues[i];
		}
		SetEntries(0, MAXBYTE-1, (*gFade)->ctTable);
	}
}

/*********************************** black_out **********************************/
static
void black_out(void)
{
	short 	i;
	
	for (i = 0; i < MAXBYTE; i++)
	{
		(*gFade)->ctTable[i].rgb.red   = 0;
		(*gFade)->ctTable[i].rgb.green = 0;
		(*gFade)->ctTable[i].rgb.blue  = 0;
	}
	SetEntries(0, MAXBYTE-1, (*gFade)->ctTable);
}

/********************************** restore_clut ********************************/
void restore_clut(void)
{
	short 	i;
	
	for (i = 0; i < MAXBYTE; i++)
	{
		(*gFade)->ctTable[i].rgb.red   = (*gOrig)->ctTable[i].rgb.red;
		(*gFade)->ctTable[i].rgb.green = (*gOrig)->ctTable[i].rgb.green;
		(*gFade)->ctTable[i].rgb.blue  = (*gOrig)->ctTable[i].rgb.blue;
	}
	(*gFade)->ctSeed = GetCTSeed();
	MakeITable(nil, nil, 0);
	
	HUnlock((Handle) gFade);
	HUnlock((Handle) gOrig);
}


void install_clut (short clutID)
{
	short i;
	CTabHandle newClut;

	GDHandle 	oldDev, hGD;
	
	oldDev = GetGDevice();
	hGD = GetMainDevice();
	SetGDevice(hGD);
	
	gFade = (*(*hGD)->gdPMap)->pmTable;

	if ((newClut = GetCTable(clutID)) == nil)
	{
		SysBeep(2);
		return;
	}
	
	HLock((Handle) gFade);
	HLock((Handle) newClut);
	
	for (i = 0; i < MAXBYTE; i++)
	{
		(*gFade)->ctTable[i].rgb.red   = (*newClut)->ctTable[i].rgb.red;
		(*gFade)->ctTable[i].rgb.green = (*newClut)->ctTable[i].rgb.green;
		(*gFade)->ctTable[i].rgb.blue  = (*newClut)->ctTable[i].rgb.blue;
	}
	
	SetEntries(0, MAXBYTE-1, (*gFade)->ctTable);
	
	HUnlock((Handle) gFade);
	HUnlock((Handle) newClut);

	SetGDevice(oldDev);
}
