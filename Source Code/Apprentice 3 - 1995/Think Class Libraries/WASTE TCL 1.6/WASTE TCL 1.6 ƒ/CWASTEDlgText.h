/******************************************************************************
 CWASTEDlgText.c

	A class with some of the functionality of CDialogText, but using CWASTEText

	by Dan Crevier
	version 1.2

 ******************************************************************************/


#pragma once

#include "CWASTEText.h"

#include "CDialogText.h" // to get enum for dialogTextChanged

class CDialog;

class CWASTEDlgText : public CWASTEText
{
public:
	TCL_DECLARE_CLASS

	CWASTEDlgText();
	CWASTEDlgText(CView *anEnclosure, CBureaucrat *aSupervisor,
							short aWidth, short aHeight,
							short aHEncl, short aVEncl,
							SizingOption aHSizing = sizFIXEDSTICKY,
							SizingOption aVSizing = sizFIXEDSTICKY,
							short aLineWidth = -1, Boolean aScrollHoriz = 0,
							Boolean aIsRequired = FALSE, long aMaxValidLength = MAXLONG,
							Boolean aValidateOnResign = TRUE);
	virtual ~CWASTEDlgText();

	void 		IWASTEDlgText(CView *anEnclosure, CBureaucrat *aSupervisor,
					short aWidth, short aHeight,
					short aHEncl, short aVEncl,
					SizingOption aHSizing, SizingOption aVSizing,
					short aLineWidth);

	virtual void 	IViewTemp(CView *anEnclosure, CBureaucrat *aSupervisor,
							Ptr viewData);

	virtual void 	DoKeyDown(char theChar, Byte keyCode, EventRecord *macEvent);
	virtual void 	GetTextString(StringPtr aString);
	virtual void 	PerformEditCommand(long theCommand);

	static pascal void WEPostUpdate(WEHandle hWE, long fixLength, long inputAreaStart,
						long inputAreaEnd, long pinRangeStart, long pinRangeEnd);
	
protected:

	void 			IWASTEDlgTextX();
	virtual void 	MakeBorder();

};
