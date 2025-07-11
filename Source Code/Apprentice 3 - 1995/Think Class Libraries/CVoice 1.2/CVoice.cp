/******************************************************************************
 CVoice.cp		Version1.2
 
 Written by Brian Stern  <BrianS@pbcomputing.com>
 
 Written to work with TCL 2.x
 
 	A voice class that implements a speech manager voice.  To use this class
 	most simply, call the constructor with a known voice.  To speak call SpeakTheText.
 	
 	Str255	someText = {"\pHello from planet Zircon."};
 	CVoice *theVoice = new CVoice( "\pBoris");
 	theVoice->SpeakTheText( someText, Length(someText) );

	This class is Copyright � 1994, Brian Stern.  It may be used freely in 
	any projects.  If you do anything interesting with it send me email.

******************************************************************************/

#include "CVoice.h"
#include "GestaltEqu.h"

extern	CBartender		*gBartender;	/* The menu handling object */

voicetype	CVoice::cHasSpeechMan = knotInited;
short		CVoice::cVoiceMenuID = -1;		//Initialize to invalid value

/******************************************************************************
 CVoice
 
 	Default constructor.
******************************************************************************/
 
CVoice::CVoice(void)
{
	InitVoice();

}

/******************************************************************************
 CVoice
 
 	Alternate constructor.
******************************************************************************/
 
CVoice::CVoice( const Str63 theVoice )
{
	InitVoice();
	if ( cHasSpeechMan == kHasSpeechMan )
		SetVoice( theVoice );

}

/******************************************************************************
 ~CVoice


******************************************************************************/
 
CVoice::~CVoice( void )
{
	if ( fGoodChannel )
		(void) DisposeSpeechChannel( ftheChannel );

	ForgetPtr( fTextPtr );

}

/******************************************************************************
 InitVoice

	Private initializer function.
******************************************************************************/
 
void
CVoice::InitVoice( void )
{
	OSErr			err;
	long			result;

	//Is the Speech Manager present?
	if( cHasSpeechMan == knotInited )
	{
		err = Gestalt( gestaltSpeechAttr, &result );
		if ( ( err != noErr ) || !( result & ( 1 << gestaltSpeechMgrPresent ) ) )
			cHasSpeechMan = kNoHasSpeechMan;
		else
			cHasSpeechMan = kHasSpeechMan;
	}

	fGoodChannel = FALSE;

	if ( cHasSpeechMan == kHasSpeechMan )
	{
		//Allocate a pointer to hold the string to be spoken
		fTextPtr = NewPtr( kPtrLen );
		FailNIL( fTextPtr );
		fTextPtr[0] = 0;
		fTextPtrSize = kPtrLen;
		fInputFormat = kTextInput;	//All voices default to this
	}

}

/******************************************************************************
 SetVoice

	Set the current voice for this object.  This method must be called once
	before any speaking can take place.  theVoice is a PString.  If a match
	isn't found then the system default voice is chosen as a default voice.  
	Returns speech manager errors or kNoSpManErr if Speech Man isn't present.

******************************************************************************/

OSErr 
CVoice::SetVoice( const Str63 theVoice )
{
	OSErr				err = kNoSpManErr;
	long				i;
	short				NumVoices = 0;
	VoiceDescription	VD;
	VoiceSpec			aVSpec;
	SpeechChannel		theChannel;

	if ( cHasSpeechMan == kHasSpeechMan )
	{
		(void) CountVoices( &NumVoices );
	
		for ( i = 1; i <= NumVoices; i++ )
		{
			err = GetIndVoice( i, &aVSpec );
			err = GetVoiceDescription( &aVSpec, &VD, sizeof(VD) );
				if ( EqualString( theVoice, VD.name, FALSE, FALSE ) )
					break;
		}
	
		if ( i > NumVoices )	/*  If specified voice isn't found use the default system voice */
		{
			err = GetVoiceDescription( NULL, &VD, sizeof(VD) );// Get the VSpec of the default voice
			aVSpec = VD.voice;
		}

		ftheSpec = aVSpec;
		
		// Dispose of the old channel if it exists
		if ( fGoodChannel )
		{
			(void) DisposeSpeechChannel( ftheChannel );
			fCurrVoiceItem = 0;
		}

		fGoodChannel = FALSE;

		if ( noErr == ( err = NewSpeechChannel( &aVSpec, &theChannel ) ) )
		{
			fGoodChannel = TRUE;
			ftheChannel = theChannel;
			
			if ( cVoiceMenuID > 0 )// Get the item number in the Voice menu
			{
				fCurrVoiceItem = FindVoiceItemNum( theVoice );
			}

		}

	}

	return err;

}

/******************************************************************************
 SpeakTheText

	Speak this text.  The input text is assumed to be in text format (not 
	phoneme format) and is sent to the speech manager to be spoken.  
	This method is used for speaking most text.  Note that text with 
	embedded speech commands should use this function.

******************************************************************************/

OSErr 
CVoice::SpeakTheText( const char *theText, long len )
{
	return SpeakIt( theText, len, kTextInput );
	
}

/******************************************************************************
 SpeakThePhonemes

	Speak this text.  The text is interpreted as being in phonemic format.

******************************************************************************/

OSErr 
CVoice::SpeakThePhonemes( const char *theText, long len )
{
	return SpeakIt( theText, len, kPhonInput );
	
}

/******************************************************************************
 SpeakTheText

	Speak this text.  A copy is made of the text to be spoken so it may
	be disposed of or altered after this routine returns.  Protected method.

******************************************************************************/

OSErr 
CVoice::SpeakIt( const char *theText, long len, short inputType )
{
	long		textlen = len;
	OSErr		err = kBadChanErr;
	Ptr			tPtr;
	
	if ( fGoodChannel )
	{
		//Interupt any current speech so we can manipulate the text buffer
		err = StopSpeech( ftheChannel );
		SetInputFormat( inputType );

		//Adjust size of pointer to accomodate size of text if necessary
		if ( textlen > fTextPtrSize )
		{
			//First try to increase size of pointer, likely to fail
			SetPtrSize( fTextPtr, textlen );
			if ( MemError() == noErr )
			{
				fTextPtrSize = textlen;
			}
			else
			{
				//Next try to get a new, larger ptr
				SetPtrSize( fTextPtr, 2 );//Free up some mem
				tPtr = NewPtr( textlen );
				if ( MemError() == noErr )
				{
					DisposePtr( fTextPtr );//Dispose of old Pointer
					fTextPtr = tPtr;
					fTextPtrSize = textlen;
				}
				else
				{	//If no joy then restore the ptrsize and truncate the text to be spoken
					SetPtrSize( fTextPtr, fTextPtrSize );
					textlen = fTextPtrSize;
				}
			}
		}

		BlockMoveData( theText, fTextPtr, textlen );

		err = SpeakText( ftheChannel, fTextPtr, textlen );
	}

	return err;

}

/******************************************************************************
 SetInputFormat

	Set the current input format to either TEXT or PHON.  protected method.
	This method can be faked out if the input format is changed with
	embedded speech commands.

******************************************************************************/

void 
CVoice::SetInputFormat( short inputFormat )
{
	long		theFormat;
	
	if ( fInputFormat != inputFormat )
	{
		if ( inputFormat == kPhonInput )
			theFormat = 'PHON';
		else
			theFormat = 'TEXT';

		if ( noErr == SetSpeechInfo( ftheChannel, soInputMode, &theFormat ) )
			fInputFormat = inputFormat;
	}

}

/******************************************************************************
 GetPhonemeInfo

	Return the handle containing the phoneme info.  Phoneme info is a handle
	containing ascii text showing all the phonemes recognized by this voice.

******************************************************************************/

Handle 
CVoice::GetPhonemeInfo( void )
{
	Handle		PhonDesc = NULL;
	
	if ( fGoodChannel )
		(void) GetSpeechInfo( ftheChannel, soPhonemeSymbols, &PhonDesc );
	
	return PhonDesc;

}

/******************************************************************************
 GetDescription

	Get a voice description for the current speech channel.

******************************************************************************/

OSErr 
CVoice::GetDescription( VoiceDescription *theVDesc ) const
{
	VoiceSpec		aVSpec = ftheSpec;	//Make local version of VSpec
		
	if ( fGoodChannel )
		return GetVoiceDescription( &aVSpec, theVDesc, sizeof( VoiceDescription ) );
	else
		return kBadChanErr;

}

/******************************************************************************
 GetVSpec

	Return the Voice Spec record for the current speech channel.

******************************************************************************/

void 
CVoice::GetVSpec( VoiceSpec *theVSpec ) const
{
	*theVSpec = ftheSpec;

}

/******************************************************************************
 WordToPhoneme

	Return the phonemic representation of theWord in thePhoneme.  Both are
	PStrings.  theWord can be any word or phrase.  thePhoneme will recieve
	the phonemic representation of this word or phrase.  Remember that
	phonemes are often two characters so the phonemic representation of a
	long phrase may not fit in the 255 bytes of thePhoneme.  This routine
	truncates the result and returns -2 if it's too long and has been 
	truncated.
	
******************************************************************************/

OSErr 
CVoice::WordToPhoneme( const Str255 theWord, Str255 thePhoneme )
{
	long		phonelen = 0;
	OSErr		err = kBadChanErr;
	Handle		phonemehand = NewHandle( 256 );

	if ( !MemError() && fGoodChannel )
	{
		SetInputFormat( kTextInput );	//Needs to be text format
		err = TextToPhonemes( ftheChannel, (Ptr) &theWord[1], theWord[0],
				phonemehand, &phonelen );

		if ( noErr == err)
		{
			if ( phonelen > 255 )
			{
				phonelen = 255;
				err = -2;
			}

			BlockMoveData( *phonemehand, &thePhoneme[1], phonelen );
			thePhoneme[0] = phonelen;

		}
	}
	
	ForgetHandle( phonemehand );
	
	return err;

}

/******************************************************************************
AreWeSpeaking
 
 	Return TRUE if the channel is busy.  If you wish to speak text without
 	interrupting currently speaking text, poll this method until it returns
 	FALSE.
 	
******************************************************************************/

Boolean 
CVoice::AreWeSpeaking(void)
{
	SpeechStatusInfo	SpeechInfo;
	long				result = FALSE;	//Default result
	OSErr				err;

	if ( fGoodChannel )
	{
		err = GetSpeechInfo( ftheChannel, soStatus, &SpeechInfo );
		
		if ( err == noErr && SpeechInfo.outputBusy )
	 		result = TRUE;
 	}
 
	return		result;

}

/******************************************************************************
AreAnySpeaking
 
 	Return TRUE if any channels are busy.
 	
******************************************************************************/

Boolean 
CVoice::AreAnySpeaking(void)
{
	long				result = FALSE;	//Default result
	
	if ( cHasSpeechMan == kHasSpeechMan )
	{
		if ( SpeechBusy() > 0 )
	 		result = TRUE;
	}

	return		result;

}

/******************************************************************************
AreWePaused
 
 	Return TRUE if the channel is paused.
 	
******************************************************************************/

Boolean 
CVoice::AreWePaused(void)
{
	SpeechStatusInfo	SpeechInfo;
	long				result = FALSE;	//Default result
	OSErr				err;

	if ( fGoodChannel )
	{
		err = GetSpeechInfo( ftheChannel, soStatus, &SpeechInfo );
		
		if ( err == noErr && SpeechInfo.outputPaused )
	 		result = TRUE;
 	}

	return		result;

}

/******************************************************************************
StopSpeaking
 
 	Stop speaking for this voice.  whentostop is one of the constants defined
 	in Speech.h: kImmediate, kEndOfWord, or kEndOfSentence.  It is safe
 	to call this method on an already idle channel.
 	
******************************************************************************/

OSErr 
CVoice::StopSpeaking( long whentostop )
{
	if ( fGoodChannel )
		return StopSpeechAt( ftheChannel, whentostop );
	else
		return kBadChanErr;

}

/******************************************************************************
 PauseSpeaking
 
 	Pause speaking for this voice.  whentostop is one of the constants defined
 	in Speech.h: kImmediate, kEndOfWord, or kEndOfSentence.
 	
******************************************************************************/

OSErr 
CVoice::PauseSpeaking( long whentostop )
{
	if ( fGoodChannel )
		return PauseSpeechAt( ftheChannel, whentostop );
	else
		return kBadChanErr;

}

/******************************************************************************
ContinueSpeaking
 
 	Start speaking after having been paused.
 	
******************************************************************************/

OSErr 
CVoice::ContinueSpeaking(void)
{
	if ( fGoodChannel )
		return ContinueSpeech( ftheChannel );
	else
		return kBadChanErr;

}

/******************************************************************************
NumVoices
 
 	Return the number of voices installed on this machine.  Returns -1 if
 	no Speech Man.
 	
******************************************************************************/

short 
CVoice::NumVoices(void)
{
	short		NumofVoices = -1;

	if ( cHasSpeechMan == kHasSpeechMan )
		(void) CountVoices( &NumofVoices );
		
	return		NumofVoices;

}

/******************************************************************************
GetVoicebyIndex
 
 	Return the name of the nth voice installed.  Good values are 
 	1 to NumVoices().  This routine can be used to build a list of all
 	the names installed on a machine.  The returned name is a PString and
 	if any errors occurr the string will be empty.
 	
******************************************************************************/

OSErr 
CVoice::GetVoicebyIndex( long index, Str63 theName )
{
	VoiceSpec			theVSpec;
	VoiceDescription	VD;
	OSErr				err = kNoSpManErr;

	theName[0] = 0;		//In case of errors

	if ( cHasSpeechMan == kHasSpeechMan )
	{
		err = GetIndVoice( index, &theVSpec );
		if ( err == noErr )
			err = GetVoiceDescription( &theVSpec, &VD, sizeof( VD ) );
		if ( err == noErr )
			BlockMoveData( VD.name, theName, VD.name[0]  + 1 );
	}
	
	return err;

}

/******************************************************************************
UseADictionary
 
	Add a dict resource to this speech channel.  The resource may be disposed
	after calling this method.
 	
******************************************************************************/

OSErr 
CVoice::UseADictionary( Handle thedict )
{
	OSErr				err = kBadChanErr;

	if ( fGoodChannel )
		err = UseDictionary( ftheChannel, thedict );

	return err;

}

/******************************************************************************
BuildVoiceMenu
 
	Build a menu that contains all the currently installed voices.  The menu
	must already exist.  This method would usually be called only once from
	the Application's SetUpMenus method.
 	
******************************************************************************/

OSErr 
CVoice::BuildVoiceMenu( short theMenuID )
{
	OSErr			err = kNoSpManErr;
	short			theNum;
	long			i;
	MenuHandle		VMenu;
	Str63			theName;

	if ( cHasSpeechMan == kHasSpeechMan )
	{
		VMenu = gBartender->FindMacMenu( theMenuID );
		ASSERT( VMenu );								// Does the menu exist?
		gBartender->SetDimOption( theMenuID, dimNONE );	// Don't dim every time
		gBartender->SetUnchecking( theMenuID, TRUE );	// Do uncheck every time
	
		CountVoices( &theNum );					// How many voices?
		for ( i = 1; i <= theNum; i++ )
		{
			err = GetVoicebyIndex( i, theName );// Get a voice
			if ( err == noErr )
				AppendMenu( VMenu, theName );	// Add the name to the menu
		}
	
		cVoiceMenuID = theMenuID;				// Save the menu ID in the class variable
	
	}
	
	return err;

}

/******************************************************************************
SetVoiceFromMenu
 
	Set the voice for this object.  Use this method when a Voice Menu was
	created with BuildVoiceMenu.  It is called from the DoCommand method
	of any Pane or Document that can have a voice associated with it.  Call
	it like this:
	
	default:
		if ( theCommand < 0 )
			itsVoice->SetVoiceFromMenu( theCommand );
		else
			inherited::DoCommand( theCommand );
		break;
 	
******************************************************************************/

OSErr 
CVoice::SetVoiceFromMenu( long theCommand )
{
	short			menuID;
	short			menuItem;
	MenuHandle		VMenu;
	OSErr			err;
	short			saveMenuID;
	Str63			VName;

	//Extract the menu ID and the Line number in the menu
	theCommand = -theCommand;
	menuID = HiShort( theCommand );
	menuItem = LoShort( theCommand );
	VMenu = gBartender->FindMacMenu( menuID );

	GetItem(  VMenu, menuItem, VName );	//Get the name of the voice

	//Set cVoiceMenuID to an invalid value so SetVoice won't bother 
	//trying to finding the itemnum
	saveMenuID = cVoiceMenuID;
	cVoiceMenuID = -1;
	
	err = SetVoice( VName );	//Change the voice to the voice chosen

	cVoiceMenuID = saveMenuID;	//Reset the menuID to the saved value

	fCurrVoiceItem = menuItem;	//Save the line number in the menu
	
	return err;
	
}

/******************************************************************************
UpdateMenus

 	Place a checkmark in front of the current voice in the Voice menu.  
 	Call this from the UpdateMenus method of any pane or document that has 
 	a voice associated with it if a voice menu has been created by BuildVoiceMenu.
 
******************************************************************************/

void 
CVoice::UpdateMenus(void)
{
	MenuHandle		VMenu;

	if ( cVoiceMenuID > 0 )
	{
		VMenu = gBartender->FindMacMenu( cVoiceMenuID );
		CheckItem( VMenu, fCurrVoiceItem, TRUE );

	}

}

/******************************************************************************
FindVoiceItemNum

 	Find the item number that matches the voice name in theVoice. 
 	Called by SetVoice when it doesn't know the line number in the voices 
 	menu that corresponds to a particular voice.  protected method.  
 
******************************************************************************/

short 
CVoice::FindVoiceItemNum( const Str63 theVoice )
{
	MenuHandle		VMenu;
	short			numitems;
	short			result = 0;
	Str63			VName;

	VMenu = gBartender->FindMacMenu( cVoiceMenuID );

	ASSERT( VMenu );

	numitems = CountMItems( VMenu );

	for ( int i = 1; i <= numitems; i++ )
	{
		GetItem(  VMenu, i, VName );
		if ( EqualString( theVoice, VName, TRUE, TRUE ) )
		{
			result = i;
			break;
		}
	}
	
	return result;

}
