#ifndef __SOUND_LAYER_H__
#define __SOUND_LAYER_H__

#define NUM_SOUNDS		8

enum
{
	sound_on=1000,
	sound_error,
	sound_nimplay,
	sound_gridplay,
	sound_turnplay,
	sound_wingame,
	sound_losegame,
	sound_startgame
};

extern	Boolean				gSoundToggle;
extern	Boolean				gSoundAvailable;
extern	Boolean				gSoundIsFinishedPlaying;

#ifdef __cplusplus
extern "C" {
#endif

extern	void InitTheSounds(void);
extern	void DoSound(short whichSound, Boolean async);
extern	Boolean SoundChannelIsOpenQQ(void);
extern	void CloseTheSoundChannel(void);

#ifdef __cplusplus
}
#endif

#endif
