#pragma once


void	DoOpenSoundFiles(void);
short	DoOpenSoundFile (short sndNameIndex);
void	PlayUp(void);
void	PlayDown(void);
void 	Click(short resID);
void	PlaySound(short soundResNum, short soundID);

#define	rSndName	130			// String resource for sounds file name
#define	sSndUp		1			// Index for this string
#define sSndDown	2

#define	kAsync		TRUE

#define	rUpSnd		128			// 'snd ' resource numbers
#define	rDownSnd	128


extern	short	gSoundFileUp;	// Sound file ResNum
extern	short	gSoundFileDown;

