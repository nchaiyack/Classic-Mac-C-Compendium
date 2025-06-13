
#include <Aliases.h>
#include <Finder.h>
#include <StandardFile.h>

#include "standardgetfile.h"
#include "boxmakergetfile.h"

#define sfItemSelectThisItem	10

boxmakergetfile::boxmakergetfile( short dlogID)
	: standardgetfile( dlogID)
{
	Handle theTypesHandle = Get1Resource( 'typs', 128);
	if( theTypesHandle != 0)
	{
		HLock( theTypesHandle);
			changeTypes( (OSType *)*theTypesHandle, GetHandleSize( theTypesHandle) >> 2);
		HUnlock( theTypesHandle);
		ReleaseResource( theTypesHandle);
	}
	//
	// This handle will never get freed again. This is no problem,
	// since it are only allocated once, and would only be freed at exit
	// time, anyway.
	//
	Handle theTypeCreatorPairsHandle = Get1Resource( 'tycr', 128);
	if( theTypeCreatorPairsHandle == 0)
	{
		numTypeCreatorPairs = -1;
		theTypeCreatorPairs = 0L;
	} else {
		MoveHHi( theTypeCreatorPairsHandle);
		HLock( theTypeCreatorPairsHandle);
		numTypeCreatorPairs = GetHandleSize( theTypeCreatorPairsHandle) >> 2;
		theTypeCreatorPairs = (OSTypePair *)*theTypeCreatorPairsHandle;
	}
	//
	// read extra flags:
	//
	Handle flagHandle = Get1Resource( 'flgs', 128);
	if( flagHandle)
	{
		const Boolean *flags = (const Boolean *)*flagHandle;
		enterFolders    = flags[ 0];
		passFolders     = flags[ 1];
		enterInvisibles = flags[ 2];
		passInvisibles  = flags[ 3];
		ReleaseResource( flagHandle);
	} else {
		enterFolders    = false;
		passFolders     = false;
		enterInvisibles = false;
		passInvisibles  = false;
	}
	allowFolderSelection = passFolders || enterFolders;
	selectThisItemControl = 0L;
}

Boolean boxmakergetfile::matchesTypeList( OSType ourType) const
{
	Boolean result = (numTypes == -1);

	OSType *theType = theTypes;
	
	for( int i = 0; i < numTypes; i++)
	{
		if( ourType == *theType)
		{
			result = true;
			break;
		}
	}
	return result;
}

Boolean boxmakergetfile::matchesTypeCreatorPairs(
									OSType theType, OSType theCreator) const
{
	Boolean result = (numTypeCreatorPairs == -1);

	OSTypePair *thePair = theTypeCreatorPairs;
	
	for( int i = 0; i < numTypeCreatorPairs; i++)
	{
		if( matches( theType, thePair->type) && matches( theCreator, thePair->creator))
		{
			result = true;
			break;
		}
	}
	return result;
}

Boolean boxmakergetfile::filterThisItem( const CInfoPBPtr myPB)
{
	const Boolean invisible = ((myPB->hFileInfo.ioFlFndrInfo.fdFlags & fInvisible) != 0);
	const Boolean folder    = ((myPB->hFileInfo.ioFlAttrib & ioDirMask) != 0);
	
	Boolean result = invisible && !passInvisibles;
	//
	// we do not check for 'passFolders'; folders must be visible in
	// the dialog at any time.
	//
	if( (result == false) && !folder)
	{
		result = !matchesTypeCreatorPairs(
						myPB->hFileInfo.ioFlFndrInfo.fdType,
						myPB->hFileInfo.ioFlFndrInfo.fdCreator);
	}
	return result;
}

short boxmakergetfile::handleItemPress( short item, DialogPtr theDialog)
{
	//
	// CustomGetFile calls the dialog hook for both main and subsidiary dialog boxes.
	// Make sure that dialog record indicates that this is the main GetFolder dialog.
	//
	// 950530 RV: work around a bug in SC++ 8.0. When 'enums are always ints' is off
	// character constants such as 'stdf' are treated as shorts.
	//
	#ifdef THINK_C
		if( (OSType)(((WindowPeek)theDialog)->refCon) == 'stdf')
	#else
		if( (OSType)(((WindowPeek)theDialog)->refCon) == sfMainDialogRefCon)
	#endif
	{
		//
		// Disable the 'Select this one' button when needed:
		//
		if( selectThisItemControl != 0)
		{
			const short hilite = ((sfFile.name[ 0] == 0)
						|| ((sfIsFolder || sfIsVolume) && !allowFolderSelection)) ? 255 : 0;

			HiliteControl( selectThisItemControl, hilite);
		}
		switch( item)
		{
			case sfHookFirstCall:
				{
					short	itemType;
					Rect	itemRect;
					
					GetDialogItem( theDialog, sfItemSelectThisItem, &itemType,
							(Handle *)&selectThisItemControl, &itemRect);
				}
				break;

			case sfHookLastCall:
				selectThisItemControl = 0L;
				break;

			case sfItemSelectThisItem:
				//
				// If the user clicked the select current button, force a cancel and
				// set the sfGood field of the Reply record to true, unless nothing
				// is selected or a folder is selected and passFolders is false.
				//
				item = sfItemCancelButton;
				sfGood = true;
				break;
		}
	}
	return item;
}
