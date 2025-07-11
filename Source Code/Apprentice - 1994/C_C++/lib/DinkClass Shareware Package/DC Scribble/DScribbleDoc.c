/*
	File:		DScribbleDoc.c

	Written by:	Mark Gross

	Copyright:	� 1992 by Applied Technical Software, all rights reserved.
	Use at your own risk.

*/
//This is the diffinition of the DScribbleDocument Class

#include "DScribbleDoc.h"
#include "DScribbleWind.h"




DScribbleDoc::DScribbleDoc(void)
{
	// Stub
}

DScribbleDoc::~DScribbleDoc()
{
	if( fPict != NULL)
		KillPicture( fPict);
	if(fPictHeader != NULL)
		DisposHandle(fPictHeader);
}


DDocument* DScribbleDoc::Init( Boolean OpenFromFile)
{
	DDocument *inheritedDoc;
	
	fPict = NULL;
	fPictHeader = NULL;
	
	inheritedDoc = inherited::Init(OpenFromFile);

 	return inheritedDoc;
}


void DScribbleDoc::AEInitDoc(FSSpec *theFSS)
{
	fPict = NULL;

	inherited::AEInitDoc(theFSS);		
}//end member function AEInitDoc

 
 void DScribbleDoc::HandleMenuChoice(short menuID, short menuItem)
 {
 	if(menuID == rPenMenu)
	{
		switch(menuItem)
		{
			case i1X1:
				((DScribbleWind *)fDWindow)->fPenSize = 1;
				break;
			case i2X2:
				((DScribbleWind *)fDWindow)->fPenSize = 2;
				break;
			case i3X3:
				((DScribbleWind *)fDWindow)->fPenSize = 3;
				break;
			case iBlack:
				((DScribbleWind *)fDWindow)->fPenPat = patBlack;
				break;
			case iGray:
				((DScribbleWind *)fDWindow)->fPenPat = patGray;
				break;
			case iWhite:
				((DScribbleWind *)fDWindow)->fPenPat = patWhite;
				break;
			default:
				break;
		}//end switch menuitem
	}// end if rPenMenu
		
	inherited::HandleMenuChoice(menuID, menuItem);
	
}// end of HandleMenuChoice member fuction
 
 
 			
 void DScribbleDoc::SetUpMenues(void)
 {
	MenuHandle	menu;
	
	menu = GetMHandle(rPenMenu);

	EnableMenuItem( menu, i1X1, TRUE);
	EnableMenuItem( menu, i2X2, TRUE);
	EnableMenuItem( menu, i3X3, TRUE);
	EnableMenuItem( menu, iBlack, TRUE);
	EnableMenuItem( menu, iGray, TRUE);
	EnableMenuItem( menu, iWhite, TRUE);

	CheckItem(menu,i1X1, ((DScribbleWind *)fDWindow)->fPenSize == 1);
	CheckItem(menu,i2X2, ((DScribbleWind *)fDWindow)->fPenSize == 2);
	CheckItem(menu,i3X3, ((DScribbleWind *)fDWindow)->fPenSize == 3);
	CheckItem(menu, iBlack, ((DScribbleWind *)fDWindow)->fPenPat == patBlack);
	CheckItem(menu, iGray, ((DScribbleWind *)fDWindow)->fPenPat == patGray);
	CheckItem(menu, iWhite, ((DScribbleWind *)fDWindow)->fPenPat == patWhite);
	
	inherited::SetUpMenues();
}// end of SetUpMenues function


DWindow* DScribbleDoc::MakeWindow(Boolean hasColorWindows)
{	
	DScribbleWind *newWindow;
	
	//newWindow = new DWindow;
	newWindow = new DScribbleWind;
	fDWindow = newWindow;
		
	if (newWindow->Init(this, hasColorWindows))
		return newWindow;
	else
	{
		fDWindow = NULL;
		return fDWindow;
	}	
}//end member function MakeWindow



// 
// Read data into the Scribble application spacific data structures.
// This funciton gets called from the OpenFile member function
//

OSErr DScribbleDoc::ReadData(short refNum, long *size)
{
	OSErr fileError;
	Handle data;
	long headerLength;
	
	headerLength = 512;

	fPictHeader = NewHandle(headerLength);
	if(fPictHeader == NULL)
		return FALSE;
	
	HLock(fPictHeader);
	fileError = FSRead(refNum, &headerLength, *fPictHeader);
	HUnlock(fPictHeader);
	
	if(fileError != noErr)
	{
		DisposHandle( fPictHeader);
		return fileError;
	}
	
	*size -= headerLength;

	fPict = (PicHandle)NewHandle(*size);
	if(fPict == NULL)
		return FALSE;
	
	HLock(fPict);
	fileError = FSRead(refNum, size, *fPict);
	HUnlock(fPict);
	
	if(fileError != noErr)
	{
		DisposHandle( fPict);
		return fileError;
	}
	
	fFileRef = refNum;
	return fileError;	
}

//
// Write the data out to the disk
//

OSErr DScribbleDoc::WriteData(short refNum)
{
	OSErr fileError;
	Handle data;
	long dataCount;

	data = (Handle) fPict;
	if(data == NULL)
		return FALSE;
	
	// first write out the header info
	data = fPictHeader;
	dataCount = 512;
	HLock(data);
	fileError = FSWrite(refNum, &dataCount, *data);
	HUnlock(data);

	// next write out the pict itself
	data = (Handle) fPict;
	dataCount = GetHandleSize(data);	
	HLock(data);
	fileError = FSWrite(refNum, &dataCount, *data);
	HUnlock(data);
	
	return fileError;	
}
