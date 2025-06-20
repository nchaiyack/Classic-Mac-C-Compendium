#ifndef __MY_MUSIC_LAYER_H__
#define __MY_MUSIC_LAYER_H__

enum MusicStatusTypes
{
	kMusicNever=0,
	kMusicGeneratingOnly,
	kMusicAlways
};

typedef short		MusicStatusTypes;

extern	MusicStatusTypes	gMusicStatus;
extern	Boolean				gMusicAvailable;

#ifdef __cplusplus
extern "C" {
#endif

extern	void InitTheMusic(void);
extern	void StartTheMusic(void);
extern	void CloseTheMusicChannel(void);
extern	Boolean MusicIsPlayingQQ(void);

#ifdef __cplusplus
}
#endif

#endif
