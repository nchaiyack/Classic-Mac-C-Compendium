/*
	This code is taken from Tech Note 263, "International Cancelling."
	However, unlike that note, this code assumes that we have already
	identified the event as a keyboard event, and uses the escape key
	as a synonym for command-period.
*/
#include <Script.h>

Boolean Cancel_key( EventRecord *theEvent );

#define kMaskModifiers  0xFE00     // we need the modifiers without the
                                   // command key for KeyTrans
#define kMaskVirtualKey 0x0000FF00 // get virtual key from event message
                                   // for KeyTrans
#define kUpKeyMask      0x0080
#define kShiftWord      8          // we shift the virtual key to mask it
                                   // into the keyCode for KeyTrans
#define kMaskASCII1     0x00FF0000 // get the key out of the ASCII1 byte
#define kMaskASCII2     0x000000FF // get the key out of the ASCII2 byte
#define kPeriod         0x2E       // ascii for a period
#define kEscape			0x1B		// ascii for escape

Boolean Cancel_key( EventRecord *theEvent )
{

  Boolean	fTimeToQuit;
  short		keyCode;
  long		virtualKey, keyInfo, lowChar, highChar, state, keyCId;
  Handle	hKCHR;
  Ptr		KCHRPtr;

  fTimeToQuit = false;

  // see if the command key is down.  If it is, find out the ASCII
  // equivalent for the accompanying key.

  if ((*theEvent).modifiers & cmdKey ) {

    virtualKey = (theEvent->message & kMaskVirtualKey) >> kShiftWord;
    // And out the command key and Or in the virtualKey
    keyCode    = ((*theEvent).modifiers & kMaskModifiers)  |  virtualKey;
    state      = 0;

    hKCHR = nil;  /* set this to nil before starting */
    KCHRPtr = (Ptr) GetEnvirons( smKCHRCache );

    if ( !KCHRPtr ) {
      keyCId = GetScript( GetEnvirons(smKeyScript), smScriptKeys);

      hKCHR   = GetResource('KCHR',keyCId);
      KCHRPtr = *hKCHR;
    }

    if (KCHRPtr)
    {
      /* Don't bother locking since KeyTrans will never move memory */
      keyInfo = KeyTrans( KCHRPtr, keyCode, &state );
      ReleaseResource( hKCHR );
    }
    else
     keyInfo = theEvent->message;

    lowChar =  keyInfo &  kMaskASCII2;
    highChar = (keyInfo & kMaskASCII1) >> 16;
    if (lowChar == kPeriod || highChar == kPeriod)
      fTimeToQuit = true;

  }  // end the command key is down
  else
  {
  	if ( (theEvent->message & charCodeMask) == kEscape )
    	fTimeToQuit = true;
  }

return( fTimeToQuit );
}
