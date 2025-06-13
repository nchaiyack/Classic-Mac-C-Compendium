/******************************************************************************
 CWASTEDlgText.c

	A class with some of the functionality of CDialogText, but using CWASTEText

	by Dan Crevier
	version 1.2

 ******************************************************************************/

#ifdef TCL_PCH
#include <TCLHeaders>
#endif

#include "CWASTEDlgText.h"
#include "CDialog.h"
#include <CPaneBorder.h>
#include "CBartender.h"
#include "Constants.h"
#include "CTextEditTask.h"

#define kBorderAmount	2	// white space between border and text of edit field

extern CBartender	*gBartender;
extern CBureaucrat		*gGopher;		// First in line to get commands

static pascal void WEPostUpdate(WEHandle hWE, long fixLength, long inputAreaStart,
	long inputAreaEnd, long pinRangeStart, long pinRangeEnd);

TCL_DEFINE_CLASS_D1(CWASTEDlgText, CWASTEText);


/********************************************************\
 CWASTEDlgText - default constructor
\********************************************************/

CWASTEDlgText::CWASTEDlgText()
{
	TCL_END_CONSTRUCTOR
}


/********************************************************\
 CWASTEDlgText - constructor
\********************************************************/

CWASTEDlgText::CWASTEDlgText(
	CView			*anEnclosure,
	CBureaucrat		*aSupervisor,
	short			aWidth,
	short			aHeight,
	short			aHEncl,
	short			aVEncl,
	SizingOption	aHSizing,
	SizingOption	aVSizing,
	short			aLineWidth,
	Boolean			aScrollHoriz,
	Boolean			aIsRequired,
	long			aMaxValidLength,
	Boolean			aValidateOnResign)

	: CWASTEText(anEnclosure, aSupervisor,
		aWidth, aHeight, aHEncl, aVEncl,
		aHSizing, aVSizing, aLineWidth, aScrollHoriz)
{

	IWASTEDlgTextX();
	TCL_END_CONSTRUCTOR
}


/********************************************************\
 ~CWASTEDlgText - destructor
\********************************************************/

CWASTEDlgText::~CWASTEDlgText()
{
	TCL_START_DESTRUCTOR
}


/********************************************************\
 IWASTEDlgText - initializer, if used with default
 	constructor
\********************************************************/

void CWASTEDlgText::IWASTEDlgText(CView *anEnclosure, CBureaucrat *aSupervisor,
				short aWidth, short aHeight,
				short aHEncl, short aVEncl,
				SizingOption aHSizing, SizingOption aVSizing,
				short aLineWidth)
{

	CWASTEText::IWASTEText(anEnclosure, aSupervisor,
		aWidth, aHeight, aHEncl, aVEncl,
		aHSizing, aVSizing, aLineWidth);

	IWASTEDlgTextX();
}


/********************************************************\
 IViewTemp - construct from View resource
\********************************************************/

void CWASTEDlgText::IViewTemp(CView *anEnclosure, CBureaucrat *aSupervisor,
							Ptr viewData)
{
	CWASTEText::IViewTemp(anEnclosure, aSupervisor, viewData);

	IWASTEDlgTextX();
}


/********************************************************\
 IWASTEDlgTextX - common initialization
\********************************************************/

void CWASTEDlgText::IWASTEDlgTextX()
{
#ifdef WASTE11
	static WETSMPostUpdateUPP	postProc = NULL;
#else
	WETSMPostUpdateProcPtr postProc;
#endif

	MakeBorder();
	SetWholeLines(FALSE);
	
	// set postupdate routine to WEPostUpdate
#ifdef WASTE11
	if (postProc == NULL)
		postProc = NewWETSMPostUpdateProc(WEPostUpdate);
#else
	postProc = &WEPostUpdate;
#endif
	WESetInfo(weTSMPostUpdate, (Ptr)&postProc, macWE);
}

/********************************************************\
 WEPostUpdate -- broadcast a dialog text changed message
\********************************************************/

 pascal void CWASTEDlgText::WEPostUpdate(WEHandle hWE, long fixLength, long inputAreaStart,
	long inputAreaEnd, long pinRangeStart, long pinRangeEnd)
{
	short ID;
	GrafPtr curPort;

	CWASTEText::WEPostUpdate(hWE, fixLength, inputAreaStart, inputAreaEnd, pinRangeStart,
		pinRangeEnd);
		
	if (curWASTEText)
	{
		GetPort(&curPort);
		ID = curWASTEText->ID;
		curWASTEText->BroadcastChange(dialogTextChanged, &ID);
		SetPort(curPort);
		curWASTEText->Prepare();
	}


	if (curWASTEText != NULL)
	{
		// stuff from CWASTEText's WEPostUpdate
		curWASTEText->AdjustBounds();
		curWASTEText->ScrollToSelection();
	}
}


/********************************************************\
 MakeBorder - put a border around the text
\********************************************************/

void CWASTEDlgText::MakeBorder()
{
	Rect	margin;
	CPaneBorder *border;

	border = TCL_NEW(CPaneBorder,());
	border->IPaneBorder(kBorderFrame);
	SetRect(&margin, -kBorderAmount, -kBorderAmount, kBorderAmount, kBorderAmount);
	border->SetMargin(&margin);
	SetBorder(border);
}

/********************************************************\
 DoKey - handle Tab and Return
\********************************************************/

void CWASTEDlgText::DoKeyDown(char theChar, Byte keyCode, EventRecord *macEvent)
{
	Boolean pass = TRUE;
	short	ID;

	switch (theChar)
	{
		case '\t':
		case '\r':
		case kEnterKey:
			pass = FALSE;
			break;

		case kEscapeOrClear:
			if (keyCode == KeyEscape) pass = FALSE;
			break;
	}
	if (pass)
	{
		CWASTEText::DoKeyDown(theChar, keyCode, macEvent);

		if (itsTypingTask && itsTypingTask->CanStillType())
		{
			ID = this->ID;
			BroadcastChange(dialogTextChanged, &ID);
		}
	}
	else
		itsSupervisor->DoKeyDown(theChar, keyCode, macEvent);
}

/********************************************************\
 GetTextString - return the text as a pascal string
\********************************************************/

void CWASTEDlgText::GetTextString(StringPtr aString)
{
	short length = Min(GetLength(), 255);

	StopInlineSession();
	BlockMove(*GetTextHandle(), &aString[1], length);
	aString[0] = length;
}

/********************************************************\
 PerformEditCommand - handle cut, copy, paste, and clear
 	-- check for text changed
\********************************************************/

void CWASTEDlgText::PerformEditCommand(long theCommand)
{
	short	ID;

	CWASTEText::PerformEditCommand(theCommand);

	switch( theCommand)
	{		
		case cmdCut:
		case cmdPaste:
		case cmdClear:
			ID = this->ID;
			BroadcastChange(dialogTextChanged, &ID);
			break;
		
		default:
			break;
	}

}


