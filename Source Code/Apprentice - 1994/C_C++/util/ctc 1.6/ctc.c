/**********
>>   change the file type and creator of any file dropped onto this program.
>>	 Prompts for the file type and creator, and changes all files dropped at
>>   the same time to the same type and creator.

	This program is a modified version of:

>>	ctc v1.4	
>>	Written by Brian Bechtel, based on code by Juri Munkki
>>	Feel free to use the code in your programs.
	
	History...
	V1.5		Added:	EOL - CR, LF, CR/LF translation of text files.
	V1.6	RMF	Added:	Balloon help resource for finder
						Options to change type, creator or both
						Now displays file name, old type, and creator
						Made dialog window movable
*/
#include <Gestalt.h>
#include <pascal.h>
#include <Menus.h>
#include <Files.h>
#include <Folders.h>
#include <Fonts.h>
#include <Dialogs.h>
#include <Memory.h>
#include <SegLoad.h>
#include <BDC.h>
#include <OSUtils.h>
#include "ctc.h"

/* Tells the Dialog Manager that there is an edit line in this dialog, and */ 
/* it should track and change to an I-Beam cursor when over the edit line */
pascal OSErr SetDialogTracksCursor(DialogPtr theDialog, Boolean tracks)
	= { 0x303C, 0x0306, 0xAA68 };

pascal OSErr GetStdFilterProc(ProcPtr *theProc)
		= {0x303C, 0x0203, 0xAA68};

pascal OSErr SetDialogDefaultItem (DialogPtr theDialog,
		short newItem) = {0x303C,0x0304,0xAA68};


pascal OSErr SetDialogCancelItem (DialogPtr theDialog,
		short newItem) = {0x303C,0x0305,0xAA68};


void ConvertFile (char *thefile, short EOLmode);

pascal Boolean GenericFilter();
		
/* from tickle.c */
OSErr TickleParent( FSSpec *);
OSErr MakeWDSpec( FSSpec *, short, ConstStr255Param );
/*****************
	Support for EOL code....
	
	Modified by RMF to remove unneeded: FindControl call. 
*/
void PopRadioButton (DialogPtr dPtr, short button)
{	Handle			itemHandle;
	short			itemType;
	Rect			box;
	
	GetDItem(dPtr, button, &itemType, &itemHandle, &box);
	SetCtlValue(itemHandle, false);
}

void PushRadioButton (DialogPtr dPtr, short button)
{	Handle			itemHandle;
	short			itemType;
	Rect			box;
	
	PopRadioButton (dPtr, DOS);
	PopRadioButton (dPtr, IGNORE);
	PopRadioButton (dPtr, MAC);
	PopRadioButton (dPtr, UNIX);
	GetDItem(dPtr, button, &itemType, &itemHandle, &box);
	SetCtlValue(itemHandle, true);
}

/*************************************************************
	Display error number if error occures
*/
void ReportError(OSErr err)
{	Str15 errStr;

	NumToString((long) err, errStr);
	ParamText(errStr, "\p", nil, nil);
	StopAlert(129, nil);
	ParamText("\p", nil, nil, nil);
	
}
/**********************************************************************
	Set a Edit Text area to file type or creator
*/
void SetDialogItemType(long typeCreator, short itemHit, DialogPtr dPtr)
{	Handle		itemHandle;
	short		itemType;
	Rect		box;
	Str255		itemString;

	itemString[0] = 4;	/* establish artificial length */
	GetDItem(dPtr, itemHit, &itemType, &itemHandle, &box);
	BlockMove(&typeCreator, (Ptr)&itemString[1], 4);
	SetIText(itemHandle, itemString);
	
} 	/* End of () */

/**********************************************************************
	Get from a Edit Text area a file type or creator
	
	Returns: True if got valid file Type
*/
Boolean GetDialogItemType(long *typeCreator, short itemHit, DialogPtr dPtr)
{	Handle		itemHandle;
	short		itemType;
	Rect		box;
	Str255		itemString;

	GetDItem(dPtr, itemHit, &itemType, &itemHandle, &box);
	GetIText(itemHandle, itemString);
	*typeCreator = 0L;
	if (itemString[0] >= 4) 
		BlockMove((Ptr)&itemString[1], typeCreator, 4);
	else ;					/* File type not specified */
	
	return (itemString[0] >= 4);
} 	/* End of () */

/**********************************************************************
	Get get the file file information of interest (eg. type or creator)
	
	Returns: OSErr
*/
OSErr SetUpFileParam(AppFile *thefile, CInfoPBRec *block)
{	OSErr		err;
	
	block->hFileInfo.ioCompletion=0;
	block->hFileInfo.ioNamePtr=thefile->fName;
	block->hFileInfo.ioVRefNum=thefile->vRefNum;
	block->hFileInfo.ioFVersNum=0;
	block->hFileInfo.ioFDirIndex=0;

	err = PBGetFInfoSync( block );
	
	return err;
}	/* End of () */


/**********************************************************************
	Get get the file file information of interest (eg. type or creator)
	
	Returns: OSErr
*/
OSErr FSSetUpFileParam(FSSpec *thefile, CInfoPBRec *block)
{	OSErr		err;
	
	block->hFileInfo.ioCompletion=0;
	block->hFileInfo.ioNamePtr=thefile->name;
	block->hFileInfo.ioVRefNum=thefile->vRefNum;
	block->hFileInfo.ioFVersNum=0;
	block->hFileInfo.ioFDirIndex=-1;
	block->hFileInfo.ioDirID= thefile->parID;
	block->hFileInfo.ioFlParID= thefile->parID;
	
	err = PBGetFInfoSync( block );
	
	return err;
}	/* End of () */

OSErr ReadHeader(FSSpec *sfFilePtr, long *data)
{	OSErr err;
	long longCount;
	short fRef;
	
	*data = 0L;
	
	err = FSpOpenDF(sfFilePtr, fsRdPerm, &fRef);
	if (err == noErr) {                 
		err = SetFPos(fRef, fsFromStart, 0L);	/* skip header*/
		if (err == noErr) {
	      	longCount = sizeof(long);
			err = FSRead(fRef, &longCount, (Ptr)data);
			}
		}
	FSClose(fRef);								/* close the input file */
	if (err == eofErr) err = noErr;				/* May not have an Data fork! */
	return err;
}	/* End of () */

long	LookType(FSSpec *spec, long fdtype)
{	OSErr	err;
	long	data;
	
	/* Check internal file format to suggest the correct headers */
	err = ReadHeader(spec, &data);
	if (err == noErr) {
		short	low, high;
		/* TIFF = II or MM */
		/* BMP  = BM */
		/* EPSF  = %! */
		/* GIFF = GIF87a */
		low = data & 0xffff;
		high = data >> 16;
		if (data == 'GIF8') fdtype = 'giff';
		else
		if (high == 'II' || high == 'MM') fdtype = 'TIFF';
		else
		if (high == '%!' || high == 0xC5D0) fdtype = 'EPSF';
		else
		if (high == 'BM') fdtype = 'BINA';	
		
		
	} else 
		ReportError(err);
	return fdtype;
}	/* End of () */

Boolean	AskNewTypeCreator(DialogPtr dPtr, short sysfRef,
					long *type, long *creator, 
					short *changeType, short *changeCreator, short *EOLmode, 
					FSSpec *file)
{	short		itemHit = -1;
	Handle		itemHandle;
	short		itemType;
	Rect		itemBox;
	short		temp;
	Str15		sType, sCreator;
	OSType		oldCreator;
	OSErr err;
	
	OSErr GetApplicationName(OSType fCreator, FSSpec *file, short ioVRefNum);

		/* EOL support items setup */
	PushRadioButton (dPtr, *EOLmode);
		
		/* Extra information options items setup */
	
	sCreator[0] = sType[0] = 4;
	GetDialogItemType((long *) (&sType[1]), TYPE, dPtr);
	GetDialogItemType((long *) (&sCreator[1]), CREATOR, dPtr);
	ParamText(sCreator, sType, file->name, nil);
	
	SetDialogItemType(*creator, CREATOR, dPtr);
	SetDialogItemType(*type, TYPE, dPtr);
	
	SelIText(dPtr, CREATOR, 0, 4);
	
	GetDItem(dPtr, checkCreator, &itemType, &itemHandle, &itemBox);
	SetCtlValue((ControlHandle) itemHandle, (*changeCreator == 1));
	GetDItem(dPtr, checkType, &itemType, &itemHandle, &itemBox);
	SetCtlValue((ControlHandle) itemHandle, (*changeType == 1));
	
	GetDialogItemType(creator, CREATOR, dPtr);
	GetDItem(dPtr, CreatorApplicationID, &itemType, &itemHandle, &itemBox);
	err = GetApplicationName(*creator, file, file->vRefNum);
	if (err != noErr) err = GetApplicationName(*creator, file, sysfRef);
	
	if (err == noErr) {
		SetIText(itemHandle, file->name);
		oldCreator = *creator;
	} else 
		SetIText(itemHandle, "\p");
		
	do {
	
		ModalDialog(GenericFilter, &itemHit);
		
		GetDItem(dPtr, itemHit, &itemType, &itemHandle, &itemBox);
		
		if ((radCtrl | ctrlItem) == itemType) {	/* Invert Check Boxes */
			switch (itemHit) {
				case DOS:
				case IGNORE:
				case MAC:
				case UNIX:	PushRadioButton (dPtr, *EOLmode = itemHit);
					break;
				}
		} else
		
		if ((chkCtrl | ctrlItem) == itemType) {	/* Invert Check Boxes */
			temp = GetCtlValue((ControlHandle) itemHandle);
			temp = (temp + 1) & 1;
			SetCtlValue((ControlHandle) itemHandle, temp);
			
			switch (itemHit) {
				case checkCreator:	*changeCreator = temp;	break;
				case checkType:		*changeType = temp;		break;
					}
		} else {	/* End if Check box... */
			if (itemHit == CREATOR) {
				GetDialogItemType(creator, CREATOR, dPtr);
				if (*creator != oldCreator) {
					if (GetDialogItemType((long *) (&sCreator[1]), CREATOR, dPtr)) {					
						GetDItem(dPtr, CreatorApplicationID, &itemType, &itemHandle, &itemBox);
						file->name[0] = 0;
						err = GetApplicationName((OSType) *creator,file, file->vRefNum);
						if (err != noErr)
							err = GetApplicationName(*creator, file, sysfRef);
						oldCreator = *creator;
						SetIText(itemHandle, file->name);
						}
				}
			}
			GetDialogItemType(type, TYPE, dPtr);
			if (*type == 'TEXT') {
			
			} else {
			
			}
		}
	} while ((itemHit != OK) && (itemHit != CANCEL));
	
	if (itemHit == CANCEL)	return (false);
	else {
	
		GetDialogItemType(type, TYPE, dPtr);
		GetDialogItemType(creator, CREATOR, dPtr);
		
		return (true);
		}
}	/* End of () */


void	HandleUpdates()
{
/*	Used to handle window updates for windows other than modal dialogs.*/
/*	We don't have any, so return without doing anything.*/
}

/******************************************************************************
** I put this call in a separate procedure to emphasize the fact that I'm
** modifying a global variable, dPtr, with the side effect of putting a dialog
** on the screen.  I don't want to display the dialog if we just double-click
** on the application, so I'm calling this routine at a wierd time.
*/
DialogPtr DisplayDialog()
{	DialogPtr	dPtr;

	ParamText("\p????", "\p????", nil, nil);
	dPtr = GetNewDialog(128, nil, (WindowPtr) -1);
	return dPtr;
}

OSErr DoConvertEOF(FSSpec *sfFile, short EOLmode)
{	OSErr err;
	Str255		oldName;	/** modify the EOL characters in the file */
	short		oldVol, WDRef;
	
	err = OpenWD(sfFile->vRefNum,sfFile->parID, 0, &WDRef); 
	if (err == noErr) {
		err = GetVol (oldName, &oldVol);
		err = SetVol (0, WDRef);
		
		p2cstr(sfFile->name);
		ConvertFile ((char *) sfFile->name, EOLmode);
		c2pstr((char *) sfFile->name);
		
		err = CloseWD(WDRef); 
		err = SetVol (0, oldVol);
		}
	return err;			
}	/* End of () */

void ProcessFiles(DialogPtr dPtr, short count, short sysRef)
{	AppFile		thefile;
	FInfo		ioFlFndrInfo;
	FSSpec		spec;
	OSErr 		err;
	
/********************************************************************************
>>   change the file type and creator of any file dropped onto this program.
*/
	GetAppFiles(1,&thefile);				/* get Type & Creator of 1st file. */
	err = MakeWDSpec(&spec, thefile.vRefNum, thefile.fName );
	if (err == noErr) {
		err = FSpGetFInfo(&spec, &ioFlFndrInfo);
		}
	
	if (err == noErr) {
	
		long		type, creator;
		short		EOLmode, changeType, changeCreator, i;
	
		changeType = changeCreator = true;
		EOLmode = IGNORE;

		creator = ioFlFndrInfo.fdCreator;
		SetDialogItemType(creator, CREATOR, dPtr);
		type = ioFlFndrInfo.fdType;
		SetDialogItemType(type, TYPE, dPtr);
		type = LookType(&spec, ioFlFndrInfo.fdType);
		
		/* Prompts for the file type and creator, 
		*/
		if (AskNewTypeCreator(dPtr, sysRef,
								&type, &creator, &changeType, &changeCreator, 
								&EOLmode, &spec)) {
			
			for(i=1; i<=count && err == noErr; i++)	{
				/* changes all files dropped at the same time 
				** to the same type and creator.
				*/
				GetAppFiles(i,&thefile);
				err = MakeWDSpec(&spec, thefile.vRefNum, thefile.fName );
				err = FSpGetFInfo(&spec, &ioFlFndrInfo);

				if (err == noErr) {
					Boolean changed = false;
					
					if (changeCreator && ioFlFndrInfo.fdCreator != creator)	{
						ioFlFndrInfo.fdCreator = creator;
						changed = true;
						}
					if (changeType && ioFlFndrInfo.fdType != type) {
						ioFlFndrInfo.fdType = type;
						changed = true;
						}
					
					if (changed) {		/* only write change if they happen */
						err = FSpSetFInfo(&spec,&ioFlFndrInfo);

						if (err == noErr)
							err = TickleParent(&spec);	/* Force Finder to update folder info... */
						}
					}	/* End if no err */
				
				if (err != noErr)	ReportError(err);	/* End if Err */
				else
				if (type == 'TEXT' && EOLmode != IGNORE)	{
					err = DoConvertEOF(&spec, EOLmode);
					
					err = FSpSetFInfo(&spec, &ioFlFndrInfo);

					if (err != noErr)	ReportError(err);	/* End if Err */
					}
				}	/* End for */
		}
	} else {
		ReportError(err);
		}
}	/* End of ProcessFiles() */

#ifdef SelectAFile 
Boolean SelectFile(DialogPtr dPtr, short sysRef)
{	StandardFileReply reply;
	SFTypeList typeList;
	OSErr err;
	FInfo		ioFlFndrInfo;
	long		type, creator;
	short		EOLmode, changeType, changeCreator, i;
	
	StandardGetFile(nil, -1,	typeList, &reply);
	if (reply.sfGood) {
			
		
		changeType = changeCreator = true;
		EOLmode = IGNORE;
		
		err = FSpGetFInfo(&reply.sfFile, &ioFlFndrInfo);
	
		creator = ioFlFndrInfo.fdCreator;
		type = ioFlFndrInfo.fdType;
		SetDialogItemType(creator, CREATOR, dPtr);
		SetDialogItemType(type, TYPE, dPtr);
		type = LookType(&reply.sfFile, ioFlFndrInfo.fdType);
		
		/* Prompts for the file type and creator, 
		*/
		if (err == noErr)	
		if (AskNewTypeCreator(dPtr, sysRef,
								&type, &creator, &changeType, &changeCreator, 
								&EOLmode, &reply.sfFile)) {
			
			Boolean changed = false;
				
			err = FSpGetFInfo(&reply.sfFile, &ioFlFndrInfo);
			if (err == noErr) {
				if (changeCreator && ioFlFndrInfo.fdCreator != creator)	{
					ioFlFndrInfo.fdCreator = creator;
					changed = true;
					}
				if (changeType && ioFlFndrInfo.fdType != type) {
					ioFlFndrInfo.fdType = type;
					changed = true;
					}
				
				if (changed) {		/* only write change if they happen */
					err = FSpSetFInfo(&reply.sfFile,&ioFlFndrInfo);
	
					if (err == noErr)
						err = TickleParent(&reply.sfFile);	/* Force Finder to update folder info... */
					}
				}		
			
			if (err == noErr && type == 'TEXT' && EOLmode != IGNORE)	{
				err = DoConvertEOF(&reply.sfFile, EOLmode);
				err = FSpSetFInfo(&reply.sfFile, &ioFlFndrInfo);
				}
			}	/* End if AskNewTypeCreator() */
			
		if (err != noErr)	ReportError(err);	/* End if Err */
	}
	return reply.sfGood;
}	/* End of () */
#endif

void	main()
{	short	message, count;
	Ptr	size;
	DialogPtr	dPtr;
	SysEnvRec	theWorld; 						/* Environment record */
	OSErr err;
	GrafPtr	SavedPort;
	
	size = GetApplLimit();
	SetApplLimit(size - 1024L);		/* make room on stack so Quickdraw can do big pictures */
	
 	MaxApplZone();
	
	InitMacintosh();
	
	CountAppFiles(&message, &count);
	if (message == 1) {
		SysBeep(0);		/* Print items... ignore */
		ExitToShell();
		}
		
	GetPort(&SavedPort);			
		
	err = SysEnvirons(1, &theWorld);     		/* Check how old this system is */
	dPtr = DisplayDialog();
	if (dPtr) {
		if (theWorld.systemVersion >= 0x0700)
					SetDialogTracksCursor(dPtr, true);
					
		if (count > 0) {	
			ProcessFiles(dPtr, count, theWorld.sysVRefNum);
		} else {
		#ifdef SelectAFile 
			SelectFile(dPtr, theWorld.sysVRefNum);
		#else
			SysBeep(0);
		#endif
			}
			
		DisposDialog(dPtr);
	} else {
		ReportError(MemError());	/* End if Err */
		SysBeep(0);								/* No dialog ... out of memory */
		}
	
	SetPort(SavedPort);
	ExitToShell();
}	/* End of main() */
