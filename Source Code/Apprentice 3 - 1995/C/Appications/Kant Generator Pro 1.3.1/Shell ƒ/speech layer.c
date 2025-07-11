#include "speech layer.h"
#include "memory layer.h"

		OSType			gCurrentVoiceCreator, gCurrentVoiceID;
static	Boolean			gSpeechManagerInstalled=FALSE;
static	short			gNumItemsBeforeVoices;

void InitTheSpeech(void)
{
	OSErr			err;
	long			info;
	short			numVoices;
	
	gSpeechManagerInstalled=FALSE;
	if ((err=Gestalt(gestaltSpeechAttr, &info))!=noErr)
		return;
	
	if (!(info && (1L<<gestaltSpeechMgrPresent)))
		return;

	if ((err=CountVoices(&numVoices))!=noErr)
		return;
	
	if (numVoices==0)
		return;
	
	gSpeechManagerInstalled=TRUE;
}

Boolean SpeechIsAvailableQQ(void)
{
	return gSpeechManagerInstalled;
}

void StopTheSpeech(SpeechChannel speechChannel)
{
	OSErr err;
	
	if ((err=StopSpeechAt(speechChannel, kEndOfWord))!=noErr)
		SysBeep(20);
	while (SpeechBusy() > 0) {};
	DisposeSpeechChannel(speechChannel);
}

SpeechChannel StartTheSpeech(Handle theTextHandle, short startOffset, short endOffset)
{
	VoiceSpec			voice;
	VoiceDescription	voice_info;
	SpeechChannel		speechChannel;
	OSErr				err;
	
	if (!gSpeechManagerInstalled)
		return 0L;
	
	voice.creator=gCurrentVoiceCreator;
	voice.id=gCurrentVoiceID;
	if ((err=GetVoiceDescription(&voice, &voice_info, sizeof(VoiceDescription)))!=noErr)
		return 0L;
	
	if ((err=NewSpeechChannel(&voice, &speechChannel))!=noErr)
		return 0L;
	
	HLockHi(theTextHandle);
	SpeakText(speechChannel, *theTextHandle+startOffset, endOffset-startOffset);
	
	return speechChannel;
}

void RebuildSpeechMenu(MenuHandle speechMenu)
{
	VoiceSpec			voice;
	VoiceDescription	voice_info;
	short				i, numVoices;
	Boolean				foundCurrentVoice, isCurrentVoice;
	
	if (!gSpeechManagerInstalled)
		return;
	
	gNumItemsBeforeVoices=CountMItems(speechMenu);
	
	CountVoices(&numVoices);
	
	for (i=1; i<=numVoices; i++)
	{
		if (GetIndVoice(i, &voice)==noErr)
		{
			GetVoiceDescription(&voice, &voice_info, sizeof(VoiceDescription));
			AppendMenu(speechMenu, "\p ");
			SetMenuItemText(speechMenu, CountMItems(speechMenu), voice_info.name);
			if ((isCurrentVoice=
				((gCurrentVoiceCreator==voice.creator) && (gCurrentVoiceID==voice.id)))==TRUE)
				foundCurrentVoice=TRUE;
			CheckItem(speechMenu, CountMItems(speechMenu), isCurrentVoice);
		}
	}
	
	if (!foundCurrentVoice)
	{
		GetIndVoice(1, &voice);
		gCurrentVoiceCreator=voice.creator;
		gCurrentVoiceID=voice.id;
		CheckItem(speechMenu, gNumItemsBeforeVoices+1, TRUE);
	}
}

void AdjustSpeechMenu(MenuHandle speechMenu)
{
	VoiceSpec			voice;
	VoiceDescription	voice_info;
	short				i, numVoices;
	Boolean				foundCurrentVoice, isCurrentVoice;
	
	if (!gSpeechManagerInstalled)
		return;
	
	CountVoices(&numVoices);
	
	for (i=1; i<=numVoices; i++)
	{
		if (GetIndVoice(i, &voice)==noErr)
		{
			GetVoiceDescription(&voice, &voice_info, sizeof(VoiceDescription));
			if ((isCurrentVoice=
				((gCurrentVoiceCreator==voice.creator) && (gCurrentVoiceID==voice.id)))==TRUE)
				foundCurrentVoice=TRUE;
			CheckItem(speechMenu, gNumItemsBeforeVoices+i, isCurrentVoice);
		}
	}
	
	if (!foundCurrentVoice)
	{
		SetCurrentVoiceFromMenuItem(gNumItemsBeforeVoices+1);
		AdjustSpeechMenu(speechMenu);
	}
}

void SetCurrentVoiceFromMenuItem(short menuItem)
{
	VoiceSpec		voice;
	
	if (!gSpeechManagerInstalled)
		return;
	
	GetIndVoice(menuItem-gNumItemsBeforeVoices, &voice);
	gCurrentVoiceCreator=voice.creator;
	gCurrentVoiceID=voice.id;
}

Boolean SetCurrentVoiceFromName(Str255 theVoiceStr)
/* returns TRUE on success, FALSE if can't find named voice or some other error */
{
	VoiceSpec			voice;
	VoiceDescription	voice_info;
	short				i, numVoices;
	
	if (!gSpeechManagerInstalled)
		return FALSE;
	
	if (SpeechBusy()>0)
		return FALSE;	/* can't change voices while speaking */
	
	CountVoices(&numVoices);
	
	for (i=1; i<=numVoices; i++)
	{
		if (GetIndVoice(i, &voice)==noErr)
		{
			GetVoiceDescription(&voice, &voice_info, sizeof(VoiceDescription));
			if (Mymemcompare((Ptr)theVoiceStr, (Ptr)voice_info.name, theVoiceStr[0]+1))
			{
				gCurrentVoiceCreator=voice.creator;
				gCurrentVoiceID=voice.id;
				return TRUE;
			}
		}
	}
	
	return FALSE;
}
