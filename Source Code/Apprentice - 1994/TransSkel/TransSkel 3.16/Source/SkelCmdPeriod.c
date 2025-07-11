/*
 * SkelCmdPeriod.c -- Figure out, in an internationally compatible way, whether
 * an event is a command-period key event.
 *
 * See TN TE 23, International Cancelling, for the rationale on how this works.
 */

# include	<Script.h>

# include	"TransSkel.h"


# define	kMaskModifiers	0xfe00
# define	kMaskVirtualKey	0x0000ff00
# define	kMaskAscii1		0x00ff0000
# define	kMaskAscii2		0x000000ff
# define	period			'.'


pascal Boolean
SkelCmdPeriod (EventRecord *evt)
{
short	keyCode;
long	virtualKey;
long	keyCId;
long	keyInfo;
long	state;
long	lowChar, highChar;
Handle	hKCHR;

	if (evt->what == keyDown || evt->what == autoKey)
	{
		if (evt->modifiers & cmdKey)			/* cmd key is down */
		{
			/* find out ASCII equivalent for key */
			virtualKey = (evt->message & kMaskVirtualKey) >> 8;
			/* "and" out command key, "or" in the virtual key */
			keyCode = (evt->modifiers & kMaskModifiers) | virtualKey;
			keyCId = GetScript (GetEnvirons (smKeyScript), smScriptKeys);
			hKCHR = GetResource ('KCHR', keyCId);
			if (hKCHR != (Handle) nil)
			{
				state = 0;
				/* don't bother locking because KeyTrans doesn't move memory */
				keyInfo = KeyTrans (*hKCHR, keyCode, &state);
				ReleaseResource (hKCHR);
			}
			else
				keyInfo = evt->message;

			lowChar = keyInfo & kMaskAscii2;
			highChar = (keyInfo & kMaskAscii1) >> 16;
			if (lowChar == period || highChar == period)
				return (true);
		}
	}
	return (false);
}