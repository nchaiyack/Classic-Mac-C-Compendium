/*--------------------------------------------------------------------------*/
/*																			*/
/* Append2hdlg - Append one 'hdlg' resource onto another.					*/
/*																			*/
/*	So what we want to do here is append the hdlg resource with resource ID	*/
/* srcID, onto the end of the hdlg resource, ID dstID.  Here's what these	*/
/* things look like:														*/
/*																			*/
/*					*-----------------------------------*					*/
/*					| Header (version, options, etc.)	|					*/
/*					*-----------------------------------*					*/
/*					/ # of items in item array			/					*/
/*					*-----------------------------------*					*/
/*					| Missing item marker				|					*/
/*					*-----------------------------------*					*/
/*					/ Array of items					/					*/
/*					/	�								/					*/
/*					*-----------------------------------*					*/
/*																			*/
/* Okay, so this chart breaks up the resource into logical blocks for this	*/
/* exercise.  We're most interested in the blocks marked with '/'s.  The	*/
/* first one of these is the # of items in the item array.  We need to read	*/
/* this field in the source hdlg to know how many items we are adding.  We	*/
/* need to update this field in the destination hdlg so the Help Manager 	*/
/* knows that we added some items.  This missing item is a generic item 	*/
/* used by the Help Manager anytime it can't find a resource entry for an	*/
/* item.  In most hdlg resources, this item is simply HMSkipItem (a no op),	*/
/* but this code makes no assumptions about that.  It does however assume	*/
/* that you want to preserve the missing item in the DESTINATION hdlg,		*/
/* rather than copying from the source.  									*/
/* So this code does this:													*/
/*																			*/
/*					Source							Destination				*/
/*		*---------------------------*		*---------------------------*	*/
/*		| Header					|	  	| Header					|	*/
/*		|	(version, options, etc.)|	 	|	(version, options, etc.)|	*/
/*		*---------------------------*		*---------------------------*	*/
/*		/ # of items in item array	/--Add->/ # of items in item array	/	*/
/*		*---------------------------*		*---------------------------*	*/
/*		| Missing item 				|		| Missing item 				|	*/
/*		*---------------------------*		*---------------------------*	*/
/*											/ Array of items			/	*/
/*											/	�						/	*/
/*		*---------------------------*-Copy->*---------------------------*	*/
/*		/ Array of items			/										*/
/*		/	�						/										*/
/*		*---------------------------*										*/
/*																			*/
/* Nothing to it.  The number of items is updated by adding the count from	*/
/* source hdlg, no 'minus 1' or anything tricky.  Then just append the item	*/
/* array from the source onto the end of the destination.  The Help Manager	*/
/* finds the new items no problem.  Okay, but since we are a guest in some-	*/
/* one elses closet, we need to play some games with memory.  First of all,	*/
/* we make sure that the destination 'hdlg' resource isn't already in mem-	*/
/* ory using the ol' SetResLoad trick.  If it's not there, we load it, but	*/
/* in any case, the hdlg is loaded into memory.  Once there, we make the	*/
/* rather large assumption that it's not going anywhere.  This means that	*/
/* we don't lock the hdlg handle, we don't even mark it non-purgeable.  The	*/
/* assumption is that the Help Manager will be finding the resource as soon */
/* as we are done with it, and he will take care of keeping it around after	*/
/* that.  Once the dialog is dismissed, the resource is naturally purged,	*/
/* along with the changes that we made to it.  Since we never told the		*/
/* Resource Manager that we had changed the resource, it doesn't try to save*/
/* it.																		*/

/* Types for accessing the Help Manager 'hdlg' resources. */
typedef struct {
	short	hdlgVers;
	short	hdlg1stItem;
	long	hdlgOptions;
	short	hdlgProcID;
	short	hdlgVarCode;
	short	hdlgNumItems;
} hdlgHeader, *hdlgHeaderPtr, **hdlgHeaderHdl;

typedef short *IntPtr, **IntHdl;

void Append2hdlg( short srcResID, short dstResID );

void Append2hdlg( short srcResID, short dstResID )
{
	Handle			srcHdl, dstHdl;
	Ptr				srcPtr, dstPtr;
	short			srcLength, dstLength;
	short			missingItmSz;
	SignedByte		dstHState;
	
	
	srcHdl = GetResource('hdlg', srcResID);	
	if (srcHdl != nil) {						
		SetResLoad(false);						/* System Resource, make sure it�s not	*/
		dstHdl = GetResource('hdlg', dstResID);	/* already loaded.				*/
		SetResLoad(true);
		if (*dstHdl == 0)
			dstHdl = GetResource('hdlg', dstResID);
		dstHState = HGetState(dstHdl);
		
		if (dstHdl != nil) {
			srcPtr = (Ptr)*srcHdl + sizeof(hdlgHeader);
			missingItmSz = *((IntPtr)srcPtr);
			srcLength = GetHandleSize(srcHdl) - (sizeof(hdlgHeader) - missingItmSz);

			dstLength = GetHandleSize(dstHdl);
			SetHandleSize(dstHdl, dstLength + srcLength);
			if (MemError() != noErr) {
				DebugStr("\pMemError");	/* Use this error handler, go to jail.	*/
				ExitToShell();		/*		It's the law!			*/
			}
			dstPtr = (Ptr)*dstHdl + dstLength;
			srcPtr = (Ptr)*srcHdl + sizeof(hdlgHeader) + missingItmSz;

			HLock(srcHdl);
			HLock(dstHdl);

			BlockMove(srcPtr, dstPtr, srcLength);

			HUnlock(srcHdl);
			HSetState(dstHdl, dstHState);

			((hdlgHeaderPtr)*dstHdl)->hdlgNumItems += ((hdlgHeaderPtr)*srcHdl)->hdlgNumItems;		
			
		}
	}
	ReleaseResource(srcHdl);
}
