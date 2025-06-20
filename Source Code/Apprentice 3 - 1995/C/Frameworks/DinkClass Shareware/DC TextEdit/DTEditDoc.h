/*
	File:		DTEditDoc.h

	Written by:	Mark Gross

	Copyright:	� 1992 by Applied Technical Software, all rights reserved.
	Use at your own risk.

*/

// This is the class declaration for the DTEditDoc subclass
// of DDocument

#ifndef __DTEDITDOC__
#define __DTEDITDOC__

#include "DDocument.h"

class DTEditDoc : public DDocument
{
public:
	
	TEHandle		fText;
	
	DTEditDoc();
	~DTEditDoc();
		
	virtual 	DDocument* Init( Boolean OpenFromFile);
	
	virtual void 	AEInitDoc(FSSpec *theFSS);

	virtual void HandleMenuChoice(short menuID, short menuItem);			
				
	virtual void SetUpMenues(void);
	virtual DWindow* 	MakeWindow(Boolean hasColorWindows);

	virtual OSErr 		ReadData(short refNum, long *size);
	virtual OSErr 		WriteData(short refNum);

protected:

	virtual void DoPaste(void);
	virtual void DoCopy(void);
	virtual void DoClear(void);
	virtual void DoCut(void);
	virtual void DoSellectAll(void);

}; //end of DTEditDoc class declaration


#define	kHAdjust	50
#define	kWAdjust	40

#define kTEMargin	5
#define kMaxShort	32767

#endif __DTEDITDOC__
