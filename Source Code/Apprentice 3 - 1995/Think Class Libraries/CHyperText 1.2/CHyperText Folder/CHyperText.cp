/******************************************************************************
 CHyperText.cp
 
		
	SUPERCLASS = CStyleText
	
	Copyright � 1994 Johns Hopkins University. All rights reserved.
	
	Original Author: 	Martin R. Wachter		email:	mrw@welchgate.welch.jhu.edu
	
	Modified:			4/25/94					by:		mrw			TCL Version:	1.1.3
	Modified:			4/6/94					by:		mrw			TCL Version:	1.1.3
	Created:			4/4/94					by:		mrw			TCL Version:	1.1.3

	Version change history:
	
	1.2		Fixed another coordinate bug where we track for the hypercursor.
			Stylize and now converts all text to lowercase when munging so 
			that it does not distinguish the difference between upper and 
			lowercase hyperwords.
			
			Fixed problem with hyperwords which appeared at the start or end
			of a TEXT buffer.
			
			Fixed a Stylize bug where two alphabetically listed hyperwords 
			(e.g. 'Big' and 'Big Mouth') were Stylized would cause the first to 
			be a hyperword but not the second word.  Now 'Big Mouth' will be stylized
			correctly and will be the hyperword.  In version 1.0, 'Big' would have
			been the only hyperword.  Cool?
			
			Fixed a nasty Styize bug where only 1 style was supported.  Now multiple
			styles are taken into consideration when Stylizing hyperwords.
			
			Made the gGoper point directly at this instead of using
			BecomeGopher() calls in DoClick().  This eliminates some text insertion
			caret blinking.
			
	1.1		Fixed a bug in the way I handled the hyperCursor.
	
	1.0		Initial release.
	
 ******************************************************************************/

#include <CHyperText.h>
#include <Commands.h>
#include <CPaneBorder.h>
#include <CStringArray.h>
#include <ctype.h>

#define kBorderAmount	2		//white space between border and text of edit field
#define kHyperCursorID	1028	// resource id for the magnifying glass hypercursor

extern CDesktop		*gDesktop;	// access to global CDesktop object
extern CError		*gError;	// access to global error handler
extern long			gSleepTime;	// access to global sleep time
extern CBureaucrat	*gGopher;	// access to global gGopher


		/***************     CONSTRUCTION/DESTRUCTION METHODS     ***************/

/******************************************************************************
 IHyperText
 
 	Initialize a CHyperText pane.
******************************************************************************/

void CHyperText::IHyperText(CView *anEnclosure, CBureaucrat *aSupervisor,
						short aWidth, short aHeight,
						short aHEncl, short aVEncl,
						SizingOption aHSizing, SizingOption aVSizing,
						short aLineWidth, Boolean hasBorder)
{
	CStyleText::IStyleText( anEnclosure, aSupervisor, aWidth, aHeight,
				aHEncl, aVEncl, aHSizing, aVSizing, aLineWidth);


	if (hasBorder)
		MakeBorder();

	ownsHyperList = FALSE;

	IHyperTextX();
}

/******************************************************************************
 IViewTemp
 
 	Initialize the CHyperText object from a resource template.
******************************************************************************/

void CHyperText::IViewTemp(CView *anEnclosure, CBureaucrat *aSupervisor,
							Ptr viewData)
{
	tHyperTextTempP data = (tHyperTextTempP) viewData;
	
	CStyleText::IViewTemp( anEnclosure, aSupervisor, viewData);

	itsHyperList = NULL;
		
	if (data->TEXT_Res_ID){
		MoveOffScreen();
		SetTextRes(data->TEXT_Res_ID);
		MoveOnScreen();
	}

	if (data->STRlist_Res_ID){
		SetListRes(data->STRlist_Res_ID);
	}
	else{
		ownsHyperList = FALSE;
	}
		
	if (data->makeBorder)
		MakeBorder();

	IHyperTextX();
}

/******************************************************************************
 IHyperTextX
 
 	Perform common initialization for CHyperText panes.
******************************************************************************/

void CHyperText::IHyperTextX( void)
{	
	active = FALSE;
	
	// HyperText by default is not editable, not selectable, but is styleable,
	
	clickCmd = cmdNull;
	CopyPString("\p",lastHyperword);
	numFlashes = 3;										// 3 flashes is default
	Specify( kNotEditable, kNotSelectable, kStylable);
	SetWantsClicks( TRUE);
//	SetCanBeGopher(TRUE);

	hyperCursor = GetCursor( kHyperCursorID);
	FailNILRes( hyperCursor);
	HNoPurge( (Handle) hyperCursor);

}

/******************************************************************************
 Dispose
 
 	Perform cleanup for CHyperText.
 	
******************************************************************************/

void CHyperText::Dispose(void)
{

	if (ownsHyperList && itsHyperList)
		ForgetObject(itsHyperList);
	
	CEditText:: Dispose();
}


		/***************     CURSOR TRACKING METHODS     ***************/
	
/******************************************************************************
 AdjustCursor {OVERRIDE}

		Mouse is inside an HyperText Pane. If the cursor is over a hyperword,
		change the cursor to the hyperCursor.
 ******************************************************************************/

void	CHyperText::AdjustCursor(
	Point		where,					/* Mouse location in Window coords	*/
	RgnHandle	mouseRgn)
{		
	
LongPt	framePtWhere;

	WindToFrame(where, &framePtWhere);
	if (CursorOverHyperword(framePtWhere)){
		SetCursor( *hyperCursor);
	}
	else{
		SetCursor(&arrow);
	}
	
	gSleepTime = 0;
}

/******************************************************************************
 CursorOverHyperword

		Returns TRUE if the cursor is on top of a Hyperword.
		
		where should be in Frame coords because GetCharOffset uses Frame coords.
		
 ******************************************************************************/

Boolean	CHyperText::CursorOverHyperword(LongPt where)
{
long 		offset;
LongPt		Pt;
TextStyle	theStyle;

	offset = GetCharOffset(&where);
	GetCharStyle(offset,&theStyle);
	if (theStyle.tsFace == bold + underline)
		return TRUE;
	else
		return FALSE;
}


		/***************	DRAWING METHODS		***************/

/******************************************************************************
 MakeBorder
 
 	Make a PaneBorder object for the HyperText.
******************************************************************************/

void CHyperText::MakeBorder( void)
{
	Rect	margin;
	CPaneBorder *border;
	
	Prepare();
	border = new( CPaneBorder);
	border->IPaneBorder( kBorderFrame);
	SetRect( &margin, -kBorderAmount, -kBorderAmount, kBorderAmount, kBorderAmount);
	border->SetMargin( &margin);
	SetBorder( border);

}	/* CHyperText::MakeBorder */

/******************************************************************************
 MoveOffScreen
 
 	Moves the object off the screen 1000 pixels to right and bottom of the 
 	desktop.
******************************************************************************/

void CHyperText::MoveOffScreen( void)
{
Rect		deskBounds;
	
	oldH = hEncl;
	oldV = vEncl;
	
	gDesktop->GetBounds(&deskBounds);
	Place(deskBounds.right + 1000, deskBounds.bottom + 1000, FALSE);

}	/* CHyperText::MoveOffScreen */

/******************************************************************************
 MoveOnScreen
 
 	Moves the object back on the screen after inserting text while it was
 	offscreen.
******************************************************************************/

void CHyperText::MoveOnScreen( void)
{
Rect	r;

	Place(oldH, oldV, FALSE);

}	/* CHyperText::MoveOnScreen */


		/***************     COMMAND METHODS     ***************/
		
/******************************************************************************
 DoKeyDown {OVERRIDE}
 
 	Handle keypresses in a HyperText object. Keys that have a special meaning
 	in a dialog (tab, return, enter, and Escape) are passed to itsSupervisor,
 	all other keys are passed through to the superclass.
******************************************************************************/

void CHyperText::DoKeyDown( char theChar, Byte keyCode, EventRecord *macEvent)
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
		inherited::DoKeyDown( theChar, keyCode, macEvent);
					
		if (itsTypingTask)
		{
			ID = this->ID;
			BroadcastChange( hyperTextChanged, &ID);
		}
	}
	else
		itsSupervisor->DoKeyDown( theChar, keyCode, macEvent);	

}	/* CHyperText::DoKeyDown */

/******************************************************************************
 DoClick {OVERRIDE}

		Respond to a click.
 ******************************************************************************/

void	CHyperText::DoClick(
	Point		hitPt,					/* Mouse location in Frame coords	*/
	short		modifierKeys,
	long		when)
{
long 		offset, left, right, len, ticks;
LongPt		framePt;
TextStyle	theStyle;
Handle		textH;
CBureaucrat	*oldGopher;

	framePt.v = hitPt.v;
	framePt.h = hitPt.h;
	
	if ( CursorOverHyperword(framePt)){
		
		len = GetLength();
		
		// search left of the current char offset, and then right
		// to find the whole hyperword
		
		offset = GetCharOffset(&framePt);
		left = right = offset;
		
		GetCharStyle(offset,&theStyle);
		while (theStyle.tsFace == bold + underline && left >= 0)
			GetCharStyle(--left,&theStyle);
		left++;
		if (left == -1)
			left++;
		GetCharStyle(offset,&theStyle);
		while (theStyle.tsFace == bold + underline && right < len)
			GetCharStyle(++right,&theStyle);
		right--;
		
		// save the hyperword for later use in the lastHyperword data member
		textH = GetTextHandle();
		HLockHi(textH);
		lastHyperword[0] = right - left + 1;
		BlockMove(&(*textH )[left],&lastHyperword[1],lastHyperword[0]);
		HUnlock(textH);
		
		// flash the hyperword for the user numFlashes times
		// must flash at least once
		SetSelection(left, right+1, TRUE);
		Activate();
		
		//off
		Delay(4,&ticks);
		SetSelection(0, 0, TRUE);
		Deactivate();
	
		short i;
		
		for (i=0;i<numFlashes-1;i++){
			//on
			Delay(4,&ticks);
			SetSelection(left, right+1, TRUE);
			Activate();
	
			//off
			Delay(4,&ticks);
			SetSelection(0, 0, TRUE);
			Deactivate();
		}

		// make this the gopher so whoever handles the clickCmd 
		// can access lastHyperword by typecasting gGopher like this:
		// ((CHyperText*)gGopher)->lastHyperword
		// then restore the previous gopher
		
		oldGopher = gGopher;
		gGopher = this;
		itsSupervisor->DoCommand(clickCmd);
		gGopher = oldGopher;
		
	}
}	


		/***************     TEXT METHODS     ***************/

/******************************************************************************
 SetTextRes
 
 	Sets the text and style for a CHyperText pane froma 'TEXT' an 'styl' 
 	resources.
******************************************************************************/

void CHyperText::SetTextRes(short resourceID)
{
Handle			myTextH = NULL;
StScrpHandle 	theStyles = NULL;
Size			length;

	
// get the styl resource
	theStyles = (StScrpHandle) GetResource( 'styl', resourceID);
	FailNILRes(theStyles);
	HLockHi((Handle)theStyles);
	
// get the TEXT resource
	myTextH = GetResource( 'TEXT', resourceID );
	HLockHi( myTextH );			/* so data won't move */
	
//	remove any old text
	SetSelection(0,MAXINT, FALSE);
	TEDelete(macTE);
	
//	insert new text into the pane
	length = GetHandleSize (myTextH);
	TESetText(*myTextH, length, macTE);

//	set the styles
	SetStyleScrap( 0, length, theStyles, FALSE);
	HUnlock( (Handle)theStyles );
	ReleaseResource((Handle) theStyles);

//	stylize any hyperwords if we have a HyperList
	if (itsHyperList)
		Stylize();

//	get the new style information
	SetSelection(0,MAXINT, FALSE);
	theStyles = GetStyleScrap();
	HLockHi((Handle)theStyles);

//	remove the text so we can insert it with InsertWithStyles		
	TEDelete(macTE);

//	insert the hyper-styled text
	InsertWithStyles( *myTextH, length, theStyles);
	
	HUnlock( (Handle)theStyles );
	DisposeHandle((Handle) theStyles);

	HUnlock( myTextH );
	ReleaseResource(myTextH);
	
}

/******************************************************************************
 Stylize
 
 	This routine creates the visual representation of the hypertext "hot" 
 	words by searching its text and stylizing any word which is found in 
 	itsHyperList.
******************************************************************************/

void CHyperText::Stylize(void)
{
short			i,j=0,k;
long 			lOffset, offset, findLen, buffLen, numHyperWords, numStyles;
Handle			destStrH, textBuf;
Str255			findStr;
StScrpHandle 	theStyles = NULL;

	numHyperWords = itsHyperList->GetNumItems();					// get number of strings to search for
	destStrH = GetTextHandle();										// get the text
	buffLen = GetLength();											// get the text length

	textBuf = destStrH;
		
	if (gError->CheckOSError(HandToHand(&textBuf))){
		HLockHi(textBuf);	
		
		// make the textBuf all lowercase characters
		for (j=0;j<buffLen;j++){
			(*textBuf)[j] = tolower((*textBuf)[j]);
		}
		
		// loop through each string in the HyperList and Munger a lowercase copy
		// of our text.  Any matches are stylized in bold+underline to show that
		// it is "hot".
		
		for (i = 1;i<= numHyperWords;i++){
			itsHyperList->GetItem( findStr, i);
			findLen = findStr[0];
			PtoCstr((unsigned char*)findStr);
			
			// make the findStr all lowercase characters
			for (j=0;j<findLen;j++){
				(findStr)[j] = tolower((findStr)[j]);
			}
			
			offset = 0;
			lOffset = 1;
			while (lOffset && offset < buffLen){
				lOffset = Munger(textBuf,offset,findStr,findLen,NULL,0);//search for the findStr
				if (lOffset >= 0){
					if ((((*textBuf)[lOffset-1] < 'A') || lOffset == 0)	//ignore punctuation
						&& ((*textBuf)[lOffset+findLen] < 'A')){
						
						SetSelection(lOffset, lOffset+findLen,FALSE);	// set the selection range
						theStyles = GetStyleScrap();					// get the current styles for the selection
						HLockHi((Handle)theStyles);	
						numStyles = (**theStyles).scrpNStyles;			// get the number of styles
						
						for (k=0;k<numStyles;k++){
							(((**theStyles).scrpStyleTab)[k]).scrpFace = normal;		// make it normal
							SetStyleScrap( lOffset, lOffset+findLen, theStyles, FALSE);
							(((**theStyles).scrpStyleTab)[k]).scrpFace = bold+underline;// make it bold underline
							SetStyleScrap( lOffset, lOffset+findLen, theStyles, FALSE);
						}
						
						SetSelection(0, 0,FALSE);										// select nothing
						HUnlock((Handle)theStyles);
						DisposeHandle((Handle) theStyles);
						
					}
					offset = lOffset + findLen;							// check the rest of the text
				}
				else{
					offset = buffLen;
				}
			}
		}

		HUnlock(textBuf);	
		DisposeHandle(textBuf);	
	}
}


		/***************     CLICK COMMAND METHODS     ***************/

/******************************************************************************
 SetFlashes

		Specify the number of times to flash the clicked hyperword.
 ******************************************************************************/

void	CHyperText::SetFlashes(short aFlashTimes)
{
	numFlashes = aFlashTimes;				/* Set instance variable			*/
}

/******************************************************************************
 SetClickCmd

		Specify the command which is sent to a HyperText's supervisor
		after a confirmed click
 ******************************************************************************/

void	CHyperText::SetClickCmd(long aClickCmd)
{
	clickCmd = aClickCmd;				/* Set instance variable			*/
}

/******************************************************************************
 GetClickCmd
 
 	Return the command which is sent to a HyperText's supervisor after a
 	confirmed click
******************************************************************************/
long CHyperText::GetClickCmd( void)
{
	return clickCmd;
}


		/***************     HYPERLIST METHODS     ***************/
		
/******************************************************************************
 SetListRes
 
 	Sets from a STR# resource.
******************************************************************************/

void CHyperText::SetListRes(short resourceID)
{

	itsHyperList =  new CStringArray;
	itsHyperList->IRes(resourceID, 255);
	ownsHyperList = TRUE;
	if (GetLength()){
		Stylize();
	}
}

/******************************************************************************
 SetList
 
 	Sets itsHyperList to use an existing CStringArray.
******************************************************************************/

void CHyperText::SetList(CStringArray *aStrList)
{
	itsHyperList = aStrList;
}

/******************************************************************************
 GetList
 
 	Returns itsHyperList CStringArray.
******************************************************************************/

CStringArray *CHyperText::GetList(void)
{
	return itsHyperList;
}