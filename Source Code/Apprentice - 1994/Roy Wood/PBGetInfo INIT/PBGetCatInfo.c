#include "PBGetInfo.h"



/*
	Okay, the trickiest part of the following is the use of code like this:
	
		lea.l		sysVRefNum,a1
		movea.w		@sVRN,a0
		movem.w		a0,(a1)

	The problem is that we need to remember certain things such as the dirID 
	of the system folder, the system folder's parent dirID, etc., and we don't
	really have access to globals (all local storage is allocated on the
	stack when we're called).  To get around this problem, I've done some
	assembly stuff in the middle of this routine, including leaving space
	for storage of this pseudo-global data (by the way, this routine is
	locked in the system heap, so it stays around between invocations).
	
	During the INIT sequence when this patch is installed, the INIT code
	looks up the info we need and stores it in the appropriate place in
	this routine (okay, so we're writing self-modifying code-- so sue me).
	Later, when the routine is actually called by a user doing the
	GetCatInfo A-Trap, we need to access the pseudo-global data, and the
	easiest way to do this without having to write the whole thing in
	assembler or use a lot of pointers is to copy the pseudo-global data
	into the local variables which have been allocated on the stack as
	part of the normal sequence of things (you need to understand how
	the stack and local variables are handled on the Mac using registers
	A6 and A7).  And that's what the code snippet does:
	
		lea.l		sysVRefNum,a1	; get the address of the local variable 'sysVRefNum' and store it in register A1
		movea.w		@sVRN,a0		; now move the data stored in pseudo-global sVRN into register A0
		movem.w		a0,(a1)			; now move the contents of register A0 into the place pointed at by register A1

	Easy, no?  Well, sort of.
	
	There's probably a better way, but I'm not much of a Mac assembly programmer....
	This way works, even if it is self-modifying and likely to break on a 68040
	Mac with separate instruction and data caches (is there a flush-cache command
	I can use?).
	
*/


int strcmp(char *s1, char *s2);


void main(void)
{
int				errCode,selector,sysVRefNum,sysVolNum,index,*sysFolderIndex,*locked;
long			sysFolderDirID,sysFolderParID,*lastMod;
char			forbid;
char			*lockMessage,*unlockMessage;
CInfoPBRec		*paramBlock,myPBRec;


	asm
	{
			movem.l		d0-d7/a0-a6,-(sp)	/* save registers */
			
			movem.w		d0,selector			/* move the A-Trap selector code to a local variable for easier access */
			lea.l		paramBlock,a1		/* move the pointer to the paramater block to a local variable too */
			movem.l		a0,(a1)
			
			lea.l		sysVRefNum,a1		/* move the system folder volume reference number (previously obtained during INIT execution at startup) into a local variable */
			movea.w		@sVRN,a0
			movem.w		a0,(a1)
			
			lea.l		sysVolNum,a1		/* move the previously-obtained system folder volume number into local storage */
			movea.w		@sVNUm,a0
			movem.w		a0,(a1)
			
			lea.l		locked,a1			/* is it locked or not? */
			lea.l		@lockFlag,a0
			movem.l		a0,(a1)
			
			lea.l		sysFolderDirID,a1	/* move the previously-obtained system folder dirID into local storage */
			movea.l		@dirID,a0
			movem.l		a0,(a1)
			
			lea.l		sysFolderParID,a1	/* move the previously-obtained system folder's parent folder dirID into local storage */
			movea.l		@parID,a0
			movem.l		a0,(a1)
			
			lea.l		sysFolderIndex,a1	/* this is the index position of the system folder in the directory (remember, GetCatInfo lets you index through the files/folders in a directory) */
			lea.l		@sfIndex,a0
			movem.l		a0,(a1)
			
			lea.l		lastMod,a1			/* watch for modifications, such as the system folder being moved */
			lea.l		@modDat,a0
			movem.l		a0,(a1)
			
			lea.l		lockMessage,a1		/* the CDEV can pass a secret message to this patch to tell it to lock/unlock the system folder */
			lea.l		@lockMess,a0
			movem.l		a0,(a1)
			
			lea.l		unlockMessage,a1	/* the CDEV can pass a secret message to this patch to tell it to lock/unlock the system folder */
			lea.l		@unlockMess,a0
			movem.l		a0,(a1)
			
			bra.s		@skip				/* skip over the following, which is just memory allocated for storage of our sort-of globals between calls to us */
											/* by the way, this storage technique probably breaks on 68040 machines, since they have a data and instruction cache */
											
											
			dc.b		magicWord			/* this special number is used to mark the start of the pseudo-global storage area (during the INIT installation stuff, all the pseudo-globals are set) */
			
oldTrap:	dc.l		0					/* pseudo-global data begins! */
sVRN:		dc.w		0
sVNUm:		dc.w		0
dirID:		dc.l		0
parID:		dc.l		0
lockFlag:	dc.w		0

modDat:		dc.l		0
sfIndex:	dc.w		0xFFFF
lockMess:	dc.b		LOCKMESSAGE,0
unlockMess:	dc.b		UNLOCKMESSAGE,0
			dc.b		0					/* pseudo-globals end! */

skip:		nop					
	}
	
	/* watch for the secret message from the CDEV */
	
	if (paramBlock->hFileInfo.ioVRefNum == 0 && paramBlock->hFileInfo.ioDirID == 0 && paramBlock->hFileInfo.ioFDirIndex == 0 && paramBlock->hFileInfo.ioNamePtr)
	{
		if (strcmp((char *) paramBlock->hFileInfo.ioNamePtr,lockMessage) == EQSTR)
		{
			*locked = TRUE;
			
			paramBlock->hFileInfo.ioResult = noErr;
			
			asm
			{
					movem.l		(sp)+,d0-d7/a0-a6	/* restore registers */
					unlk		a6					/* clean up the stack */
					move.w		#noErr,d0			/* E.T. go home */
					rts
			}
		}
		
		else if (strcmp((char *) paramBlock->hFileInfo.ioNamePtr,unlockMessage) == EQSTR)
		{
			*locked = FALSE;
			
			paramBlock->hFileInfo.ioResult = noErr;
			
			asm
			{
					movem.l		(sp)+,d0-d7/a0-a6	/* restore registers */
					unlk		a6					/* clean up the stack */
					move.w		#noErr,d0			/* E.T. go home */
					rts
			}
		}
	}
	
	forbid = FALSE;
	
	/* check for indexed GetCatInfo calls */
	
	if (*locked && selector == 9 && paramBlock->hFileInfo.ioFDirIndex > 0)
	{
		/* check to see if the number of entries in the system folder's parent folder has changed, and if so, get the new index position of the system folder */
		
		if (paramBlock->hFileInfo.ioVRefNum == sysVolNum && paramBlock->hFileInfo.ioDirID == sysFolderParID)
		{
			myPBRec.dirInfo.ioCompletion = 0L;
			myPBRec.dirInfo.ioNamePtr = 0L;
			myPBRec.dirInfo.ioVRefNum = 0;
			myPBRec.dirInfo.ioFDirIndex = -1;
			myPBRec.dirInfo.ioDrDirID = sysFolderParID;
			
			asm
			{
				lea.l		myPBRec,a0		/* call the old GetCatInfo */
				move.w		#9,d0
				movea.l		@oldTrap,a1
				jsr			(a1)
				
				lea.l		errCode,a0		/* move the result into local storage */
				movem.w		d0,(a0)
			}
			
			
			if (errCode != noErr)
				*sysFolderIndex = -1;		/* oops, an error occurred! */
			
			else
			{
				if (myPBRec.dirInfo.ioDrMdDat > *lastMod)		/* oh ho!  the parent folder has been modified since we last checked it! */
				{
					*lastMod = myPBRec.dirInfo.ioDrMdDat;
					
					index = 1;
					
					/* now search through the folder to find the index position of the system folder */
					
					do
					{
						myPBRec.dirInfo.ioCompletion = 0L;
						myPBRec.dirInfo.ioNamePtr = 0L;
						myPBRec.dirInfo.ioVRefNum = -1;
						myPBRec.dirInfo.ioFDirIndex = index;
						myPBRec.dirInfo.ioDrDirID = sysFolderParID;
						
						asm
						{
							lea.l		myPBRec,a0
							move.w		#9,d0
							movea.l		@oldTrap,a1
							jsr			(a1)
							
							lea.l		errCode,a0
							movem.w		d0,(a0)
						}
						
						if (errCode == noErr && myPBRec.dirInfo.ioDrDirID == sysFolderDirID)
						{
							*sysFolderIndex = index;	/* found the new position of the system folder! */
							errCode = fnfErr;			/* stop indexing */
						}
						
						else
							index++;
						
					} while (errCode == noErr);
					
					if (errCode != fnfErr)
						*sysFolderIndex = -1;
				}
			}
			
			/* if the caller is searching the system folder's parent folder, we may have to bump the index by one to skip over the system folder */
			
			if (*sysFolderIndex > 0 && paramBlock->hFileInfo.ioFDirIndex >= *sysFolderIndex)
					paramBlock->hFileInfo.ioFDirIndex += 1;
		}
		
		/* is the caller searching the system folder using a working directory reference number? */
		
		else if (paramBlock->hFileInfo.ioVRefNum == sysVRefNum)
			forbid = TRUE;
		
		/* is the caller searching the system folder using a dirID? */
		
		else if (paramBlock->hFileInfo.ioDirID == sysFolderDirID)
			forbid = TRUE;
	}
	
	if (forbid)
	{
		SysBeep(1);		/* beep to indicate that we've denied access */
		
		paramBlock->hFileInfo.ioResult = ioErr;
		
		asm
		{
				movem.l		(sp)+,d0-d7/a0-a6	/* restore registers */
				unlk		a6					/* clean up stack frame */
				move.w		#ioErr,d0			/* return an error, since we're not allowing the caller to peek into the system folder */
				rts
		}
	}
	
	else
	{
		asm 
		{
				movem.l		(sp)+,d0-d7/a0-a6	/* restore registers */
				unlk		a6					/* clean up stack frame */
				move.l		@oldTrap,-(sp)		/* jump to the old trap, since the caller is looking somewhere other than the system folder, or the system folder is unlocked */
				rts
		}
	}
}



int strcmp(char *s1, char *s2)
{
	while(*s1 == *s2 && *s2 && *s1)
	{
		s1++; 
		s2++;
	}
	
	if (*s1 == *s2) 
		return(EQSTR);
	else
		return(!EQSTR);
}
