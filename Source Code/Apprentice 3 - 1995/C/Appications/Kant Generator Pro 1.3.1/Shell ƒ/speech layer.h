#ifndef __MY_SPEECH_LAYER_H__
#define __MY_SPEECH_LAYER_H__

extern	OSType			gCurrentVoiceCreator, gCurrentVoiceID;

#include <Speech.h>

#ifdef __cplusplus
extern "C" {
#endif

extern	void InitTheSpeech(void);
Boolean SpeechIsAvailableQQ(void);
extern	void StopTheSpeech(SpeechChannel speechChannel);
extern	SpeechChannel StartTheSpeech(Handle theTextHandle, short startOffset, short endOffset);
extern	void RebuildSpeechMenu(MenuHandle speechMenu);
extern	void AdjustSpeechMenu(MenuHandle speechMenu);
extern	void SetCurrentVoiceFromMenuItem(short menuItem);
extern	Boolean SetCurrentVoiceFromName(Str255 theVoiceStr);

#ifdef __cplusplus
}
#endif

#endif
