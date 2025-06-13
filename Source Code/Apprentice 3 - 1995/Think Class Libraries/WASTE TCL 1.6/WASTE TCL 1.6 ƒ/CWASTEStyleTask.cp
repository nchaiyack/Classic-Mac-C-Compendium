/*
 *
 *	File:		CWASTEStyleTask.cp
 *	Module:		CWASTEStyleTask method definitions
 *	System:		Mark's Class Library
 *	Evironment:	MacOS 7.0/THINK C 6.0/TCL 1.1.3
 *	Author:		Mark Alldritt
 *
 *
 *		Copyright © 1994	All Rights Reserved
 *		Mark Alldritt
 *		1571 Deep Cove Road
 *		N. Vancouver, B.C. CANADA
 *
 *	
 *	Description:
 *
 *  This class modifies the CTextStyleTask to allow it to work with CWASTEPane.
 *
 *
 *	Acknowledgemets:
 *
 *	None.
 *
 *
 *	Notes:
 *
 *	This source file depends on the "Scriptable TCL+MacTraps" pre-compiled header for all TCL
 *	definitions.
 *
 *
 *	Edit History:
 *
 *	V1-0.0	Mark Alldritt	10-May-1994
 *	- Initial version of module.
 *
 */


//#include "GlobalVars.h"
#include "CWASTEText.h"
#include "CWASTEStyleTask.h"


extern RGBColor RGBBlack;
extern RGBColor RGBRed;
extern RGBColor RGBGreen;
extern RGBColor RGBBlue;
extern RGBColor RGBCyan;
extern RGBColor RGBMagenta;
extern RGBColor RGBYellow;



CWASTEStyleTask::CWASTEStyleTask(CWASTEText *aTextPane, 
								 long aStyleCmd, short firstTaskIndex)
	: CTextStyleTask( aTextPane, aStyleCmd, firstTaskIndex)
{
	long selStart, selEnd;
	
	oldStyles = NULL;
	
	itsTextPane->GetSelection( &selStart, &selEnd);
	this->selStart = selStart;
	this->selEnd = selEnd;
	
}	/* CWASTEStyleTask::IWASTEStyleTask */


CWASTEStyleTask::~CWASTEStyleTask( void)
{
	TCLForgetHandle( oldStyles);
}

/******************************************************************************
 Do  MF nov -- we have to alter this for cmdJustify and color

 	Saves the original formatting, then performs the user's formatting command
 	on the text pane.
******************************************************************************/

void  CWASTEStyleTask::Do()
	{

 	SaveStyle();

	oldAlignCmd = itsTextPane->GetAlignCmd();
	oldSpacingCmd = itsTextPane->GetSpacingCmd();
	
	switch (styleCmd)
		{
		case cmdBlack:
				((CWASTEText*)itsTextPane)->SetFontColor(&RGBBlack);
				styleAttribute = doColor;
				break;
		case cmdRed:
				((CWASTEText*)itsTextPane)->SetFontColor(&RGBRed);
				styleAttribute = doColor;
				break;
		case cmdGreen:
				((CWASTEText*)itsTextPane)->SetFontColor(&RGBGreen);
				styleAttribute = doColor;
				break;
		case cmdBlue:
				((CWASTEText*)itsTextPane)->SetFontColor(&RGBBlue);
				styleAttribute = doColor;
				break;
		case cmdCyan:
				((CWASTEText*)itsTextPane)->SetFontColor(&RGBCyan);
				styleAttribute = doColor;
				break;
		case cmdMagenta:
				((CWASTEText*)itsTextPane)->SetFontColor(&RGBMagenta);
				styleAttribute = doColor;
				break;
		case cmdYellow:
				((CWASTEText*)itsTextPane)->SetFontColor(&RGBYellow);
				styleAttribute = doColor;
				break;
				
	
		case cmdJustify:
				itsTextPane->SetAlignCmd(styleCmd);
				styleAttribute = doAlign;
				break;
		default:
			inherited::Do();
		}
	}



/******************************************************************************
 Undo
 
 	Undo the previously applied formatting. This method swaps the current and
 	previous styles, so it can perform both undo and redo.
******************************************************************************/

void CWASTEStyleTask::Undo( void)
{
	StScrpHandle	swapStyles, newStyles;
	CWASTEText		*styleText = (CWASTEText*) itsTextPane;
	
	swapStyles = newStyles = NULL;

	try_
	{
		if (styleAttribute < doAlign)
		{
			
			itsTextPane->Prepare();	
			itsTextPane->SetSelection( selStart, selEnd, TRUE);
			
			swapStyles = oldStyles;		/* save the current styling		*/
			oldStyles = NULL;
			
			SaveStyle();
		
			newStyles = oldStyles;
			oldStyles = swapStyles;		/* restore the previous styling */
			swapStyles = NULL;
		
			RestoreStyle();
			
			ForgetHandle( oldStyles);
			
			oldStyles = newStyles;		/* swap current and previous	*/
			
			styleText->SetSpacingCmd( styleText->GetSpacingCmd());
			styleText->SetWholeLines( styleText->GetWholeLines());
			styleText->AdjustBounds();
			
			undone = !undone;

		}
		else inherited::Undo();
	}
	catch_all_()
	{
		ForgetHandle( oldStyles);
		ForgetHandle( newStyles);
		ForgetHandle( swapStyles);
	}
	end_try_;

}	/* CWASTEStyleTask::Undo */

/******************************************************************************
 SaveStyle
 
 	Saves the current styling into oldStyles.
******************************************************************************/

void CWASTEStyleTask::SaveStyle( void)
{
	oldStyles = NULL;	/* GetStyleScrap may fail	*/
	
	oldStyles = ((CWASTEText*)itsTextPane)->GetTheStyleScrap();

}	/* CWASTEStyleTask::SaveStyle */

/******************************************************************************
 RestoreStyle
 
 	Restores the saved styles from oldStyles back to the text.
******************************************************************************/

void CWASTEStyleTask::RestoreStyle( void)
{
	if (styleAttribute < doAlign)
	{	
		/* +++ work around problem with redoing cmdPlain on styled text? */
		if ((styleCmd == cmdPlain) && (undone == TRUE))
			itsTextPane->SetFontStyle( NOTHING);
		else
			((CWASTEText *) itsTextPane)->SetTheStyleScrap(selStart, selEnd, oldStyles, TRUE);		
	}
	else
		inherited::RestoreStyle();
		
}	/* CWASTEStyleTask::RestoreStyle */
