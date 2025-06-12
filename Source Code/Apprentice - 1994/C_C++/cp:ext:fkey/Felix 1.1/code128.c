/* Felix - June 29th, 1993 */

/*
	This is a patch for _FSDispatch. We are interested only in the PBDTGetAPPL routine, which corresponds to selector 0x27.
	We don't do anything unless the Control key is down. If it is, we call StandardGetFile in order to allow the user to choose which
	app he wants to use to open the file.
	
	[1994 note : The details of this patch are very hairy. You should only use the basic techniques: how to check for the selector,
	how to return directly to the caller, how to chain to the original trap.]

	Okay, it's not quite that simple... Hairy things happen. If the Finder asks the Disk 1 Desktop Database which app is to be used,
	and if as a result we give an app located on Disk 2, the Finder notices that something's wrong and calls us again with the same
	parameters (only ioIndex is incremented).
	To work around this, we must give an answer to the Finder iff the chosen app is on the same disk as the database we are supposed
	to use. To do that, I use PBDTGetPath with the app's vRefNum, and I compare the resulting ioDTRefNum with the one passed by
	the Finder. If they match, I return the app specification; otherwise, I return afpItemNotFound, and I wait for a call with the right
	ioDTRefNum.
	To do all this, we need globals:
		replyReady		if set, we have already put up a dialog, reply is valid and is waiting to be passed to the Finder.
		reply				contains the file chosen by the user

*/

#include "globals.c"

Boolean ControlIsPressed (void);
Boolean GetAPPL (DTPBPtr paramBlock, long *result, GlobalsPtr G);

/* Since the Custom Header option is used, main must be the first procedure */

void main (void)
{
	long 							selector, result;
	DTPBPtr					paramBlock;
	GlobalsPtr					G;

	asm {
			movem.l d0-d7/a0-a4, -(SP)																		/* Save registers */
			move.l a0, paramBlock																					/* Get parameters */
			move.l d0, selector
			move.l @data, G																							/* Get the globals' address */
	}
	
	if (selector == 0x27) {																							/* Check for PBDTGetAPPL's selector */
		if (ControlIsPressed() || G->replyReady) {
			if (GetAPPL(paramBlock, &result, G)) {
				asm {
					move.l result, (SP)																				/* This will put result into D0 */
					movem.l (SP)+, d0-d7/a0-a4																/* Restore registers */
					unlk a6
					rts																										/* Return directly to the caller */
				}
			}
		}
		else G->replyReady = false;
	}

	asm {
			movem.l (SP)+, d0-d7/a0-a4																		/* Restore registers */
			unlk	a6 												
			move.l @moof, -(SP)																						/* And chain to the real _FSDispatch */
			rts
			
	moof:	dc 'Mo', 'of', '!\0'	 																					/* Here the loader writes the org trap address */
	data:		dc 'Da', 'ta', '!\0'																						/* and here a pointer to the globals */
	}
}


/* 	GetAPPL replaces PBDTGetAPPL, and returns a Boolean telling whether the original PBDTGetAPPL should be skipped */

#define CurDirStore 0x398																					/* Contains SFGetFile's current directory */

Boolean GetAPPL (DTPBPtr paramBlock, long *result, GlobalsPtr G)
{
	SFTypeList				typeList;
	DTPBRec					params;
	OSErr						e;
	
	if (!G->replyReady) {	
		*((long*) CurDirStore) = G->dirID;																/* Set the default directory */
		typeList [0] = 'APPL';																					/* Show only applications */
		typeList [1] = 'adrp';																					/* and aliases to applications */
		StandardGetFile (NULL, 2, typeList, &(G->reply));										/* Display the dialog */
		if (G->reply.sfGood)																						
			G->replyReady = true;																				/* Remember we have displayed it */
		else 
			return false;																								/* The guy cancelled, forget everything */
	}
	
	params.ioNamePtr = nil;																					/* Make sure that the user chosen app resides */
	params.ioVRefNum = G->reply.sfFile.vRefNum;													/* on the same disk as the desktop database */
	e = PBDTGetPath (&params);																			/* which we are supposed to have looked up */
	
	if (e != noErr) {																									/* Error: this is probably a disk without a DTDB. */
		G->replyReady = false;																					/* Let's use the standard routine */
		return false;
	}
	
	if (params.ioDTRefNum != paramBlock->ioDTRefNum) {										/* Wrong disk. Let's report we didn't find anything */
		*result = afpItemNotFound;
		return true;
	}
	
	G->replyReady = false;																						/* Everything's fine, let's return the result */
	*result = noErr;

	BlockMove (&(G->reply.sfFile.name), paramBlock->ioNamePtr, 64);					/* Copy the name */
	paramBlock->ioAPPLParID = G->reply.sfFile.parID;											/* and the parent dir ID */
	paramBlock->ioResult = noErr;
	return true;
}

/* 	ControlIsPressed checks to see whether the Control key is down. This code is taken out of THINK Ref 2.0 */

#define ControlScanCode 0x3B

Boolean ControlIsPressed (void)
{
	unsigned char km[16];

	GetKeys ((long *) km);
	return ((km [ControlScanCode >> 3] >> (ControlScanCode & 7)) & 1);
}