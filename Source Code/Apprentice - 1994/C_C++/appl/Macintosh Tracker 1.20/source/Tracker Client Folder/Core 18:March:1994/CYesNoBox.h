/* CYesNoBox.h */

#pragma once

#include "CModalDialog.h"
#include "CSimpleButton.h"

/* possible strings for the buttons */
#define YesStringID (132L*65536L + 1)
#define NoStringID (132L*65536L + 2)
#define CancelStringID (132L*65536L + 3)

struct	CYesButton	:	CSimpleButton
	{
		MyBoolean*	AnswerLoc;

		void				IYesButton(CWindow* TheModalDialog, MyBoolean* Result, Handle Text);
		MyBoolean		DoThang(void);
	};

struct	CNoButton	:	CSimpleButton
	{
		MyBoolean*	AnswerLoc;

		void				INoButton(CWindow* TheModalDialog, MyBoolean* Result, Handle Text);
		MyBoolean		DoThang(void);
	};

struct	CYesNoBox	:	CModalDialog
	{
		MyBoolean		ShouldIDoIt(long MessageID, Handle ExtraText, long YesTextID, long NoTextID);
	};
