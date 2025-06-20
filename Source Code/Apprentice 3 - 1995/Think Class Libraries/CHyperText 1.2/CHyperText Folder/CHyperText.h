/******************************************************************************
 CHyperText.cp

		
	SUPERCLASS = CStyleText
	
	Copyright � 1994 Johns Hopkins University. All rights reserved.
	
	Original Author: 	Martin R. Wachter		email:	mrw@welchgate.welch.jhu.edu
	Created:			4/4/94					by:		mrw			TCL Version:	1.1.3
	Modified:			4/5/94					by:		mrw			TCL Version:	1.1.3


 ******************************************************************************/

#pragma once

#include "CStyleText.h"

enum {

	kNoBorder =	FALSE,
	kDrawBorder

};

CLASS CStringArray;

class CHyperText : public CStyleText 
{
public:
				/*  Data Members  */
	long			clickCmd;			// Command to issue when clicked
	
	CStringArray	*itsHyperList;		// CStringArray which holds a list of
										// hypertext keywords in memory.
										// Usually initialized from an STR#
										// resource via SetTextRes()
										
	Boolean			ownsHyperList;		// used for proper disposal
	
	short			oldH, oldV;			// used for moving the pane offscreen
	
	short			numFlashes;		// number of times to flash the hyperword
	
	CursHandle		hyperCursor;		// used to show the user that the mouse
										// is over a hyperword
										
	Str255			lastHyperword;		// the string of the most recent hyperhit

	void			IHyperText(CView *anEnclosure, CBureaucrat *aSupervisor,
							short aWidth, short aHeight,
							short aHEncl, short aVEncl,
							SizingOption aHSizing, SizingOption aVSizing,
							short aLineWidth, Boolean hasBorder);
	
	virtual void 	IViewTemp(CView *anEnclosure, CBureaucrat *aSupervisor,
							Ptr viewData);
	virtual void	Dispose( void);
	virtual void	AdjustCursor(Point where,RgnHandle	mouseRgn);
	virtual Boolean	CursorOverHyperword(LongPt where);
	virtual void	DoClick(Point hitPt, short modifierKeys, long when);
	virtual void 	DoKeyDown( char theChar, Byte keyCode, EventRecord *macEvent);

	virtual void 	SetTextRes(short resourceID);
	virtual void	Stylize(void);

	virtual void	SetFlashes(short aFlashTimes);
	
	virtual void	SetClickCmd(long aClickCmd);
	virtual long	GetClickCmd( void);
	virtual void 	SetListRes(short resourceID);
	virtual void 	SetList(CStringArray *aStrList);
	virtual CStringArray* GetList(void);

	virtual void 	MoveOffScreen( void);
	virtual void 	MoveOnScreen( void);

protected:
						
	void			IHyperTextX( void);
	virtual void 	MakeBorder( void);

};

enum
{
			/* indicates the text has changed in some way 		*/
			/* info parameter is pointer (short*) to pane ID	*/
			
	hyperTextChanged = textLastChange + 1,
	
	hyperTextLastChange = hyperTextChanged
};

typedef struct tHyperTextTemp
{
	tAbstractTextTemp absTextTmp;
	short		TEXT_Res_ID;
	short		STRlist_Res_ID;	
	short		makeBorder;	
} tHyperTextTemp, *tHyperTextTempP;