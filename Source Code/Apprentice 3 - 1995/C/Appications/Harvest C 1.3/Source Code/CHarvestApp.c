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

/*****
 * CHarvestApp.c
 *
 *	Application methods for Harvest C
 *
 *  Copyright © 1990 Symantec Corporation.  All rights reserved.
 *
 *****/

#include "CHarvestApp.h"
#include "CHarvestDoc.h"
#include "CDLOGDialog.h"
#include "CDialogText.h"
#include "CDLOGDirector.h"
#include "CAppleEvent.h"
#include "CBartender.h"
#include "Commands.h"
#include "HarvestCommands.h"
#include "CSourceFile.h"
#include "CDataFile.h"
#include "CList.h"
#include "Global.h"
#include <AppleEvents.h>
#include <TBUtilities.h>

extern	OSType	gSignature;
extern  CBartender *gBartender;
extern  CDesktop *gDesktop;
extern  CHarvestApp *gApplication;
extern	CHarvestDoc *gProject;
extern CursHandle	gIBeamCursor;		/* I-beam for text views			*/
extern CursHandle	gWatchCursor;		/* Watch cursor for waiting			*/
extern CBureaucrat	*gGopher;			/* First in line to get commands	*/

Str255 MyVersion;

#define		kExtraMasters		4
#define		kRainyDayFund		20480
#define		kCriticalBalance	20480
#define		kToolboxBalance		20480

int app_refnum;

OSErr
FindSystemHeaders(short vRefNum,long dirID,char *name,FSSpec *buf)
{
	OSErr result;
	short vnum;
	long id;
	CInfoPBRec spec1;
	CInfoPBRec spec2;
	HParamBlockRec pb;
	Str255 pname;
	strcpy((char *) pname,name);
	c2pstr(pname);
	
	HGetVol(NULL,&vnum,&id);
	
	spec1.hFileInfo.ioNamePtr = pname;
	spec1.hFileInfo.ioFlAttrib = 0x10;
	spec2.hFileInfo.ioNamePtr = NULL;
	spec2.hFileInfo.ioFlAttrib = 0x10;
	pb.csParam.ioNamePtr = NULL;
	pb.csParam.ioCompletion = NULL;
	pb.csParam.ioSearchBits = fsSBFullName + fsSBFlAttrib;
	pb.csParam.ioSearchInfo1 = &spec1;
	pb.csParam.ioSearchInfo2 = &spec2;
	pb.csParam.ioSearchTime = -1;
	pb.csParam.ioCatPosition.initialize = 0;
	pb.csParam.ioOptBuffer = NULL;
	pb.csParam.ioOptBufSize = 0;
	pb.csParam.ioVRefNum = vnum;
	pb.csParam.ioMatchPtr = buf;
	pb.csParam.ioReqMatchCount = 1;
	
	result = PBCatSearchSync(&pb);
	if (!pb.csParam.ioActMatchCount) return -43;
	return result;
}

static short		_current_working_directory = 0;
static short		_current_working_vrefnum = 0;
static long			_current_working_dirid = 2;

set_current_wd(vRefNum, dirID)
int		vRefNum;
long	dirID;
	{
	int			myerr;
	WDPBRec		wpb;

	wpb.ioCompletion = 0;
	wpb.ioVRefNum = vRefNum;
	wpb.ioNamePtr = (unsigned char *) NULL;
	wpb.ioWDDirID = dirID;
	myerr = PBHSetVol(&wpb, FALSE);
	if (myerr != noErr) {
		return myerr;
		}
	else {
		wpb.ioWDProcID = 'ERIK';
		myerr = PBOpenWD(&wpb, FALSE);
		if (myerr == noErr) {
			_current_working_directory = wpb.ioVRefNum;
			_current_working_vrefnum = wpb.ioWDVRefNum;
			_current_working_dirid = dirID;
			}
		else {
			return myerr;
			}
		}
	}

void CHarvestApp::IHarvestApp(void)

{
	CApplication::IApplication( kExtraMasters, kRainyDayFund, 
						kCriticalBalance, kToolboxBalance);
    

 	Init(0,NULL);
 	
 	BeachBalls[0] = GetCursor(500);
 	FailNILRes(BeachBalls[0]);
	HNoPurge( (Handle) BeachBalls[0]);
	
 	BeachBalls[1] = GetCursor(501);
 	FailNILRes(BeachBalls[1]);
	HNoPurge( (Handle) BeachBalls[1]);
	
 	BeachBalls[2] = GetCursor(502);
 	FailNILRes(BeachBalls[2]);
	HNoPurge( (Handle) BeachBalls[2]);
	
 	BeachBalls[3] = GetCursor(503);
 	FailNILRes(BeachBalls[3]);
	HNoPurge( (Handle) BeachBalls[3]);
	
	BeachIndex = 0;

	app_refnum = CurResFile();
	
    {
    	FSSpec spec;

    	FCBPBRec fcbpb;
    	OSErr err;
    	fcbpb.ioNamePtr = (void *) HarvestCTree.name;
    	fcbpb.ioVRefNum = 0;
    	fcbpb.ioRefNum = CurResFile();
    	fcbpb.ioFCBIndx = 0;
    	err = PBGetFCBInfoSync(&fcbpb);
    	HarvestCTree.vRefNum = fcbpb.ioVRefNum;
    	HarvestCTree.parID = fcbpb.ioFCBParID;
    	
    	set_current_wd(HarvestCTree.vRefNum,HarvestCTree.parID);

		{
			CInfoPBRec cfb;
			Str255 tempPath;
			CopyPString("\p(Harvest C Headers)",tempPath);
			cfb.hFileInfo.ioNamePtr = tempPath;
			cfb.hFileInfo.ioDirID = HarvestCTree.parID;
			cfb.hFileInfo.ioVRefNum = HarvestCTree.vRefNum;
			cfb.hFileInfo.ioFDirIndex = 0;
			err = PBGetCatInfoSync(&cfb);
			if (!err) {
				StdIncludeDir = cfb.hFileInfo.ioDirID;
				StdIncludeVol = cfb.hFileInfo.ioVRefNum;
			}
			else {
				StdIncludeDir = 0;
				StdIncludeVol = 0;
			}
		}

		{
			CInfoPBRec cfb;
			Str255 tempPath;
			CopyPString("\p(Harvest C Libraries)",tempPath);
			cfb.hFileInfo.ioNamePtr = tempPath;
			cfb.hFileInfo.ioDirID = HarvestCTree.parID;
			cfb.hFileInfo.ioVRefNum = HarvestCTree.vRefNum;
			cfb.hFileInfo.ioFDirIndex = 0;
			err = PBGetCatInfoSync(&cfb);
			if (!err) {
				StdLibDir = cfb.hFileInfo.ioDirID;
				StdLibVol = cfb.hFileInfo.ioVRefNum;
			}
			else {
				StdLibDir = 0;
				StdLibVol = 0;
			}
		}
    }
    {
    	VersRecHndl v;
    	v = (VersRecHndl) Get1Resource('vers',1);
    	if (v) {
    		CopyPString((*v)->shortVersion,MyVersion);
    	}
    	else {
    		CopyPString("\pVersion???",MyVersion);
    	}
    }
}

void CHarvestApp::DoAppleEvent( CAppleEvent *anAppleEvent)
{
	DescType	eventClass;
	DescType	eventID;
	OSErr		err;
	
	eventClass = anAppleEvent->GetEventClass();
	eventID = anAppleEvent->GetEventID();
	
	if (eventClass == kCoreEventClass)
	{
		switch (eventID)
		{
			case kAEOpenApplication:
					if (anAppleEvent->GotRequiredParams())
					{
						anAppleEvent->SetErrorResult( noErr);
					}
					break;
					
			case kAEOpenDocuments:
					DoOpenOrPrintDocEvent( anAppleEvent);
					break;
					
			case kAEPrintDocuments:
					err = anAppleEvent->RequestInteraction( MAXLONG);
					if (err == noErr)
					{
						DoOpenOrPrintDocEvent( anAppleEvent);
					}
					else
						anAppleEvent->SetErrorResult( err);
					break;
				
			case kAEQuitApplication:
			
					if (anAppleEvent->GotRequiredParams())
					{
						err = anAppleEvent->RequestInteraction( MAXLONG);
						if (err == noErr)
						{
							Quit();
						
						}
						anAppleEvent->SetErrorResult( err);
					}
					break;
		}		
	}
}

void	CHarvestApp::StartUpAction(
	short	numPreloads)
{
	FlushEvents(everyEvent, 0);
	
	if (!gSystem.hasAppleEvents && (numPreloads == 0))
	{
		gGopher->DoCommand(cmdAbout);
	}
}


void CHarvestApp::SetUpFileParameters(void)

{
	inherited::SetUpFileParameters();	/* Be sure to call the default method */

		/**
		 **	sfNumTypes is the number of file types
		 **	your application knows about.
		 **	sfFileTypes[] is an array of file types.
		 **	You can define up to 4 file types in
		 **	sfFileTypes[].
		 **
		 **/

	sfNumTypes = 3;
	sfFileTypes[0] = 'Hprj';
	sfFileTypes[1] = 'TEXT';
	sfFileTypes[2] = 'OBJ ';

	gSignature = 'Jn15';
}


void CHarvestApp::SetUpMenus()
 {

  inherited::SetUpMenus();  /*  Superclass takes care of adding     
                                menus specified in a MBAR id = 1    
                                resource    
                            */                          

        /* Add your code for creating run-time menus here */    
 }


/*
 * Handle selection of About item from Apple menu
 */

void CHarvestApp::DoAbout(void)
{
	CDLOGDirector *dialog;
	CDialogText *version;
	
	dialog = new CDLOGDirector;	
	dialog->IDLOGDirector( 130, this);
	dialog->BeginDialog();
	version = (CDialogText *) dialog->itsWindow->FindViewByID(3);
	if (version) {
		version->Deactivate();
		version->SetFontNumber(geneva);
		version->SetTextString(MyVersion);
	}
		
	dialog->DoModalDialog( cmdOK);
	ForgetObject(dialog);
}

/*
 * Handle selection of Registration... item from Apple menu
 */

void CHarvestApp::DoRegistration(void)
{
	CDLOGDirector *dialog;
	
		dialog = new CDLOGDirector;	
		dialog->IDLOGDirector( 230, this);
		dialog->BeginDialog();
			
		dialog->DoModalDialog( cmdOK);
		ForgetObject(dialog);
}

void	CHarvestApp::ChooseProjectFile(
	SFReply		*macSFReply)
{
	Point		corner;					/* Top left corner of dialog box	*/
	Boolean		wasLocked;
	SFTypeList	oneType;
										/* Center dialog box on the screen	*/
	FindDlogPosition('DLOG', sfGetDLOGid, &corner);
	
	wasLocked = Lock( TRUE);
	
	oneType[0] = 'Hprj';
	SFPGetFile(corner, "\p", sfFileFilter, 1, oneType,
				sfGetDLOGHook, macSFReply, sfGetDLOGid, sfGetDLOGFilter);
				
	Lock( wasLocked);
}

Boolean SamePStrings(char *a,char *b)
{
	int c;
	c = *a;
	while (c--) {
		if (*a != *b) return FALSE;
		a++;b++;
	}
	return TRUE;
}

pascal Boolean SourceFileFilter(FileParam * p)
{
	int len;
	int CountSourceFiles;
	int i;
	CSourceFile *aFile;
	/* Now check if it's in the project already. */
	CountSourceFiles = gProject->itsSourceFiles->GetNumItems();
	for (i=1;i<=CountSourceFiles;i++) {
		aFile = (CSourceFile *) gProject->itsSourceFiles->NthItem(i);
		if (SamePStrings((char *) p->ioNamePtr,(char *) aFile->theFile->name)) {
			return TRUE;
		}
	}
	if (p->ioFlFndrInfo.fdType == 'TEXT') {
		len = p->ioNamePtr[0];	/* assume pascal string, of course */
		if (p->ioNamePtr[len] == 'c') {
			if (p->ioNamePtr[len-1] == '.') {
				return FALSE;
			}
		}
	}
	else if (p->ioFlFndrInfo.fdType == 'OBJ ') {
		return FALSE;
	}
	else if (p->ioFlFndrInfo.fdCreator == 'RSED') {
		return FALSE;
	}
	else if (p->ioFlFndrInfo.fdType == 'rsrc') {
		return FALSE;
	}
	return TRUE;
}

void	CHarvestApp::ChooseSourceFile(
	SFReply		*macSFReply)
{
	Point		corner;					/* Top left corner of dialog box	*/
	Boolean		wasLocked;
	SFTypeList	oneType;
										/* Center dialog box on the screen	*/
	FindDlogPosition('DLOG', sfGetDLOGid, &corner);
	
	wasLocked = Lock( TRUE);
	
	oneType[0] = 'TEXT';
	SFPGetFile(corner, "\p", (FileFilterProcPtr) SourceFileFilter, -1, oneType,
				sfGetDLOGHook, macSFReply, sfGetDLOGid, sfGetDLOGFilter);
				
	Lock( wasLocked);
}

void	CHarvestApp::ChooseLibFile(
	SFReply		*macSFReply)
{
	Point		corner;					/* Top left corner of dialog box	*/
	Boolean		wasLocked;
	SFTypeList	oneType;
										/* Center dialog box on the screen	*/
	FindDlogPosition('DLOG', sfGetDLOGid, &corner);
	
	wasLocked = Lock( TRUE);
	
	oneType[0] = 'OBJ ';
	SFPGetFile(corner, "\p", sfFileFilter, 1, oneType,
				sfGetDLOGHook, macSFReply, sfGetDLOGid, sfGetDLOGFilter);
				
	Lock( wasLocked);
}

void
DoOpenProject(void)
{
	SFReply macSFReply;
	gApplication->ChooseProjectFile(&macSFReply);
	if (macSFReply.good) {
		SetCursor(*gWatchCursor);
		gApplication->OpenProject(&macSFReply);
	}
}

void CHarvestApp::DoCommand(long theCommand)
{
	SFReply macSFReply;
	int oldNumTypes;
	switch (theCommand) {
	
		case cmdAbout:
			DoAbout();
			break;
		case cmdRegistration:
			DoRegistration();
			break;
		case cmdNewProject:
		case cmdNew:
				SetCursor(*gWatchCursor);
			CreateProject();
		break;
		case cmdOpenProject:
		case cmdOpen:
				DoOpenProject();
		break;
		case cmdDebugger:
			Debugger();
			break;
		case cmdSwitchToAlpha:
			{
				ProcessSerialNumber process;
				ProcessInfoRec InfoRec;
				FSSpec theSpec;
				if (FindAProcess('ALFA',&process,&InfoRec,&theSpec)) {
					SetFrontProcess(&process);
				}
					/* Maybe later, locate the appl using the desktop manager,
						and launch it. */
			}
			break;
		default:	inherited::DoCommand(theCommand);
					break;
	}
}


 void CHarvestApp::UpdateMenus()
 {
  inherited::UpdateMenus();     /* Enable standard commands */ 
  gBartender->SetDimOption(MENUproject,dimNONE);     
  gBartender->SetDimOption(MENUsources,dimALL);
  gBartender->EnableCmd(cmdTclShell);
  gBartender->EnableCmd(cmdSwitchToAlpha);
  gBartender->EnableCmd(cmdDebugger);
  gBartender->EnableCmd(cmdNewProject);
  gBartender->EnableCmd(cmdOpenProject);
  gBartender->DisableCmd(cmdPageSetup);   
  gBartender->EnableCmd(cmdNew);   
  gBartender->EnableCmd(cmdOpen);
   
  gBartender->DisableCmd(cmdSave);   
  gBartender->DisableCmd(cmdSaveAs);
     
  gBartender->DisableCmd(cmdCloseProject);   
  gBartender->DisableCmd(cmdClose);  
   
  gBartender->DisableCmd(cmdOptions);
  gBartender->DisableCmd(cmdWarnings);
  gBartender->DisableCmd(cmdSetProjectInfo);   
  gBartender->DisableCmd(cmdClean);   
  gBartender->DisableCmd(cmdBringUpToDate);   
  gBartender->DisableCmd(cmdBuildApplication);   
  gBartender->DisableCmd(cmdCheckLink);   
  gBartender->DisableCmd(cmdRun);   

    /* Enable the commands handled by your Application class */ 
 }


void CHarvestApp::Exit()

{
	/* your exit handler here */
}


void CHarvestApp::CreateProject()

{
	CHarvestDoc *newDocument;
	newDocument = new(CHarvestDoc);
	newDocument->IHarvestDoc(this,false);
	newDocument->NewFile();
}

void CHarvestApp::CreateDocument()

{
	CHarvestDoc *newDocument;
	newDocument = new(CHarvestDoc);
	newDocument->IHarvestDoc(this,false);
	newDocument->NewFile();
}

void CHarvestApp::OpenDocument(SFReply *macSFReply)

{
	CHarvestDoc	*theDocument = NULL;
	
	TRY
	{
	
		theDocument = new(CHarvestDoc);
			
			/**
			 **	Send your document an initialization
			 **	message. The first argument is the
			 **	supervisor (the application). The second
			 **	argument is TRUE if the document is printable.
			 **
			 **/
		
		theDocument->IHarvestDoc(this, false);
	
			/**
			 **	Send the document an OpenFile() message.
			 **	The document will open a window, open
			 **	the file specified in the macSFReply record,
			 **	and display it in its window.
			 **
			 **/
		theDocument->OpenFile(macSFReply);
	}
	
	CATCH
	{
		/*
		 * This exception handler gets executed if a failure occurred 
		 * anywhere within the scope of the TRY block above. Since 
		 * this indicates that the document could not be opened, we
		 * send it a Dispose message. The exception will propagate up to
		 * CSwitchboard's exception handler, which handles displaying
		 * an error alert.
		 */
		 
		 if (theDocument) theDocument->Dispose();
	}
	ENDTRY;
}

void CHarvestApp::OpenProject(SFReply *macSFReply)

{
	CHarvestDoc	*theDocument = NULL;
	
	TRY
	{
	
		theDocument = new(CHarvestDoc);
			
			/**
			 **	Send your document an initialization
			 **	message. The first argument is the
			 **	supervisor (the application). The second
			 **	argument is TRUE if the document is printable.
			 **
			 **/
		
		theDocument->IHarvestDoc(this, false);
	
			/**
			 **	Send the document an OpenFile() message.
			 **	The document will open a window, open
			 **	the file specified in the macSFReply record,
			 **	and display it in its window.
			 **
			 **/
		theDocument->OpenFile(macSFReply);
	}
	
	CATCH
	{
		/*
		 * This exception handler gets executed if a failure occurred 
		 * anywhere within the scope of the TRY block above. Since 
		 * this indicates that the document could not be opened, we
		 * send it a Dispose message. The exception will propagate up to
		 * CSwitchboard's exception handler, which handles displaying
		 * an error alert.
		 */
		 
		 if (theDocument) theDocument->Dispose();
	}
	ENDTRY;
}

OSErr
FindOMFFile(char *name,short volrefnum, long dirID, FSSpec *f)
{
  FInfo junk;
  OSErr bad;
  Str255 pname;

  strcpy(f->name,name);
  strcpy(pname,name);
  c2pstr(pname);
  
  /* Search path : current app dir, lib dir, etc... */
  
  bad = HGetFInfo(f->vRefNum = volrefnum, f->parID = dirID,pname,&junk);
  if (!bad) return bad;

  bad = HGetFInfo(f->vRefNum = gProject->StdAppVol, f->parID = gProject->StdAppDir,pname,&junk);
  if (!bad) return bad;

  bad = HGetFInfo(f->vRefNum = gApplication->StdLibVol, f->parID = gApplication->StdLibDir,pname,&junk);
  if (!bad) return bad;
  
  return bad;
}

OSErr
FindCFile(char *name,short volrefnum, long dirID, FSSpec *f)
{
  FInfo junk;
  OSErr bad;
  Str255 pname;

  strcpy(f->name,name);
  strcpy(pname,name);
  c2pstr(pname);
  
  /* Search path : current app dir, lib dir, etc... */
  
  bad = HGetFInfo(f->vRefNum = volrefnum, f->parID = dirID,pname,&junk);
  if (!bad) return bad;

  bad = HGetFInfo(f->vRefNum = gProject->StdAppVol, f->parID = gProject->StdAppDir,pname,&junk);
  if (!bad) return bad;

  bad = HGetFInfo(f->vRefNum = gApplication->StdLibVol, f->parID = gApplication->StdLibDir,pname,&junk);
  if (!bad) return bad;
  
  return bad;
}

void CHarvestApp::SpinCursor(void)
{
	EventRecord junk;
	BeachIndex = (BeachIndex + 1) % 4;
	SetCursor(*(BeachBalls[BeachIndex]));

	/* should maybe give time to background processes here, but maybe not */
}

