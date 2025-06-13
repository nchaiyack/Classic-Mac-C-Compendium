/*
** File:		FileUtils.c
**
** Written by:	Bill Hayden
**				Nikol Software
**
** Copyright © 1995 Nikol Software
** All rights reserved.
*/


#include "FileUtils.h"
#include "StringUtils.h"



/*****************************************************************************/


Boolean	SelectWorldFile(Str255	theFileName)
{
	SFTypeList			sfList = {'TEXT'};
	StandardFileReply	sfReply;

	StandardGetFile(nil, 1, sfList, &sfReply);

	if (sfReply.sfGood)
		{
		PathNameFromDirID(sfReply.sfFile.vRefNum, sfReply.sfFile.parID, theFileName);
		pcat(theFileName, sfReply.sfFile.name);
		}
		
	return sfReply.sfGood;
}


/*****************************************************************************/


OSErr PathNameFromDirID(short vRefNum, long dirID, Str255 fullPathName)
{
	Str255		dirName;
	DirInfo		dirInf;
	OSErr		err;

	*fullPathName = 0;

	dirInf.ioNamePtr = dirName;
	dirInf.ioDrParID = dirID;

	do {
		dirInf.ioVRefNum = vRefNum;
		dirInf.ioFDirIndex = -1; 				// -1 means use ioDrDirIDÉ
		dirInf.ioDrDirID = dirInf.ioDrParID;

		err = PBGetCatInfoSync((CInfoPBPtr)&dirInf);

		if (err == noErr)
			{
			dirName[++dirName[0]] = ':';

			if (dirName[0] + fullPathName[0] > 255)
				err = bdNamErr; 				// too big to eat!
			else
				{
				pcat(dirName, fullPathName);
				pcpy(fullPathName, dirName);
				}
			}

	} while (dirInf.ioDrDirID != fsRtDirID && err == noErr);

	return err;

}	// PathNameFromDirID


/*****************************************************************************/


OSErr	GetFilenameFromPathname(ConstStr255Param pathname, Str255 filename)
{
	short	index;
	short	nameEnd;

	/* default to no filename */
	filename[0] = 0;

	/* check for no pathname */
	if ( pathname == NULL )
		return ( notAFileErr );
	
	/* get string length */
	index = pathname[0];
	
	/* check for empty string */
	if ( index == 0 )
		return ( notAFileErr );
	
	/* skip over last trailing colon (if any) */
	if ( pathname[index] == ':' )
		--index;

	/* save the end of the string */
	nameEnd = index;

	/* if pathname ends with multiple colons, then this pathname refers */
	/* to a directory, not a file */
	if ( pathname[index] == ':' )
		return ( notAFileErr );
		
	
	/* parse backwards until we find a colon or hit the beginning of the pathname */
	while ( (index != 0) && (pathname[index] != ':') )
	{
		--index;
	}
	
	/* if we parsed to the beginning of the pathname and the pathname ended */
	/* with a colon, then pathname is a full pathname to a volume, not a file */
	if ( (index == 0) && (pathname[pathname[0]] == ':') )
		return ( notAFileErr );
	
	/* get the filename and return noErr */
	filename[0] = (char)(nameEnd - index);
	BlockMoveData(&pathname[index+1], &filename[1], nameEnd - index);
	return ( noErr );
}



/*****************************************************************************/



// strongly based on KillEveryoneButMe.c by C. K. Haun of Apple Computer

void QuitEverythingButMe(void)
{
    ProcessSerialNumber	myProc, processSN;
    ProcessSerialNumber	finderPSN;
    ProcessInfoRec		infoRec;
    Str31				processName;
    FSSpec				procSpec;
    OSErr				myErr = noErr;
    OSErr				otherError;
    AppleEvent			theEvent;
    AEDesc				theAddress;
    Boolean				ourFlag, notFinder;
    Boolean				finderFound = false;
    
    
    GetCurrentProcess(&myProc);
    /* Preset the PSN to no PSN, see IM VI, the Process Manager */
    processSN.lowLongOfPSN = kNoProcess;
    processSN.highLongOfPSN = kNoProcess;
    finderPSN.lowLongOfPSN = nil;
    finderPSN.highLongOfPSN = nil;
    
    do {
        myErr = GetNextProcess(&processSN);
        SameProcess(&myProc, &processSN, &ourFlag);
        if (!ourFlag && !finderFound)
        	{
            /* see if it's the Finder, we have to kill the finder LAST */
            /* or else non-sys 7 apps won't get killed */
            /* since the Finder must be there to convert the AppleEvent to Puppet Strings */
            /* if the app is not APpleEvent aware */
            infoRec.processInfoLength = sizeof(ProcessInfoRec);
            infoRec.processName = processName;
            infoRec.processAppSpec = &procSpec;
            GetProcessInformation(&processSN, &infoRec);
            if (infoRec.processSignature == 'MACS' && infoRec.processType == 'FNDR')
            	{
                /* save this number for later  */
                finderPSN = processSN;
                notFinder = false;
				}
			else
				{
                notFinder = true;
                finderFound = true;
				}
			}
			
        if (!myErr && !ourFlag && notFinder)
        	{
            otherError = AECreateDesc(typeProcessSerialNumber, (Ptr)&processSN, sizeof(processSN), &theAddress);
            if (!otherError)
                otherError = AECreateAppleEvent(kCoreEventClass, kAEQuitApplication, &theAddress, kAutoGenerateReturnID,
                                                kAnyTransactionID, &theEvent);
            if (!otherError)
                AEDisposeDesc(&theAddress);
            /* Again, the Finder will convert the AppleEvent to puppetstrings if */
            /* the application is a System 6 or non-AE aware app.  This ONLY  */
            /* happens for the 4 required (oapp,odoc,pdoc, and quit) AppleEvents  */
            /* and ONLY if you use the PSN for the address */
            if (!otherError)
                AESend(&theEvent, nil, kAENoReply + kAEAlwaysInteract + kAECanSwitchLayer, kAENormalPriority, kAEDefaultTimeout,
                       nil, nil);
            AEDisposeDesc(&theEvent);
			}
    } while (!myErr);
    
    /* Now, if the finder was running, it's safe to kill it */
    if (finderPSN.lowLongOfPSN || finderPSN.highLongOfPSN)
		{
        otherError = AECreateDesc(typeProcessSerialNumber, (Ptr)&finderPSN, sizeof(processSN), &theAddress);
        if (!otherError)
            otherError = AECreateAppleEvent(kCoreEventClass, kAEQuitApplication, &theAddress, kAutoGenerateReturnID,
                                            kAnyTransactionID, &theEvent);
        if (!otherError)
            AEDisposeDesc(&theAddress);
            
        if (!otherError)
            AESend(&theEvent, nil, kAENoReply + kAEAlwaysInteract + kAECanSwitchLayer, kAENormalPriority, kAEDefaultTimeout, nil,
                   nil);
                   
        AEDisposeDesc(&theEvent);
		}
}

