#include <Sound.h>

#define NUM_SOUNDS		10

enum
{
	sound_keyclick = 1000,
	sound_playword_good,
	sound_badword,
	sound_wingame,
	sound_losegame,
	sound_virgin,
	sound_on,
	sound_aboutMSG,
	sound_fluff,
	sound_bluffing
};

extern	SndChannelPtr		myChannel;
extern	Handle				MySounds[NUM_SOUNDS];
extern	Boolean				gSoundToggle;
extern	Boolean				gSoundAvailable;
extern	Boolean				gSoundIsFinishedPlaying;

void InitTheSounds(void);
void DoSound(short whichSound, Boolean async);
void CloseTheSoundChannel(void);
