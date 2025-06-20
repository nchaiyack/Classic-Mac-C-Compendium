/*
IsCmdPeriod.c
You pass it an event record and it tells you whether or not the user has hit
command-period (i.e. "cancel"). The straightforward way breaks on some non-US
Systems because the keys get remapped (i.e. period!='.'). The elaborate way is
supposed to work on all Systems, but hasn't been tested.

WARNING:
This hasn't been tested. I haven't read the Script Manager chapter. The caching
scheme (saving "period") might fail if you change script systems between calls
to IsCmdPeriod.

dgp.

Copied from comp.sys.mac.programming:

From: k044477@hobbes.kzoo.edu (Jamie R. McCarthy)
Date: 13 May 93 19:41:38 GMT
Organization: Kalamazoo College

>PLEASE everybody read the "International Canceling" tech note (in
>"Text"), or in the official jargon: M.TE.InternationalCancel (yech).
>
>It explains that Command key combinations with non-letter keys may
>map to Command-Shift key codes on international keyboards. And we
>all know that Command-Shift gets intercepted by the operating system.
>This is a pain, because it means you won't be able to cancel operations,
>or have access to similar keyboard shortcuts.
 
Here's the function I use.  I think I took it pretty much straight from
the TN, but I might have made some minor improvement, I don't recall
what.  Anyway, maybe this'll save someone a few minutes' worth of
typing:
- -- 
 Jamie McCarthy 	Internet: k044477@kzoo.edu	AppleLink: j.mccarthy

HISTORY:
13 May 93 Jamie McCarthy posted it on comp.sys.mac.programming
6/23/93	dgp corrected bug of using "=" instead of "==" in an if statement.
	Check for valid handle before dereferencing. Tightened up code a bit.
	Not tested. 
6/30/93	dgp The elaborate method required for international systems seems
excessive for routine use, so I added a shortcut. We go through the whole 
rigamarole once, and note what the Cmd-period key presses were translated to.
On subsequent occasions we just (quickly) check for that. 
	
*/
 
Boolean IsCmdPeriod(EventRecord *theEvent);
#include <Script.h>

Boolean IsCmdPeriod(register EventRecord *theEvent)
{
	static char period=0;	// what cmd-period gets mapped to.

	if(period) return (theEvent->what == keyDown || theEvent->what == autoKey)
		&& (theEvent->modifiers & cmdKey)
		&& (theEvent->message & charCodeMask) == period;
	else{
		Boolean isCmdPeriod=FALSE;
		short keyCode;
		long keyInfo,state,keyCID;
		Handle hKCHR;
		Ptr pKCHR;
		
		if ((theEvent->modifiers & cmdKey)
			&& (theEvent->what == keyDown || theEvent->what == autoKey)){
			pKCHR=(Ptr)GetEnvirons(smKCHRCache);	// returns NULL under System 6
			if(pKCHR == NULL){
				keyCID=GetScript(GetEnvirons(smKeyScript),smScriptKeys);
				hKCHR=GetResource('KCHR',keyCID);
				if(hKCHR!=NULL)pKCHR=*hKCHR;
			}else hKCHR=NULL;
			if(pKCHR != NULL){
				// re-translate the virtual key to a char, without the command modifier
				keyCode=(theEvent->message & keyCodeMask) >> 8;	// the virtual key
				keyCode |= theEvent->modifiers & 0xFF00 & ~cmdKey;
				state=0;
				keyInfo=KeyTrans(pKCHR,keyCode,&state);
				if((keyInfo&charCodeMask) == '.' 
					|| ((keyInfo>>16)&charCodeMask) == '.')isCmdPeriod=TRUE;
				if (hKCHR != NULL) ReleaseResource(hKCHR);
			}else if((theEvent->message&charCodeMask) == '.')isCmdPeriod=TRUE;
			if(isCmdPeriod)period=theEvent->message&charCodeMask;
		}
		return isCmdPeriod;
	}
}
