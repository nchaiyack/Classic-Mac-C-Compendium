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

/****
 * CHarvestDoc.c
 *
 *	Document methods for Harvest C
 *
 *  Copyright © 1990 Symantec Corporation.  All rights reserved.
 *
 ****/

#include "Global.h"
#include "Constants.h"
#include "Commands.h"
#include "CApplication.h"
#include "CBartender.h"
#include "CDataFile.h"
#include "CDecorator.h"
#include "CDesktop.h"
#include "CError.h"
#include "CPanorama.h"
#include "CScrollPane.h"
#include "CHarvestApp.h"
#include "CHarvestDoc.h"
#include "CHarvestOptions.h"
#include "CHarvestPane.h"
#include "COptionsDialog.h"
#include "CProjectInfoDialog.h"
#include "CCheckBox.h"
#include "CRadioControl.h"
#include "CRadioGroupPane.h"
#include "CWarningsDialog.h"
#include "CWarningsArray.h"
#include "CDialogText.h"
#include "CIntegerText.h"
#include "TBUtilities.h"
#include "CWindow.h"
#include "CFile.h"
#include "CList.h"
#include "CResourceFile.h"
#include "CSourceFile.h"
#include "CLibraryFile.h"
#include <Packages.h>
#include "AppleEvents.h"
#include "Aliases.h"
#include "HarvestCommands.h"
#include "CErrorLog.h"
#include "structs.h"
#include "linkstruct.h"

#define	WINDHarvest		500		/* Resource ID for WIND template */

extern	CHarvestApp *gApplication;	/* The application */
extern	tSystem		gSystem;
extern	CHarvestDoc	*gProject;
extern	CErrorLog	*gErrs;
extern	CSourceFile *gCurSourceFile;
extern	CBartender	*gBartender;	/* The menu handling object */
extern	CDecorator	*gDecorator;	/* Window dressing object	*/
extern	CDesktop	*gDesktop;		/* The enclosure for all windows */
extern	CBureaucrat	*gGopher;		/* The current boss in the chain of command */
extern	OSType		gSignature;		/* The application's signature */
extern	CError		*gError;		/* The global error handler */
extern	int                             LinkErrorCount;
extern	SegmentVia_t                    SegmentList;
extern	DATAZone_t                      theDATA;
extern	CursHandle			gWatchCursor;		/* Watch cursor for waiting			*/

enum {
	kTrigraphsItem = 4,
	kBigGlobalsItem,
	k68020Item,
	k68881Item,
	kSignedCharItem,
	kMacsBugitem
};

enum {
	kSigFieldItem = 3,
	kPartitionItem,
	kSizeFlagsItem
};

Boolean FindAProcess(OSType signature,ProcessSerialNumber *process,
						ProcessInfoRec *InfoRec,
						FSSpecPtr aFSSpecPtr)
{
	process->highLongOfPSN = 0;
	process->lowLongOfPSN = kNoProcess;
	
	InfoRec->processInfoLength = sizeof(ProcessInfoRec);
	InfoRec->processName = (StringPtr) NewPtr(32);
	InfoRec->processAppSpec = aFSSpecPtr;
	
	while (GetNextProcess(process) == noErr) {
		if (GetProcessInformation(process,InfoRec) == noErr) {
			if ((InfoRec->processType == 'APPL') &&
				(InfoRec->processSignature == signature)) {
				return TRUE;
			}
		}
	}
	return FALSE;
}



/***
 * IHarvestDoc
 *
 *	This is your document's initialization method.
 *	If your document has its own instance variables, initialize
 *	them here.
 *
 *	The least you need to do is invoke the default method.
 *
 ***/

void CHarvestDoc::IHarvestDoc(CApplication *aSupervisor, Boolean printable)

{
	CDocument::IDocument(aSupervisor, printable);

	itsSourceFiles = new(CList);
	itsSourceFiles->IList();
	itsOptions = new CHarvestOptions;
	itsOptions->IHarvestOptions();
	itsWarnings = new CWarningsArray;
	itsWarnings->IWarningsArray(itsOptions);
	itsWarnings->AddWarning("\pEmpty expression stmt",WARN_emptystatement,TRUE);
	itsWarnings->AddWarning("\pMulti-character constant",WARN_multicharconstant,FALSE);
	itsWarnings->AddWarning("\pRedundant cast",WARN_redundantcast,FALSE);
	itsWarnings->AddWarning("\pEquivalence test of floating type",WARN_floateqcompare,TRUE);
	itsWarnings->AddWarning("\pDiscarded function result",WARN_discardfuncresult,FALSE);
	itsWarnings->AddWarning("\pAssignment as if conditional",WARN_assignif,TRUE);
	itsWarnings->AddWarning("\pNon-void function has no return statement",WARN_nonvoidreturn,TRUE);
	itsWarnings->AddWarning("\pConstant expression as if condition",WARN_constantif,TRUE);
	itsWarnings->AddWarning("\pComparison of pointer and integer",WARN_comparepointerint,TRUE);
	itsWarnings->AddWarning("\pAssignment of nonequivalent type to a pointer",WARN_pointernonequivassign,TRUE);
	itsWarnings->AddWarning("\pReturn of nonequivalent type to a pointer",WARN_pointernonequivreturn,TRUE);
	itsWarnings->AddWarning("\pPass of nonequivalent type to a pointer",WARN_pointernonequivarg,TRUE);
	itsWarnings->AddWarning("\pConstant expression as switch condition",WARN_constantswitch,TRUE);
	itsWarnings->AddWarning("\pConstant expression as while loop condition",WARN_constantwhile,TRUE);
	itsWarnings->AddWarning("\pConstant expression as do-while loop condition",WARN_constantdowhile,TRUE);
	itsWarnings->AddWarning("\pConstant expression as for loop condition",WARN_constantfor,TRUE);
	itsWarnings->AddWarning("\pSwitch expression not of integral type",WARN_nonintegralswitch,TRUE);
	itsWarnings->AddWarning("\pVolatile is not handled by this compiler",WARN_novolatile,TRUE);
	itsWarnings->AddWarning("\pUnused variable: ",WARN_unusedvariable,TRUE);
	itsWarnings->AddWarning("\pDead code",WARN_deadcode,TRUE);
	itsWarnings->AddWarning("\pImplicit decl : ",WARN_implicitdecl,FALSE);
	itsWarnings->AddWarning("\pRe#definition : ",WARN_preprocredef,FALSE);
	itsWarnings->AddWarning("\pPossible nested comment",WARN_nestedcomment,TRUE);
	itsWarnings->AddWarning("\pgoto statement found",WARN_goto,TRUE);
	itsWarnings->AddWarning("\pMultiple function returns",WARN_multireturn,TRUE);
	itsWarnings->AddWarning("\pEmpty compound statement",WARN_emptycompound,TRUE);
	itsWarnings->AddWarning("\pMissing function return type - default to int",WARN_missingreturntype,TRUE);
	itsWarnings->AddWarning("\pTrigraph found",WARN_trigraphs,TRUE);
	itsWarnings->AddWarning("\ppascal keyword found",WARN_pascal,FALSE);
	itsWarnings->AddWarning("\pSemicolon after function body",WARN_semiafterfunction,TRUE);

	itsPrefix = NULL;
	StdAppVol = -1;
	StdAppDir = -1;
	CopyPString("\papp.out",StdAppName);
	itsPartition = 384*1024;
	itsSignature = '????';
	itsSizeFlags = 0;
	gProject = this;
}

/***
 * AddLibraryFileHFS
 *
 *
 ***/

void CHarvestDoc::AddLibraryFileHFS(Str63 aName,short aVolNum,long aDirID)

{
	CDataFile *newFile;
	CLibraryFile *newLibraryFile;
	newFile = new(CDataFile);
	newFile->IDataFile();
	newFile->SpecifyHFS(aName,aVolNum,aDirID);
	newLibraryFile = new(CLibraryFile);
	newLibraryFile->ILibraryFile(newFile);
	itsSourceFiles->Append((CObject *) newLibraryFile);
	itsTable->AddRow(1,-1);
}

/***
 * AddSourceFileHFS
 *
 *
 ***/

void CHarvestDoc::AddSourceFileHFS(Str63 aName,short aVolNum,long aDirID)

{
	CDataFile *newFile;
	CSourceFile *newSourceFile;
	newFile = new(CDataFile);
	newFile->IDataFile();
	newFile->SpecifyHFS(aName,aVolNum,aDirID);
	newSourceFile = new(CSourceFile);
	newSourceFile->ISourceFile(newFile);
	itsSourceFiles->Append((CObject *) newSourceFile);
	itsTable->AddRow(1,-1);
}

/***
 * AddSourceFile
 *
 *
 ***/

void CHarvestDoc::AddSourceFile(SFReply *theFile)

{
	CDataFile *newFile;
	FInfo fileInfo;
	CSourceFile *newSourceFile;
	CLibraryFile *newLibFile;
	CResourceFile *newResourceFile;
	newFile = new(CDataFile);
	newFile->IDataFile();
	newFile->SFSpecify(theFile);
	/* Here, we check the format of the name and the file type, and add the
	   appropriate object, library/source/resource to the project */
	newFile->GetMacFileInfo(&fileInfo);
	if (fileInfo.fdType == 'TEXT') {
		newSourceFile = new(CSourceFile);
		newSourceFile->ISourceFile(newFile);
		itsSourceFiles->Append((CObject *) newSourceFile);
		itsTable->AddRow(1,-1);
	}
	else if (fileInfo.fdType == 'OBJ ') {
		newLibFile = new(CLibraryFile);
		newLibFile->ILibraryFile(newFile);
		itsSourceFiles->Append((CObject *) newLibFile);
		itsTable->AddRow(1,-1);
	}
	else if (fileInfo.fdCreator == 'RSED') {
		newResourceFile = new(CResourceFile);
		newResourceFile->IResourceFile(newFile);
		itsSourceFiles->Append((CObject *) newResourceFile);
		itsTable->AddRow(1,-1);
	}
	else if (fileInfo.fdType == 'rsrc') {
		newResourceFile = new(CResourceFile);
		newResourceFile->IResourceFile(newFile);
		itsSourceFiles->Append((CObject *) newResourceFile);
		itsTable->AddRow(1,-1);
	}
}

/***
 * AddResourceFileHFS
 *
 *
 ***/

void CHarvestDoc::AddResourceFileHFS(Str63 aName,short aVolNum,long aDirID)

{
	CDataFile *newFile;
	CResourceFile *newResourceFile;
	newFile = new(CDataFile);
	newFile->IDataFile();
	newFile->SpecifyHFS(aName,aVolNum,aDirID);
	newResourceFile = new(CResourceFile);
	newResourceFile->IResourceFile(newFile);
	itsSourceFiles->Append((CObject *) newResourceFile);
	itsTable->AddRow(1,-1);
}

/***
 * AddResourceFile
 *
 *
 ***/

void CHarvestDoc::AddResourceFile(SFReply *theFile)

{
	CDataFile *newFile;
	CResourceFile *newResourceFile;
	newFile = new(CDataFile);
	newFile->IDataFile();
	newFile->SFSpecify(theFile);
	newResourceFile = new(CResourceFile);
	newResourceFile->IResourceFile(newFile);
	itsSourceFiles->Append((CObject *) newResourceFile);
	itsTable->AddRow(1,-1);
}

/***
 * AddLibFile
 *
 *
 ***/

void CHarvestDoc::AddLibFile(SFReply *theFile)

{
	CDataFile *newFile;
	CLibraryFile *newSourceFile;
	newFile = new(CDataFile);
	newFile->IDataFile();
	newFile->SFSpecify(theFile);
	newSourceFile = new(CLibraryFile);
	newSourceFile->ILibraryFile(newFile);
	itsSourceFiles->Append((CObject *) newSourceFile);
	itsTable->AddRow(1,-1);
}

/***
 * Dispose
 *
 *	This is your document's destruction method.
 *	If you allocated memory in your initialization method
 *	or opened temporary files, this is the place to release them.
 *
 *	Be sure to call the default method!
 *
 ***/

void CHarvestDoc::Dispose(void)

{
#ifdef Undefined	/* caused illegal inst 5 March */
	itsSourceFiles->Dispose();
	itsOptions->Dispose();
	itsWarnings->Dispose();
#endif
	inherited::Dispose();
}

void CHarvestDoc::DoOptions(CHarvestOptions *theOpts)
{
	COptionsDialog *dialog;
	long cmd;
	CCheckBox *trigraphsBox;
	CCheckBox *globalsBox;
	CCheckBox *mc68020Box;
	CCheckBox *mc68881Box;
	CCheckBox *signedCharsBox;
	CCheckBox *MacsBugBox;

	dialog = new COptionsDialog;
	dialog->IOptionsDialog(500,this);
	
	/* Now initialize states of the boxes */
	trigraphsBox = (CCheckBox *) dialog->itsWindow->FindViewByID( kTrigraphsItem);
	globalsBox = (CCheckBox *) dialog->itsWindow->FindViewByID( kBigGlobalsItem);
	mc68020Box = (CCheckBox *) dialog->itsWindow->FindViewByID( k68020Item);
	mc68881Box = (CCheckBox *) dialog->itsWindow->FindViewByID( k68881Item);
	signedCharsBox = (CCheckBox *) dialog->itsWindow->FindViewByID( kSignedCharItem);
	MacsBugBox = (CCheckBox *) dialog->itsWindow->FindViewByID( kMacsBugitem);
	
	trigraphsBox->SetValue(theOpts->trigraphs ? BUTTON_ON : BUTTON_OFF);
	globalsBox->SetValue(theOpts->bigGlobals ? BUTTON_ON : BUTTON_OFF);
	mc68020Box->SetValue(theOpts->useMC68020 ? BUTTON_ON : BUTTON_OFF);
	mc68881Box->SetValue(theOpts->useMC68881 ? BUTTON_ON : BUTTON_OFF);
	signedCharsBox->SetValue(theOpts->signedChars ? BUTTON_ON : BUTTON_OFF);
	MacsBugBox->SetValue(theOpts->MacsBugSymbols ? BUTTON_ON : BUTTON_OFF);

	
	dialog->BeginDialog();
	cmd = dialog->DoModalDialog(cmdOK);
	if (cmd == cmdOK)
	{
		theOpts->trigraphs = trigraphsBox->IsChecked();
		theOpts->bigGlobals = globalsBox->IsChecked();
		theOpts->useMC68020 = mc68020Box->IsChecked();
		theOpts->useMC68881 = mc68881Box->IsChecked();
		theOpts->signedChars = signedCharsBox->IsChecked();
		theOpts->MacsBugSymbols = MacsBugBox->IsChecked();
	}
	ForgetObject(dialog);
}

void CHarvestDoc::DoWarnings(CHarvestOptions *theOpts,CWarningsArray *theWarnings)
{
	CWarningsDialog *dialog;
	long cmd;

	dialog = new CWarningsDialog;
	dialog->IWarningsDialog(theOpts,theWarnings);
	
	dialog->BeginDialog();
	cmd = dialog->DoModalDialog(cmdOK);
	if (cmd == cmdOK)
	{
	}
	ForgetObject(dialog);
}

char *OSTypeToString(OSType val,char *s)
{
	*((long *) s) = val;
	s[4] = 0;
	return s;
}

void CHarvestDoc::DoSetProjectInfo(void)
{
	Str255 theSig;
	CProjectInfoDialog *dialog;
	long cmd;
	CDialogText *sigField;
	CIntegerText *partitionField;
	CIntegerText *sizeFlagsField;
	char sigString[8];

	dialog = new CProjectInfoDialog;
	dialog->IProjectInfoDialog(501,this);
	
	sigField = (CDialogText *) dialog->itsWindow->FindViewByID( kSigFieldItem);
	partitionField = (CIntegerText *) dialog->itsWindow->FindViewByID( kPartitionItem);
	sizeFlagsField = (CIntegerText *) dialog->itsWindow->FindViewByID( kSizeFlagsItem);

	/* Now initialize states of the textfields */
	
	OSTypeToString(itsSignature,sigString);
	c2pstr(sigString);
	sigField->SetTextString((unsigned char *) sigString);
	partitionField->SetIntValue(itsPartition/1024);
	sizeFlagsField->SetIntValue(itsSizeFlags);

	dialog->BeginDialog();
	cmd = dialog->DoModalDialog(cmdOK);
	if (cmd == cmdOK)
	{
		sigField->GetTextString(theSig);
		p2cstr(theSig);
		itsSignature = MakeOSType((char *) theSig);
		itsPartition = partitionField->GetIntValue()*1024;
		itsSizeFlags = sizeFlagsField->GetIntValue();
	}
	ForgetObject(dialog);
}

/***
 * DoCommand
 *
 *	This is the heart of your document.
 *	In this method, you handle all the commands your document
 *	deals with.
 *
 *	Be sure to call the default method to handle the standard
 *	document commands: cmdClose, cmdSave, cmdSaveAs, cmdRevert,
 *	cmdPageSetup, cmdPrint, and cmdUndo. To change the way these
 *	commands are handled, override the appropriate methods instead
 *	of handling them here.
 *
 ***/

void CHarvestDoc::DoCommand(long theCommand)

{
	SFReply theFile;
	Cell aCell = {0,0};
	Boolean thereIsASelection = false;
			if (itsTable->GetSelect(false,&aCell)) {
				thereIsASelection = true;
			}
			else if (itsTable->GetSelect(true,&aCell)) {
				thereIsASelection = true;
			}
	ResetGopher();
	switch (theCommand) {

		case cmdAddCFile:
				gApplication->ChooseSourceFile(&theFile);
				if (theFile.good) {
					AddSourceFile(&theFile);
				}
				break;
		case cmdRemove:
			/* Remove the currently selected file */
			if (thereIsASelection) {
				/* Delete the item from the itsSourceFiles list and
					the row from the table */
				itsSourceFiles->Remove(itsSourceFiles->NthItem(aCell.v+1));
				itsTable->DeleteRow(1,aCell.v);
				itsTable->DeselectAll(true);
			}
		break;
		case cmdOpenSourceFile:
			if (thereIsASelection) {
				OSType theSig;
				CSourceFile *theSourceFile = (CSourceFile *) itsSourceFiles->NthItem(aCell.v+1);
				theSig = 0;
				if (theSourceFile->itsKind == H_ResourceFile) theSig = 'RSED';
				else if (theSourceFile->itsKind == H_SourceFile) theSig = 'ALFA';
				else theSig = 0;
				gApplication->InspectSystem();
				if (gSystem.hasAppleEvents && theSig)
				{
					AERecord ae;
					AEDescList aeList;
					AEDesc docDesc;
					AEAddressDesc alfa;
					AliasHandle withThis;
					OSErr err;
					FSSpec theSpec;
					
					FSMakeFSSpec(theSourceFile->theFile->volNum,
								theSourceFile->theFile->dirID,
								theSourceFile->theFile->name,
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
		break;
		case cmdCompile:
			/* Compile the currently selected file */
			if (thereIsASelection) {
				CSourceFile *theFile;
				theFile = ((CSourceFile *) itsSourceFiles->NthItem(aCell.v+1));
				if (theFile->itsKind == H_SourceFile) {
					theFile->Compile();
				}
			}
		break;
		case cmdSetProjectInfo:
			DoSetProjectInfo();
		break;
		case cmdOptions:
			DoOptions(itsOptions);
		break;
		case cmdWarnings:
			DoWarnings(itsOptions,itsWarnings);
		break;
		case cmdSave:
			SysBeep(1);
			break;
		case cmdCloseProject:
		case cmdClose:
			this->Close(false);
			break;
	
		case cmdBringUpToDate:
			BringUpToDate();
			break;
		case cmdRun:
			RunApp();
			break;
		case cmdBuildApplication:
			Link();
			break;
		case cmdCheckLink:
			CheckLink();
			break;
		case cmdClean:
			Clean();
			break;

		default:	inherited::DoCommand(theCommand);
					break;
	}
}


/***
 * UpdateMenus
 *
 *  In this method you can enable menu commands that apply when
 *  your document is active.
 *
 *  Be sure to call the inherited method to get the default behavior.
 *  The inherited method enables these commands: cmdClose, cmdSaveAs,
 *  cmdSave, cmdRevert, cmdPageSetup, cmdPrint, cmdUndo.
 *
***/

 void CHarvestDoc::UpdateMenus()

 {
 	Cell aCell = {0,0};
 	Boolean thereIsASelection = false;
	CSourceFile *theSourceFile;
	inherited::UpdateMenus();

    /* Enable your menu commands here (enable each one with a call to   
       gBartender->EnableCmd(command_number)).  
    */                      
	gBartender->EnableCmd(cmdAddCFile);
	/* others in the Sources menu will be enabled when there are actually
		things in the project */
	if (itsTable->GetSelect(false,&aCell)) {
		thereIsASelection = true;
	}
	else if (itsTable->GetSelect(true,&aCell)) {
		thereIsASelection = true;
	}
	theSourceFile = (CSourceFile *) itsSourceFiles->NthItem(aCell.v+1);
	if (thereIsASelection) {
		gBartender->EnableCmd(cmdRemove);
		gBartender->DisableCmd(cmdGetInfo);
		if (theSourceFile->itsKind == H_SourceFile) {
			/* TODO enable these when they are implemented */
			gBartender->DisableCmd(cmdCheckSyntax);
			gBartender->DisableCmd(cmdPreprocess);
			gBartender->EnableCmd(cmdCompile);
			gBartender->EnableCmd(cmdOpenSourceFile);
		}
		else if (theSourceFile->itsKind == H_ResourceFile) {
			gBartender->EnableCmd(cmdOpenSourceFile);
		}
	}
		
	gBartender->DisableCmd(cmdSave);   
	gBartender->DisableCmd(cmdSaveAs);
	gBartender->DisableCmd(cmdNewProject);
	gBartender->DisableCmd(cmdNew);
	gBartender->DisableCmd(cmdOpenProject);
	gBartender->DisableCmd(cmdOpen);
	gBartender->EnableCmd(cmdOptions);
	gBartender->EnableCmd(cmdWarnings);
	gBartender->EnableCmd(cmdCloseProject);
	gBartender->EnableCmd(cmdClose);
	gBartender->EnableCmd(cmdSetProjectInfo);
	gBartender->EnableCmd(cmdClean);
	gBartender->EnableCmd(cmdBuildApplication);
	gBartender->DisableCmd(cmdCheckLink);
	gBartender->EnableCmd(cmdBringUpToDate);
	gBartender->EnableCmd(cmdRun);
	gBartender->DisableCmd(cmdPrint);
 }


/***
 * FSNewFile
 *
 ***/
void CHarvestDoc::FSNewFile(char *name, short vRefNum, long dirID)

{    
    Str255  wTitle;     /* Window title string.         */
    short   wCount;     /* Index number of new window.  */
    Str255  wNumber;    /* Index number as a string.    */
    CDataFile *theFile;
    
		/**
		 **	BuildWindow() is the method that
		 **	does the work of creating a window.
         ** Its parameter should be the data that
		 **	you want to display in the window.
		 **	Since this is a new window, there's nothing
		 **	to display.
		 **
		 **/

	BuildWindow(NULL);

	theFile = new(CDataFile);
	theFile->IDataFile();
	theFile->SpecifyHFS((unsigned char *) name,vRefNum,dirID);
	
    
        /**
         ** Append an index number to the
         ** default name of the window.
         **/
    theFile->GetName(wTitle);
    if (theFile->ExistsOnDisk())
    	theFile->ThrowOut();
    theFile->CreateNew(gSignature,'Hprj');
    theFile->Open(fsWrPerm);

    itsFile = theFile;
    itsWindow->SetTitle(wTitle);

		/**
		 **	Send the window a Select() message to make
		 **	it the active window.
		 **/
	
	itsWindow->Select();
	
	if (gApplication->StdLibVol || gApplication->StdLibDir) {
		AddLibraryFileHFS("\pInterface.o",gApplication->StdLibVol,
			gApplication->StdLibDir);
		AddLibraryFileHFS("\pRuntime.o",gApplication->StdLibVol,
			gApplication->StdLibDir);
	}
}

/***
 * NewFile
 *
 *	When the user chooses New from the File menu, the CreateDocument()
 *	method in your Application class will send a newly created document
 *	this message. This method needs to create a new window, ready to
 *	work on a new document.
 *
 *	Since this method and the OpenFile() method share the code for creating
 *	the window, you should use an auxiliary window-building method.
 *
 ***/
void CHarvestDoc::NewFile(void)

{    
    Str255  wTitle;     /* Window title string.         */
    short   wCount;     /* Index number of new window.  */
    Str255  wNumber;    /* Index number as a string.    */
    SFReply macSFReply;
    CDataFile *theFile;
    
		/**
		 **	BuildWindow() is the method that
		 **	does the work of creating a window.
         ** Its parameter should be the data that
		 **	you want to display in the window.
		 **	Since this is a new window, there's nothing
		 **	to display.
		 **
		 **/

	PickFileName(&macSFReply);
	if (macSFReply.good) {
		BuildWindow(NULL);
	
		theFile = new(CDataFile);
		theFile->IDataFile();
		theFile->SFSpecify(&macSFReply);
		
	    
	        /**
	         ** Append an index number to the
	         ** default name of the window.
	         **/
	    theFile->GetName(wTitle);
	    if (theFile->ExistsOnDisk())
	    	theFile->ThrowOut();
	    theFile->CreateNew(gSignature,'Hprj');
	    theFile->Open(fsWrPerm);
	
	    itsFile = theFile;
	    itsWindow->SetTitle(wTitle);
	
			/**
			 **	Send the window a Select() message to make
			 **	it the active window.
			 **/
		
		itsWindow->Select();
		
		if (gApplication->StdLibVol || gApplication->StdLibDir) {
			AddLibraryFileHFS("\pInterface.o",gApplication->StdLibVol,
				gApplication->StdLibDir);
			AddLibraryFileHFS("\pRuntime.o",gApplication->StdLibVol,
				gApplication->StdLibDir);
		}
	}
}


/***
 * OpenFile
 *
 *	When the user chooses OpenÉ from the File menu, the OpenDocument()
 *	method in your Application class will let the user choose a file
 *	and then send a newly created document this message. The information
 *	about the file is in the SFReply record.
 *
 *	In this method, you need to open the file and display its contents
 *	in a window. This method uses the auxiliary window-building method.
 *
 ***/

void CHarvestDoc::OpenFile(SFReply *macSFReply)

{
	CDataFile	*theFile;
	Handle		theData = NULL;
	Str63		theName;
	FInfo		fileInfo;
	OSErr		theError;
	
	TRY
	{
	
			/**
			 ** Create a file and send it a SFSpecify()
			 **	message to set up the name, volume, and
			 **	directory.
			 **
			 **/
	
		theFile = new(CDataFile);
		theFile->IDataFile();
		theFile->SFSpecify(macSFReply);
		theFile->GetMacFileInfo(&fileInfo);
		if (fileInfo.fdType != 'Hprj') {
			Failure(kSilentErr,0);
		}
			/**
			 **	Be sure to set the instance variable
			 **	so other methods can use the file if they
			 **	need to. This is especially important if
			 **	you leave the file open in this method.
			 **	If you close the file after reading it, you
			 **	should be sure to set itsFile to NULL.
			 **
			 **/
	
		itsFile = theFile;

			/**
			 **	Send the file an Open() message to
			 **	open it. You can use the ReadSome() or
			 **	ReadAll() methods to get the contents of the file.
			 **
			 **/
	
		theFile->Open(fsRdWrPerm);
		
			/**
			 **	Make sure that the memory request to read
			 **	the data from the file doesn't use up any
			 **	of our rainy day fund and that the GrowMemory()
			 **	method (in the application) knows that it's OK
			 **	if we couldn't get enough memory.
			 **
			 **/
	
	
	    theData = theFile->ReadAll();     /* ReadAll() creates the handle */
	    
	    if (!theData) {
			Failure(kSilentErr,0);
		}
		
		BuildWindow(theData);
	
		if (StdAppName[0] == 0) {
		    itsFile->GetName(StdAppName);
		}
	    if (StdAppName[StdAppName[0]] == '¹') {
	  	  if (StdAppName[StdAppName[0]-1] == '.') {
	  	  	StdAppName[0] -= 2;
	  	  }
	  	  else {
	  	  	StdAppName[0] -= 1;
	  	  }
	    }
			/**
			 **	In your application, you'll probably store
			 **	the data in some form as an instance variable
			 **	in your document class. For this example, there's
			 **	no need to save it, so we'll get rid of it.
			 **
			 **/
	
		DisposHandle(theData);
		theData = NULL;
	
			/**
			 **	In this implementation, we leave the file
			 **	open. You might want to close it after
			 **	you've read in all the data.
			 **
			 **/
	
		itsFile->GetName(theName);
		itsWindow->SetTitle(theName);
		itsWindow->Select();			/* Don't forget to make the window active */
	}
	
	CATCH
	{
		/*
		 * This exception handler will be executed if an exception occurs
		 * anywhere within the scope of the TRY block above.
		 * You should perform any cleanup of things that won't be needed
		 * since the document could not be opened. By convention,
		 * the creator of an object is responsible for sending it
		 * the Dispose message. This means that we should only dispose
		 * of things that would not be taken care of in Dispose.
		 * In this case, we just make sure that the Handle theData
		 * has been disposed of. The exception will propagate up to
		 * CApplications's exception handler, which handles displaying
		 * an error alert.
		 */
		 
		 if (theData) DisposHandle( theData);
		 
	}
	ENDTRY;
}

/***
 * FSOpenFile
 *
 *	When the user chooses OpenÉ from the File menu, the OpenDocument()
 *	method in your Application class will let the user choose a file
 *	and then send a newly created document this message. The information
 *	about the file is in the SFReply record.
 *
 *	In this method, you need to open the file and display its contents
 *	in a window. This method uses the auxiliary window-building method.
 *
 ***/

void CHarvestDoc::FSOpenFile(char *name, short vRefNum, long dirID)

{
	CDataFile	*theFile;
	Handle		theData = NULL;
	Str63		theName;
	FInfo		fileInfo;
	OSErr		theError;
	
	TRY
	{
	
			/**
			 ** Create a file and send it a SFSpecify()
			 **	message to set up the name, volume, and
			 **	directory.
			 **
			 **/
	
		theFile = new(CDataFile);
		theFile->IDataFile();
		theFile->SpecifyHFS((unsigned char *) name,vRefNum,dirID);
		theFile->GetMacFileInfo(&fileInfo);
		if (fileInfo.fdType != 'Hprj') {
			gProject = NULL;
			return;
		}
			/**
			 **	Be sure to set the instance variable
			 **	so other methods can use the file if they
			 **	need to. This is especially important if
			 **	you leave the file open in this method.
			 **	If you close the file after reading it, you
			 **	should be sure to set itsFile to NULL.
			 **
			 **/
	
		itsFile = theFile;

			/**
			 **	Send the file an Open() message to
			 **	open it. You can use the ReadSome() or
			 **	ReadAll() methods to get the contents of the file.
			 **
			 **/
	
		theFile->Open(fsRdWrPerm);
		
			/**
			 **	Make sure that the memory request to read
			 **	the data from the file doesn't use up any
			 **	of our rainy day fund and that the GrowMemory()
			 **	method (in the application) knows that it's OK
			 **	if we couldn't get enough memory.
			 **
			 **/
	
	
	    theData = theFile->ReadAll();     /* ReadAll() creates the handle */
	    
	    if (!theData) {
			Failure(kSilentErr,0);
		}
		
		BuildWindow(theData);
	
		if (StdAppName[0] == 0) {
		    itsFile->GetName(StdAppName);
		}
	    if (StdAppName[StdAppName[0]] == '¹') {
	  	  if (StdAppName[StdAppName[0]-1] == '.') {
	  	  	StdAppName[0] -= 2;
	  	  }
	  	  else {
	  	  	StdAppName[0] -= 1;
	  	  }
	    }
			/**
			 **	In your application, you'll probably store
			 **	the data in some form as an instance variable
			 **	in your document class. For this example, there's
			 **	no need to save it, so we'll get rid of it.
			 **
			 **/
	
		DisposHandle(theData);
		theData = NULL;
	
			/**
			 **	In this implementation, we leave the file
			 **	open. You might want to close it after
			 **	you've read in all the data.
			 **
			 **/
	
		itsFile->GetName(theName);
		itsWindow->SetTitle(theName);
		itsWindow->Select();			/* Don't forget to make the window active */
	}
	
	CATCH
	{
		/*
		 * This exception handler will be executed if an exception occurs
		 * anywhere within the scope of the TRY block above.
		 * You should perform any cleanup of things that won't be needed
		 * since the document could not be opened. By convention,
		 * the creator of an object is responsible for sending it
		 * the Dispose message. This means that we should only dispose
		 * of things that would not be taken care of in Dispose.
		 * In this case, we just make sure that the Handle theData
		 * has been disposed of. The exception will propagate up to
		 * CApplications's exception handler, which handles displaying
		 * an error alert.
		 */
		 
		 if (theData) DisposHandle( theData);
		 gProject = NULL;
		 
	}
	ENDTRY;
}



/***
 * BuildWindow
 *
 *	This is the auxiliary window-building method that the
 *	NewFile() and OpenFile() methods use to create a window.
 *
 *	In this implementation, the argument is the data to display.
 *
 ***/

void CHarvestDoc::BuildWindow (Handle theData)

{
	CScrollPane		*theScrollPane;
	CHarvestPane	*theMainPane;
	Rect theRect = {75,175,300,175};

		/**
		 **	First create the window and initialize
		 **	it. The first argument is the resource ID
		 **	of the window. The second argument specifies
		 **	whether the window is a floating window.
		 **	The third argument is the window's enclosure; it
		 **	should always be gDesktop. The last argument is
		 **	the window's supervisor in the Chain of Command;
		 **	it should always be the Document object.
		 **
		 **/
	itsWindow = new(CWindow);
	itsWindow->IWindow(WINDHarvest, FALSE, gDesktop, this);
	
		/**
		 **	After you create the window, you can use the
		 **	SetSizeRect() message to set the windowÕs maximum
		 **	and minimum size. Be sure to set the max & min
		 **	BEFORE you send a PlaceNewWindow() message to the
		 **	decorator.
		 **
		 ** The default minimum is 100 by 100 pixels. The
		 **	default maximum is the bounds of GrayRgn() (The
		 **	entire display area on all screens.)
		 **
         ** We'll use the defaults.
         **
		 **/
	itsWindow->SetSizeRect(&theRect);
		/**
         ** Our window will contain a ScrollPane,
         ** which in turn will contain a Panorama.
         ** Now, let's create the ScrollPane.
         **/

    theScrollPane = new(CScrollPane);
    
        /**
		 **	You can initialize a scroll pane two ways:
		 **		1. You can specify all the values
		 **		   right in your code, like this.
		 **		2. You can create a ScPn resource and
		 **		   initialize the pane from the information
		 **		   in the resource.
		 **
		 **/

	theScrollPane->IScrollPane(itsWindow, this, 10, 10, 0, 0,
								sizELASTIC, sizELASTIC,
								TRUE, TRUE, TRUE);

		/**
		 **	The FitToEnclFrame() method makes the
		 **	scroll pane be as large as its enclosure.
		 **	In this case, the enclosure is the window,
		 **	so the scroll pane will take up the entire
		 **	window.
		 **
		 **/

	theScrollPane->FitToEnclFrame(TRUE, TRUE);


		/**
		 **	itsMainPane is the document's focus
		 **	of attention. Some of the standard
		 **	classes (particularly CPrinter) rely
		 **	on itsMainPane pointing to the main
		 **	pane of your window.
		 **
		 **	itsGopher specifies which object
         ** should become the gopher when the document
         ** becomes active. By default
		 **	the document becomes the gopher. ItÕs
		 **	likely that your main pane handles commands
		 **	so youÕll almost always want to set itsGopher
		 **	to point to the same object as itsMainPane.
		 **
		 **	Note that the main pane is the
		 **	panorama in the scroll pane and not
		 **	the scroll pane itself.
		 **
		 **/

	theMainPane = new(CHarvestPane);
    theMainPane->IHarvestPane(theScrollPane, this);
		// finally initialize the table
		// insert some rows and columns into the new table
	
	itsMainPane = itsTable = theMainPane;
	itsTable->AddRow( 0, -1);
	itsTable->AddCol( 1, -1);
	itsTable->SetDblClickCmd(cmdOpenSourceFile);
	itsGopher = theMainPane;
	
		/**	The FitToEnclosure() method makes the pane
		 **	fit inside the enclosure. The inside (or
		 **	interior) of a scroll pane is defined as
		 **	the area inside the scroll bars.
		 **/

	theMainPane->FitToEnclosure(TRUE, TRUE);

		/**
		 **	Send the scroll pane an InstallPanorama()
         ** message to associate the panorama with 
         ** the scroll pane.
		 **
		 **/

	theScrollPane->InstallPanorama(theMainPane);
	
		/**
		 **	The Decorator is a global object that takes care
		 **	of placing and sizing windows on the screen.
		 **	You don't have to use it.
		 **
		 **/

	gDecorator->PlaceNewWindow(itsWindow);
			
	itsWindow->Select();

	if (theData) {
		if (GetHandleSize(theData))
			ReadFromHandle(theData);
	}
}

#define HWriteShort(h,i,val) if ((i+2)>=dataSize) SetHandleSize(h,dataSize*=2);BlockMove(&val,&((*(h))[(i)]),sizeof(short));i+=sizeof(short);
#define HWriteLong(h,i,val) if ((i+4)>=dataSize) SetHandleSize(h,dataSize*=2);BlockMove(&val,&((*(h))[(i)]),sizeof(long));i+=sizeof(long);

#define HReadShort(h,i,val) BlockMove(&((*(h))[(i)]),&val,sizeof(short));i+=sizeof(short);
#define HReadLong(h,i,val)  BlockMove(&((*(h))[(i)]),&val,sizeof(long));i+=sizeof(long);

#ifdef Undefined
#define HWriteShort(h,i,val) (*(h))[(i)] = HiByte((val));(*(h))[(i)+1] = LoByte((val));i+=2;
#define HWriteLong(h,i,val)	(*(h))[(i)] = HiByte((HiShort((val)))); \
							(*(h))[(i)+1] = LoByte((HiShort((val)))); \
							(*(h))[(i)+2] = HiByte((LoShort((val)))); \
							(*(h))[(i)+3] = LoByte((LoShort((val))));i+=4;
#endif
#define HWriteCString(h,i,s) if ((i+strlen(s))>=dataSize) SetHandleSize(h,dataSize*=2);strcpy(&((*(h))[(i)]),s);i+=strlen(s)+1;
#define HWritePString(h,i,s) if ((i+*s)>=dataSize) SetHandleSize(h,dataSize*=2);CopyPString(s,((unsigned char *) &((*(h))[(i)])));i+=*s+1;

#ifdef Undefined
#define HReadShort(h,i,val) val = ((((int) ((*(h))[(i)])) << 8) | ((int) ((*(h))[(i)+1])));i+=2;
#define HReadLong(h,i,val) val = (((int) ((*(h))[(i)])) << 24) | (((int) ((*(h))[(i)+1])) << 16) \
								| (((int) ((*(h))[(i)+2])) << 8) | (((int) ((*(h))[(i)+3])));i+=4;
#endif
								
#define HReadCString(h,i,s) strcpy(s,&((*(h))[(i)]),s);i+=strlen(s)+1;
#define HReadPString(h,i,s) CopyPString(((unsigned char *) &((*(h))[(i)])),s);i+=*s+1;

Handle CHarvestDoc::WriteToHandle(void)
{
	long ndx = 0;
	long i;
	Handle theData;
	CSourceFile *aFile;
	Handle aHandle;
	long dataSize = 4000;	/* Danger - this should not be hard coded */
	long k2;
	short CountSourceFiles;
	
	theData = NewHandleClear(dataSize);

	aHandle = (Handle) itsOptions;
	BlockMove(*aHandle,(*theData + ndx),ndx += GetHandleSize(aHandle));
	
	CountSourceFiles = itsSourceFiles->GetNumItems();
	HWriteShort(theData,ndx,CountSourceFiles);
	HWriteLong (theData,ndx,itsSignature);
	HWriteLong (theData,ndx,itsPartition);
	HWriteShort(theData,ndx,StdAppVol);
	HWriteLong (theData,ndx,StdAppDir);
	HWritePString(theData,ndx,StdAppName);
	
	for (i=1;i<=CountSourceFiles;i++) {
		short k;
		aFile = (CSourceFile *) itsSourceFiles->NthItem(i);
		if (aFile->itsOptions) {
			k = 1;
			HWriteShort(theData,ndx,k);
			aHandle = (Handle) aFile->itsOptions;
			BlockMove(*aHandle,(*theData + ndx),ndx += GetHandleSize(aHandle));
		}
		else {
			k = 0;
			HWriteShort(theData,ndx,k);
		}
		HWriteShort(theData,ndx,aFile->itsKind);
		HWriteShort(theData,ndx,aFile->theFile->volNum);
		HWriteLong(theData,ndx,aFile->theFile->dirID);
		HWritePString(theData,ndx,aFile->theFile->name);
	}
	
	k2 = strlen(itsPrefix);
	HWriteLong(theData,ndx,k2);
	HWriteCString(theData,ndx,itsPrefix);
	k2 = 0;
	HWriteLong(theData,ndx,k2);

	return theData;
}

/*
	This function reads the Harvest C project file format
*/
void CHarvestDoc::ReadFromHandle(Handle theData)
{
	long ndx = 0;
	long i;
	Handle aHandle;
	short CountSourceFiles;
	short flag;
	short vol;
	long dirID;
	long scratch;
	Str63 name;
	
	StdAppVol = itsFile->volNum;
	StdAppDir = itsFile->dirID;
	
	aHandle = (Handle) itsOptions;
	BlockMove((*theData + ndx),*aHandle,ndx += GetHandleSize(aHandle));

	HReadShort(theData,ndx,CountSourceFiles);
	HReadLong (theData,ndx,itsSignature);
	HReadLong (theData,ndx,itsPartition);
	HReadShort(theData,ndx,StdAppVol);
	HReadLong (theData,ndx,StdAppDir);
	HReadPString(theData,ndx,StdAppName);
	
	for (i=1;i<=CountSourceFiles;i++) {
		short kind;
		HReadShort(theData,ndx,flag);
		if (flag) {
			aHandle = (Handle) new CHarvestOptions;
			BlockMove(*aHandle,(*theData + ndx),ndx += GetHandleSize(aHandle));
		}
		HReadShort(theData,ndx,kind);
		HReadShort(theData,ndx,vol);
		HReadLong(theData,ndx,dirID);
		HReadPString(theData,ndx,name);

		/* the following two lines were added to fix the problem with project files
			not working on other machines.  These lines force a restriction: the C
			source files must be in the same folder as the project file.
		*/
		vol = itsFile->volNum;
		dirID = itsFile->dirID;

		if (kind == H_LibraryFile)
			AddLibraryFileHFS(name,vol,dirID);
		else if (kind == H_ResourceFile)
			AddResourceFileHFS(name,vol,dirID);
		else
			AddSourceFileHFS(name,vol,dirID);
	}
	
	HReadLong(theData,ndx,scratch);
	itsPrefix = (char *) icemalloc(scratch);
	HReadCString(theData,ndx,itsPrefix);
	HReadLong(theData,ndx,scratch);
}

/******************************************************************************
 Close {OVERRIDE}

		Close a Document
 ******************************************************************************/

Boolean	CHarvestDoc::Close(
	Boolean		quitting)				/* Is this part of a quit sequence?	*/
{
	/* Ignore quitting -- We always save the file. */

	Handle theData;
	
		if (!itsFile) {
			/* We need to specify a file */
		}
		else {
			theData = WriteToHandle();
			((CDataFile *) itsFile)->SetMark(0,fsFromStart);
			((CDataFile *) itsFile)->WriteAll(theData);
			if (itsFile != NULL) {			/* Yes. Close associated file.		*/
				itsFile->Close();
				itsFile = NULL;
			}
			DisposHandle(theData);
		}
	
		inherited::Close(quitting);
		gProject = NULL;
		return(TRUE);					/* Document has been closed			*/
		
}
	
/***
 * DoSave
 *
 *	This method handles what happens when the user chooses Save from the
 *	File menu. This method should return TRUE if the file save was successful.
 *	If there is no file associated with the document, you should send a
 *	DoSaveFileAs() message.
 *
 ***/

Boolean CHarvestDoc::DoSave(void)

{
		/**
		 **	If you closed your file in your NewFile() method,
		 **	you'll need a different way than this to determine
		 **	if there's a file associated with your document.
		 **
		 **/

	if (itsFile == NULL)
		return(DoSaveFileAs());
	else {
			
		/**
		 **	In your application, this is where you'd
		 **	write out your file. if you left it open,
		 **	send the WriteSome() or WriteAll() mesages
		 **	to itsFile.
		 **
		 **/
			
		dirty = FALSE;					/* Document is no longer dirty		*/
		gBartender->DisableCmd(cmdSave);
		return(TRUE);					/* Save was successful				*/
	}
}


/***
 * DoSaveAs
 *
 *	This method handles what happens when the user chooses Save AsÉ from
 *	File menu. The default DoCommand() method for documents sends a DoSaveFileAs()
 *	message which displays a standard put file dialog and sends this message.
 *	The SFReply record contains all the information about the file you're about
 *	to create.
 *
 ***/

Boolean CHarvestDoc::DoSaveAs(SFReply *macSFReply)

{
		/**
		 **	If there's a file associated with this document
		 **	already, close it. The Dispose() method for files
		 **	sends a Close() message to the file before releasing
		 **	its memory.
		 **
		 **/
		 
	if (itsFile != NULL)
		itsFile->Dispose();


		/**
		 **	Create a new file, and then save it normally.
		 **
		 **/

	itsFile = new(CDataFile);
	((CDataFile *)itsFile)->IDataFile();
	itsFile->SFSpecify(macSFReply);
	itsFile->CreateNew(gSignature, 'TEXT');
	itsFile->Open(fsRdWrPerm);
	
	itsWindow->SetTitle(macSFReply->fName);

	return( DoSave() );
}


/***
 * DoRevert
 *
 *	If your application supports the Revert command, this method
 *	should close the current file (without writing anything out)
 *	and read the last saved version of the file.
 *
 ***/

void CHarvestDoc::DoRevert(void)

{
}

Boolean CHarvestDoc::BringUpToDate(void) {
	int CountSourceFiles;
	int i;
	CSourceFile *aFile;
	Boolean result;
	
	result = TRUE;

	CountSourceFiles = itsSourceFiles->GetNumItems();
	for (i=1;i<=CountSourceFiles;i++) {
		aFile = (CSourceFile *) itsSourceFiles->NthItem(i);
		if (aFile->itsKind == H_SourceFile) {
			if (aFile->GetSourceModDate() >= aFile->GetObjectModDate()) {
				if (!aFile->Compile()) {
					result = FALSE;
				}
			}
		}
	}
	return result;
}

void
CopyAllResources(CSourceFile *aFile,short theApp)
{
		/* Now we copy all resources from theResFile to theApp */
		short typeCount;
		short resCount;
		short j;
		short k;
		ResType aType;
		Handle aRez;
		Handle dupRez;
		short resID;
		Str255 resName;
		short theResFile;
		ResType type2;
		short resAttrs;

		theResFile = HOpenResFile(aFile->theFile->volNum,aFile->theFile->dirID,aFile->theFile->name,1);
		UseResFile(theResFile);
		typeCount = Count1Types();
		for (j=1;j<=typeCount;j++) {
			UseResFile(theResFile);
			Get1IndType(&aType,j);
			resCount = Count1Resources(aType);
			for (k=1;k<=resCount;k++) {
				UseResFile(theResFile);
				aRez = Get1IndResource(aType,k);
				GetResInfo(aRez,&resID,&type2,resName);
				resAttrs = GetResAttrs(aRez);
				DetachResource(aRez);
				UseResFile(theApp);
				dupRez = Get1Resource(aType,resID);
				if (!dupRez) {
					AddResource(aRez,aType,resID,resName);
					SetResAttrs(aRez,resAttrs);
					ChangedResource(aRez);
				}
			}
		}
		CloseResFile(theResFile);
}

Boolean CHarvestDoc::DoLink(void)
{
#ifdef HDEBUG
	extern FILE *dumpFile;
#endif

    LinkerFileVia_t                 curf;
    int                             UserAbort = 0;
    InFileVia_t                     curin;
    LinkerFileVia_t                 files = NULL;

    char                            objname[64];
    char                            msg[128];
    char                            err[64];
    char                            errname[64];
    CursHandle                      curs;
    Str63 fname;
    int CountSourceFiles;
    int i;
    CDataFile *objFile;
    CSourceFile *aFile;
    CWindow *theWindow;
    extern unsigned short NextSegID;
    char mesg[64];

	LinkErrorCount = 0;
	
	curf = NULL;

#ifdef HDEBUG
	dumpFile = fopen("Harvest C Debug Info","w");
#endif

	gCurSourceFile = NULL;
	gErrs = new CErrorLog;
	gErrs->IErrorLog();
	gErrs->BeginDialog();

    UserAbort = 0;
    
    gApplication->SpinCursor();
    
    NextSegID = 2;

  	theWindow = gErrs->GetWindow();
	sprintf(mesg,"Linking %#s",StdAppName);
#ifdef HDEBUG
	fprintf(dumpFile,"%s\n",mesg);
#endif
	c2pstr(mesg);
	theWindow->SetTitle((unsigned char *) mesg);

	CountSourceFiles = itsSourceFiles->GetNumItems();
	for (i=1;i<=CountSourceFiles;i++) {
    	gApplication->SpinCursor();
		aFile = (CSourceFile *) itsSourceFiles->NthItem(i);
		if (aFile->itsKind != H_ResourceFile) {
			objFile = aFile->GetObjectFile();
			curf = NULL;
			if (objFile) {
				CopyPString(objFile->name,fname);
				p2cstr(fname);
		   		curf = ReadObjectFile((char *) fname, objFile->volNum, objFile->dirID);
			}
		    if (curf) {
				Via(curf)->next = files;
				files = curf;
		    } else {
		    	objFile->GetName((unsigned char *) objname);
		    	p2cstr(objname);
				sprintf(err, "Unable to access file %s", objname);
				LinkerError(err);
		    }
	    }
    }
    if (files && !LinkErrorCount) {
		ResolveAllReferences(files);
		/* TODO Smart link */
		AssignMPWOffsets(files);
		AdjustAllReferences(files);
    }
    if (files && !LinkErrorCount) {
    	Str63 cname;
    	CopyPString(StdAppName,cname);
    	p2cstr(cname);
    	gErrs->Hprintf("References resolved ok");
		if (FlushLink((char *) cname, StdAppVol, StdAppDir, files)) {
			short theApp;
			short theResFile;
			theApp = HOpenResFile(StdAppVol, StdAppDir, StdAppName, 2);
			if (theApp == -1) {
			    LinkerError("Couldn't open app file for adding resources");
			}
			else {
				for (i=1;i<=CountSourceFiles;i++) {
			    	gApplication->SpinCursor();
					aFile = (CSourceFile *) itsSourceFiles->NthItem(i);
					if (aFile->itsKind == H_ResourceFile) {
						CopyAllResources(aFile,theApp);
				    }
			    }
			    CloseResFile(theApp);
			}
		}
	}
    else
		LinkerError("Link errors prevent completion");
		
    KillLinkFiles(files);
    KillSegments(SegmentList);
    KillModPacks(theDATA.ModuleList);
    KillImage(theDATA.Image);

#ifdef HDEBUG
	fclose(dumpFile);
#endif

    InitCursor();
}

/******************************************************************************
 PickAppName

		Put up a standard put file dialog box to allow the user to
		specify a new name under which to save the linked application.
 ******************************************************************************/

Boolean	CHarvestDoc::PickAppName(void)
{
	Point			corner;				/* Top left corner of dialog box	*/
	Str255			origName;			/* Default name for file			*/
	StringHandle	prompt;				/* Prompt string					*/
	SFReply macSFReply;
	CDataFile *aFile;
	FSSpec fileSpec;
		
										/* Center dialog box on the screen	*/
	FindDlogPosition('DLOG', putDlgID, &corner);
	
	CopyPString(StdAppName,origName);
	
	prompt = GetString(1000);
	FailNILRes( prompt);
	
	MoveHHi( (Handle) prompt);
	HLock((Handle) prompt);
	SFPPutFile(corner, *prompt, origName, NULL, &macSFReply, putDlgID, NULL);
	HUnlock((Handle) prompt);
	if (macSFReply.good) {
		aFile = new CDataFile;
		aFile->IDataFile();
		aFile->SFSpecify(&macSFReply);
		aFile->GetFSSpec(&fileSpec);
		StdAppVol = fileSpec.vRefNum;
		StdAppDir = fileSpec.parID;
		CopyPString(fileSpec.name,StdAppName);
		return TRUE;
	} 
	else return FALSE;
}


ProcessSerialNumber CHarvestDoc::RunApp(void)
{
	FSSpec theApp;
	CFile *theFile;
	ProcessSerialNumber defaultResult = {0};
	FSMakeFSSpec(StdAppVol,
				StdAppDir,
				StdAppName,
				&theApp);
	theFile = new CFile;
	theFile->IFile();
	theFile->SpecifyFSSpec(&theApp);
	if (theFile->ExistsOnDisk()) {
		LaunchParamBlockRec myLaunchParams;
		OSErr launchErr;
		ForgetObject(theFile);
		myLaunchParams.launchBlockID = extendedBlock;
		myLaunchParams.launchEPBLength = extendedBlockLen;
		myLaunchParams.launchFileFlags = 0;
		myLaunchParams.launchControlFlags = launchContinue + launchNoFileFlags;
		myLaunchParams.launchAppSpec = &theApp;
		myLaunchParams.launchAppParameters = NULL;
		launchErr = LaunchApplication(&myLaunchParams);
		FailOSErr(launchErr);
		return myLaunchParams.launchProcessSN;
	}
	else {
		SysBeep(1);
	}
	ForgetObject(theFile);
	return defaultResult;
}

void CHarvestDoc::Clean(void) {
	int CountSourceFiles;
	int i;
	CSourceFile *aFile;
	CDataFile *objFile;

	CountSourceFiles = itsSourceFiles->GetNumItems();
	for (i=1;i<=CountSourceFiles;i++) {
		aFile = (CSourceFile *) itsSourceFiles->NthItem(i);
		if (aFile->itsKind == H_SourceFile) {
			objFile = aFile->GetObjectFile();
			if (objFile->ExistsOnDisk())
				objFile->ThrowOut();
		}
	}
}

Boolean CHarvestDoc::Link(void) {
	if (PickAppName()) {
		if (BringUpToDate()) {
			return DoLink();
		}
		return FALSE;
	}
	return FALSE;
}

Boolean CHarvestDoc::CheckLink(void) {
	SysBeep(1);
}

Boolean EqualPStrings(unsigned char *a,unsigned char *b) {
	int i,len;
	if (*a == *b) {
		len = *a;
		i = 1;
		while (a[i] == b[i]) {
			i++;
			if (i>len) return TRUE;
		}
	}
	return FALSE;
}

int CHarvestDoc::FindFile(char *name,short vRefNum,long dirID)
{
	/* Locate the given file, return its index in the list */
	int CountSourceFiles;
	int i;
	CSourceFile *aFile;
	CDataFile *theFile;

	CountSourceFiles = itsSourceFiles->GetNumItems();
	for (i=1;i<=CountSourceFiles;i++) {
		aFile = (CSourceFile *) itsSourceFiles->NthItem(i);
		theFile = aFile->theFile;
		if (theFile->volNum == vRefNum) {
			if (theFile->dirID == dirID) {
				if (EqualPStrings((unsigned char *) name,theFile->name)) {
					return i;
				}
			}
		}
	}
	return 0;
}

void CHarvestDoc::ResetGopher(void)
{
	gGopher = itsGopher = itsMainPane;
}

void CHarvestDoc::Activate(void)
{
	ResetGopher();
	inherited::Activate();
}
