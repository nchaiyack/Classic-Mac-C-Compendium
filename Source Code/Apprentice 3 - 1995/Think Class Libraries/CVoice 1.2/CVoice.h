/******************************************************************************
 CVoice.h		Version1.2
 
 Written by Brian Stern  <BrianS@pbcomputing.com>
 
 Written to work with TCL 2.x
 
 	Interface file for CVoice.
 	
	This class is Copyright © 1994, Brian Stern.  It may be used freely in 
	any projects.  If you do anything interesting with it send me email.

******************************************************************************/

#pragma once

#include <Speech.h>

#define kPtrLen 	256L
#define kNoSpManErr	-1
#define kBadChanErr -2
#define kTruncErr	-3

enum {
	kTextInput,
	kPhonInput
};

enum voicetype{
	knotInited,
	kHasSpeechMan,
	kNoHasSpeechMan
};


class CVoice 
{		

protected:
								/** Instance Variables **/
	SpeechChannel		ftheChannel;		//Speech channel for this voice
	VoiceSpec			ftheSpec;			//VoiceSpec for this voice
	Boolean				fGoodChannel;		//Did the speech channel initialize OK?
	Ptr					fTextPtr;			//Buffer for text to be spoken
	long				fTextPtrSize;		//Current size of the buffer
	long				fInputFormat;		//Speaking in text or phonemic format
	short				fCurrVoiceItem;		//Item number of the voice name in Voice menu

static	voicetype		cHasSpeechMan;		//Class variable for presence of Speech Manager
static	short			cVoiceMenuID;		//Class variable for ID of Voice Menu if used

public:
								/** Instance Methods **/

							CVoice(void);
							CVoice( const Str63 theVoice );
	virtual 				~CVoice();
	
	virtual Boolean			HasSpeechMan( void ) { return cHasSpeechMan == kHasSpeechMan; }

	virtual OSErr			SetVoice( const Str63 theVoice );
	
	virtual OSErr			SpeakTheText( const char *theText, long len );
	virtual OSErr			SpeakThePhonemes( const char *theText, long len );

	virtual Handle			GetPhonemeInfo( void );

	virtual OSErr			GetDescription( VoiceDescription *theDesc ) const;

	virtual void			GetVSpec( VoiceSpec *theVSpec ) const;

	virtual OSErr			WordToPhoneme( const Str255 theWord, Str255 thePhoneme );

	virtual OSErr			StopSpeaking( long whentostop );
	virtual OSErr			PauseSpeaking( long whentostop );
	virtual OSErr			ContinueSpeaking(void);

	virtual Boolean			AreWeSpeaking(void);
	virtual Boolean			AreAnySpeaking(void);
	virtual Boolean			AreWePaused(void);

	virtual short			NumVoices(void);
	virtual OSErr			GetVoicebyIndex( long index, Str63 theName );

	virtual OSErr			BuildVoiceMenu( short theMenuID );
	virtual OSErr			SetVoiceFromMenu( long theCommand );
	virtual void			UpdateMenus(void);

	virtual OSErr			UseADictionary( Handle thedict );

protected:

	virtual OSErr			SpeakIt( const char *theText, long len, short inputType );
	
	virtual void			SetInputFormat( short inputFormat );

	virtual short			FindVoiceItemNum( const Str63 theVoice );

private:

	void	InitVoice( void );
	
};
