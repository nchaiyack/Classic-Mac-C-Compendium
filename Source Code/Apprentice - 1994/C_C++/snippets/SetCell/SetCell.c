/*
SetCell.c
by Dale M. Greer, 31 March 1994
Written with MPW C 3.3
Internet: greer@utdallas.edu

** Launches MicroSoft Excel and enters specified data
** into specified cell using AppleEvents

Many thanks to John Nagle who sent me an example
of a received AppleEvent from a similar project.

The command we want to send is of the English form
"set cell 2 of row 3 of the document named Worksheet1 to 2.3"
The preposition "of" signals the existence of a container.  Objects in the 
AppleEvent record must be nested as the containers are nested.  An English 
version of the actual AppleEvent description of this English command might 
look something like 

set data
	(of object [cell
		(of object [row
			(of object [document named Worksheet1])
		3]
	2])
to 2.3

In practice, the object part comes out like this...

wanttypeccelfromobj Œwanttypecrowfromobj JwanttypedocufromnullformenumnameTEXT
Worksheet1formenumindxseldTEXT3formenumindxseldTEXT2

...but to make it you have to work top down, even though it comes 
out backwards, Comprende?  First you make a Document container, 
then put a row container inside that, then put a cell container 
inside that.

The rest is fairly straightforward and is explained in the code.

DMG

*/

#pragma mbg on
#pragma processor 68020
#pragma once

// #pragma load "SetCell.dumpfile" // Save time loading precompiled headers

#include <AEObjects.h>
#include <AEPackObject.h>
#include <AERegistry.h>
#include <AppleEvents.h>
#include <Controls.h>
#include <Desk.h>
#include <Dialogs.h>
#include <Errors.h>
#include <FCntl.h>
#include <Fonts.h>
#include <GestaltEqu.h>
#include <Memory.h>
#include <OSEvents.h>
#include <OSUtils.h>
#include <Quickdraw.h>
#include <Resources.h>
#include <stdio.h>
#include <StdLib.h>
#include <String.h>
#include <TextEdit.h>
#include <Types.h>
#include <ToolUtils.h>
#include <Windows.h>

void main (void);
void SetCell(AEAddressDesc *theAddress, char *cell, char *row, char *data);
void MyCreateDocContainer(AEDesc *myDocContainer, char *docName);
void MyCreateCellContainer(char *cell, char *row, AEDesc *myCellContainer, char *docName);
void MyCreateRowContainer(char *row, AEDesc *myRowContainer, char *docName);
OSErr FindAndLaunchExcel(AEAddressDesc *theAddress);

#define kSearch 129		// Simple message DLOG used while waiting to find Excel
LaunchParamBlockRec launchThis;

#define kAEName 'name'		// My AERegistry.h didn't have these, but I found them with the code to 
#define kAEIndex 'indx'		// WritesWell Jr.  You can always just use the straight constants anyway.

void main (void)
{
	AEAddressDesc theAddress;
	SetApplLimit((GetApplLimit())+8192);
	
	MaxApplZone();

	InitGraf((Ptr)&qd.thePort);
    InitFonts();
    InitWindows();
    TEInit();
    InitDialogs(nil);
    InitCursor();
	
	FindAndLaunchExcel(&theAddress);
	
	SetCell(&theAddress, "1", "1", "2.3");
	SetCell(&theAddress, "1", "2", "4.5");
	SetCell(&theAddress, "1", "3", "=r1c1*r2c1");
}

void SetCell(AEAddressDesc *theAddress, char *cell, char *row, char *data)
{
	OSErr err;
	AppleEvent appleEvent, reply;
	AEDesc myCellContainer, theData;

	err = AECreateAppleEvent(kAECoreSuite, kAESetData, theAddress, 		// Start with a Set Data appleEvent
			kAutoGenerateReturnID, 1L, &appleEvent);

	MyCreateCellContainer(cell, row, &myCellContainer, "Worksheet1");	// Append the destination descriptor
	AEPutParamDesc(&appleEvent, keyDirectObject, &myCellContainer);		// to the appleEvent

	AECreateDesc(typeChar, data, strlen(data), &theData);				// Append the data descriptor
	AEPutParamDesc(&appleEvent, keyAEData, &theData);					// to the appleEvent

	err = AESend(&appleEvent, &reply, kAENoReply + kAENeverInteract,	// Send it to Excel
			kAENormalPriority, kAEDefaultTimeout, nil, nil);

	AEDisposeDesc(&myCellContainer);
	AEDisposeDesc(&theData);
	AEDisposeDesc(&appleEvent);
	AEDisposeDesc(&reply);
}

void MyCreateDocContainer(AEDesc *myDocContainer, char *docName)
{
	AEDesc myDocDescRec, nullDescRec;
	OSErr err;

	err = AECreateDesc(typeNull, nil, 0, &nullDescRec);	// Prepare to make something from nothing 
	err = AECreateDesc(typeChar, docName, strlen(docName), 
			&myDocDescRec);				// This document container just contains the document name
	err = CreateObjSpecifier(cDocument, &nullDescRec, kAEName,
			&myDocDescRec, true, myDocContainer);		// Encapsulate it
	AEDisposeDesc(&nullDescRec);
}

void MyCreateRowContainer(char *row, AEDesc *myRowContainer, char *docName)
{
	AEDesc myDocDescRec, myRowDescRec;
	OSErr err;

	MyCreateDocContainer(&myDocDescRec, docName);		// First you have to make the doc container 
														// into which the row container fits.
	err = AECreateDesc(typeChar, row, strlen(row),
			&myRowDescRec);				// This row container just contains the row number
	err = CreateObjSpecifier(cRow, &myDocDescRec, kAEIndex,
			&myRowDescRec, true, myRowContainer);		// Encapsulate it
	AEDisposeDesc(&myDocDescRec);
}

void MyCreateCellContainer(char *cell, char *row, AEDesc *myCellContainer, char *docName)
{
	AEDesc myCellDescRec, myRowDescRec;
	OSErr err;

	MyCreateRowContainer(row, &myRowDescRec, docName);	// First you have to make the row container 
														// into which the cell container fits.
	err = AECreateDesc(typeChar, cell, strlen(cell),
			&myCellDescRec);			// This cell container just contains the cell number
	err = CreateObjSpecifier(cCell, &myRowDescRec, kAEIndex,
			&myCellDescRec, true, myCellContainer);		// Encapsulate it

	AEDisposeDesc(&myCellDescRec);
	AEDisposeDesc(&myRowDescRec);
}

// This function is from AE Interaction Sample by C.K.Haun of Apple DTS
OSErr FindAndLaunchExcel(AEAddressDesc *theAddress)
{
    OSErr myError;
	DialogPtr search = GetNewDialog(kSearch, nil, (WindowPtr)-1);
    CSParamPtr csBlockPtr = NewPtrClear(sizeof(CSParam));
    long dirIDUnused;
    Str32 nulString = "\p";

    /* initialize the parameter block */
    DrawDialog(search);

    if (csBlockPtr) {
        csBlockPtr->ioSearchInfo1 = (CInfoPBPtr)NewPtrClear(sizeof(CInfoPBRec));
        csBlockPtr->ioSearchInfo2 = (CInfoPBPtr)NewPtrClear(sizeof(CInfoPBRec));
        if (csBlockPtr->ioSearchInfo1 && csBlockPtr->ioSearchInfo2) {
            csBlockPtr->ioMatchPtr = (FSSpecPtr)NewPtrClear(sizeof(FSSpec) * 1);	/* only looking for 1 */
            if (csBlockPtr->ioMatchPtr) {
                /* Now see if we can create an optimization buffer */
                csBlockPtr->ioOptBuffer = NewPtr(2048);
                if (csBlockPtr->ioOptBuffer)
                    csBlockPtr->ioOptBufSize = 2048;
                else
                    csBlockPtr->ioOptBufSize = 0;           /* no buffer, sorry */
                csBlockPtr->ioReqMatchCount = 1;
                csBlockPtr->ioSearchTime = 0;               /* no timeout */
            }
        }
    }
    HGetVol(nil, &csBlockPtr->ioVRefNum, &dirIDUnused);     /* get default volume for search */
    csBlockPtr->ioSearchInfo1->hFileInfo.ioNamePtr = nil;
    csBlockPtr->ioSearchInfo2->hFileInfo.ioNamePtr = nil;
    csBlockPtr->ioSearchInfo1->hFileInfo.ioFlFndrInfo.fdCreator = 'XCEL';	// Hardwired to launch Excel now
    csBlockPtr->ioSearchInfo1->hFileInfo.ioFlFndrInfo.fdType = 'APPL';
    csBlockPtr->ioSearchBits = fsSBFlFndrInfo;
    csBlockPtr->ioSearchInfo2->hFileInfo.ioFlFndrInfo.fdCreator = 0xFFFFFFFF;
    csBlockPtr->ioSearchInfo2->hFileInfo.ioFlFndrInfo.fdType = 0xFFFFFFFF;
    
    myError = PBCatSearch(csBlockPtr, false);               /* search sync */
    if (myError == noErr && csBlockPtr->ioActMatchCount != 0) {
        /* we found it, so launch it */
        
        launchThis.launchBlockID = extendedBlock;
        launchThis.launchEPBLength = extendedBlockLen;
        launchThis.launchFileFlags = nil;
        launchThis.launchControlFlags = launchContinue + launchNoFileFlags + launchDontSwitch;
        launchThis.launchAppSpec = &csBlockPtr->ioMatchPtr[0];
        myError = LaunchApplication(&launchThis);
        if (myError == noErr) {
            /* it launched fine.  we can use the PSN to make a target */
            AECreateDesc(typeProcessSerialNumber, (Ptr)&launchThis.launchProcessSN, 
					sizeof(ProcessSerialNumber), theAddress);
        }
    }
    
    /* no matter what happened, kill the memory we had allocated */
    if (csBlockPtr) {
        if (csBlockPtr->ioSearchInfo1)
            DisposPtr((Ptr)csBlockPtr->ioSearchInfo1);
        if (csBlockPtr->ioSearchInfo2)
            DisposPtr((Ptr)csBlockPtr->ioSearchInfo2);
        if (csBlockPtr->ioMatchPtr)
            DisposPtr((Ptr)csBlockPtr->ioMatchPtr);
        if (csBlockPtr->ioOptBuffer)
            DisposPtr((Ptr)csBlockPtr->ioOptBuffer);
        DisposPtr((Ptr)csBlockPtr);
    }
    
    /* catsearch section end */
	DisposDialog(search);
	return(myError);
}


