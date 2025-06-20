#include "SpeakIt.h"
#include "Speech.h"

Boolean	 gSpeechMgrPresent = false;

void CheckSpeechMgr()
{	// Warning: for SPM 8 C++, set SpeechLib.xcoff to soft import binding or else won't run on a PPC
	long			response;
	OSErr			iErr;
	Boolean			hasGestalt, okay;
	short			voiceCount;

	hasGestalt = (Gestalt (gestaltVersion, &response) == noErr);
	if(hasGestalt) {
		iErr = Gestalt( gestaltSpeechAttr, &response );
		okay = ( iErr == noErr ) && BitTst( &response, 31 - gestaltSpeechMgrPresent );
#if defined powerc
		if( okay )
			okay = BitTst( &response, 31 - gestaltSpeechHasPPCGlue );
#endif
		if( okay ) {
			CountVoices(&voiceCount);
			if( voiceCount > 0 ) {
				gSpeechMgrPresent = true;	// passed all tests!
			}
		}
	}
}


void SpeakIt( char *speakText, short waitWhileTalking )
{
/*	speakText: c text string
	waitWhileTalking: if true, do not exit this routine until we're finished "speaking."  Otherwise, 
						exit immediately.
*/
	static SpeechChannel	speechChannel = nil;
	Fixed	sInfo;
	Boolean	okay;

	if( !gSpeechMgrPresent )
		return;

	if (speechChannel) {
		DisposeSpeechChannel(speechChannel);
		speechChannel = nil;
	}

	c2pstr( speakText );
	SpeakString ( (unsigned char*)speakText );

	if( waitWhileTalking ) {
		while ( SpeechBusy() )
			{
			}
	}
}
