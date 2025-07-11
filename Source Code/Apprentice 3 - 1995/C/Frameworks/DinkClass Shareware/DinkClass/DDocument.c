/*
	File:		DDocument.cp

	Contains:	xxx put contents here xxx

	Written by:	Mark Gross

	Copyright:	� 1992 by Applied Technical Software, all rights reserved.

	Change History (most recent first):

		 <6>	12/31/92	MTG		making the code conditionaly compiled so         that I am
									always working with a current         version in either think c
									or MPW C++
		 <5>	11/14/92	MTG		Bringing the C++ version up to date WRT the ThinkC version.
		 <4>	 9/20/92	MTG		Bringing this C++ code uptodate with the THINK C version
		 <3>	  8/9/92	MTG		merging changes from the ThinkC version
		 <2>	  8/8/92	MTG		cleaning up un used variables

	To Do:
*/

// the is the Class definition for the DDocument class


#include "DDocument.h"
#include "DWindow.h"
#include "DScrollWindow.h"
#include "DEventHandler.h"
#include "DApplication.h"
#include <string.h>

DDocument::DDocument(void)
{
	fDWindow = NULL;
	fDataHandle = NULL;
	fFileRef = 0;
	fKMNxtCalledFromWindow = FALSE;
	fNeedToSave = FALSE;	
}


DDocument::~DDocument()
{
	DisposPtr( (Ptr) fFileReply);
	if (fFileRef != 0) 
		FSClose(fFileRef);
		
	if(fDataHandle != NULL)
	{
		DisposHandle(fDataHandle);
		fDataHandle = NULL;
	}
}// end of document destructor

		
DDocument* DDocument::Init( Boolean OpenExistingFile)
{
	SFTypeList localTypeList;
	
	fDWindow = NULL;
	fDataHandle = NULL;
	fFileRef = 0;
	fFileReply = (StandardFileReply *)NewPtr( sizeof(StandardFileReply) );
	
	fNeedToSave = FALSE;
	
	fNextHandler = gApplication;
	fCreator = gApplication->fCreator;
	
	gApplication->GetFileType(localTypeList);
	fFileType = localTypeList[0];
	
	if (OpenExistingFile)
	{
		SellectFile(fFileReply);
		if( fFileReply->sfGood)
		{
			fFileType = fFileReply->sfType;
			if(FALSE == OpenFile( &(fFileReply->sfFile)) )
		 		return NULL; 
		 }// end if user didn't cancel
		 else
		 	return NULL;
	}
	else
	{
		fNeedToSave = FALSE;
	}
	
	return this;
		
}//end member function Init


void DDocument::AEInitDoc(FSSpec *theFSS)
{
	SFTypeList localTypeList;
	StringPtr title;
	
	fDWindow = NULL;
	fDataHandle = NULL;
	fFileRef = 0;
	fFileReply = (StandardFileReply *)NewPtr( sizeof(StandardFileReply) );
	
	BlockMove( theFSS, &(fFileReply->sfFile), sizeof(FSSpec));
			// we need to copy the FSSpec to the fFileReply member
			// for the Save() function to opperate correctly.	
	fNeedToSave = FALSE;
	
	fNextHandler = gApplication;
	fCreator = gApplication->fCreator;
	
	gApplication->GetFileType(localTypeList);
	fFileType = localTypeList[0];
	
	OpenFile( theFSS );
	title = theFSS->name;

	MakeWindow(HasColorQD());
	if(fFileRef != 0) //fFileRef is the file open flag, if its false then we just opened a
		SetWTitle(fDWindow->fWindowPtr, title); // stationary pad.
		
}//end member function AEInitDoc



Boolean DDocument::DoSaveAs(void)
{
	if (fFileRef != 0) 
		FSClose(fFileRef);
		
	fFileRef = 0;
	
	return SaveFile();;
}// end of member function DoSaveAs



Boolean DDocument::WindowClosed(DWindow *deadWindow)
{
	short saveit;
		
	if( fNeedToSave )
	{
		saveit = WantToSave(fDWindow->fWindowPtr);
		if(saveit == iYes)
		{
			if(SaveFile())
			{
				fKMNxtCalledFromWindow = TRUE;
				KillMeNext();
				return TRUE;
			}
			else
			{
				gApplication->fDone = FALSE;// if quitting then user has change thier mind....
				return FALSE;// USER CALNCELD OPPERATION!!!
			}
		}
		else if (saveit == iCancel)
		{
			gApplication->fDone = FALSE;// if quitting then user has change thier mind....
			return FALSE;// USEER CANCELED THE OPPERATION!!!!!
		}
	}
	if( fDataHandle)
	{
		DisposHandle( fDataHandle);
		fDataHandle = NULL;
	}

	fKMNxtCalledFromWindow = TRUE;
	KillMeNext();
	return TRUE;
	
}// end of member function WindowClosed


//
// DDocument::KillMeNext has 2 entry points, one from windowclosed
// the other is hidden in DApplciation::CleanUp.  
// Hence, I do noting and return FALSE unless the call comes from 
// WindowClosed.
//
Boolean DDocument::KillMeNext(void)
{
	Boolean inheritedSuccess = FALSE;

	if(fAlive) // fAlive flag is here to make the 
	{			// kill them all process of quitting function correctly
				// Without it, is code could be exicuted 2 times.
				// Resulting in two WantToSave calls at best, crash at worst.
		if (fKMNxtCalledFromWindow )		
			inheritedSuccess = inherited::KillMeNext();

		return inheritedSuccess;// only TRUE if called from fDWindow...
	}

	return inheritedSuccess;
}//end of KillMeNext method.


DWindow* DDocument::MakeWindow(Boolean hasColorWindows)
{	
	DWindow *newWindow;
	
	//newWindow = new DWindow;
	newWindow = new DScrollWindow;
	fDWindow = newWindow;
		
	if (newWindow->Init(this, hasColorWindows))
		return newWindow;
	else
	{
		fDWindow = NULL;
		return fDWindow;
	}	
}//end member function MakeWindow



short DDocument::WantToSave(WindowPtr theWindow)
{
	Str255	title;
	Str255	nullStr;
	
	*nullStr = 0;
	
	if(theWindow)
	{
		GetWTitle(theWindow, title);
		ParamText(title, nullStr, nullStr, nullStr);
	}
	else
		ParamText(nullStr,nullStr,nullStr,nullStr);
	
	return Alert(rWantToSave, NULL);
}// end of WantToSave member function


//
// DEventHandler overrides
//

void DDocument::HandleMenuChoice(short menuID, short menuItem)
{
	if( menuID == rFileMenu)
	{
		switch (menuItem)
		{
			case iSave:
				SaveFile();
				break;
				
			case iSaveAs:
				DoSaveAs();
				break;
				
			case iClose:
				fDWindow->KillMeNext(); // the Window then informs document that its ok to rest in peace.
				break;

			case iPrint:
				fDWindow->DoPrint();
				break;

			case iPageSetup:
				fDWindow->DoPageSetUp();
				break;

			default:
				break;
		}// end switch on menuItem
	}// end if fileMenu
	
	inherited::HandleMenuChoice( menuID, menuItem);
}			

void DDocument::SetUpMenues(void)
{
	MenuHandle	menu;
	
	menu = GetMHandle(rFileMenu);
	EnableMenuItem( menu, iClose, TRUE);
	//EnableMenuItem( menu, iSave, TRUE);
	//EnableMenuItem( menu, iSaveAs, TRUE);
	//EnableMenuItem( menu, iPageSetup, TRUE);
	//EnableMenuItem( menu, iPrint, TRUE);
	
	
	inherited::SetUpMenues( );
}



void	DDocument::DoCut(void)
{
	
}// end of DoCut member function


void	DDocument::DoCopy(void)
{
}// end of DoCopy member function


void	DDocument::DoPaste(void)
{
	// big STUB!!!
}

void	DDocument::DoClear(void)
{
	// big STUB!!!
}


void	DDocument::DoUndo(void)
{
	// big STUB!!!
}


void	DDocument::DoSellectAll(void)
{
	// big STUB!!!
}

 void DDocument::SellectFile(StandardFileReply* reply)
{
	SFTypeList theTypeList;
	int numOfTypes;
			
	numOfTypes = gApplication->GetFileType(theTypeList);
	
	StandardGetFile(NULL, numOfTypes, theTypeList, reply);
	
}// end member function SellectFile


			
	
		
Boolean DDocument::OpenFile(FSSpecPtr specPtr)
{
	OSErr fileError;
	short fileRef;
	long dataCount;
	Boolean isStationary;
	FInfo theFInfo;

	
	fileError = FSpOpenDF( specPtr, fsCurPerm, &fileRef);
	if(fileError != noErr)
	{
		if(fileError == opWrErr)
			ErrorAlert(rErrorStrings, sFileOpen);
		else
			ErrorAlert(rErrorStrings, sUnknownErr);
				
		 return FALSE;
	}
	fFileRef = fileRef; // ok, we have an open file lets try to get
						// the length of this guy.
	
	fileError = GetEOF( fileRef, &dataCount);
	if(fileError != noErr)	
	{
		FSClose(fileRef);
		fFileRef = 0;
		return FALSE;
	}

//
// Ok, we've gotten this far, the file is open and
// its time to read the data.
//
	if( ReadData(fileRef, &dataCount) != noErr)
	{
		FSClose(fileRef);
		fFileRef = 0;
		return FALSE;
	}

	FSpGetFInfo(specPtr, &theFInfo); //get finder info to check for stationary padness
	
	isStationary = ( (theFInfo.fdFlags & 0x800) != 0);
	if(isStationary)
	{
		FSClose(fileRef);// close that file and set
		fFileRef = 0;	// the flag indicating that no file has been opened (keep the window untitled!
		fNeedToSave = TRUE;
	}
	
	return TRUE;
}// end member function OpenFile



OSErr DDocument::ReadData(short refNum, long *size)
{
	OSErr fileError;
	Handle data;

//
// first make some place to put the data
//
	data = NewHandle(*size);
	if(data == NULL)
		return memFullErr;
//
// get it ready, and read into the space...
//	
	HLock(data);
	fileError = FSRead(refNum, size, *data);
	HUnlock(data);

//
// if problem, free space up an return error.
//	
	if(fileError != noErr)
	{
		DisposHandle( data);
		return fileError;
	}
//
// otherwise put the data in an object member, for your use.
// note that your overrides of this need not use fDataHandle.
// In fact fDataHandle realy shouldn't be in this class,  your
// sub-classes should have thier own place to put thier data.
// (its here for guidence)
//	
	fDataHandle = data;
	return fileError;	
}// end member function ReadData




Boolean DDocument::SaveFile(void)
{
	OSErr fileError;
	FSSpecPtr specPtr;
	short refNum;
	FInfo theFInfo;
	StandardFileReply* reply;			
	
	if (fFileRef == 0)	// then we need to save as, and better define a file for it.
	{
		reply = fFileReply;
		StandardPutFile("\pSave file as:", "\pUntitled", reply);
		fFileReply = reply;
	}
	else
	{
		FSClose(fFileRef);
		fFileRef = 0;
		fFileReply->sfReplacing = TRUE;
	}
	
	if(!fFileReply->sfGood)
	{
		return FALSE;
		// user canceled save opperation
	}
	
	
	specPtr = &(fFileReply->sfFile);
//
// Buffer up any finder info stuff the file may have, so that we
// may transfrer to the replaceing file yet to be created.
//	
	if( fFileReply->sfReplacing)
	{
		FSpGetFInfo(specPtr, &theFInfo); //get finder info BEFORE the file is deleted
		fFileReply->sfFlags = theFInfo.fdFlags; // keep it correct for the file to be created...
		
		fileError = FSpRstFLock(specPtr);
		if( !((fileError == fnfErr) || (fileError == noErr)) )
			return FALSE;
	
		if(fileError == noErr)
		{
			fileError = FSpDelete(specPtr);//risky for my liking, but its the easy way to do files.
			if(fileError != noErr)
				return FALSE;
		}
	}
	fileError = FSpCreate( specPtr, gApplication->fCreator, 
				fFileType, reply->sfScript);
				
	if ( fileError != noErr)
		return FALSE;
	if( fFileReply->sfReplacing)
		FSpSetFInfo(specPtr, &theFInfo);
//		
// regardless if a save as was done, we now have a closed file redy to be written
// too and it needs opening. We may have deleted
// and open file, too bad.
//

	fileError = FSpOpenDF(specPtr, fsCurPerm, &refNum);
	if ( fileError != noErr)
		return FALSE;

// 
// Now we are ready to write the data out.
//		
	fileError = WriteData(refNum);
	if ( fileError != noErr)
		return FALSE;
		
	fFileRef = refNum;	
	fNeedToSave = FALSE;
	return TRUE;
	
}//end member function SaveFile




OSErr DDocument::WriteData(short refNum)
{
	OSErr fileError;
	Handle data;
	long dataCount;

//
// get the data into a local var and get its size
//
	data = fDataHandle;
	if(data == NULL)
		return FALSE;
	dataCount = GetHandleSize(data);
	
//
// Lock that pup down and write out the info
//
	HLock(data);
	fileError = FSWrite(refNum, &dataCount, *data);
	HUnlock(data);
	
	return fileError;	
}//end member function WriteData

