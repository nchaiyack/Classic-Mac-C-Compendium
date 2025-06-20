/*
	Harvest C
	Copyright 1992 Eric W. Sink.  All rights reserved.
	
	This file is part of Harvest C.
	
	Harvest C is free software; you can redistribute it and/or modify
	it under the terms of the GNU Generic Public License as published by
	the Free Software Foundation; either version 2, or (at your option)
	any later version.
	
	Harvest C is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with Harvest C; see the file COPYING.  If not, write to
	the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
	
	Harvest C is not in any way a product of the Free Software Foundation.
	Harvest C is not GNU software.
	Harvest C is not public domain.

	This file may have other copyrights which are applicable as well.

*/

/******************************************************************************
 CErrorLog.c

		
	SUPERCLASS = CDLOGDirector
	
	Copyright � 1991 Symantec Corporation. All rights reserved.
	

 ******************************************************************************/

#include "CErrorLog.h"
#include "CStringTable.h"
#include "CStringArray.h"
#include "CScrollPane.h"
#include "CDialogText.h"
#include "CDialog.h"
#include "CPaneBorder.h"
#include "CApplication.h"
#include "CDecorator.h"
#include "CHarvestApp.h"
#include "CHarvestDoc.h"
#include "CDataFile.h"
#include "CSourceFile.h"
#include "Commands.h"
#include "HarvestCommands.h"
#include "AppleEvents.h"
#include "Aliases.h"
#include "stdarg.h"

#define	kDemoDlgID			1030
#define openFileTCLID		5000

enum		/* window item numbers	*/
{
	kListScrollPaneItem = 1,
	kEditItem
};

extern CHarvestApp	*gApplication;
extern CHarvestDoc	*gProject;
extern tSystem gSystem;
extern CSourceFile *gCurSourceFile;
extern CErrorLog *gErrs;
extern	CDecorator	*gDecorator;	/* Window dressing object	*/

extern Boolean FindAProcess(OSType signature,ProcessSerialNumber *process,
						ProcessInfoRec *InfoRec,
						FSSpecPtr aFSSpecPtr);
char *GetPathName(char *s,char *name,short vRefNum,long dirID);

/******************************************************************************
 IErrorLog
******************************************************************************/

#define maxStringLen 255

void CErrorLog::IErrorLog( void)
{
	CScrollPane	*scrollPane;
	CPaneBorder *listBorder;
	Rect	margin;
	Cell	aCell;
	
	ErrorCount = 0;

	if (gProject) {
		CDLOGDirector::IDLOGDirector( kDemoDlgID, gProject);
	}
	else {
		CDLOGDirector::IDLOGDirector( kDemoDlgID, gApplication);
	}
	
	itsEditItem = (CDialogText*) itsWindow->FindViewByID( kEditItem);
	itsEditItem->SetFontNumber(geneva);
	
	scrollPane = (CScrollPane*) itsWindow->FindViewByID( kListScrollPaneItem);
	
	if (scrollPane)
	{				
		itsStringTable = new( CStringTable);
		itsStringTable->IStringTable( scrollPane, itsWindow, 0, 0, 0, 0,
			sizELASTIC, sizELASTIC);
		itsStringTable->FitToEnclosure( TRUE, TRUE);
		itsStringTable->SetDrawActiveBorder( TRUE);
		itsStringTable->SetDblClickCmd(cmdOpenErrorLine);
		
		itsStringTable->SetID( 10);

		listBorder = new( CPaneBorder);
		listBorder->IPaneBorder( kBorderFrame);
		itsStringTable->SetBorder( listBorder);
				
		itsStringTable->SetSelectionFlags( selOnlyOne);
		
		scrollPane->InstallPanorama( itsStringTable);
		
		itsStrings = new( CStringArray);
		itsStrings->IStringArray(maxStringLen);
		itsStringTable->SetArray( itsStrings, FALSE);
		
		SetCell( aCell, 0, 0);
		itsStringTable->SelectCell( aCell, FALSE, FALSE);
	}
	
	SetupItems();
	gDecorator->StaggerWindow(itsWindow);
}	/* CErrorLog::IErrorLog */

void CErrorLog::Hprintf(char *s, ...)
{
	va_list ap;
	char *m;
	va_start(ap,s);
	m = (char *) icemalloc(maxStringLen+1);
	vsprintf(m,s,ap);
	va_end(ap);
	c2pstr(m);
	ErrorCount++;
	itsStrings->InsertAtIndex((Ptr) m,ErrorCount);
}

/******************************************************************************
 DoCommand
******************************************************************************/

void CErrorLog::DoCommand( long aCmd)
{
	Cell	selectedCell;
	Str255	string;
	Boolean haveSelection;
	
	SetCell( selectedCell, 0, 0);
	haveSelection = itsStringTable->GetSelect( TRUE, &selectedCell);
	
	switch (aCmd)
	{
		case cmdClose:
			gErrs = NULL;
			gProject->ResetGopher();
			inherited::DoCommand(aCmd);
			break;
	/* The functionality of opening error messages in Alpha has been removed */
#ifdef OLDM
		case cmdOpenErrorLine:
			/* Now we send an open message to Alpha */
			gApplication->InspectSystem();
			if (gSystem.hasAppleEvents && gCurSourceFile)
			{
				AERecord ae;
				AEDescList aeList;
				AEDesc docDesc;
				AEAddressDesc alfa;
				OSType theSig = 'ALFA';
				OSErr err;
				FSSpec theSpec;
				char theScript[512];
				char theName[64];
				char justName[64];
				short itsVol;
				long itsDir;
				long theLine;
				int ndx;
				char path[512];
				Cell aCell = {0,0};
				Boolean thereIsASelection = false;
				StringHandle	openScript;

				if (itsStringTable->GetSelect(false,&aCell)) {
					thereIsASelection = true;
				}
				else if (itsStringTable->GetSelect(true,&aCell)) {
					thereIsASelection = true;
				}
				itsStringTable->GetCellText(aCell,63,(StringPtr) theName);
				p2cstr(theName);
				if (theName[0] == '<') {
					/* Error appears in System Header */
					ndx = 1;
					while (theName[ndx] != '>') {
						justName[ndx-1] = theName[ndx];
						ndx++;
					}
					justName[ndx] = 0;
					itsVol = gApplication->StdIncludeVol;
					itsDir = gApplication->StdIncludeDir;
				}
				else if (theName[0] == '\"') {
					/* Error appears in user header or file */
					ndx = 1;
					while (theName[ndx] != '\"') {
						justName[ndx-1] = theName[ndx];
						ndx++;
					}
					justName[ndx] = 0;
					itsVol = gProject->itsFile->volNum;
					itsDir = gProject->itsFile->dirID;
				}
				else {
					justName[0] = 0;
				}
#ifdef USE_TCL
				if (justName[0] && gSystem.hasAppleEvents) {
					GetPathName(path,justName,itsVol,itsDir);
					p2cstr(path);
					while (!isdigit(theName[ndx])) ndx++; /* skip the colon */
					theLine = atoi(theName+ndx);
			
					openScript = GetString(openFileTCLID);
					FailNILRes( openScript);
					HLock(openScript);
					sprintf(theScript,(char *) (*openScript),path,theLine);
					HUnlock(openScript);
					
					err = AECreateDesc(typeApplSignature,(Ptr) &theSig,(Size) sizeof(theSig),&alfa);
					FailOSErr(err);
					err = AECreateAppleEvent(kTclClass,kAEEval,&alfa,kAutoGenerateReturnID,
						kAnyTransactionID,&ae);
					FailOSErr(err);
					err = AECreateList(NULL,0,FALSE,&aeList);
					FailOSErr(err);
					err = AECreateDesc(typeCString,(Ptr) theScript, strlen(theScript)+1,
						&docDesc);
					FailOSErr(err);
					err = AEPutDesc(&aeList,0,&docDesc);
					FailOSErr(err);
					err = AEPutParamDesc(&ae,keyDirectObject,&aeList);
					FailOSErr(err);
					err = AESend(&ae,NULL,kAENoReply,kAENormalPriority,0,NULL,NULL);
					FailOSErr(err);
					if (!err) {
						ProcessSerialNumber process;
						ProcessInfoRec InfoRec;
						FSSpec theSpec;
						if (FindAProcess('ALFA',&process,&InfoRec,&theSpec)) {
							SetFrontProcess(&process);
						}
					}
				}
#else
				if (justName[0]) {
				if (gSystem.hasAppleEvents && theSig)
				{
					AliasHandle withThis;
					
					GetPathName(path,justName,itsVol,itsDir);
					FSMakeFSSpec(itsVol,
								itsDir,
								path,
								&theSpec);
					err = AECreateDesc(typeApplSignature,(Ptr) &theSig,(Size) sizeof(theSig),&alfa);
					FailOSErr(err);
					err = AECreateAppleEvent(kCoreEventClass,kAEOpenDocuments,&alfa,kAutoGenerateReturnID,
						kAnyTransactionID,&ae);
					FailOSErr(err);
					err = AECreateList(NULL,0,FALSE,&aeList);
					FailOSErr(err);
					NewAlias(NULL,&theSpec,&withThis);
					HLock((Handle) withThis);
					err = AECreateDesc(typeAlias,(Ptr) *withThis, GetHandleSize((Handle) withThis),
						&docDesc);
					FailOSErr(err);
					HUnlock((Handle) withThis);
					err = AEPutDesc(&aeList,0,&docDesc);
					FailOSErr(err);
					err = AEPutParamDesc(&ae,keyDirectObject,&aeList);
					FailOSErr(err);
					err = AESend(&ae,NULL,kAENoReply,kAENormalPriority,0,NULL,NULL);
					FailOSErr(err);
					if (!err) {
						ProcessSerialNumber process;
						ProcessInfoRec InfoRec;
						FSSpec theSpec;
						if (FindAProcess(theSig,&process,&InfoRec,&theSpec)) {
							SetFrontProcess(&process);
						}
					}
				}
				}
#endif
				}
#endif /* OLDM */
			break;
		default: 
				inherited::DoCommand( aCmd);
				break;
	}
				
}	/* CErrorLog::DoCommand */

/******************************************************************************
 SetupItems
******************************************************************************/

void CErrorLog::SetupItems( void)
{
	Cell	selectedCell;
	Boolean haveSelection;
	CDialog	*dialog;
	Str255	str;

	/* determine if any cells are now selected	*/
	
	SetCell( selectedCell, 0, 0);
	haveSelection = itsStringTable->GetSelect( TRUE, &selectedCell);
	
	dialog = (CDialog*) itsWindow;
	
	if (haveSelection)
		itsStrings->GetItem( str, selectedCell.v+1);
	else
		str[0] = 0;

	itsEditItem->SetTextString( str);
	itsEditItem->SelectAll( kRedraw);

}	/* CErrorLog::SetupItems */

/******************************************************************************
 ProviderChanged
******************************************************************************/

void CErrorLog::ProviderChanged( CCollaborator *aProvider, long reason,
										void *info)
{
	Cell	selectedCell;
	Boolean haveSelection;

	if ((aProvider == itsStringTable) && (reason == tableSelectionChanged))
	{
		SetupItems();
	}
	else
		inherited::ProviderChanged( aProvider, reason, info);

}	/* CErrorLog::ProviderChanged */

/******************************************************************************
 Dispose
******************************************************************************/

void CErrorLog::Dispose( void)
{
	ForgetObject( itsStrings);
	ForgetObject( itsStringTable);
	inherited::Dispose();
}

