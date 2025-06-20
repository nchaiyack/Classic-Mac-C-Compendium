/*
	File:		DTEditDoc.c

	Written by:	Mark Gross

	Copyright:	� 1992 by Applied Technical Software, all rights reserved.
	Use at your own risk.

*/

//This is the diffinition of the DTEditDoc-ument Class

#include "DTEditDoc.h"
#include "DTEditWind.h"
#include "DApplication.h"

 
DTEditDoc::DTEditDoc()
{
	fText != NULL;
}

DTEditDoc::~DTEditDoc()
{
	if( fText != NULL)
		TEDispose( fText);
}


DDocument* DTEditDoc::Init( Boolean OpenFromFile)
{
	Rect	view, dest;
	DDocument *inheritedDoc;
	
	dest.left = 4;
	dest.top = 4;
	dest.right = 400;
	dest.bottom = kMaxShort;
	view.left = 0; view.top = 0; view.right = 0; view.bottom = 0;
	fText = TENew(&dest, &view);
	
	inheritedDoc = inherited::Init(OpenFromFile);

 	return inheritedDoc;
}

void DTEditDoc::AEInitDoc(FSSpec *theFSS)
{
	Rect	view, dest;
	DDocument *inheritedDoc;
	
	dest.left = 4;
	dest.top = 4;
	dest.right = 400;
	dest.bottom = kMaxShort;
	view.left = 0; view.top = 0; view.right = 0; view.bottom = 0;
	fText = TENew(&dest, &view);
	
	inherited::AEInitDoc(theFSS);		
}//end member function AEInitDoc


void DTEditDoc::HandleMenuChoice(short menuID, short menuItem)
{
	if( menuID == rEditMenu)
	{
		switch (menuItem)
		{
			case iUndo:
				;//do nothing, for now
				break;
				
			case iCut:
				DoCut();
				break;
				
			case iCopy:
				DoCopy();
				break;
				
			case iPaste:
				DoPaste();
				break;
				
			case iClear:
				DoClear();
				break;
				
			case iSelectAll:
				DoSellectAll();
				break;
				
			default:
				break;
				
		}// end switch on menuItem
	}

	inherited::HandleMenuChoice(menuID, menuItem);
}// end of HandleMenuChoice member fuction
 
 			
void	DTEditDoc::DoSellectAll(void)
{
	TESetSelect(0, kMaxShort, fText);
}


 void DTEditDoc::SetUpMenues(void)
 {
	MenuHandle	menu;
	
	menu = GetMHandle(rEditMenu);

	EnableMenuItem( menu, iUndo, FALSE);
	if ( (**fText).selStart != (**fText).selEnd )
	{
		EnableMenuItem( menu, iCut, TRUE);
		EnableMenuItem( menu, iCopy, TRUE);
		EnableMenuItem( menu, iClear, TRUE);
	}
	else
	{
		EnableMenuItem( menu, iCut, FALSE);
		EnableMenuItem( menu, iCopy, FALSE);
		EnableMenuItem( menu, iClear, FALSE);
	}
	if ( (gApplication->fClipData)&&(gApplication->fClipType == 'TEXT') )
		EnableMenuItem( menu, iPaste, TRUE);
	else
		EnableMenuItem( menu, iPaste, FALSE);
	
	EnableMenuItem( menu, iSelectAll, TRUE);
	
	inherited::SetUpMenues();
}// end of SetUpMenues function


DWindow* DTEditDoc::MakeWindow(Boolean hasColorWindows)
{	
	DTEditWind *newWindow;
	Rect	view;
		
	newWindow = new DTEditWind;
	fDWindow = newWindow;
		
	if (newWindow->Init(this, hasColorWindows))
	{
		newWindow->GetContentRect( &view);
		(**fText).viewRect = view;
		TEAutoView(TRUE, fText);
		
		// if your application needs a click loop set it here using
		// SetClikLoop(MyClickLoop, fTEHandle)
		
		newWindow->ValidateVertScrollRange();

		return newWindow;
	}
	else
	{
		fDWindow = NULL;
		return fDWindow;
	}	
	
}//end member function MakeWindow


OSErr DTEditDoc::ReadData(short refNum, long *size)
{
	OSErr fileError;
	Handle data;
	
	data = NewHandle(*size);
	if(*size > kMaxShort)
		*size = kMaxShort;
	HLock(data);
	fileError = FSRead(refNum, size, *data);
	
	if(fileError != noErr)
	{
		HUnlock(data);
		TEDispose( fText);
		fText = NULL;
		return fileError;
	}
	
	TESetText(*data, *size, fText);
	TESetSelect( 0, 0, fText);
	HUnlock(data);
	DisposHandle(data);
	
	fFileRef = refNum;
	return fileError;	
}


OSErr DTEditDoc::WriteData(short refNum)
{
	fDataHandle = (Handle)TEGetText(fText);
	return inherited::WriteData(refNum);
}



void	DTEditDoc::DoCopy(void)
{
	Handle TEData;

	TECopy(fText);
	TEData = TEScrapHandle();
	
	gApplication->GiveDataToApp(TEData, 'TEXT');

}// end of DoCopy member fuction



void	DTEditDoc::DoClear(void)
{
	TEDelete(fText);
	fNeedToSave = TRUE;
	((DTEditWind *)fDWindow)->SynchScrollBars();
}// end of DoCopy member fuction



void	DTEditDoc::DoCut(void)
{
	DoCopy();
	DoClear();
}// end of DoCut member fuction



void	DTEditDoc::DoPaste(void)
{
	OSType type;
	Handle tempHandle;
	Handle teScrap;
	long 	scrapLen;
	
	tempHandle = gApplication->GetClipCopyFromApp(&type);
	if(type == 'TEXT')
	{
		teScrap = TEScrapHandle( );			
		HandAndHand( tempHandle, teScrap);
		
		scrapLen = GetHandleSize(teScrap);
		TESetScrapLen(scrapLen);

		TEPaste(fText);
		fNeedToSave = TRUE;
		((DTEditWind *)fDWindow)->SynchScrollBars();
	}
}// end of DoPaste member fuction


