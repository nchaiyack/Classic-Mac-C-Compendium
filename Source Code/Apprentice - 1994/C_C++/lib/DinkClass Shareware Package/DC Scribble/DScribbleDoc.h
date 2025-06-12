/*
	File:		DScribbleDoc.h

	Written by:	Mark Gross

	Copyright:	© 1992 by Applied Technical Software, all rights reserved.
	Use at your own risk.

*/

// This is the class declaration of the DScribbleDoc
// sub-class to DDocument
#ifndef __DSCRIBBLEDOC__
#define __DSCRIBBLEDOC__

//class DDocument;
#include "DDocument.h"
#include "DScribbleWind.h"


class DScribbleDoc : public DDocument
{
public:
	
	PicHandle		fPict;
	Handle			fPictHeader;
	
	DScribbleDoc(void);
	~DScribbleDoc(void);
		// the destructor cleans up the member variable handles
		// and the constructor is just a stub there for style....
		
	virtual 	DDocument* Init( Boolean OpenFromFile);
			// put variables into a safe state
	
	virtual void 	AEInitDoc(FSSpec *theFSS);

	virtual void HandleMenuChoice(short menuID, short menuItem);			
	virtual void SetUpMenues(void);
	virtual DWindow* 	MakeWindow(Boolean hasColorWindows);

	virtual OSErr 		ReadData(short refNum, long *size);
	virtual OSErr 		WriteData(short refNum);

}; //end of DScribbleDoc class declaration


#define	kHAdjust	50
#define	kWAdjust	40

#define rPenMenu	131
#define i1X1	1
#define i2X2	2
#define i3X3	3
#define iBlack	5
#define iGray	6
#define iWhite	7
#define	kEveryItem	0

#endif __DSCRIBBLEDOC__
