/******************************************************************************
 CPStyleText.c
 
	 						  The PStyleText Class
							 		   by:
								John A. Love, III
								
							NO Copyright � it's free!
			   (mentioning my name would be VERY MUCH appreciated)
 	
 	
							 SUPERCLASS = CPEditText
							 		   by:
							  Christopher R. Wysocki
					  Copyright � 1992, All Rights Reserved



	                         ---- DESCRIPTION ----
 
	CPStyleText is a class for version 1.1.x of Symantec's THINK Class
	Library that implements a styled text editing pane.  It can be used
	as a direct replacement for the standard TCL CStyleText class without
	TextEdit's limitation to a maximum 32k of text.
	
	
	                       ---- VERSION HISTORY ----
	
		� 1.3b1 (25 September 1993)
			- Started working and working and working and ...
	
		� 1.3b2 (28 December 1993)
			- Introduced left and right margins
			- Reinserted center and right text alignment
			  ( Chris dropped these in his version 1.2 of CPEditText )
			- Introduced word-wrap:
			  (a) to window width
			  (b) to line width
			  (c) to left and right margins
	
		� 1.3b3 ( 6 January 1994)
			- Introduced full justify text alignment.  Created
			  a CStylePStyleTask class to help effect this.
	
		� 1.3b4 (13 January 1994)
			- Modified Chris' ScrollToSelection and ScrollToOffset methods
			- Changed my WrapLineStarts method to
			  accomodate breaking SUPER long words
	
		� 1.3b5 (28 January 1994)
			- Added an autoscroll capability when the user presses the
			  <Option> key while clicking the mouse in the text pane.
			  Idea came from my use of Symantec's "THINK Reference" app.
			  This autoscrolling scrolls just one line at a time.
	
		� 1.3b6 ( 6 February 1994)
			- Added another autoscroll capability effected when the
			  user presses the <Option> key while dragging the thumb
			  in either scroll bar.  Created the CPStyleScrollPane and
			  CPStyleScrollBar classes to make this happen.  Idea came
			  from my use of Jersey Scientific's "CMaster".  The user
			  would implement this type of autoscrolling when s/he
			  wishes to scroll VERY fast since the scrolling speed is
			  limited only by the speed of the mouse movement.
	
		� 1.3b7 (10 February 1994)
			- Modified Chris' DrawCaret method for drawing the caret
			  at the end of a "sub-line" of a word-wrapped line.
			  Chris' DrawCaret method is declared NON-virtual within
			  his "CPEditText.h" interface file.  So the most simple!
			  way I can override it for both THINK C and Symantec C++
			  is to declare Chris' DrawCaret a virtual method within
			  that interface file.
	
		� 1.3	(26 February 1994)
			- Started the 'mother' of all additions ... style info ...
			  Before this addition is finished, it'll change many, many
			  times.  For the moment, I have presented just a shell for
			  us to chew on.
	
		� 1.4b1	(?????)
			- Another BIG challenge is to significantly speed things
			  up for super large files (> 100k), for example, reading
			  them in, displaying them, word-wrap, scrolling, etc.
			

	                      ---- LICENSE AGREEMENT ----
	
	The source code unique to CPStyleText was written by and is the property
	of John A. Love, III.  Any users of CPStyleText are legally bound by the
	LICENSE AGREEMENT authored by Christopher R. Wysocki.  No other Copyright
	is stated or implied.
	
	                  ---- ELECTRONIC MAIL ADDRESSES ----
	
		America Online:		John Love

	                  		 ---- OTHER INFO ----
	
		Telephone:			(703) 569-2294		<your nickel>
		
		Letter:				6413 Wyngate Drive
							Springfield, Virginia   22152
 ******************************************************************************/




/** Includes **/

#include <Commands.h>
#include <Constants.h>
#include <CBartender.h>
#include <CClipboard.h>
#include <CSizeBox.h>
#include <OSChecks.h>
#include <Script.h>
#include <TBUtilities.h>
#include <TCLUtilities.h>

#include "CPEditScrollPane.h"
#include "CPStyleText.h"


/** TCL Global Variables **/

extern CBartender	*gBartender;
extern CClipboard	*gClipboard;
extern short		gClicks;


/**				Added Constant					 **/
/**												 **/
/** New resources in my "CPStyleText.rsrc" file. **/
/** 'CURS' and 'crsr' resources adapted from	 **/
/** Symantec's "THINK Reference" application.	 **/

#define rAutoScrollCursor	4000


/**			 Speaking of resources ...						   **/
/**															   **/
/** This is the ID of my 'Estr' resource that the TCL uses	   **/
/** for Error Handling when you call:						   **/
/**															   **/
/** if ( MemError() )										   **/
/** 	FailOSErr( FileTooBig )								   **/
/**															   **/
/** This is clearly more specific than calling:				   **/
/**															   **/
/**		FailMemError();										   **/
/**															   **/
/** Not only does this isolate the problem for the programmer, **/
/** but it also gives the user specific information rather	   **/
/** than the statement that "there was not enough memory".	   **/
/**															   **/
/** This resource is included within "CPStyleText.rsrc" and	   **/
/** has the following "THINK Rez" source:					   **/
/**															   **/
/** resource	'Estr'	(-26000, "Text Size Error", purgeable) **/
/** {														   **/
/**		"the file is too big -- 156,000 bytes maximum"		   **/
/** }														   **/

enum
{
	FileTooBig = -26000
};


/*
	Initialize the Class Variables:
	
	Idea for cursors generated by my use of
	Symantec's "THINK Reference" application.
*/

CursHandle		CPStyleText::cAutoScrollCursor = NULL;
CCrsrHandle		CPStyleText::cAutoScrollCRSR   = NULL;
Boolean			CPStyleText::cHasTrueType	   = TrapAvailable( 0xA854 );



/******************************************************************************
 IsWordBreakChar

		Method to determine if a character is a word-break character.
 ******************************************************************************/

	static long		wordBreakFlags[8] =
	{
		/*
			Packed Boolean flags from Chris' WordBreakHook method.
			Note that if a character is represented here, it is
			NOT a word-break character.  Conversely, if a character
			is NOT here, it IS a word-break character.
		*/
		
		0x00000000, 0x0000FFC0,
		0x7FFFFFE1, 0x7FFFFFE0,
		0x00000000, 0x00000000,
		0x00000000, 0x00000000
	};
	
	
	
	static Boolean		IsWordBreakChar (unsigned char ch)
	{
		/*
			Note the NOT transposition.  Remember, if a
			character is NOT in the above packed Boolean
			array, it IS a word-break character.
		*/
			
		
		return	(
				  !(
					  ((long*) wordBreakFlags)[ch >> 5] & (0x80000000U >> (ch & 0x1F))
				   )
				);

	}	/* IsWordBreakChar */



/**** C O N S T R U C T I O N / D E S T R U C T I O N   M E T H O D S ****/


void	CPStyleText::IPStyleText (CView *anEnclosure, CBureaucrat *aSupervisor,
								  short aWidth, short aHeight,
								  short aHEncl, short aVEncl,
								  SizingOption aHSizing, SizingOption aVSizing,
								  short aLineWidth)
{

	CPEditText::IPEditText( anEnclosure, aSupervisor,
							aWidth, aHeight, aHEncl, aVEncl,
							aHSizing, aVSizing );

	/*
		Unfortunately, IPEditText does not accept a passed
		line width as IAbstractText does ... a shame because
		CAbstractText is CPEditText's superclass ... but
		this IS consistent with version 1.2 of Chris'
		CPEditText which does NOT support word-wrapping.
		So Chris just passes:
		
			aLineWidth = kDefaultBoundsWidth
			
		to IAbstractText within his IPEditText method.
		
		Of course, if all this were false, then the
		following statement would NOT be required
		because IAbstractText would do it for us.
	*/
	lineWidth = aLineWidth;

	IPStyleTextX();
												
}	/* IPStyleText */



/* OVERRIDE: */
void	CPStyleText::IViewTemp (CView *anEnclosure, CBureaucrat *aSupervisor,
								Ptr viewData)
{

	CPEditText::IViewTemp( anEnclosure, aSupervisor, viewData );
	
 /*
	IViewTemp reads the line width
	from the viewData template:
	
	lineWidth = aLineWidth;
 */

	IPStyleTextX();
												
}	/* IViewTemp */



void	CPStyleText::IPStyleTextX (void)
{
		LongRect	theInterior;
		CWindow		*theWindow;
		Boolean		savedAlloc;
		CArray		*theLines = NULL;
		
	
 /* CPEditText::IPEditTextX();  --  already called by IPEditText or by IViewTemp. */
 
	autoRefresh = fWordWrap = lineWidth <= 0;	/* Examine IEditTextX ...		  */
												/* Changed later by SetWordWrap.  */
												
 /* Change CAbstractText's instance variable: */
 
	fixedLineHeights = FALSE;
	/*
		Because we change fixedLineHeights, we need to re-call
		SetWholeLines whose response is a function of fixedLineHeights:
	*/
 /* wholeLines = TRUE;			--  set by IPEditTextX.							  */
	SetWholeLines( wholeLines );
	
 /* Change a couple of Chris' instance variables: */
 
	fOutlineHilite = TRUE;
	fUseItalicCaret = TRUE;
	 
 /* Initialize my added instance variables: */

	itsAlignCmd = cmdAlignLeft;
	itsLeftMargin = 0L;
	itsRightMargin = MAXINT;
	itsViewRect = aperture;
	GetInterior( &theInterior );
	itsDestRect = theInterior;
	itsDestRect.bottom = (long) MAXINT;			/* Will NEVER change --			  */
												/* ONLY the bounds will			  */
	fOldEndWrapLineCaret = FALSE;
	fNewEndWrapLineCaret = FALSE;
	/*
		The parent CPane of my CPStyleText may
		simply scroll to home via:
		
			GetHomePosition( &homePos );
			ScrollTo( &homePos, TRUE );
		
		rather than call:
			
			SetSelection (0L, 0L, TRUE);
			ScrollToSelection();
			
		SetSelection is where I set my caret
		flag = fEndWrapLine.  Therefore, I
		better turn it off here in case
		SetSelection is NOT called as I have
		just described.
	*/
	fEndWrapLine = FALSE;
	
	/*
		Word-wrap becomes a very slow process for long docs
		if the wrap width becomes too narrow.  Therefore,
		override the TCL's default value of 100 pixels:
	*/
	
	theWindow = GetWindow();
	theWindow->sizeRect.left = Max( theWindow->sizeRect.left + (short)itsLeftMargin,
									MIN_WINDOW_WIDTH );

	/* The BIGGEE !!! */
	
	theLines = new (CArray);
	theLines->IArray( sizeof(LineRecord) );
	itsLineInfo = theLines;
	AddLine( atEnd );
	
	/* Various Class Variables ... */
	
	if (!gSystem.hasColorQD)
	{
		if (cAutoScrollCursor == NULL)
		{
			savedAlloc = SetAllocation( kAllocCanFail );
			cAutoScrollCursor = GetCursor( rAutoScrollCursor );
			SetAllocation( savedAlloc );
			FailNILRes( cAutoScrollCursor );
			HNoPurge( (Handle)cAutoScrollCursor );
			;
			cAutoScrollCRSR = NULL;
			
		}	/* NOT previously initialized */
		
	}	/* "Yucky" black-and-white */
	
	else
	{
		if (cAutoScrollCRSR == NULL)
		{
			savedAlloc = SetAllocation( kAllocCanFail );
			cAutoScrollCRSR = GetCCursor( rAutoScrollCursor );
			SetAllocation( savedAlloc );
			FailNILRes( cAutoScrollCRSR );
			HPurge( (Handle)cAutoScrollCRSR );			/* See IM 5. */
			;
			cAutoScrollCursor = NULL;
			
		}	/* NOT previously initialized */
		
	}	/* color Quickdraw */
		
}	/* IPStyleTextX */



/* OVERRIDE: */
void	CPStyleText::Dispose (void)
{
		register long	line, numLines;
		LineRecord		lineInfo;
		
		
	/*
		Need to dump each line's component StyleRecord
		CArray before we dispose of the master LineRecord:
	*/
	
	numLines = itsLineInfo->GetNumItems();
	;
	for (line = 1; line <= numLines; ++line)
	{
		itsLineInfo->GetItem( &lineInfo, line );		/* 1-based */
		ForgetObject( lineInfo.lStyles );
	}
	;
	ForgetObject( itsLineInfo );
	
	CPEditText::Dispose();
	
}	/* Dispose */



/**** D I S P L A Y   M E T H O D S ****/


/* OVERRIDE: */
void	CPStyleText::Draw (Rect *area)
{
		LongRect		longArea;
		register long	numLines  = itsNumLines;
		register long	startPointHt, endPointHt;
		register long	cumLineHt;
		register long	startLine, endLine,
						vertInset = VertTopInset();
	
	
		/*
			Compute and draw the visible text lines.  Added
			for-loops for eventual incorporation of styled
			text wherein each line can have a unique height.
		*/
	
	QDToFrameR( area, &longArea );
	;
	cumLineHt = 0;
	startPointHt = longArea.top - vertInset;
	for (startLine = 0; startLine < numLines; startLine++)
	{
	 /*
		Because this for-loop consumes a lot of time,
		every time-savings helps.  Calling Get1Height
		just calls GetHeight with identical lines:

		cumLineHt += Get1Height( startLine );
	 */
		
		cumLineHt += GetHeight( startLine, startLine );
		if (cumLineHt > startPointHt)
			break;
	}
	startLine = Min( startLine, numLines - 1 );
	;
 /* cumLineHt = 0;  --  start where you left off */
	endPointHt = longArea.bottom - vertInset;
	for (endLine = startLine + 1; endLine < numLines; endLine++)
	{
		/*
			cumLineHt above includes the line
			height of startLine, so start this
			2nd for-loop at startLine + 1.
		*/
		
		cumLineHt += GetHeight( endLine, endLine );
		if (cumLineHt >= endPointHt)
			break;
	}
	endLine = Min( endLine, numLines - 1 );
	
	DrawLineRange( startLine, endLine, 0, kDontEraseText );
	
		/*
			If printing is not in progress, highlight the current
			selection or draw the insertion caret, as appropriate
		*/
	if (
		 !this->printing						&&
		 this->wantsClicks						&&
		 (fReallyActive || fOutlineHilite)
	   )
	{
		if (itsSelStart != itsSelEnd)
			HiliteTextRange( itsSelStart, itsSelEnd );
		else if (fCaretVisible)
			DrawCaret();
	}
	
}	/* Draw */



/* OVERRIDE: */
void	CPStyleText::SetSelection (long selStart, long selEnd, Boolean fRedraw)
{
	/*
		Overridden for special caret placement at the end
		of a "sub-line" of a longer word-wrapped line:
	*/
	
		enum
		{
			/*
				These dudes are in Chris' "CPEditText.c"
				and so must be repeated here:
			*/
			
			kUnhideSelection = FALSE,
			kHideSelection	 = TRUE
		};

	
	selStart = Max( selStart, 0 );				/* Ensure positions are valid ...  */
	selStart = Min( selStart, itsTextLength );
	selEnd = Max( selEnd, 0 );
	selEnd = Min( selEnd, itsTextLength );
	selEnd = Max( selEnd, selStart );
			
	/*
		Unlike Chris, I do NOT take a quick exit if the
		selection range has stayed the same.  The reason
		is that the selection at the end of a "sub-line"
		of a longer word-wrapped line is actually the
		beginning of the next "sub-line".
		
		I use this overridden SetSelection for special
		caret placement.  Clicking at the end of a
		"sub-line" demands different caret placement
		than clicking at the far left of the next
		"sub-line", even tho` the selection is identical.
	*/
	
		/* Unhilite the old selection range: */
		
	fEndWrapLine = fOldEndWrapLineCaret;
	HideSelection( kHideSelection, fRedraw );
		/*
			Once you've erased the old caret, you're done with
			it until you click somewhere else or press a key:
		*/
	fOldEndWrapLineCaret = FALSE;
			
	itsSelStart = selStart;					/* Update our instance variables ...   */
	itsSelEnd = selEnd;
	fUpDownArrow = FALSE;
	
		/* Ensure that the anchor point is one of the selection endpoints: */
		
	if ( (itsSelAnchor != selStart) && (itsSelAnchor != selEnd) )
		itsSelAnchor = selStart;
		
		/* Hilite the new selection range: */
		
	if ( editable || (selStart != selEnd) )
	{
		fEndWrapLine = fNewEndWrapLineCaret;
		HideSelection( kUnhideSelection, fRedraw );
	}
	
}	/* SetSelection */



/**** C A L I B R A T I O N   M E T H O D S ****/


/* OVERRIDE: */
void	CPStyleText::UpdateMenus (void)
{
	/*
		If wrapping to the window width, the various
		alignments do NOT make much sense.

		Within your CApplication::SetUpMenus method,
		you could designate all the alignments as
		items of the main MENU, in which case the
		"else" clause to follow is NOT required.
		However, if you designated these alignments
		as items of a hierarchical MENU, you would
		probably specify "dimNONE" as the dimming
		option for the parent MENU item.  In this
		last instance, "if-else" are required.
	*/
		

	CPEditText::UpdateMenus();
	
	if (lineWidth != -1)
	{
		gBartender->EnableCmd( cmdAlignLeft );
		gBartender->EnableCmd( cmdAlignCenter );
		gBartender->EnableCmd( cmdAlignRight );
		gBartender->EnableCmd( cmdJustify );
	}
	else
	{
		gBartender->DisableCmd( cmdAlignLeft );
		gBartender->DisableCmd( cmdAlignCenter );
		gBartender->DisableCmd( cmdAlignRight );
		gBartender->DisableCmd( cmdJustify );
	}
	
}	/* UpdateMenus */



/* OVERRIDE: */
void	CPStyleText::ResizeFrame (Rect *delta)
{		
	
	CPEditText::ResizeFrame( delta );			/* Calls ForceNextPrepare() */
	
	itsDestRect.left += delta->left;
	itsDestRect.top += delta->top;

	CalcPERects();
	
	/*
		If wrapping by the window width, recalculate line starts.
		RecalcLineStarts calls WrapLineStarts and AdjustBounds().
		The latter sets the bounds width of the CPanorama to the
		width of itsDestRect and the bounds height to itsNumLines:
	*/
	if (lineWidth == -1)
		if (delta->left || delta->right)
		{
			/* Recalculate ONLY with horizontal changes: */
			
			RecalcLineStarts();
		}
	
}	/* ResizeFrame */



/* OVERRIDE: */
void	CPStyleText::SetWholeLines (Boolean aWholeLines)
{
	/*
		Overridden to ONLY change the frame size when
		there are more lines than frame to draw them in.
	*/
	
		LongPt		charPos;
		long		charOffset, topLine, bottomLine, totalHeight;
		
		
	frame.bottom = frame.top + height;

	wholeLines = aWholeLines;

	if (wholeLines)
	{
		if (fixedLineHeights)
		{
			/* Use the height of the first line as the standard height: */
			
				short lineHeight = GetHeight( 1, 1 );	
			
			if (lineHeight > 0)
				frame.bottom = frame.top + lineHeight * (height / lineHeight);
		}
		else
		{
			/* Find the line at top of frame: */
			
			SetLongPt( &charPos,  frame.left + 1, frame.top + 1 );
			charOffset = GetCharOffset( &charPos );
			topLine = FindLine( charOffset ) ;
			
			/* Find the line at bottom of frame: */
			
			SetLongPt( &charPos, frame.left + 1, frame.top + height - 1 );
			charOffset = GetCharOffset( &charPos );
			
		 /*
			Chris' <even CEditText's> FindLine addresses
			the case charOffset = 0.  Besides, the "else"
			clause should read:
			
				bottomLine = GetNumLines() - 1;
		 
			if (charOffset > 0)
				bottomLine = FindLine( charOffset );	
			else
				bottomLine = GetNumLines();
		 */
				
			bottomLine = FindLine( charOffset );	

			if (bottomLine > topLine)
			{
				totalHeight = GetHeight( topLine, bottomLine + 1 );
				
				if (totalHeight > height)
				{
					/*
						Mess with the frame size ONLY when there
						are more lines than frame to draw them in.
						In this way, the frame stays put if we've
						only got a few lines of text.
						
						Also, if the frame stays put, the cursor
						won't change from the I-beam as long as
						the Mouse is inside the text area.  Before,
						it changed to an Arrow in the bottom part
						of the text area because with just a few
						lines of text, the frame's bottom was very
						close to the window's top.
					*/
					
					while ( (totalHeight > height) && (bottomLine > topLine) )		
					{
						totalHeight = GetHeight( topLine, bottomLine-- );
					}
		
					/*
						We now know how many full lines will fit.  Resize frame,
						while ensuring that at least one line is visible:
					*/
	
					if (totalHeight < height)
					{
						frame.bottom = frame.top + totalHeight;
					}
					
				}	/* more lines than frame */
				
			}	/* bottomLine > topLine */
			
		}	/* NOT fixedLineHeights */
		
	}	/* if (wholeLines) */

	CalcAperture();	

}	/* SetWholeLines */



/**** A C C E S S I N G   M E T H O D S ****/


/* OVERRIDE: */
void	CPStyleText::SetBounds (LongRect *aBounds)
{
	/*
		CPEditText's SetBounds does some preliminary scrolling
		which generates an update Event.  SetBounds is called
		by AdjustBounds which, in turn, is called by a bunch
		of methods, most notably, ResizeFrame.  Chris' update
		amounts to a SECOND update in addition to the one to be
		generated after CPanorama's ChangeSize calls ResizeFrame.
		Because of this duplication, the screen flashes.
		
		In addition, CPanorama's SetBounds calls:
		
			ASSERT( position.v <= bounds.bottom );

		which presents a problem while you are calling CPane's
		ChangeSize for positioning your various CPanes within
		your CDocument's BuildWindow method.  This is why
		Chris did the preliminary scrolling.
		
		Note that CEditText (no "P") does not override CPanorama's
		SetBounds and does not call it.  Instead, CEditText's
		AdjustBounds does everything that CPanorama's SetBounds
		does, but withOUT the above assertion check.
		
		So, do the same here by duplicating CPanorama's SetBounds,
		but withOUT this check.
	*/


	bounds = *aBounds;
	
	if (itsScrollPane != NULL)
		itsScrollPane->AdjustScrollMax();
	
}	/* SetBounds */



/* OVERRIDE: */
long	CPStyleText::GetHeight (long startLine, long endLine)
{
	/*
		Will eventually incorporate styled text
		wherein each line can have a unique height.
		
		I used to have range checking here, such as:
		
			0 => startLine, endLine <= itsNumLines - 1
			
		However, all the methods which call GetHeight
		accomplish their own range checking beforehand,
		so I just call the inherited method (so far!).
	*/
	
	
	return	( CPEditText::GetHeight(startLine, endLine) );
		
}	/* GetHeight */



/* OVERRIDE: */
long	CPStyleText::GetCharOffset (LongPt *aPt)
{
		register long		numLines = itsNumLines;
		register long		line, cumLineHt, pointHt;
		register short		offset;
		register short		lineLen;
		register short		width, lastWidth;
		register short		horiz;
		register short		*widthsP;
		ShortHandle			widthsH;
	
	
	Prepare();
	
	/*
		Initialize at the beginning just in case we
		encounter any of the special cases to follow.
	*/
	
	fNewEndWrapLineCaret = FALSE;

		/*
			Determine which line the point lies on.
			Implemented for-loop to find line number
			because eventually I will incorporate
			styled text wherein each line will have
			a unique height.
		*/
		
	pointHt = aPt->v - VertTopInset();
	if (pointHt < 0L)				/* Handle special case of above the first line. */
		return	(0L);
	;
	cumLineHt = 0;
	for (line = 0; line < numLines; line++)
	{
		/* See comments within Draw method: */
		
		cumLineHt += GetHeight( line, line );
		if (cumLineHt > pointHt)
			break;
	}
	if (line > numLines - 1)		/* Handle special case of below the last line. */
		return	(itsTextLength);
	
		/* Get the number of characters in the line */
		
	lineLen = GetLineLength( line ) ;
	
		/*
			Determine pixel width that is available for text.
			This available space appears AFTER the left margin
			and goes out to the passed point = the point where
			the user clicked (in frame coordinates):
		*/
	
	horiz = aPt->h - HorizLeftInset();
	
		/* Calculate character pixel offsets for the given line: */
		
	if (horiz <= itsLeftMargin)		offset = 0;	
	else
	{
		widthsH = MeasureLineWidths( line );
		widthsP = *widthsH;
		
			/*
				NON-left text alignments eliminated from version 1.2:
				
				For all text alignments, decrease the available
				width.  This decrease should match the corresponding
				increase by GetCharPoint(...).  As stated above,
				this width appears AFTER the left margin.
			*/
		
		if (itsAlignCmd == cmdAlignLeft)
			horiz -= itsLeftMargin;
		else if (itsAlignCmd == cmdAlignRight)
			horiz -= HorizPixelExtent() - widthsP[lineLen];
		else if (itsAlignCmd == cmdAlignCenter)
			/*
				Center between left margin and bounds' right edge:
			*/
			horiz -= itsLeftMargin +
						(HorizPixelExtent() - itsLeftMargin - widthsP[lineLen]) / 2;
		else
		{
				/*
					For full justify alignment, the "widthsH"
					Handle addresses the point displacement:
				*/
			
			horiz -= itsLeftMargin;
		}
		
		offset = lastWidth = 0;
		
		while (
				(offset				  < lineLen)	&&
				((width = *++widthsP) < horiz  )
			  )
		{
			lastWidth = width;
			++offset;
		}
		
		DisposHandle( (Handle)widthsH );
		
		if (offset < lineLen)
		{
			/*
				If we clicked somewhere ON a character, check
				whether on its first or last half.  If the last
				half, then bump the offset to the NEXT character.
				
				Do this first in case the offset is bumped beyond
				a word-break character:
			*/
			
			if (width > lastWidth)
				if (horiz > (lastWidth + width) / 2)
					++offset;
					
		}	/* offset < lineLen */
		
		if (offset == lineLen)
		{
			/* We have clicked to the FAR right of the line's end: */
			
				long		endOfLine = (*itsLineStarts)[line] + lineLen;
				tCharBuf	theChar;
				
			GetCharBefore( &endOfLine, theChar );
			if ( theChar[0] )
			{
					register Byte	ch = theChar[1];
					
				if (ch == kReturn)
				{
					--offset;
					/*
						Special case where we click to the far right
						of the last line which is empty, i.e., the
						user pressed <CR> after the last full line
						and stopped typing.
					*/
					offset = Max( offset, 0 );
				}
				else
					fNewEndWrapLineCaret = IsWordBreakChar( ch );
			}
			
		}	/* offset = lineLen */
		
	}	/* horiz > itsLeftMargin */

	return	( (*itsLineStarts)[line] + offset );
	
}	/* GetCharOffset */



/* OVERRIDE: */
void	CPStyleText::GetCharPoint (long charOffset, LongPt *aPt)
{
		long			line;
		ShortHandle		widthsH;
	
	
		/* Ensure that the offset is within bounds: */
	
	charOffset = Max( charOffset, 0 );
	charOffset = Min( charOffset, itsTextLength );
		
		/*
			Determine which line the given character offset is
			on and calculate the pixel widths for that line:
		*/
	
	line = FindLine( charOffset );
	widthsH = MeasureLineWidths( line );
	
		/* Compute the pixel coordinates for the character: */
	
	aPt->v = VertTopInset() + GetHeight( 0, line - 1 ) + itsFontAscent;
	aPt->h = HorizLeftInset() + (*widthsH)[charOffset - (*itsLineStarts)[line]];
		
		/*
			NON-left text alignments eliminated from version 1.2:
				
			For all text alignments, increase horizontal coordinate.
			This addition should include the left margin.  For
			further info, see comments within GetCharOffset(...).
		*/
 	
	if (itsAlignCmd == cmdAlignLeft)
		aPt->h += itsLeftMargin;
	else if (itsAlignCmd == cmdAlignRight)
		aPt->h += HorizPixelExtent() - (*widthsH)[GetLineLength(line)];
	else if (itsAlignCmd == cmdAlignCenter)
		aPt->h += itsLeftMargin + (
									HorizPixelExtent() - itsLeftMargin -
									(*widthsH)[GetLineLength(line)]
								  ) / 2;
	else
	{
			/*
				For full justify alignment, the "widthsH"
				Handle addresses the point displacement:
			*/

		aPt->h += itsLeftMargin;
	}
	
		/* Dispose of the pixel widths handle: */
	
	DisposHandle( (Handle) widthsH );
	
}	/* GetCharPoint */



/* OVERRIDE: */
void	CPStyleText::SetMargins (long leftMargin, long rightMargin)
{
	/*
		Calls to SetWordWrap and
		DoWordWrap should follow:
		
		( Passed parms are in pixels )
	*/
		
	itsLeftMargin = Max( leftMargin, 0L );					/* Store new value		*/

	itsRightMargin = Max( rightMargin, 0L );				/* Store new value		*/
	if (itsRightMargin == 0L)	itsRightMargin = 576L;		/* Default = 8.0 inches	*/
	
}	/* SetMargins */



/* OVERRIDE: */
void	CPStyleText::GetMargins (long *leftMargin, long *rightMargin)
{

	*leftMargin = itsLeftMargin;
	*rightMargin = itsRightMargin;
	
}	/* GetMargins */



/**** M O U S E   A N D   K E Y S T R O K E   M E T H O D S ****/


/* OVERRIDE: */
void	CPStyleText::DoClick (Point hitPt, short modifierKeys, long when)
{
	/*
		Override to select the end of a "sub-line" of a word-wrapped line.
		Also, to enable autoscrolling when the user presses the <Option>
		key while clicking the mouse in the text pane.
	*/
	
		LongPt		framePoint, autoScrollPoint;
		long		selStart, selEnd;
		long		charOffset, origCharOffset, lastCharOffset;
		long		newSelStart, newSelEnd;
		tCharBuf	breakChar;
		/* <Option> scrolling ... */
		Boolean		optionKeyDown  = ( (modifierKeys & optionKey) != 0 );
		long		saveSelStart, saveSelEnd;
		Point		movedPt;
		LongPt		framePt;
		enum
		{ 
			/*
				These dudes are in Chris' "CPEditText.c"
				and so must be repeated here:
			*/
			
			kUnhideSelection	   = FALSE,
			kHideSelection		   = TRUE
		};
		
		
	fOldEndWrapLineCaret = fNewEndWrapLineCaret;
	fNewEndWrapLineCaret = FALSE;					/* Assume NOT !!! */
	
	if (gClicks == 3)
	{
		/* Use Chris'method for paragraph selection: */
		
		CPEditText::DoClick( hitPt, modifierKeys, when );
	}
	
	else if (gClicks == 2)
	{
		/* First, get the current selection range for word selection: */
		
		GetSelection( &selStart, &selEnd );
	
		/* Then, determine which character was clicked on: */
		
		QDToFrame( hitPt, &framePt );
		charOffset = GetCharOffset( &framePt );

		/*
			Finally, determine which word ... the word at
			the end of a "sub-line" of a word-wrapped line
			or a regular word.  The fNewEndWrapLineCaret
			Boolean was determined by GetCharOffset above:
		*/
		
		if (fNewEndWrapLineCaret)	--charOffset;
		GetWordBounds( charOffset, &selStart, &selEnd );
		SetSelection( selStart, selEnd, kRedraw );
		
	}	/* double-click */
	
	else if (optionKeyDown)
	{
		/*
			Autoscrolling ...
			
			a gadget whose idea came from my use of
			Symantec's "THINK Reference" application.
		*/
	
		/* Unhilite the current selection range: */
		
		HideSelection( kHideSelection, TRUE );
		
		if (itsSelStart == itsSelEnd)
		{
			saveSelStart = saveSelEnd = itsSelStart;
		}
		else	/* finite range */
		{
			/*
				Save selection range for restoration later.
				Then, set new selection with zero range.
				Otherwise, when you scroll beyond the prior
				selection range and subsequently scroll it back
				into view, it's still hilited.
			*/
			
			saveSelStart = itsSelStart;
			saveSelEnd = itsSelEnd;
			SetSelection( itsSelStart, itsSelStart, FALSE );
		}
		
		/*
			Nifty Cursor adapted from Symantec's
			"THINK Reference" application:
		*/
		
		if (gSystem.hasColorQD)
		{
			/* May have been purged: */
			LoadResource( (Handle)cAutoScrollCRSR );
			SetCCursor( cAutoScrollCRSR );
		}
		else
		{
			SetCursor( *cAutoScrollCursor );
		}

		while ( WaitMouseUp() )
		{			
			/* Get the current mouse position in frame coordinates: */
			
			GetMouse( &movedPt );
			QDToFrame( movedPt, &autoScrollPoint );
		
			/*
				Scroll the text automatically while
				the user is holding down the Mouse:
			*/
			(void) AutoScroll( &autoScrollPoint );

		}	/* while loop */
		
		/* Rehilite the current selection range: */
		
		if (saveSelStart != saveSelEnd)
		{
			SetSelection( saveSelStart, saveSelEnd, FALSE );
		}
		HideSelection( kUnhideSelection, TRUE );
			
	}	/* Autoscrolling with <Option> key down + just one click */
	
	else
	{
		GetSelection( &selStart, &selEnd );
	
		/* Determine which character was clicked on: */
	
		QDToFrame( hitPt, &framePoint );
		charOffset = GetCharOffset( &framePoint );
		origCharOffset = lastCharOffset = charOffset;
	
		/* Determine the new selection range: */
	
		if (modifierKeys & shiftKey)
		{
			/* If the Shift key is down, extend the selection: */
			
			origCharOffset = lastCharOffset = itsSelAnchor;
		}
		else
			SetSelection( charOffset, charOffset, kRedraw );	

		/* Ensure that the insertion caret is visible: */
	
		if (selStart == selEnd)
			ShowCaret();
	
		/* Track the selection while the mouse is down: */
	
		while ( StillDown() )
		{
			/* Get the current mouse position in frame coordinates: */
		
			GetMouse( &hitPt );
			QDToFrame( hitPt, &framePoint );
			autoScrollPoint = framePoint;
		
			/* Check if the mouse has moved to a new character: */
		
			PinInRect( &frame, &framePoint );
			charOffset = GetCharOffset( &framePoint );
			if (charOffset != lastCharOffset)
			{
				/* Remember the current selection range: */
			
				selStart = itsSelStart;
				selEnd = itsSelEnd;
			
				/* Determine the new selection range: */
			
				newSelStart = Min( origCharOffset, charOffset );
				newSelEnd = Max( origCharOffset, charOffset );

				/* Adjust the hilited text: */
			
				if (
					  (selStart == selEnd)		&&
					  fCaretVisible				&&
					  ((selStart != newSelStart) || (selStart != newSelEnd))
				   )
					HideCaret();
				
				if (selStart != newSelStart)
					HiliteTextRange( Min(selStart, newSelStart),
									 Max(selStart, newSelStart) );
				if (selEnd != newSelEnd)
					HiliteTextRange( Min(selEnd, newSelEnd),
									 Max(selEnd, newSelEnd) );
			
				SetSelection( newSelStart, newSelEnd, kNoRedraw );
			
				if (newSelStart == newSelEnd)
					ShowCaret();
			
				lastCharOffset = charOffset;	/* Remember current offset. */
				
			}	/* charOffset � lastCharOffset */
		
			/* Scroll text automatically while the user is selecting text: */
			
			AutoScroll( &autoScrollPoint );
		
		}	/* while StillDown() */
	
		/* Determine the anchor point for the selection: */
	
		itsSelAnchor = origCharOffset;

		/*
			For non-editable but selectable text, we want to display a selection
			range so the user can copy, but we don't want to display a caret:
		*/
		if ( !this->editable && (itsSelStart == itsSelEnd) )
			HideCaret();
			
		/* Notify dependents that the selection has changed: */
		
		SelectionChanged();
	
	}	/* no <Option> key + gClicks = 1 */
	
}	/* DoClick */



/* OVERRIDE: */
void	CPStyleText::DoKeyDown (char theChar, Byte keyCode, EventRecord *macEvent)
{
	/*
		DoKeyDown is the common denominator even if the
		<CMD> key is pressed.  If the <CMD> key is pressed,
		CSwitchboard's DoKeyEvent will scan the MENU list
		for <CMD> key equivalents by calling _MenuKey
		and if it finds none will send a DoKeyDown message
		to the gGopher which, of course, is us.
		
		As a result, Chris' DoKeyDown is called below.  If we
		pressed a <CMD> Arrow key, Chris calls DoArrowKey.
		If not a <CMD> Arrow key, then he calls CAbstractText's
		DoKeyDown.  If an unmodified or <Option>/<Shift> Arrow
		key, then the latter method will call TypeChar.
		
		The point of all this is that since DoKeyDown is
		the common denominator as mentioned above, then here
		is where we must set up the special Caret flags.
		We also do it for mouse clicks within DoClick, of course.
	*/
	
	
	fOldEndWrapLineCaret = fNewEndWrapLineCaret;
	fNewEndWrapLineCaret = FALSE;					/* Assume NOT !!! */

    if (theChar == kEnterKey)
    {
    	ScrollToSelection();
    }
	else
		CPEditText::DoKeyDown( theChar, keyCode, macEvent );
	
}	/* DoKeyDown */



/* OVERRIDE: */
void	CPStyleText::TypeChar (char theChar, short theModifiers)
{
		
	if ( IsArrowKey(theChar) )
	{
		/*
			Chris removed the call to SelectionChanged from
			DoArrowKey.  To compensate, he added this call
			to CPEditText's DoKeyDown, but seemingly forgot
			to do likewise within his TypeChar.
			
			WRONG !!!!!
			
			Look at CPEditText's DoKeyDown(�) and notice that
			if the <CMD> key is not pressed then CAbstractText's
			DoKeyDown is called.  The latter calls our TypeChar
			and follows with a call to SelectionChanged.  In
			short, the TCL calls SelectionChanged for us.
		*/
		
		DoArrowKey( theChar, theModifiers );
	}
	
	else
	{
		/*
			DeleteTextRange, ReplaceSelection and InsertTextPtr
			eventually call my AdjustLineStarts.  The latter
			calls my WrapLineStarts which will re-draw text
			lines that were wrapped.
		*/
		
			long		selStart = itsSelStart;
			long		selEnd	 = itsSelEnd;
			Boolean		beyondWordWrapEdge;
			LongPt		selPt;
		
		/* Obscure the cursor so it is hidden until the mouse is moved */
		ObscureCursor();
		
		/* Now perform the typing */
		if (theChar == kBackspace)
		{
			if (selStart == selEnd && selStart > 0)
			{
				HideCaret();
				DeleteTextRange( selStart - 1, selStart, kRedraw );
				ShowCaret();
			}
			else
				DeleteTextRange( selStart, selEnd, kRedraw );
		}
		else
		{
			GetCharPoint( selStart, &selPt );
			beyondWordWrapEdge = ( selPt.h + CharWidth(theChar) > itsDestRect.right );
			if (selStart != selEnd)
			{
				ReplaceSelection( &theChar, sizeof(char) );
				
				/*
					ReplaceSelection passes FALSE to all its methods
					for drawing, so make up for it here if required:
				*/
				
				if (beyondWordWrapEdge)
				{
					RefreshTextAfter( selStart, FALSE /* refreshOnlyLine */ );
					
				}	/* word-wrap needed */
				
			}
			else
			{
				/* InsertTextPtr addresses re-drawing for us: */
				
				InsertTextPtr( &theChar, sizeof(char), beyondWordWrapEdge );
			}
			
		}	/* NOT a backspace */
		
		/* Ensure that the insertion point is visible: */
		
		ScrollToOffset( itsSelStart );
		
	}	/* NOT an Arrow */
	
}	/* TypeChar */



/******************************************************************************
 DoArrowKey {OVERRIDE}
 
 	Handle a cursor key.  The behavior is as follows:
 		
							  modifier key
							  ============
					 <none>		 Option		 Command
					 ------		 ------		 -------
 			   		 
		up			prev line	prev page	first line
		down		next line	next page	last line
		left		prev char	prev word	line start
		right		next char	next word	line end
 		 
	Override to select the end of a "sub-line" of a word-wrapped line.
 		
 ******************************************************************************/

/* OVERRIDE: */
void	CPStyleText::DoArrowKey (char theChar, short theModifiers)
{
		Boolean			commandKeyDown = ( (theModifiers & cmdKey	) != 0 ),
						optionKeyDown  = ( (theModifiers & optionKey) != 0 ),
						shiftKeyDown   = ( (theModifiers & shiftKey	) != 0 ),
						isInsertion, isUpDownArrow;
		register long	nonAnchor;
		long			selChar;
		long			numLines, selLine;
		long			selStart, selEnd;
		LongPt			charPt;
		tCharBuf		breakChar;
	
	
	if (optionKeyDown || shiftKeyDown)
	{
		CPEditText::DoArrowKey( theChar, theModifiers );
	}
	
	else if (theChar == kUpCursor || theChar == kDownCursor)
	{
		if (commandKeyDown)
		{
			CPEditText::DoArrowKey( theChar, theModifiers );
		}
		
		else
		{
			GetSelection( &selStart, &selEnd );
			numLines = GetNumLines();
	
			/* Determine if we have an insertion point or not */
	
			isInsertion = (selStart == selEnd);
	
			/* Reset the selection anchor position if necessary */
	
			if (isInsertion)
				itsSelAnchor = selStart;
			else if (itsSelAnchor < 0)
			{
				if (theChar == kUpCursor)
					itsSelAnchor = selEnd;
				else
					itsSelAnchor = selStart;
			}
	
			/* Determine the non-anchored selection position */
	
			nonAnchor = (itsSelAnchor == selEnd ? selStart : selEnd);
	
			/* Now handle the keystroke */
	
			selLine = FindLine( nonAnchor );
		
			if ( (theChar == kUpCursor) && (selLine == 0) )
			{
				selChar = 0;
				;
				isUpDownArrow = FALSE;
			}
			
			else if ( (theChar == kDownCursor) && (selLine == numLines - 1) )
			{
				selChar = itsTextLength;
				;
				isUpDownArrow = FALSE;
			}
			
			else
			{
				/* Somewhere in the middle of the text. */
					
				if (fOldEndWrapLineCaret)
				{
					/*
						At end of a word-wrapped "sub-line".
					
						Further down we call GetCharOffset which sets the
						fNewEndWrapLineCaret flag depending on whether or
						not we're at the end of a wrapped "sub-line".
						Upon re-entry to DoKeyDown, fOldEndWrapLineCaret
						is set equal to fNewEndWrapLineCaret.
					
						See additional comments below when <CMD>-Right Arrow:
					*/
					
					if ( !fUpDownArrow )
					{
						/*
							This instance Boolean = FALSE when the:
							
								last Arrow keypress was a Right or Left Arrow	OR
								last Up Arrow after already at the top			OR
								last Down Arrow after already at the bottom		OR
								user clicked elsewhere							OR
								user typed a non-Arrow character
						
							Note that "itsUpDownHOffset" stays defined as here
							until we satisfy one of the five above conditions
							AND eventually press an un-modified Up/Down Arrow.
							Only then do we come back here to re-define this
							instance variable and start fresh with a new value.
						*/
					
						selChar = nonAnchor - 1;
						GetCharPoint( selChar, &charPt );
						GetCharAfter( &selChar, breakChar );
						itsUpDownHOffset = charPt.h + CharWidth( breakChar[1] );

					}	/* !fUpDownArrow */
					
					if (theChar == kUpCursor)
					{
						/*
							Chris' GetLineStart below will
							compensate if we back up too far:
						*/
						
						selLine -= 2;
					}
					else
					{
					 /*
						++selLine;  --  already there
					 */
					}

				}	/* fOldEndWrapLineCaret */
				
				else
				{
					if ( !fUpDownArrow )
					{
						GetCharPoint( nonAnchor, &charPt );
						itsUpDownHOffset = charPt.h;
					}
				
					if (theChar == kUpCursor)
						--selLine;
					else
						++selLine;

				}	/* not at end of a wrapped "sub-line" */

				GetCharPoint( GetLineStart(selLine), &charPt );
				charPt.h = (long)itsUpDownHOffset;
				selChar = GetCharOffset( &charPt );

				isUpDownArrow = TRUE;
					
			}	/* in the middle of the text */
		
			SetSelection( selChar, selChar, kRedraw );

			ScrollToOffset( selChar );
	
			fUpDownArrow = isUpDownArrow;
			
		}	/* unmodified kUpCursor or kDownCursor */

	}	/* kUpCursor || kDownCursor */
	
	else
	{
		GetSelection( &selStart, &selEnd );
		numLines = GetNumLines();
	
		/* Determine if we have an insertion point or not: */
	
		isInsertion = (selStart == selEnd);
	
		/* Reset the selection anchor position if necessary: */
	
		if (isInsertion)
			itsSelAnchor = selStart;
		else if (itsSelAnchor < 0)
		{
			if (theChar == kLeftCursor)
				itsSelAnchor = selEnd;
			else
				itsSelAnchor = selStart;
		}
	
		/*
			Determine the non-anchored selection
			position and its corresponding line:
		*/
	
		nonAnchor = (itsSelAnchor == selEnd ? selStart : selEnd);
		selLine = FindLine( nonAnchor );
	
		/* Now handle the keystroke: */
	
		if (commandKeyDown)
		{			
			if (theChar == kLeftCursor)
			{
				if (fOldEndWrapLineCaret)
				{
					/*
						Former caret position at end of THIS "sub-line".
						Remember, the end of a "sub-line" is really
						the beginning of the next "sub-line".  See
						comments about the selection point below:
					*/
					
					selLine = Max( selLine - 1, 0 );
				}
				
				selChar = GetLineStart( selLine );
				
			}	/* kLeftCursor */
			
			else
			{
				selChar = GetLineEnd( selLine );
				GetCharBefore( &selChar, breakChar );
				if ( breakChar[0] )
				{
						unsigned char	ch = breakChar[1];
						
					if (ch != kReturn)
					{
						/*
							Note that I do NOT count a <CR> as a word break
							character.  This is because for a <CR> the
							selection is BEFORE the <CR> and for this there
							is no special Caret placement.
							
							Chris, however, does count the <CR> because
							when double clicking on the last word of a line
							ending in a <CR>, he doesn't want to select the
							<CR> itself.  The only way, therefore, to delete
							a <CR> is to select the beginning of the NEXT
							line and then backspace.
						*/
					
						fNewEndWrapLineCaret = IsWordBreakChar( ch );
						
						/*
							Note that GetCharBefore decrements selChar.
							selChar should remain decremented for a line
							ending with a <CR>.  Otherwise, we need to
							increment the selection point back to beyond
							any other character so both backspacing and
							my DrawCaret method function properly:
						*/
						++selChar;
						
					}	/* not a <CR> */
				}
				
			}	/* kRightCursor */
			
		}	/* <CMD> key */
		
		/*
			We've eliminated the <Option> and <Shift> keys by
			going directly to Chris' method above, so what's
			left must be just the plain, un-modified Arrow key:
		*/
			
		else if (isInsertion)
		{
			if (theChar == kLeftCursor)
			{
				selChar = Min( Max(nonAnchor - 1, 0), itsTextLength );
			}
			
			else
			{
				/*
					Here we're going PAST the line's end whereas with
					<CMD>-Right Arrow above we went TO the line's end:
				*/
				
				selChar = Min( Max(nonAnchor + 1, 0), itsTextLength );
				if (selChar == GetLineEnd(selLine) )
				{
					GetCharBefore( &selChar, breakChar );
					if ( breakChar[0] )
					{
							unsigned char	ch = breakChar[1];
							
						fNewEndWrapLineCaret = (ch != kReturn)		&&
											   IsWordBreakChar( ch );
					}
					
					/*
						Remember that GetCharBefore decrements selChar and
						that the selection point is beyond the last char.
						
						If the last character is a word break character,
						the new selection point is really the beginning of
						the NEXT "sub-line".  My DrawCaret method, however,
						will position the Caret at the end of THIS "sub-line".
						
						If we're going past a <CR>, the new selection is the
						beginning of the next whole line and that's where
						the Caret is drawn.
					*/
					
					++selChar;
					
				}	/* at the end */
				
			}	/* kRightCursor */
			
		}	/* isInsertion */
		
		else
			selChar = ((theChar == kLeftCursor) ? selStart : selEnd);
	
		SetSelection( selChar, selChar, kRedraw );

		ScrollToOffset( selChar );
	
		fUpDownArrow = FALSE;

	}	/* left, right Arrow or <CMD> left, right Arrow */

}	/* DoArrowKey */



/**** T E X T   S P E C I F I C A T I O N   M E T H O D S ****/


/* OVERRIDE: */
void	CPStyleText::SetTextPtr (Ptr textPtr, long numChars)
{
	/*
		SetTextPtr is typically called to install text
		data into a window.  CAbstractText's SetWholeLines
		is typically called by its ResizeFrame.  However,
		until text data is installed, there are NO lines
		so SetWholeLines does nothing.  Here is our first
		chance to resize the frame when there ARE lines.
		
		One other general series of comments ...
		
		A lot of methods call AdjustBounds but withOUT calling
		SetWholeLines first.  The problem here centers on the
		fact that SetWholeLines resets the frame size based on
		line count and height that vary with typing or inserting
		text which actions eventually call AdjustLineStarts.
		Line count and height also vary with font size etc. and
		these actions eventually call CalcLineHeight.  Both of
		these methods call AdjustBounds.
		
		WithOUT calling SetWholeLines first, AdjustBounds calls
		SetBounds which will call CScrollPane::AdjustScrollMax
		to adjust the scroll bars maximum scroll position based
		on an NON-updated frame, thus messing up the works.
	*/
	
	
	CPEditText::SetTextPtr( textPtr, numChars );
	
	SetWholeLines( wholeLines );
	
}	/* SetTextPtr */



/* OVERRIDE: */
void	CPStyleText::ReplaceSelection (Ptr replacePtr, long replaceLen)
{
		Boolean		refresh;
		long		selStart;
		long		selEnd;
		long		numReplaceCRs;
		long		numInsertCRs;
		long		numLinesDelta;
	
	
	CheckInsertion( replacePtr, replaceLen, kUseSelection,
					&numInsertCRs, &numReplaceCRs );
	
	/* Delete the current selection, if any */
	
	GetSelection( &selStart, &selEnd );
	if (selStart < selEnd)
	{
		DeleteText( selStart, selEnd, numReplaceCRs, kNoRedraw );
		numLinesDelta = -numReplaceCRs;
		/*
			Passing FALSE to DeleteText does
			NOT call RefreshTextAfter:
		*/
		refresh = TRUE;
	}
	else
	{
		numLinesDelta = 0;
		refresh = FALSE;
	}
	
	/* Insert the new text, if any */
	
	if ((replacePtr != NULL) && (replaceLen > 0))
	{
		InsertText( replacePtr, replaceLen, numInsertCRs, kNoRedraw );
	 /*
		Called by InsertText:
		
		SetSelection( selStart + replaceLen, selStart + replaceLen, kNoRedraw );
	 */
		/*
			Passing FALSE to InsertText
			DOES call RefreshTextAfter:
		*/
		refresh = FALSE;
	}
	
	/* Refresh the text if necessary */
	
	if (refresh)
		RefreshTextAfter( selStart, (numLinesDelta == 0) );
	
}	/* ReplaceSelection */



/**** W O R D   W R A P   M E T H O D S ****/


/* OVERRIDE: */
void	CPStyleText::SetWordWrap (Boolean doWrap, long lineLengthInChars)
{
	/*
		The calling sequence is:
		
			SetMargins(�);
			SetWordWrap(�);
			DoWordWrap();
	*/
	
		
	fWordWrap = doWrap;
	
	if (doWrap)
	{
		/* If we're wrapping to the Window, then the passed length <= 0: */
		
		if (lineLengthInChars <= 0)
		{
			SetAlignCmd( cmdAlignLeft );	/* See comments within UpdateMenus. */
			
			if (lineWidth != -1)
			{
				/*
					Previously set to other than wrap-to-window-width.
					Scroll home so word wrap can be properly setup:
				*/
				
					LongPt		homePos;
					
				GetHomePosition( &homePos );
				ScrollTo( &homePos, TRUE );
			
				/*
					SetAlignCmd does nothing if already wrapped to window
					width.  So set lineWidth = 1 AFTER SetAlignCmd:
				*/
				lineWidth = -1;
			}
		}	
		else
		{
			/* lineLengthInChars > 0 */
			
				FontInfo	fInfo;
			
			GetMacFontInfo( &fInfo );
			lineWidth = lineLengthInChars * fInfo.widMax;
		}
	}
	else
	{
		/* Word wrap OFF: */
		
		lineWidth = itsRightMargin;
	}
	
	autoRefresh = (lineWidth == -1);
		
}	/* SetWordWrap */



/* OVERRIDE: */
Boolean		CPStyleText::GetWordWrap (void)
{
	/*
		You can retrieve the wrap line width
		via CAbstractText's "lineWidth".
	*/
	
	
	return	(fWordWrap);
	
}	/* GetWordWrap */



/* OVERRIDE: */
void	CPStyleText::DoWordWrap (void)
{
	/*
		The calling sequence is:
		
			SetMargins(�);
			SetWordWrap(�);
			DoWordWrap();
	*/
	
	
	ForceNextPrepare();
		
	/*
		CalcPERects adjusts the CPStyleText pane's
		destRect.right to the window's interior if
		its lineWidth is negative or, if positive,
		to the specified lineWidth.
	*/
	CalcPERects();	

	/*
		RecalcLineStarts adjusts itsNumLines and the
		itsLineStarts array if required to effect
		word-wrap to the new "itsDestRect".  It then
		determines the new bounds based on these data
		by calling my AdjustBounds():
	*/
	RecalcLineStarts();
	
	Refresh();
	
}	/* DoWordWrap */



/**** S T Y L I N G   M E T H O D S ****/


/* OVERRIDE: */
void	CPStyleText::SetSpacingCmd (long aSpacingCmd )
{
	/*
		CPEditText does NOT support variable spacing,
		so we need to "roll our own".  Get spacing
		either for the current paragraph if nothing
		is selected or for current selection:
	*/
	
		long			oldSpacingCmd, startLine, endLine, i,
						selStart, selEnd;
		TEStyleHandle	styleHdl;
		LHHandle		lineHeightHdl;
		SignedByte		hState;
		short			lineHt,
						extra;					/* Extra space between lines. */
		StScrpHandle	scrapHdl;
		Handle			theText;
		
		
	CPEditText::SetSpacingCmd( aSpacingCmd );

}	/* SetSpacingCmd */



/* OVERRIDE: */
void	CPStyleText::SetAlignCmd (long anAlignCmd)
{

	/* See comments within UpdateMenus: */
	
	if (lineWidth == -1)	return;
	
	if (itsAlignCmd != anAlignCmd)
	{
		itsAlignCmd = anAlignCmd;
	
		SetAlignment( (short)anAlignCmd );
	
		Refresh();
	}
	
}	/* SetAlignCmd */



/* OVERRIDE: */
void	CPStyleText::SetAlignment (short anAlignment )
{
	/* Set the alignment ONLY for the current paragraph: */
	
		long		selStart, selEnd;
		
	
	switch	(anAlignment)
	{
		case cmdAlignRight:
		
			break;
			
		case cmdAlignCenter:
		
			break;

		case cmdAlignLeft:
		
			break;
		
		case cmdJustify:
		
			break;
	}
	
}	/* SetAlignment */



/* OVERRIDE: */
long	CPStyleText::GetAlignCmd (void)
{

	return	(itsAlignCmd);
	
}	/* GetAlignCmd */



/* OVERRIDE: */
StScrpHandle	CPStyleText::GetStyleScrap (void)
{
	/*
		For now, just duplicate CTextStyleTask's SaveStyle:
	*/
	
	
		StScrpHandle		theScrap	  = NULL;
		CTextStyleTask		*theStyleTask = (CTextStyleTask*) itsLastTask;
		TextStyle			style;
		short				styleAttrib;


	styleAttrib = doAll;
	GetTextStyle( &styleAttrib, &style );
	theStyleTask->oldStyle = style;
	
	return	(theScrap);
	
}	/* GetStyleScrap */



/* OVERRIDE: */
void	CPStyleText::SetStyleScrap (long rangeStart, long rangeEnd,
									StScrpHandle styleScrap, Boolean redraw)
{
	/*
		For now, just duplicate CTextStyleTask's RestoreStyle:
	*/
	
	
		CTextStyleTask		*theStyleTask = (CTextStyleTask*) itsLastTask;
		long				alignCmd, spacingCmd; 


	Prepare();

	switch (theStyleTask->styleAttribute)
	{
		case doFont:
			SetFontNumber( theStyleTask->oldStyle.tsFont );
			break;
			
		case doFace:
			SetFontStyle( 1 << (theStyleTask->styleCmd - cmdBold) );
			break;
			
		case doSize:
			SetFontSize( theStyleTask->oldStyle.tsSize );
			break;
			
		case doAlign:
			alignCmd = GetAlignCmd();
			SetAlignCmd( theStyleTask->oldAlignCmd );
			theStyleTask->oldAlignCmd = alignCmd;
			break;
			
		case doSpacing:
			spacingCmd = GetSpacingCmd();
			SetSpacingCmd( theStyleTask->oldSpacingCmd );
			theStyleTask->oldSpacingCmd = spacingCmd;
			break;

	}	/* switch */

	/*
		See comments within SetTextPtr method:
	*/
	
	SetWholeLines( wholeLines );
	AdjustBounds();

}	/* SetStyleScrap */



/******************************************************************************
	Note that the line numbers passed to the following six
	methods are 0-based just as is Chris' "itsLineStarts"
	Handle.  However, because the various CArray methods
	such as GetItem and InsertAtIndex are 1-based, so are
	the passed style indices.
 ******************************************************************************/

	static long		Convert (CObject *theObject, long num)
	{
	
		if ( member(theObject, CPStyleText) )
		{
			if (num == atBeginning)
				return	(0L);
			else if (num == atEnd)
				return	( ((CPStyleText*) theObject)->itsNumLines - 1 );
			else
				return	(num);
		}
		
		else if ( member(theObject, CArray) )
		{
			if (num == atBeginning)
				return	(1L);
			else if (num == atEnd)
				return	( ((CArray*) theObject)->GetNumItems() );
			else
				return	(num);
		}
		
		else
		{
			ASSERT(
					 member(theObject, CPStyleText)
							||
					 member(theObject, CArray)
				  );
		}
		
	}	/* Convert */
	
	
	
/* OVERRIDE */
void	CPStyleText::AddLine (long afterLine)
{
		long			lineNum, styleInLine;
		CArray			*newStyles = NULL;
		StyleRecord		newStyleInfo;
		LineRecord		newLineInfo;
		
	
	ASSERT( itsLineInfo != NULL );

	ASSERT( itsNumLines < MAX_LINES );

	/*
		First fill in the component StyleRecord and then
		fill in the remaining parts of the LineRecord item.
	*/
	
	lineNum = Convert( this, afterLine );
	
	newStyles = new (CArray);
	newStyles->IArray( sizeof(StyleRecord) );

	if (itsTextLength == 0)
	{
		/* A brand new text window: */
		
	 // newLineInfo.lStyles			= newStyles;  --  later ...
		newLineInfo.lSpacingCmd		= itsSpacingCmd;
		newLineInfo.lHeight			= itsLineHeight;
		newLineInfo.lJustAmount		= macPort->spExtra;
		newLineInfo.lTabSpaces		= itsTabSpaces;
		newLineInfo.lTabWidth		= itsTabWidth;

		newStyleInfo.sStart			= GetLineEnd( lineNum );
		newStyleInfo.sEnd			= newStyleInfo.sStart;
		newStyleInfo.sHeight		= itsLineHeight;
		newStyleInfo.sFont			= itsTextFont;
		newStyleInfo.sSize			= itsTextSize;
		newStyleInfo.sFace			= itsTextFace;
		newStyleInfo.sMode			= itsTextMode;
		newStyleInfo.sColor.red		= 0;					/* Black ... */
		newStyleInfo.sColor.green	= 0;
		newStyleInfo.sColor.blue	= 0;
		newStyleInfo.sMaxCharWidth	= itsMaxCharWidth;
	}
	
	else
	{
		/*
			Duplicate either the first line or
			continue from the previous line ...
		*/
		
		/*
			CArray's GetItem _BlockMove's the
			array element into our local struct:
		*/
		itsLineInfo->GetItem( &newLineInfo, lineNum + 1 );
		if (afterLine == atBeginning)
			styleInLine = 1;
		else
			styleInLine = newLineInfo.lStyles->GetNumItems();
		newLineInfo.lStyles->GetItem( &newStyleInfo, styleInLine );
		
		/* Now, change the needed fields: */
		
		if (afterLine == atBeginning)
			newStyleInfo.sStart	= GetLineStart( lineNum );
		else
			newStyleInfo.sStart	= GetLineEnd( lineNum );
		newStyleInfo.sEnd	= newStyleInfo.sStart;
		
	}	/* text length > 0 */

	/*
		Now stuff the copies in place ...
	*/
	
	newStyles->InsertAtIndex( &newStyleInfo, 1 );		/* Make it the first item. */
	newLineInfo.lStyles = newStyles;
	if (afterLine == atBeginning)
		itsLineInfo->InsertAtIndex( &newLineInfo, 1 );	/* Ditto				   */
	else
		itsLineInfo->InsertAtIndex( &newLineInfo, lineNum + 2 );
	
}	/* AddLine */



/* OVERRIDE */
void	CPStyleText::DeleteLine (long lineNum)
{
	/*
		This method is called when we backspace over a <CR>.
		We need to merge the styles from the PREVIOUS line,
		if any, with the styles of THIS line.  The PREVIOUS
		line is the line whose trailing <CR> we are deleting
		and THIS line is the line we're deleting.
		
		Remember ... itsLineInfo is an ordered array of
		LineRecords each of which corresponds to the text
		in a <CR>-delimited line.  In short ... word wrapping
		does NOT affect this array.
	*/
	
		LineRecord		prevLineInfo, thisLineInfo;
		StyleRecord		styleInfo;
		long			numStylesInThisLine, styleIndex;


	ASSERT( itsLineInfo != NULL );

	lineNum = Convert( this, lineNum );
	
	itsLineInfo->GetItem( &thisLineInfo, lineNum + 1 );
	
	if (lineNum > 0)
	{
		/* There IS a previous line ... */
		
		numStylesInThisLine = thisLineInfo.lStyles->numItems;
		
		for (styleIndex = 1; styleIndex <= numStylesInThisLine; ++styleIndex)
		{
			/* Merge styles in THIS line to those in PREVIOUS line: */
			
			GetStyleInfo( lineNum, styleIndex, &styleInfo );	/* THIS line	 */
			AddStyle( lineNum - 1, atEnd, &styleInfo );			/* PREVIOUS line */
		}
	}
	
	/*
		Need to dump the LineRecord item's component StyleRecord
		CArray before we dispose of the entire LineRecord item:
	*/

	ForgetObject( thisLineInfo.lStyles );

	itsLineInfo->DeleteItem( lineNum + 1 );
	
}	/* DeleteLine */



/* OVERRIDE */
void	CPStyleText::AddStyle (long lineNum, long afterStyleIndex,
							   StyleRecord *styleInfo)
{
	/*
		Remember ...
		
		... lineNum is 0-based and afterStyleIndex is 1-based.
	*/
	
		LineRecord		lineInfo;
		long			numStyles;


	ASSERT( itsLineInfo != NULL );

	lineNum = Convert( this, lineNum );
	itsLineInfo->GetItem( &lineInfo, lineNum + 1 );

	numStyles = lineInfo.lStyles->GetNumItems();
	ASSERT( numStyles < MAX_STYLES_PER_LINE );

 /*
	Because of the "after" in the passed index,
	Convert does not QUITE work ...
	
	afterStyleIndex = Convert( lineInfo.lStyles, afterStyleIndex );
 */
	if (afterStyleIndex == atBeginning)
		afterStyleIndex = 0;
	else if (afterStyleIndex == atEnd)
		afterStyleIndex = numStyles;
	lineInfo.lStyles->InsertAtIndex( styleInfo, afterStyleIndex + 1 );
		
}	/* AddStyle */



/* OVERRIDE */
void	CPStyleText::DeleteStyle (long lineNum, long styleIndex)
{
		LineRecord		lineInfo;


	ASSERT( itsLineInfo != NULL );

	lineNum = Convert( this, lineNum );
	itsLineInfo->GetItem( &lineInfo, lineNum + 1 );
	
	styleIndex = Convert( lineInfo.lStyles, styleIndex );
	lineInfo.lStyles->DeleteItem( styleIndex );
		
}	/* DeleteStyle */



/* OVERRIDE */
void	CPStyleText::GetStyleInfo (long lineNum, long styleIndex,
								   StyleRecord *styleInfo)
{
		LineRecord		lineInfo;
		
		
	ASSERT( itsLineInfo != NULL );

	lineNum = Convert( this, lineNum );
	itsLineInfo->GetItem( &lineInfo, lineNum + 1 );			/* 0-based */
		
	styleIndex = Convert( lineInfo.lStyles, styleIndex );
	lineInfo.lStyles->GetItem( styleInfo, styleIndex );		/* 1-based */
	
}	/* GetStyleInfo */



/* OVERRIDE */
void	CPStyleText::SetStyleInfo (long lineNum, long styleIndex,
								   StyleRecord *styleInfo)
{
		LineRecord		lineInfo;
		
		
	ASSERT( itsLineInfo != NULL );

	lineNum = Convert( this, lineNum );
	itsLineInfo->GetItem( &lineInfo, lineNum + 1 );
		
	styleIndex = Convert( lineInfo.lStyles, styleIndex );
	lineInfo.lStyles->SetItem( styleInfo, styleIndex );
	
}	/* SetStyleInfo */



/* OVERRIDE */
Boolean		CPStyleText::FindStyle (long startPos, long endPos,
									long *startLine, long *endLine,
									long *startIndex, long *endIndex)
{
	/*
		My overridden methods = SetFontNumber, SetFontSize etc.
		call FindStyle.  When FindStyle returns, the line numbers
		and indices corresponding to the passed positions are
		filled in.  These indices are the indices to the style
		runs which bracket the passed positions.
		
		FindStyle returns TRUE only if the passed positions match
		the positions of the already stored style runs exactly.
	*/
	
		Boolean			foundExactStartIndex = FALSE,
						foundExactEndIndex	 = FALSE;
		CArray			*theStyles;
		LineRecord		lineInfo;
		StyleRecord		styleInfo;
		register long	startLn, endLn,
						numStyles, styleIndex;
		
		
	ASSERT( itsLineInfo != NULL );
	
	startLn = *startLine = FindLine( startPos );
	endLn = *endLine = FindLine( endPos );
	
	itsLineInfo->GetItem( &lineInfo, startLn + 1 );
	theStyles = lineInfo.lStyles;
	numStyles = theStyles->GetNumItems();
	
	for (styleIndex = 1; styleIndex <= numStyles; ++styleIndex)
	{
		theStyles->GetItem( &styleInfo, styleIndex );
		if ( styleInfo.sStart == startPos )
		{
			/* An exact match ... */
				
			*startIndex = styleIndex;			/* Current style run. */
			foundExactStartIndex = TRUE;
			break;
		}
			
		else if ( styleInfo.sStart > startPos )
		{
			if (styleIndex == 1)
				*startIndex = styleIndex;		/*       ?????		   */
			else
				*startIndex = styleIndex - 1;	/* Previous style run. */
			break;
		}

	}	/* for-loop */
		
	if (styleIndex > numStyles)
	{
		*startIndex = atEnd;
		*endIndex = atEnd;
		
		/*
			No sense worrying about the end position
			since we can't even bracket the start position:
		*/
		return	(FALSE);
	}
	
	/*
		Okay, we've got the startIndex, so
		let's continue with the endIndex ...
	*/
	
	itsLineInfo->GetItem( &lineInfo, endLn + 1 );
	theStyles = lineInfo.lStyles;
	numStyles = theStyles->GetNumItems();
	
	for (styleIndex = 1; styleIndex <= numStyles; ++styleIndex)
	{
		theStyles->GetItem( &styleInfo, styleIndex );
		if ( styleInfo.sEnd == endPos )
		{
			/* An exact match ... */
				
			*endIndex = styleIndex;				/* Current style run. */
			foundExactEndIndex = TRUE;
			break;
		}
			
		else if ( styleInfo.sEnd > endPos )
		{
			*endIndex = styleIndex;				/* Current style run. */
			break;
		}

	}	/* for-loop */
	
	if (styleIndex > numStyles)
		*endIndex = atEnd;
		
	return	( foundExactStartIndex && foundExactEndIndex );

}	/* FindStyle */



/* OVERRIDE */
void	CPStyleText::SetFontNumber (short aFontNumber)
{
		Boolean			foundStyle, twoParts;
		long			startLine, endLine,
						startIndex, endIndex,
						saveSelEnd;
		register long	line, styleIndex, numStyles;
		LineRecord		lineInfo;
		CArray			*theStyles;
		StyleRecord		styleInfo;
	
	
	foundStyle = FindStyle( itsSelStart, itsSelEnd,
							&startLine, &endLine,
							&startIndex, &endIndex );
	
	itsLineInfo->GetItem( &lineInfo, startLine + 1 );
	theStyles = lineInfo.lStyles;
		
	if (startLine == endLine)
	{
		if (foundStyle)
		{
			/*
				Change leading StyleRecord in the returned index
				range and then delete the excess that follow, if any:
			*/
		
			GetStyleInfo( startLine, startIndex, &styleInfo );
			styleInfo.sEnd = itsSelEnd;
			styleInfo.sFont = aFontNumber;
			SetStyleInfo( startLine, startIndex, &styleInfo );
		
			/* Always delete backwards: */
			for (styleIndex = endIndex; styleIndex >= startIndex + 1; --styleIndex)
			{				
				theStyles->DeleteItem( styleIndex );
			}
			
		}	/* found exact match */
		
		else
		{
			if (startIndex != atEnd && endIndex != atEnd)
			{
				/*
					Change first and last StyleRecords in the returned index
					range and then delete those in the middle, if any:
				*/
			
				GetStyleInfo( startLine, startIndex, &styleInfo );
				saveSelEnd = styleInfo.sEnd;
				if (styleInfo.sStart != itsSelStart)
				{
					styleInfo.sEnd = itsSelStart;
					SetStyleInfo( startLine, startIndex, &styleInfo );
					;
					twoParts = TRUE;
				}
				else
					twoParts = FALSE;
				
				if (endIndex != startIndex)
				{
					GetStyleInfo( startLine, endIndex, &styleInfo );
					if (styleInfo.sEnd != itsSelEnd)
					{
						styleInfo.sStart = itsSelEnd;
						SetStyleInfo( startLine, endIndex--, &styleInfo );
					}
					
					for (
						   styleIndex = endIndex;
						   styleIndex >= startIndex + 1;
						   --styleIndex
						)
					{
						theStyles->DeleteItem( styleIndex );
					}
					
					GetStyleInfo( startLine, startIndex, &styleInfo );
					styleInfo.sStart = itsSelStart;
					styleInfo.sEnd = itsSelEnd;
					styleInfo.sFont = aFontNumber;
					if (twoParts)
						AddStyle( startLine, startIndex, &styleInfo );
					else
						SetStyleInfo( startLine, startIndex, &styleInfo );
					
				}	/* endIndex � startIndex */
				
				else
				{
					if (saveSelEnd != itsSelEnd)
					{
						styleInfo.sStart = itsSelStart;
						styleInfo.sEnd = itsSelEnd;
						styleInfo.sFont = aFontNumber;
						if (twoParts)
							AddStyle( startLine, startIndex++, &styleInfo );
						else
							SetStyleInfo( startLine, startIndex, &styleInfo );
						;
						styleInfo.sStart = itsSelEnd;
						styleInfo.sEnd = saveSelEnd;
						AddStyle( startLine, startIndex, &styleInfo );
					}
					else
					{
						styleInfo.sStart = itsSelStart;
					 // styleInfo.sEnd = saveSelEnd;  --  already there!
						styleInfo.sFont = aFontNumber;
						if (twoParts)
							AddStyle( startLine, startIndex, &styleInfo );
						else
							SetStyleInfo( startLine, startIndex, &styleInfo );
					}
					
				}	/* endIndex = startIndex */
				
			}	/* startIndex � atEnd && endIndex � atEnd */
			
			else if (startIndex == atEnd)
			{
				GetStyleInfo( startLine, atEnd, &styleInfo );
				styleInfo.sStart = itsSelStart;
				styleInfo.sEnd = itsSelEnd;
				styleInfo.sFont = aFontNumber;
				AddStyle( startLine, atEnd, &styleInfo );
				
			}	/* startIndex = atEnd */
			
			else
			{
				GetStyleInfo( startLine, startIndex, &styleInfo );
				if (styleInfo.sStart != itsSelStart)
				{
					styleInfo.sEnd = itsSelStart;
					SetStyleInfo( startLine, startIndex++, &styleInfo );
				}
				
				GetStyleInfo( startLine, atEnd, &styleInfo );
				styleInfo.sStart = itsSelStart;
				styleInfo.sEnd = itsSelEnd;
				styleInfo.sFont = aFontNumber;
				AddStyle( startLine, atEnd, &styleInfo );
				
				for (
						styleIndex = endIndex;
						styleIndex >= startIndex;
						--styleIndex
					)
				{
					theStyles->DeleteItem( styleIndex );
				}
				
			}	/* endIndex = atEnd */
			
		}	/* did NOT find exact match */
		
	}	/* startLine = endLine */
	
	else
	{
		/* Selection spans multiple lines: */
		
		GetStyleInfo( startLine, startIndex, &styleInfo );

		if (startIndex != atEnd)
		{
			/* We found the starting style ... */
			
			if (styleInfo.sStart != itsSelStart)
			{
				styleInfo.sEnd = itsSelStart;
				SetStyleInfo( startLine, startIndex--, &styleInfo );
				;
				twoParts = TRUE;
			}
			else
				twoParts = FALSE;
				
			/*
				We found at least the starting style, so we need
				to delete from here to the end of the line.  Once
				again, note we are deleting in reverse:
			*/
			
			numStyles = theStyles->GetNumItems();		/* In startLine. */
			for (styleIndex = numStyles; styleIndex >= startIndex + 1; --styleIndex)
				theStyles->DeleteItem( styleIndex );
		}
		else
			twoParts = FALSE;
		
		styleInfo.sStart = itsSelStart;
		styleInfo.sEnd = GetLineEnd( startLine );
		styleInfo.sFont = aFontNumber;
		if (twoParts)
			AddStyle( startLine, startIndex, &styleInfo );
		else
			SetStyleInfo( startLine, startIndex, &styleInfo );
		
		/* The startLine is now taken care of.  Next is the endLine: */
		
		itsLineInfo->GetItem( &lineInfo, endLine + 1 );
		theStyles = lineInfo.lStyles;
		
		GetStyleInfo( endLine, endIndex, &styleInfo );
		
		if (endIndex != atEnd)
		{
			if (styleInfo.sEnd != itsSelEnd)
			{
				styleInfo.sStart = itsSelEnd;
				SetStyleInfo( endLine, endIndex--, &styleInfo );
				;
				twoParts = TRUE;
			}
			else
				twoParts = FALSE;

			/*
				We found at least the ending style, so we need
				to delete from here to the start of the line:
			*/
			
			for (styleIndex = endIndex; styleIndex >= 1; --styleIndex)
				theStyles->DeleteItem( styleIndex );
		}
		else
			twoParts = FALSE;
		
		styleInfo.sStart = GetLineStart( startLine );
		styleInfo.sEnd = itsSelEnd;
		styleInfo.sFont = aFontNumber;
		if (twoParts)		AddStyle( startLine, atBeginning, &styleInfo );
		else				SetStyleInfo( startLine, atBeginning, &styleInfo );
		
		/* Finally, address all the lines inbetween: */
		
		for (line = startLine + 1; line <= endLine - 1; ++line)
		{
			itsLineInfo->GetItem( &lineInfo, line );
			theStyles = lineInfo.lStyles;
			numStyles = theStyles->GetNumItems();			/* In this line. */
			/*
				Delete all styles AFTER the first:
			*/
			for (styleIndex = numStyles; styleIndex >= 2; --styleIndex)
				theStyles->DeleteItem( styleIndex );
			
			GetStyleInfo( line, 1, &styleInfo );
			styleInfo.sStart = GetLineStart( line );
			styleInfo.sEnd = GetLineEnd( line );
			styleInfo.sFont = aFontNumber;
			SetStyleInfo( line, 1, &styleInfo );
			
		}	/* for lines inbetween */
		
	}	/* startLine � endLine */
	
}	/* SetFontNumber */



/* OVERRIDE */
void	CPStyleText::SetFontSize (short aSize)
{
}	/* SetFontSize */



/* OVERRIDE */
void	CPStyleText::SetFontStyle (short aStyle)
{
}	/* SetFontStyle */



/* OVERRIDE */
void	CPStyleText::SetTextMode (short aMode)
{
}	/* SetTextMode */



/**** C O M M A N D   M E T H O D S ****/


/* OVERRIDE */
void	CPStyleText::DoCommand (long theCommand)
{

	switch (theCommand)
	{
		case cmdJustify:
		 // if (makeStyleTask && stylable)	--  which are TRUE !!!
		 // {
					CStylePStyleTask	*styleTask;
					
				itsTypingTask = NULL;
				
				styleTask = NULL;
				styleTask = (CStylePStyleTask*) MakeStyleTask( theCommand );
				itsLastTask = styleTask;
				itsSupervisor->Notify( styleTask );
				styleTask->Do();
		 // }

			break;
			
		default:
			CPEditText::DoCommand( theCommand);
			break;
	}

}	/* DoCommand */



/* OVERRIDE */
void	CPStyleText::PerformEditCommand (long theCommand)
{
		Handle		textH;
		long		selStart;
		long		selEnd;
	
	
	Prepare();
	
	GetSelection( &selStart, &selEnd );
	
	/* Copy the selection range to the Clipboard for Cut or Copy */
	
	if (
		  (theCommand == cmdCut || theCommand == cmdCopy)	&&
		  (selStart != selEnd)
	   )
	{
		textH = CopyTextRange( selStart, selEnd );
		TRY
		{
			gClipboard->EmptyScrap();
			gClipboard->PutData( 'TEXT', textH );
			DisposHandle( textH );
		}
		CATCH
		{
			ForgetHandle( textH );
		}
		ENDTRY
	}
	
	/* Delete the selection range for Cut or Clear */
	
	if (
		  (theCommand == cmdCut || theCommand == cmdClear)	&&
		  (selStart < selEnd)
	   )
		DeleteTextRange( selStart, selEnd, kRedraw );
		
	/* Replace the current selection with the contents of the Clipboard for Paste */
	
	else if (
			  (theCommand == cmdPaste)	&&
			  gClipboard->GetData('TEXT', &textH)
			)
	{
			Boolean		beyondWordWrapEdge;
			long		txLength, txWidth;
			LongPt		selPt;
			
		/*
			Get existing point BEFORE we replace the selection.
			AFTER replacement, this will be different:
		*/
		GetCharPoint( selStart, &selPt );

		HLockHi( textH );
		TRY
		{
			txLength = gClipboard->DataSize( 'TEXT' );
			txWidth = TextWidth( *textH, 0, (short) txLength );
			ReplaceSelection( *textH, txLength );
			DisposHandle( textH );
		}
		CATCH
		{
			ForgetHandle( textH );
		}
		ENDTRY
		
		beyondWordWrapEdge = ( selPt.h + txWidth > itsDestRect.right );
		if (beyondWordWrapEdge)
		{
			/*
				ReplaceSelection only calls RefreshTextAfter ...
			*/
			
			Refresh();
		}
		
	}	/* cmdPaste */
	
	/* Ensure that the insertion caret is visible */
	
	ScrollToOffset( itsSelStart );
	
}	/* PerformEditCommand */



/* OVERRIDE: */
void	CPStyleText::CaretHook (const Rect *caretRect)
{
	/* Change for the inactive case just BECAUSE ... */
	
		PenState	prevPenState;
	
	
	GetPenState( &prevPenState );
	PenMode( patXor );
	
	/* Draw the caret in gray if the pane isn't active: */
	
	if ( !fReallyActive && fOutlineHilite )
	{
		PenPat( gray );
	}
	
	/* Draw the caret: */
	
	if ((itsTextFace & italic) && fUseItalicCaret)
	{
		PenSize( 1, 1 );
		MoveTo( caretRect->left, caretRect->bottom );
		LineTo( caretRect->left + (caretRect->bottom - caretRect->top) / 2,
				caretRect->top );
	}
	else
		PaintRect( caretRect );
	
	SetPenState( &prevPenState );
		
}	/* CaretHook */



/**** S C R O L L I N G   M E T H O D S ****/


/* OVERRIDE: */
void	CPStyleText::Scroll (long hDelta, long vDelta, Boolean redraw)
{
		long	hPixels, vPixels;
	
	
	hPixels = hDelta * hScale;
	vPixels = vDelta * vScale;

	OffsetLongRect( &itsViewRect, hPixels, vPixels );
		
	CPEditText::Scroll( hDelta, vDelta, redraw );
	
}	/* Scroll */



/* OVERRIDE: */
void	CPStyleText::ScrollToSelection (void)
{
	/*
		Change Chris' algorithm for computing the
		horizontal coord. of the new scroll position:
	*/
	
	
		long		startLine, endLine;
		short		hSpan, vSpan;
		LongPt		selPos, selPt;
		long		selStart, selEnd;
	
	
	/* Calculate the number of panorama units spanned by the frame: */
	
	GetFrameSpan( &hSpan, &vSpan );
	
	/* Determine the starting and ending lines for the selection: */
	
	GetSelection( &selStart, &selEnd );
	startLine = FindLine( selStart );
	endLine = (selStart == selEnd ? startLine : FindLine(selEnd));
	
	/*
		Calculate the new vertical scroll position.
		This position is close to the vertical middle
		of the window unless it's already visible:
	*/
	
	if (startLine >= position.v + vSpan)
	{
		selPos.v = startLine - (vSpan / 2) + 1;
		selPos.v = Max( selPos.v, 0 );
		selPos.v = Min( selPos.v, itsNumLines - vSpan );
	}
	else if (endLine < position.v)
	{
		selPos.v = endLine - (vSpan / 2) + 1;
		selPos.v = Max( selPos.v, 0 );
		selPos.v = Min( selPos.v, itsNumLines - vSpan );
	}
	else
	{
		/* Already visible: */
		
		selPos.v = position.v;
	}
	
	/*
		Calculate the new horizontal scroll position.
		When horizontal scrolling is permitted, this
		position is close to the horizontal middle
		of the window unless it's already visible or
		we've selected wrap-to-window-width.  In these
		last cases, do NOT scroll horizontally at all.
		If wrapping to window width, force the user to
		grow the window to get the selection in view:

	*/
	
	if (!scrollHoriz)
	{
		/*
			Class instance variable set to
			turn OFF horizontal scrolling:
		*/
		
		selPos.h = position.h;
	}
	else
	{
		/****   C H A N G E D   A L G O R I T H M   ****/
		
		if (lineWidth == -1)
		{
			/* wrap-to-window-width: */
			
			selPos.h = position.h;
		}
		else
		{
			GetCharPoint( endLine == startLine ? selEnd : (selStart + selEnd) / 2,
						  &selPt );
			selPos.h = selPt.h / hScale;
			if ( (selPos.h >= position.h) && (selPos.h <= position.h + hSpan) )
			{
				/* Already visible, so do NOT scroll horizontally: */
			
				selPos.h = position.h;
			}
			else
			{
				/*
					Scroll so the selection is close to
					the horizontal middle of the window:
				*/
			
				selPos.h = Max( selPos.h - hSpan / 2, 0 );
			}
			
		}	/* NOT wrapping to window width */
				
	}	/* scrollHoriz is TRUE */
	
	/* Scroll the pane if necessary: */
	
	if ((selPos.v != position.v) || (selPos.h != position.h))
		ScrollTo( &selPos, kRedraw );
		
}	/* ScrollToSelection */



/* OVERRIDE: */
void	CPStyleText::ScrollToOffset (long charOffset)
{
	/*
		Change Chris' algorithm for computing the
		horizontal coord. of the new scroll position:
	*/
	
	
		long		charLine;
		short		hSpan, vSpan;
		LongPt		selPos, selPt;
	
	
	/* Calculate the number of panorama units spanned by the frame: */
	
	GetFrameSpan( &hSpan, &vSpan );
	
	/* Calculate the new vertical scroll position: */
	
	charLine = FindLine( charOffset );
	if (charLine >= position.v + vSpan)
		selPos.v = charLine - vSpan + 1;
	else if (charLine < position.v)
		selPos.v = charLine;
	else
		selPos.v = position.v;
	
	/*
		Calculate the new horizontal scroll position.
		When horizontal scrolling is permitted, if
		we're already visible or we have selected
		wrap-to-window-width, do NOT scroll horizontally
		at all.  If wrapping to window width, force user
		to grow the window to get the offset in view:
	*/
	
	if (!scrollHoriz)
	{
		/*
			Class instance variable set to
			turn OFF horizontal scrolling:
		*/
		
		selPos.h = position.h;
	}
	else
	{
		/****   C H A N G E D   A L G O R I T H M   ****/
		
		if (lineWidth == -1)
		{
			/* wrap-to-window-width: */
			
			selPos.h = position.h;
		}
		else
		{
			GetCharPoint( charOffset, &selPt );
			selPos.h = selPt.h / hScale;
			if (selPos.h > position.h + hSpan)		/* NOT >= */
				selPos.h -= (hSpan * 3) / 4;
			else if (selPos.h < position.h)
				selPos.h -= hSpan / 4;
			else
				selPos.h = position.h;				/* Already visible. */
				
			selPos.h = Max(selPos.h, 0);
			
		}	/* NOT wrapping to window width */
		
	}	/* scrollHoriz is TRUE */
	
	/* Scroll the pane if necessary: */
	
	if ((selPos.v != position.v) || (selPos.h != position.h))
		ScrollTo( &selPos, kRedraw );
		
}	/* ScrollToOffset */



/**** P R I N T I N G   M E T H O D S ****/


/* OVERRIDE: */
void	CPStyleText::PrintPage (short pageNum, short pageWidth,
								short pageHeight, CPrinter *aPrinter)
{

	if (printClip == clipPAGE)
	{
		/*
			Expand viewRect to the size of a page.
			It will be restored by DonePrinting().
		*/
		
		itsViewRect.right = itsViewRect.left + pageWidth;
		
		if (wholeLines)
			itsViewRect.bottom = itsViewRect.top + vScale * (pageHeight / vScale);
		else
			itsViewRect.bottom = itsViewRect.top + pageHeight;
		
	}	/* clipPAGE */

	CPEditText::PrintPage( pageNum, pageWidth, pageHeight, aPrinter );

}	/* PrintPage */



/* OVERRIDE: */
void	CPStyleText::DonePrinting (void)
{

	CPEditText::DonePrinting();

	/*
		TCL 1.1:
		
		Call CalcTERects (CalcPERects) rather
		than setting the viewRect directly.
	*/
	
	CalcPERects();
	
	if (active)		HideSelection (FALSE /* show caret */, TRUE /* re-draw */ );

}	/* DonePrinting */



/**** I N T E R N A L   M E T H O D S ****/


/**** C O M M A N D   M E T H O D S ****/


/* OVERRIDE */
CTextStyleTask	*CPStyleText::MakeStyleTask (long styleCmd)
{
		CStylePStyleTask	*newTask = NULL;
		short				taskIndex;
	
	
	TRY
	{
		taskIndex = cFirstTaskIndex > 0 ? cFirstTaskIndex + undoFormatting : 0;
		newTask = new (CStylePStyleTask);
		
		newTask->IStylePStyleTask( this, styleCmd, taskIndex );
	}
	CATCH
	{
		ForgetObject( newTask );
	}
	ENDTRY;
	
	return	(newTask);
	
}	/* MakeStyleTask */



/**** D I S P L A Y   M E T H O D S ****/


/******************************************************************************
 CountRangeSPACEs

		Internal method to count space characters
		in the given range of text.
 ******************************************************************************/

	static long CountSPACEs (register Ptr textP, register long numChars)
	{
			register Byte	space	  = kSpace;
			register long	numSPACEs = 0;
	
	
		while (numChars--)
		{
			if (*textP++ == space)
				++numSPACEs;
		}
	
		return (numSPACEs);
	
	}	/* CountSPACEs */



long	CPStyleText::CountRangeSPACEs (long start, long end)
{
	/*
		Counts the number of spaces in the given range of text.
	*/
	
	
		Ptr		textP		= *itsTextHandle;
		long	length		= end - start;
#if qPEUseInsertionGap
		long	gapPosition	= itsGapPosition;
		long	gapLength	= itsGapLength;
#endif
		long	spaceCount;
	
	
	if (start >= end)
		spaceCount = 0;
#if qPEUseInsertionGap
	else if (end <= gapPosition)
		spaceCount = CountSPACEs( textP + start, length );
	else if (start >= gapPosition)
		spaceCount = CountSPACEs( textP + start + gapLength, length );
	else
	{
			long	lenBeforeGap = gapPosition - start;
			
		spaceCount = CountSPACEs( textP + start, lenBeforeGap );
		spaceCount += CountSPACEs( textP + start + gapLength + lenBeforeGap,
								   length - lenBeforeGap );
	}
#else
	else
		spaceCount = CountSPACEs( textP + start, length );
#endif
	
	return	(spaceCount);
	
}	/* CountRangeSPACEs */



/* OVERRIDE: */
void	CPStyleText::DrawLineRange (long startLine, long endLine,
									long startLineOffset, Boolean erase)
{
		Boolean			showInvisibles = fShowInvisibles;
		char			textHState;
		register char	tab;
		Handle			invisH;
		long			line;
		long			lineStart;
		long			lineEnd;
		long			firstChar;
		long			boundsHExtent;
		long			horizInset	   = HorizLeftInset();
#if qPEUseInsertionGap
		long			gapPosition	   = itsGapPosition;
		long			gapLength	   = itsGapLength;
#endif
		LongPt			textPt;
		LongRect		lr;
		Point			penPt;
		Point			startPt;
#if qPEUseInsertionGap
		Ptr				gapP;
#endif
		Rect			eraseRect;
		RgnHandle		clipRgn;
		register Ptr	textP;
		register short	numChars;
		register short	index;
		register short	tabWidth	   = itsTabWidth;
		ShortHandle		widthsH		   = NULL;
		short			width;
		short			*widthsP;
		short			fontAscent	   = itsFontAscent;
		short			lineHeight;
	
	
	/* Lock down the text Handle: */
	
	textHState = HGetState( itsTextHandle );
	HLock( itsTextHandle );

	/* If we are erasing, save the current clipping region: */
	
	if (erase)
	{
		GetClip( cSaveClipRgn );
		clipRgn = NewRgn();
	}
	
	/* Compute the vertical coordinate for the first line of text: */
	
	textPt.v = GetHeight( 0, startLine - 1 ) + fontAscent + VertTopInset();
	
	/* Draw the specified lines of text: */
	
	for (line = startLine; line <= endLine; ++line)
	{
		lineHeight = GetHeight( line, line );		/* See comments within Draw. */
		
		/* Compute the number of characters to draw: */
		
		lineStart = GetLineStart( line );
		lineEnd = GetLineEnd( line );
		
		widthsH = MeasureLineWidths( line );

		if ((line == startLine) && (startLineOffset > 0))
			textPt.h = (*widthsH)[startLineOffset] + horizInset;
		else
			textPt.h = horizInset;
			
		/* See comments within GetCharPoint: */
		
		if (itsAlignCmd == cmdAlignLeft)
			textPt.h += itsLeftMargin;
		else if (itsAlignCmd == cmdAlignRight)
			textPt.h += HorizPixelExtent() - (*widthsH)[GetLineLength(line)];
		else if (itsAlignCmd == cmdAlignCenter)
			textPt.h += itsLeftMargin + (
										   HorizPixelExtent() - itsLeftMargin -
										   (*widthsH)[GetLineLength(line)]
										) / 2;
		else
		{
			/*
				For full justify alignment, the point where the
				first text character is drawn is on the FAR left:
			*/
			textPt.h += itsLeftMargin;
		}
		
		ForgetHandle( widthsH );		/* We're finished with this dude!!! */

		/* Erase the background behind the text, if desired: */
		
		if (erase)
		{
			SetLongRect( &lr,
						 (line == startLine && startLineOffset > 0 ?
														  textPt.h : frame.left),
						 textPt.v - fontAscent,
						 frame.right,
						 textPt.v - fontAscent + lineHeight );			
			FrameToQDR( &lr, &eraseRect );
			
			/* Adjust the clipping region so italic characters aren't clipped: */

			RectRgn( clipRgn, &eraseRect );
			SectRgn( clipRgn, cSaveClipRgn, clipRgn );
			SetClip( clipRgn );
		}
		
		firstChar = ( line == startLine && startLineOffset > 0 ?
					  			   lineStart + startLineOffset :
					  			   lineStart );
		numChars = lineEnd - firstChar;
		
		/*
			Convert invisible characters to their
			visible counterparts if desired:
		*/
		if (showInvisibles)
		{
			invisH = CopyTextRange( firstChar, firstChar + numChars );
			numChars = ConvertInvisibles( invisH, numChars );
			HLock( invisH );
			textP = *invisH;
#if qPEUseInsertionGap
			gapP = NULL;
			gapPosition = gapLength = 0;
#endif
		}
		else
		{
#if qPEUseInsertionGap
			textP = *itsTextHandle;
			gapP = textP + gapPosition;
			textP += firstChar;
			if (firstChar > gapPosition)
				textP += gapLength;
#else
			textP = *itsTextHandle + firstChar;
#endif
		}
		
		/* Now draw the line of text: */
		
		FrameToQD( &textPt, &startPt );
		penPt = startPt;
		MoveTo( penPt.h, penPt.v );
		if (erase)		EraseRect( &eraseRect );
		
#if THINK_C
		asm
		{
			clr.w	index					; reset index
			move.b	#kTab,tab				; keep tab character in
											;   register for speed
		@Loop:
			cmp.w	numChars,index			; have we drawn all the text?
			bge.s	@Done					; branch if we have
#if qPEUseInsertionGap
			cmpa.l	gapP,textP				; are we at the gap?
			bne.s	@NotAtGap				; branch if not
			
		@AtGap:
			bsr.s	@DrawText				; else draw the text to the
											;   left of the gap
			adda.l	gapLength,textP			; and skip over the gap
		
		@NotAtGap:
#endif
			addq.w	#1,index				; else bump the index		
			cmp.b	(textP)+,tab			; is character from text a tab?
			bne.s	@Loop					; loop if not
			
		@IsTab:
			subq.w	#1,textP				; decrement ptr to text
			subq.w	#1,index				; decrement # of characters to draw
			bsr.s	@DrawText				; draw the text to the left of the tab
			
			movea.l	thePort,a0				; get current GrafPtr
			move.l	OFFSET(GrafPort,pnLoc)(a0),penPt ; get pen location
			
			tst.b	showInvisibles			; are we displaying invisible characters?
			beq.s	@NoInvis1				; branch if not
			move.w	#kInvisTab,-(a7)		; else push invisible tab character
			_DrawChar						; and draw it
			
		@NoInvis1:
			moveq	#0,d0					; clear out all of D0
			move.w	penPt.h,d0				; get horiz pen location
			sub.w	startPt.h,d0			; subtract starting location
			divu.w	tabWidth,d0				; divide by tab width
			addq.w	#1,d0					; add one to tab width
			mulu.w	tabWidth,d0				; get pixel offset
			add.w	startPt.h,d0			; add starting location
			
			move.w	d0,-(a7)				; push horiz coordinate
			move.w	penPt.v,-(a7)			; push vert coordinate
			_MoveTo							; move pen location
			
			addq.w	#1,textP				; bump ptr to text
			subq.w	#1,numChars				; subtract 1 from length of text
			bra.s	@Loop					; and loop
	
		@DrawText:
			tst.w	index					; any text to draw?
			beq.s	@NoText					; skip if not
			
			movea.l	textP,a0				; get ptr to next character
			suba.w	index,a0				; subtract index
			move.l	a0,-(a7)				; push ptr to text
			clr.w	-(a7)					; push offset into text
			move.w	index,-(a7)				; push length of text to draw
			_DrawText						; call DrawText
			
			sub.w	index,numChars			; subtract from length of text
			clr.w	index					; reset index to 0
			
		@NoText:
			rts
	
		@Done:
			bsr.s	@DrawText				; draw rest of text
		}
		
#else	/* not THINK_C */

	#if qPEUseInsertionGap
		AsmDrawLineRange( textP, numChars, tabWidth, gapP,
						  gapLength, startPt, showInvisibles );
	#else
		AsmDrawLineRange( textP, numChars, tabWidth, NULL,
						  0, startPt, showInvisibles );
	#endif

#endif
					
		/* Dispose the temporary handle for invisible characters: */
		
		if (showInvisibles)
			DisposHandle( invisH );
			
		/* Increment the text vertical coordinate: */
		
		textPt.v += lineHeight;
		
	}	/* for-loop */
	
	/*
		Restore the previous clipping region and
		the previous state of the text handle:
	*/
	if (erase)
	{
		SetClip( cSaveClipRgn );	
		DisposeRgn( clipRgn );
	}
	
	HSetState( itsTextHandle, textHState );

}	/* DrawLineRange */



/* OVERRIDE: */
ShortHandle		CPStyleText::MeasureTextWidths (long startPos, long endPos,
												short maxWidth)
{
		long	extraWidthPerSpace;
		
		
	if (itsAlignCmd == cmdJustify)
	{
			long			numSpaces, availableWidth;
			ShortHandle		compactWidthsH;
			
		numSpaces = CountRangeSPACEs( startPos, endPos );
		if (numSpaces > 0)
		{
			availableWidth = itsDestRect.right - itsDestRect.left - itsLeftMargin;
			if (availableWidth <= 0)
				extraWidthPerSpace = 0;
			else
			{
				/*
					Set "spExtra" field of "thePort" to zero in
					order to measure the UNexpanded widths.  Below,
					we'll set "spExtra" to its expanded value so
					we can return the expanded widths Handle:
				*/
				SpaceExtra( 0 );
				compactWidthsH = CPEditText::MeasureTextWidths( startPos, endPos,
																maxWidth );
				extraWidthPerSpace = (
										availableWidth -
										(*compactWidthsH)[endPos - startPos]
									
									 ) / numSpaces;
				extraWidthPerSpace = Max( extraWidthPerSpace, 0 );
			}
			
		}	/* numSpaces > 0 */
		
		else
			extraWidthPerSpace = 0;
			
	}	/* cmdJustify */
	
	else
		extraWidthPerSpace = 0;
		
	SpaceExtra( (Fixed) (extraWidthPerSpace << 16) );

	return	( CPEditText::MeasureTextWidths(startPos, endPos, maxWidth) );
	
}	/* MeasureTextWidths */



/* OVERRIDE: */
void	CPStyleText::HiliteTextRange (long startChar, long endChar)
{
		Boolean		isActive   = fReallyActive;
		long		startLine;
		long		endLine;
		LongPt		startPt;
		LongPt		endPt;
		LongRect	hiliteRect;
		PenState	penState;
		Rect		qdRect;
		RgnHandle	hiliteRgn;
		RgnHandle	rectRgn;
		short		hSpan;
		short		vSpan;
		short		fontAscent = itsFontAscent;
		short		lineHeight;
	
	
	if ( isActive || fOutlineHilite )
	{
			/* Outline the selection range if the pane is not active */
		
		if (!isActive)
		{
			hiliteRgn = NewRgn();
			rectRgn = NewRgn();
		}
		
			/* Get the starting and ending selection lines	*/
			/* and the number of lines spanned by the frame	*/
			
		startLine = FindLine( startChar );
		endLine = FindLine( endChar );
		GetFrameSpan( &hSpan, &vSpan );
		
			/* Take a quick exit if the selection is */
			/* not visible within the frame span	 */
		
		if (startLine >= position.v + vSpan || endLine < position.v)
			return;
		
			/* Adjust the starting and ending selection */
			/* lines if they are outside the frame		*/
		
		if (startLine < position.v)
		{
			startLine = position.v;
			startChar = GetLineStart(startLine);
		}
		
		if (endLine > position.v + vSpan)
		{
			endLine = position.v + vSpan;
			endChar = GetLineEnd( endLine );
		}
		
			/* Get the frame coordinates corresponding to the */
			/* start and end of the selection range			  */
			
		GetCharPoint( startChar, &startPt );
		GetCharPoint( endChar, &endPt );
		
			/* Adjust the horizontal coordinates if the either of the starting */
			/* or ending characters lies at the beginning of a line			   */
		
		if ( startChar == GetLineStart(startLine) )
			startPt.h = frame.left;
		if ( endChar == GetLineStart(endLine) )
			endPt.h = frame.left;
		
			/* Check for and handle a multiple-line selection range */
		
		if (startPt.v != endPt.v)
		{
				/* Highlight the first line of the selection range */
			
			lineHeight = GetHeight( startLine, startLine );
			SetLongRect( &hiliteRect,
						 startPt.h,
						 startPt.v - fontAscent,
						 frame.right,
						 startPt.v - fontAscent + lineHeight );
			FrameToQDR( &hiliteRect, &qdRect) ;
			
			if (isActive)
				HiliteHook( &qdRect );
			else
			{
				qdRect.left -= 1;
				RectRgn( rectRgn, &qdRect );
				UnionRgn( rectRgn, hiliteRgn, hiliteRgn );
			}
			
				/* Highlight the middle lines of the selection range */
			
			hiliteRect.left = frame.left;
			
			if (isActive)
			{
				FrameToQDR( &hiliteRect, &qdRect );
				
				while (++startLine < endLine)
				{
					lineHeight = GetHeight( startLine, startLine );
					qdRect.top += lineHeight;
					qdRect.bottom += lineHeight;
					HiliteHook( &qdRect );
				}
			}
			else
			{
				hiliteRect.top += lineHeight;
				hiliteRect.left -= 1;
				hiliteRect.bottom = endPt.v - fontAscent + 1;
				FrameToQDR( &hiliteRect, &qdRect );
				RectRgn( rectRgn, &qdRect );
				UnionRgn( rectRgn, hiliteRgn, hiliteRgn );
			}
			
			SetLongPt( &startPt, hiliteRect.left, endPt.v );
			
		}	/* startPt.v � endPt.v */
		
			/* Hilite the last part of the selection range */
		
		lineHeight = GetHeight( endLine, endLine );
		SetLongRect( &hiliteRect,
					 startPt.h,
					 endPt.v - fontAscent,
					 endPt.h,
					 endPt.v - fontAscent + lineHeight );
		FrameToQDR( &hiliteRect, &qdRect );
		
		if (isActive)
			HiliteHook( &qdRect );
		else
		{
			if (qdRect.right > qdRect.left + 1)
			{
				qdRect.bottom += 1;
				RectRgn( rectRgn, &qdRect );
				UnionRgn( rectRgn, hiliteRgn, hiliteRgn );
			}
			
				/* Intersect the highlight region with the pane frame */
			
			FrameToQDR( &frame, &qdRect );
			RectRgn( rectRgn, &qdRect );
			SectRgn( rectRgn, hiliteRgn, hiliteRgn );
			DisposeRgn( rectRgn ) ;
			
				/* Outline the highlight region */
			
			GetPenState( &penState );
			PenNormal();
			PenMode( patXor );
			FrameRgn( hiliteRgn );
			DisposeRgn( hiliteRgn );
			SetPenState( &penState );
			
		}	/* NOT active */
		
	}	/* if ( isActive || fOutlineHilite ) */

}	/* HiliteTextRange */



/* OVERRIDE: */
void	CPStyleText::CalcLineHeight (void)
{
		FontInfo		macFontInfo;
	
	
		/* Update itsLineHeight and itsFontAscent */ 
	
	GetMacFontInfo( &macFontInfo );
	itsLineHeight = macFontInfo.ascent + macFontInfo.descent + macFontInfo.leading;
	itsFontAscent = macFontInfo.ascent;
	itsMaxCharWidth = macFontInfo.widMax;
	
	if (itsSpacingCmd == cmd1HalfSpace)
	{
		itsLineHeight *= 3;
		itsLineHeight /= 2;
	}
	else if (itsSpacingCmd == cmdDoubleSpace)
		itsLineHeight *= 2;
	
	CalcTabWidth();
	
		/* Refresh the contents of the pane */
	
	Refresh();
	SetScales( macFontInfo.widMax, itsLineHeight );
	/*
		See comments within SetTextPtr method:
	*/
	SetWholeLines( wholeLines );
	AdjustBounds();
	Refresh();
	
}	/* CalcLineHeight */



/* OVERRIDE: */
void	CPStyleText::RefreshTextAfter (long afterPos, Boolean refreshOnlyLine)
{
		long		startLine, endLine;
		long		vertInset = VertTopInset();
		long		cumLineHt, endPointHt;
		LongRect	lr;
		Rect		r;
	
	
	Prepare();
	
		/* Redraw the text after the given offset on the line */
	
	startLine = FindLine( afterPos );
	if (refreshOnlyLine)
	{
		endLine = startLine;
	}
	else
	{
		/*
			Instituted for-loop because eventually
			I will effect styled text wherein each
			line can have a unique height:
		*/
		
		cumLineHt = 0;
		endPointHt = frame.bottom - vertInset;
		for (endLine = 0; endLine < itsNumLines; endLine++)
		{
			cumLineHt += GetHeight( endLine, endLine );		/* See Draw method. */
			if (cumLineHt >= endPointHt)
				break;
		}
		endLine = Min( endLine, itsNumLines - 1 );
	}
	
	DrawLineRange( startLine, endLine, afterPos - GetLineStart(startLine),
				   kEraseText );
		
		/* Erase the area below the last text line, if necessary */
	
	if (!refreshOnlyLine)
	{
		SetLongRect( &lr,
					 frame.left, (endLine + 1) * itsLineHeight + vertInset,
					 frame.right, frame.bottom );
		
		if (lr.bottom > lr.top)
		{
			FrameToQDR( &lr, &r );
			EraseRect( &r );
		}
	}
	
}	/* RefreshTextAfter */



/**** W O R D   W R A P   M E T H O D S ****/


/* OVERRIDE: */
void	CPStyleText::CalcLineStarts (void)
{
	/*
		First call inherited method to calculate the pristine
		line count and line starts array.  However, instead of
		just calling Chris' method in one shot, expand it here,
		but withOUT its final call to AdjustBounds().  The reason
		is that WrapLineStarts() below may change itsNumLines
		which directly affects the new CPanorama bounds.
		
		As long as we're re-presenting it here, modify it for
		speed enhancement.  See comments below.
	*/
	
		register Byte		cr			 = kReturn;
		register long		charPos;
		register long		textLength	 = itsTextLength;
		register long		*lineStarts;
		register long		numLines;
#if qPEUseInsertionGap
		long				gapLength	 = itsGapLength;
		long				gapPosition	 = itsGapPosition;
		register Ptr		gapP;
#endif
		register Ptr		textP;
	
	
	/*
		For speed enhancement, allocate the line
		starts array to the maximum size possible.
		Re-sized later after the lines are counted.
	*/
	
	ResizeHandleCanFail( (Handle)itsLineStarts, MAX_LINES * sizeof(long) );
	if ( MemError() )
		FailOSErr( FileTooBig );
	
	/* Count the lines and build the line starts array: */
	
	lineStarts = *itsLineStarts + 1;
	textP = *itsTextHandle;
#if qPEUseInsertionGap
	gapP = textP + gapPosition;
#endif
	charPos = 0;
	numLines = 1;
	
	while (++charPos <= textLength)
	{
#if qPEUseInsertionGap
		if (textP == gapP)
			textP += gapLength;
#endif
		if (*textP++ == cr)
		{
			*lineStarts++ = charPos;
			AddLine( numLines - 1 );
			++numLines;
		}
	}
	
	/* Resize Handle as promised and then update line count: */

	SetHandleSize( (Handle)itsLineStarts, numLines * sizeof(long) );
	itsNumLines = numLines;
	
	/*****   F O L D   I N   W O R D   W R A P   B Y   *****/
	/*****   S C A N N I N G   N E W   L I N E S       *****/
	
	WrapLineStarts( 0L, numLines - 1, kWillBeRefreshed );
	
	/*****   T H E N   A D J U S T   T H E   B O U N D S   *****/

	/*
		See comments within SetTextPtr method:
	*/
	SetWholeLines( wholeLines );
	AdjustBounds();
		
}	/* CalcLineStarts */



/* OVERRIDE: */
void	CPStyleText::RecalcLineStarts (void)
{
	
	/*
		Called by ResizeFrame and DoWordWrap,
		this method folds in word-wrap by
		scanning all the lines in the text
		AND then re-adjusts the bounds:
	*/
	
	WrapLineStarts( 0L, itsNumLines - 1, kWillBeRefreshed );
	
	/*
		See comments within SetTextPtr method:
	*/
	SetWholeLines( wholeLines );
	AdjustBounds();
	
}	/* RecalcLineStarts */



/* OVERRIDE: */
void	CPStyleText::WrapLineStarts (register long startLine, register long endLine,
									 Boolean willBeRefreshed)
{
	/*
		Called by my CalcLineStarts and RecalcLineStarts
		to change line count and line starts array as
		needed to accomodate word-wrap.  Note that we
		may have to scan or break a line several times
		BEFORE we proceed to the next one.
		
		( Used LOTS!!! of registers for speed )
	*/
	
		LongHandle				saveLineStarts;
	 // char					savedLSHState;
		register long			*lineStarts;
		Boolean					redrawWrappedLines = FALSE;
		register long			numLines		   = itsNumLines;
		register long			lineNbr;
		tCharBuf				theChar;
		Byte					cr				   = kReturn;
		long					textLength		   = itsTextLength;
		long					startUnbrokenLineNbr, endUnbrokenLineNbr;
		long					deltaParts;
		long					startUnbrokenLinePos, endUnbrokenLinePos;
		long					startUnbrokenWordLineNbr;
		register long			brokenLineNbr;
	 // Boolean					wordWrapThisLine, breakThisLineAgain;
		ShortHandle				widthsH;
		register short			*widthsP;
		short					totalSpaceForText;
		register short			spaceAvailable;
		long					wholeTextBreak;
		register short			lineBreak;
		register short			unbrokenLineLen, lineSegmentLength;
		short					endTextPt, widthTextRemaining;
		long					wordStart, wordEnd;
		long					nextLine;
		register short			offset;
		register short			width /*, lastWidth */;
		register short			*breakWidthsP;
	 // long					numLinesDelta;
	 // Boolean					savedAlloc;
		Boolean					justify,
								tooBig	  = FALSE;

		#define DOTIMING		TRUE
#if DOTIMING
		long					startTime = TickCount(),
								endTime, elapsedTime;
#endif

	
	Prepare();
		
	totalSpaceForText = itsDestRect.right - itsDestRect.left;
	
	/*
		Compute the available remaining space for text.
		
		This statement would have to appear after the
		"while (breakLineAgain)" statement if we had to
		adjust "spaceAvailable" for different text
		alignments.  See comments below about alignments.
		
		Note that this remaining space appears AFTER the
		left margin.  Therefore, since totalSpaceForText
		includes the left margin, this margin must be
		subtracted here:
	*/
	spaceAvailable = totalSpaceForText - itsLeftMargin;
	
	if (spaceAvailable <= 0)	return;		/* CYA !!! */
		
	/*
		For speed enhancement, allocate the line starts
		array to the maximum size possible so we don't
		have to call SetHandleSize as we go.  Re-sized
		just once after ALL the lines are wrapped.
		
		In addition, I save a copy of the original
		line starts Handle just in case we need to go
		beyond MAX_LINES as we're wrapping.
		
		I also save one de-reference by doing one at
		the beginning rather than:
		
			(*itsLineStarts)[...]
		
		everytime.
	*/
	
	/*
		The original itsLineStarts is NOT changed,
		but saveLineStarts is changed in place:
	*/
	
	saveLineStarts = itsLineStarts;
	if ( HandToHand((Handle*) &saveLineStarts) )
		FailOSErr( FileTooBig );
	ResizeHandleCanFail( (Handle)saveLineStarts, MAX_LINES * sizeof(long) );
	if ( MemError() )
		FailOSErr( FileTooBig );
 /*
	Gonna eventually dispose of the Handle's copy ...

	savedLSHState = HGetState( (Handle)saveLineStarts );
 */
	HLock( (Handle)saveLineStarts );
	lineStarts = *saveLineStarts;
	
	for (lineNbr = startLine; lineNbr <= endLine; ++lineNbr)
	{
		if (tooBig)
		{
			/* Very large file required a wrapped line count > MAX_LINES: */
			
			break;		/* out of for-loop */
		}
		
		/*
			First, reconstitute lines to UNbroken lines ...
			
			OUTER while-loop gets the start of the UNbroken
			line and the INNER while-loop gets the end.
		*/
		
		brokenLineNbr = lineNbr;
		
		while	(brokenLineNbr >= 0)
		{
			/* Not possible for line number being > last line: */

			startUnbrokenLinePos = ( /* brokenLineNbr >= numLines ? textLength : */
						 						lineStarts[brokenLineNbr] );
			GetCharBefore( &startUnbrokenLinePos, theChar );
			if (
				  Length(theChar) == 0	/* 1st line				  */	||
				  theChar[1]	  == cr	/* start of unbroken line */
			   )
			{
				startUnbrokenLineNbr = brokenLineNbr;
				brokenLineNbr = lineNbr;
				while	(/* brokenLineNbr <= numLines - 1 */ TRUE)
				{
					endUnbrokenLinePos = ( brokenLineNbr == numLines - 1 ?
										   textLength :
										   lineStarts[brokenLineNbr + 1] - 1 );
					GetCharAfter( &endUnbrokenLinePos, theChar );
					if (
						  Length(theChar) == 0	/* last line			*/	||
						  theChar[1]	  == cr	/* end of unbroken line */
					   )
					{
						endUnbrokenLineNbr = brokenLineNbr;
						
						break;	/* out of INNER while-loop */
					}
					
					++brokenLineNbr;
					
				}	/* inner while-loop */

				break;	/* out of OUTER while-loop */
				
			}	/* reached the start of the UNbroken line */
			
			--brokenLineNbr;

		}	/* outer while-loop */
		
		if ( (deltaParts = endUnbrokenLineNbr - startUnbrokenLineNbr + 1) > 1 )
		{
			/*
				Line is split due to word-wrap, so first
				UNwrap the line before we REwrap it:
			*/
			
			BlockMove( lineStarts + startUnbrokenLineNbr + deltaParts,
					   lineStarts + startUnbrokenLineNbr + 1,
					   (numLines - endUnbrokenLineNbr - 1) * sizeof(long) );
			numLines -= deltaParts - 1;
		 // SetHandleSize( (Handle)itsLineStarts, numLines * sizeof(long) );

			/* Receed for-loop variables: */
			
			if (lineNbr == startLine)	startLine = startUnbrokenLineNbr;
			lineNbr	  = startUnbrokenLineNbr;
			if (endLine > endUnbrokenLineNbr)	endLine -= deltaParts - 1;
			else								endLine = startUnbrokenLineNbr;
			
		}	/* line is split due to word-wrap */
			
		/****							 ****/
		/****   N O W,   R E - W R A P   ****/
		/****							 ****/
		
		/* For SUPER long words: */
		startUnbrokenWordLineNbr = startUnbrokenLineNbr;
	
		/* startUnbrokenLinePos = GetLineStart( lineNbr ); */
		startUnbrokenLinePos = (
								  lineNbr >= numLines ? textLength :
								  						lineStarts[lineNbr]
							   );
		/* endUnbrokenLinePos = GetLineEnd( lineNbr ); */
		endUnbrokenLinePos = (
								lineNbr >= numLines - 1 ? textLength :
														  lineStarts[lineNbr + 1]
							 );
		
		/*
			Before we retrieve the widths Handle, see if we have
			selected full text justification.  If we have, then
			set a flag and also change the instance variable =
			"itsAlignCmd" to anything OTHER THAN "cmdJustify".
			Because we are word-wrapping, we need to retrieve
			an UNexpanded widths Handle to determine if we
			NEED to word-wrap.  As soon as we retieve the
			UNexpanded widths Handle, we immediately reset
			"itsAlignCmd" for drawing after leaving WrapLineStarts.
			
			At the very end of this method, we check to see if
			we need to update the text.  If we have selected
			full text justification, we ALWAYS need to update.
		*/
		
		if (itsAlignCmd == cmdJustify)
		{
			justify = TRUE;
			itsAlignCmd = cmdAlignLeft;
		}
		else
			justify = FALSE;
		widthsH = MeasureTextWidths( startUnbrokenLinePos, endUnbrokenLinePos,
									 MAXINT );
		if (justify)	itsAlignCmd = cmdJustify;
		
		HLock( (Handle)widthsH );		/* Returned Handle is UNlocked: */
		widthsP = *widthsH;
		
		unbrokenLineLen = endUnbrokenLinePos - startUnbrokenLinePos;
		
		lineBreak = 0;				/* A fresh line has NO line breaks. */
		
	 // breakThisLineAgain = TRUE;
	 
		while (TRUE /* breakThisLineAgain */)
		{
			if ( numLines > MAX_LINES )		/* CYA !!! */
			{
				tooBig = TRUE;
				
				break;			/* out of while-loop */
			}

			/* See GetCharPoint ... */
			
			widthTextRemaining = widthsP[unbrokenLineLen] - widthsP[lineBreak];
			
			/*
				Below we compare endTextPt with totalSpaceForText.
				Since totalSpaceForText includes the left margin
				so must endTextPt:
			*/
			endTextPt = itsLeftMargin + widthTextRemaining;
			
		 /*
			DrawLineRange addresses text alignments,
			so pay attention here just to raw widths:
			
			if (itsAlignCmd == cmdAlignLeft)
				;
			else if (itsAlignCmd == cmdAlignRight)
				endTextPt += HorizPixelExtent() - itsLeftMargin - widthTextRemaining;
			else if (itsAlignCmd == cmdAlignCenter)
				endTextPt += (
								HorizPixelExtent() - itsLeftMargin -
								  widthTextRemaining
							 ) / 2;
			else	// cmdJustify
			{
				;
			}
		 */
		 	
			if (endTextPt > totalSpaceForText)
			{
				/*****   W E   N E E D   T O   B R E A K   T H E   L I N E   *****/
				
			 /*
				See above comment:
					
				if (itsAlignCmd == cmdAlignLeft)
					;
				else if (itsAlignCmd == cmdAlignRight)
					spaceAvailable -= HorizPixelExtent() - itsLeftMargin -
										widthTextRemaining;
				else if (itsAlignCmd == cmdAlignCenter)
					spaceAvailable -= (
										HorizPixelExtent() - itsLeftMargin -
										  widthTextRemaining
									  ) / 2;
				else	// cmdJustify
				{
					;
				}
			 */
					
				offset = 0;
			 /* lastWidth = 0;  --  see below */
				lineSegmentLength = (
										lineNbr < numLines - 1 ?
												lineStarts[lineNbr + 1] :
												textLength
									) - lineStarts[lineNbr];
				breakWidthsP = widthsP + lineBreak;
					
				while (
						(offset < lineSegmentLength)	&&
						(
						  (width = *++breakWidthsP - widthsP[lineBreak]) <
						  spaceAvailable
						)
					  )
				{
				 /* lastWidth = width;  --  see below */
					++offset;
				}
					
			 /*
				GetCharOffset bumps the offset to the character's right
				if the available width intersects the char.  Do NOT do
				this here, but rather keep the offset at the char's left:

				if (width > lastWidth)
					++offset;

				if (offset > lineSegmentLength)
					offset = lineSegmentLength;
			 */
				
				if (offset == 0)
				{
					/*
						SUPER-DUPER large font size, e.g. 72 points,
						resulting in a single letter larger than
						the available width:
					*/
					
					offset = 1;
				}
				
				wholeTextBreak = lineStarts[lineNbr] + offset;
				
				wordStart = WordBreakHook( wholeTextBreak, kBreakLeft );
				if (wordStart == lineStarts[startUnbrokenWordLineNbr])
				{
					/*
						Breaking a SUPER long word, that is, ONE
						word spans the ENTIRE line.  Therefore,
						break this super long word close to the
						right border of the available width:
					*/
					
				 /* wholeTextBreak = lineStarts[lineNbr] + offset; */

				}
				else if (wordStart == lineStarts[lineNbr])
				{
					/*
						Still breaking a SUPER long word, but
						this word is on a "sub-line" component
						of a longer word-wrapped line:
					*/
					
					startUnbrokenWordLineNbr = lineNbr;

				}
				else
				{
					wholeTextBreak = wordStart;
				}
				
				lineBreak = wholeTextBreak - startUnbrokenLinePos;

				/*
					Find the line corresponding to where we're breaking the
					text and then start the adjustment with the NEXT one.
					Also, increment the outer while-loop counter to
					correspond to adding the one line.
				
					This entire algorithm looks VERY similar to that in
					Chris' AdjustLineStarts method, but without the added
					overhead in order to enhance speed:
				*/
	
				nextLine = ++lineNbr;
				++endLine;

			 // numLinesDelta = 1L;
			 // lineNbr += numLinesDelta;

				/*
					Insert new entries in the line starts
					array starting at startLine, but withOUT
					resizing as explained above:
				*/
		
			 // savedAlloc = SetAllocation( kAllocCantFail );
			 // SetHandleSize( (Handle) itsLineStarts,
			 // 			   (numLines + 1 /* numLinesDelta */) * sizeof(long) );
			 // (void) SetAllocation( savedAlloc );
			 // if ( MemError() )		FailOSErr( FileTooBig );
		
				if (nextLine < numLines)
					BlockMove( lineStarts + nextLine,
							   lineStarts + nextLine + 1 /* numLinesDelta */,
							   (numLines - nextLine) * sizeof(long) );
			 // numLines += numLinesDelta;
				++numLines;
		
				/* Insert value for the new entry */
		
				lineStarts[nextLine] = wholeTextBreak;
				
			 // wordWrapThisLine = TRUE;
			 
				redrawWrappedLines = TRUE;
			
			}	/* endTextPt > totalSpaceForText */
			
			else
			{
			 // wordWrapThisLine = FALSE;
			 
				break;						/* Finished breaking the line */
			}
			
		 // breakThisLineAgain = wordWrapThisLine;
										
		}	/* while (breakThisLineAgain) */
		
		DisposHandle( (Handle)widthsH );	/* We're finally finished with it !!! */

	}	/* for-loop */
	
	if (tooBig)
	{
		/*
			A very large file size required a wrapped line
			count > MAX_LINES.  Therefore, just dispose of
			the Handle copy and leave itsLineStarts and
			itsNumLines in their pristine state.
			
			Yes, the line starts and line count won't be
			altered.  However, if this excessive line count
			was caused by too closely separated margins,
			for example, then the UNwrapped lines will still
			be drawn between these close margins and perhaps
			not erasing where the text WAS.  Therefore, we
			need to ensure that the whole CPane gets drawn.
		*/
		
	 // HSetState( (Handle)saveLineStarts, savedLSHState );
		DisposeHandle( (Handle)saveLineStarts );

		Refresh();
		
		FailOSErr( FileTooBig );
	}
	
	else
	{
		/* Update instance variable and resize Handle as promised: */
	
		itsNumLines = numLines;
		SetHandleSize( (Handle)itsLineStarts, numLines * sizeof(long) );
		BlockMove( *(Handle)saveLineStarts, *(Handle)itsLineStarts,
				   numLines * sizeof(long) );

		/* Contents now moved to instance variable, so dump the copy: */
		
		DisposeHandle( (Handle)saveLineStarts );
	}
		
#if DOTIMING
	endTime = TickCount();
	elapsedTime = endTime - startTime;
#endif

	if ( !willBeRefreshed || redrawWrappedLines || justify )
	{
		/*
			Redraw immediately rather than calling
			Refresh, but ONLY the visible text lines.
			Note that we canNOT simply call the Draw
			method because it passes kDontEraseText
			to DrawLineRange.
			
			Calling RefreshTextAfter not only satisfies
			the necessity of passing kEraseText to
			DrawLineRange, but it also erases the area
			below the last line if required.  This
			erasure is necessary, for example, if
			deleting the last line.
		*/
			
			Boolean		eraseAreaBelowLastLine = FALSE;
			long		startPos			   = (*itsLineStarts)[startLine];
			
		RefreshTextAfter( startPos, eraseAreaBelowLastLine );
	}
	
}	/* WrapLineStarts */



/* OVERRIDE: */
void	CPStyleText::AdjustLineStarts (long startChar, register long numCharsDelta,
									   long numLinesDelta )
{
	/*
		As with CalcLineStarts, instead of just
		calling the inherited method, duplicate
		it withOUT its final call to AdjustBounds().
		First, call WrapLineStarts() for each case
		of numLinesDelta.  THEN, call AdjustBounds().
		
		Note that within the various methods that
		call AdjustLineStarts, the selection is NOT
		adjusted until AFTER AdjustLineStarts.
		However, "itsTextLength" is adjusted BEFORE.
	*/
	
		register Byte		cr = kReturn;
		register long		*lineStarts;
		register long		count;
		long				startLine, nextLine;
		long				startLineCount;
		register Ptr		textP;
#if qPEUseInsertionGap
		register Ptr		gapP;
#endif
	
	
		/* Starting line count */
		
	startLineCount = itsNumLines;
	
		/*
			Find the line that follows
			the one corresponding to the
			starting character offset
		*/
	
	startLine = FindLine( startChar );
	nextLine = startLine + 1;
	
		/* Adjust the line starts array depending */
		/* on the change in the number of lines	  */
	
	if (numLinesDelta == 0)
	{
		
			/*
				Bump the values in the line starts array, starting at nextLine.
				Note that neither Chris nor I test for numCharsDelta = 0.  This
				number is either > 0 for inserting or < 0 for deleting:
			*/
		
		lineStarts = *itsLineStarts + nextLine;
		count = itsNumLines - startLine;
		while (--count > 0)
			*lineStarts++ += numCharsDelta;
			
			/*
				Even if deleting characters, re-wrapping of lines MAY be needed:
			*/

		WrapLineStarts( startLine, startLine, kWontBeRefreshed );
		
	}	/* numLinesDelta = 0 */
	
	else if (numLinesDelta > 0)
	{
		/* "numCharsDelta" MUST be > 0 if we're adding lines */
		
		
			Boolean		savedAlloc;
#if qPEUseInsertionGap
			long		gapPosition	= itsGapPosition;
			long		gapLength	= itsGapLength;
#endif
		
		
			/* Insert new entries in the line starts array starting at nextLine  */
		
		savedAlloc = SetAllocation( kAllocCantFail );
		SetHandleSize( (Handle) itsLineStarts,
					   (itsNumLines + numLinesDelta) * sizeof(long) );
		(void) SetAllocation( savedAlloc );
		if ( MemError() )		FailOSErr( FileTooBig );
		
		if (nextLine < itsNumLines)
			BlockMove( *itsLineStarts + nextLine,
					   *itsLineStarts + nextLine + numLinesDelta,
					   (itsNumLines - nextLine) * sizeof(long) );
		itsNumLines += numLinesDelta;
		
			/* Calculate values for the new entries */
		
		lineStarts = *itsLineStarts + nextLine;
		
		textP = *itsTextHandle;
#if qPEUseInsertionGap
		gapP = textP + gapPosition;
#endif
		textP += startChar;
#if qPEUseInsertionGap
		if (startChar >= gapPosition)
			textP += gapLength;
#endif
					
			/*
				This "while-loop" does NOT execute if "numCharsDelta"
				is zero or negative.  However, "numCharsDelta" MUST
				be > 0 if we're adding lines.
			*/
		
		count = numCharsDelta;
		while (--count >= 0)
		{
#if qPEUseInsertionGap
			if (textP == gapP)
				textP += gapLength;
#endif
			if (*textP++ == cr)
				*lineStarts++ = startChar + (numCharsDelta - count);
		}
		
			/* Then, adjust the values of the remaining entries */
		
		count = itsNumLines - (nextLine + numLinesDelta);
		while (--count >= 0)
			*lineStarts++ += numCharsDelta;
		
			/*
				Finally, wrap the inserted lines.  Note that
				we may be adding alot of characters to startLine
				BEFORE the first <CR> is encountered, which
				is why we begin with startLine.
			*/
			
		WrapLineStarts( startLine, startLine + numLinesDelta, kWontBeRefreshed );
		
	}	/* numLinesDelta > 0 */
	
	else	/* numLinesDelta < 0 */
	{
			/*
				Delete entries from the line starts array
				starting at nextLine.  "numCharsDelta"
				MUST be < 0 if we're deleting lines.
			*/
		
		itsNumLines += numLinesDelta;
		if (nextLine < itsNumLines)
			BlockMove( *itsLineStarts + nextLine + (-numLinesDelta),
					   *itsLineStarts + nextLine,
					   (itsNumLines - nextLine) * sizeof(long) );
		SetHandleSize( (Handle) itsLineStarts, itsNumLines * sizeof(long) );
		
			/* Bump the values in the remaining entries */
		
		lineStarts = *itsLineStarts + nextLine;
		count = itsNumLines - startLine;
		while (--count > 0)
			*lineStarts++ += numCharsDelta;

			/*
				Then, wrap the line which housed the start of the deletion:
			*/
			
		WrapLineStarts( startLine, startLine, kWontBeRefreshed );
		
	}	/* numLinesDelta < 0 */
	
		/* Adjust the bounds rectangle if the number of lines changed */
	
	if (itsNumLines != startLineCount)
	{
		/*
			See comments within SetTextPtr method:
		*/
		SetWholeLines( wholeLines );
		AdjustBounds();
	}
	
}	/* AdjustLineStarts */



/* OVERRIDE: */
void	CPStyleText::CalcPERects (void)
{
	/*
		Accomodates word wrap:
		
		Called by ResizeFrame, DoWordWrap and
		DonePrinting which call ForceNextPrepare().
	*/
	
	itsViewRect = aperture;
	
	if (fWordWrap)
	{
		/*
			If lineWidth = -1, user has
			selected wrapping to the Window:

			( see SetWordWrap method )
		*/
	
		itsDestRect.right = itsDestRect.left +
										   ( lineWidth == -1 ? width : lineWidth );
	}
	else
	{
		itsDestRect.right = itsDestRect.left +
									itsRightMargin /* = lineWidth from SetWordWrap */;
	}

}	/* CalcPERects */



/* OVERRIDE: */
void	CPStyleText::AdjustBounds (void)
{
	/* Overridden to accomodate word wrap: */
			
		long		widthDestRect;
		LongRect	newBounds;
	
			
	widthDestRect = itsDestRect.right - itsDestRect.left;
	newBounds.top = newBounds.left = 0;
	newBounds.bottom = GetNumLines();
	if ( (lineWidth != -1) && (width < widthDestRect) )
	{
		/*
			NOT wrapping to window width and window
			narrower than bounds.  This adaption is
			required to force the activation of the
			horizontal Scroll Bar as close as possible
			to the specified line width:
		*/
		
		newBounds.right = (widthDestRect - 1) / hScale + 1;
	}
	else
		newBounds.right = widthDestRect / hScale;
		
	SetBounds( &newBounds );

}	/* AdjustBounds */



/* OVERRIDE: */
void	CPStyleText::DrawCaret (void)
{
	/*
		Modified Chris' DrawCaret method for drawing the caret
		at the end of a "sub-line" of a word-wrapped line.
		Chris' DrawCaret method is declared NON-virtual within
		his "CPEditText.h" interface file.  So the only simple!
		way I can override it for both THINK C and Symantec C++
		is to declare Chris' DrawCaret a virtual method within
		that interface file.
	*/

			
	if (fEndWrapLine)
	{
		/*
			We WERE or ARE at the end of a
			"sub-line" of a word-wrapped line:
		*/
	
			LongPt		caretPt;
			LongRect	caretRect;
			Rect		qdRect;
			tCharBuf	breakChar;
			long		beforeBreakPos;
			short		delta;
			
		
		if (itsSelStart == itsSelEnd)
		{			
			beforeBreakPos = itsSelStart - 1;
			GetCharPoint( beforeBreakPos, &caretPt );
			GetCharAfter( &beforeBreakPos, breakChar );
			delta = CharWidth( breakChar[1] );
			SetLongRect( &caretRect,
						 caretPt.h - 1 + delta,
						 caretPt.v - itsFontAscent,
						 caretPt.h + delta,
						 caretPt.v - itsFontAscent + itsLineHeight );
			FrameToQDR( &caretRect, &qdRect );
			CaretHook( &qdRect );
		
			/*
				Once you've erased the old caret, you're done with
				it until you click somewhere else or press a key:
			*/
		
			fOldEndWrapLineCaret = FALSE;
			
		}	/* itsSelStart = itsSelEnd */
		
	}	/* special caret placement */
	
	else
		CPEditText::DrawCaret();

}	/* DrawCaret */



/**** C O N S T R U C T I O N / D E S T R U C T I O N   M E T H O D S ****/


void	CPStyleScrollPane::IPStyleScrollPane (CView *anEnclosure,
											  CBureaucrat *aSupervisor,
											  short aWidth, short aHeight,
											  short aHEncl, short aVEncl,
											  SizingOption aHSizing,
											  SizingOption aVSizing,
											  Boolean hasHoriz, Boolean hasVert,
											  Boolean hasSizeBox)
{

	CPane::IPane( anEnclosure, aSupervisor, aWidth, aHeight,
				  aHEncl, aVEncl, aHSizing, aVSizing );
	
	hStep = vStep = 1;
	hOverlap = vOverlap = 1;
	hUnit = vUnit = 1;
	
	IPStyleScrollPaneX( hasHoriz, hasVert, hasSizeBox );
	
	hScale = vScale = 1;
	
}	/* IPStyleScrollPane */



void	CPStyleScrollPane::IPStyleScrollPaneX (Boolean hasHoriz, Boolean hasVert,
											   Boolean hasSizeBox)
{
		short				shrinkage;
		CPStyleScrollBar	*theHorizSBar, *theVertSBar;
		
	
	wantsClicks = TRUE;
	
	shrinkage = (hasSizeBox || (hasHoriz && hasVert)) ? SBARSIZE1 : 0;
	
	if (hasHoriz)
	{
		theHorizSBar = new (CPStyleScrollBar);
		theHorizSBar->IPStyleScrollBar( this, this, HORIZONTAL, width - shrinkage,
										frame.left, frame.bottom - SBARSIZE );
		theHorizSBar->SetActionProc( SBarActionProc );
		theHorizSBar->SetThumbFunc( SBarThumbFunc );
		;
		itsHorizSBar = theHorizSBar;
	}
	else
	{
		itsHorizSBar = NULL;
	}
	
	if (hasVert)
	{
		theVertSBar = new (CPStyleScrollBar);
		theVertSBar->IPStyleScrollBar( this, this, VERTICAL, height - shrinkage,
									   frame.right - SBARSIZE, frame.top );
		theVertSBar->SetActionProc( SBarActionProc );
		theVertSBar->SetThumbFunc( SBarThumbFunc );
		;
		itsVertSBar = theVertSBar;
	}
	else
	{
		itsVertSBar = NULL;
	}
	
	if (hasSizeBox)
	{
		itsSizeBox = new (CSizeBox);
		itsSizeBox->ISizeBox( this, this );
	}
	else
	{
		itsSizeBox = NULL;
	}
	
	itsPanorama = NULL;

}	/* IPStyleScrollPaneX */



void	CPStyleScrollBar::IPStyleScrollBar (CView *anEnclosure,
											CBureaucrat *aSupervisor,
											Orientation anOrientation, short aLength,
											short aHEncl, short aVEncl)
{

	CScrollBar::IScrollBar( anEnclosure, aSupervisor, anOrientation,
							aLength, aHEncl, aVEncl );
	
}	/* IPStyleScrollBar */



/**** M O U S E   A N D   K E Y S T R O K E   M E T H O D S ****/


/* OVERRIDE */
void	CPStyleScrollBar::DoClick (Point hitPt, short modifierKeys, long when)
{
	/*
		Added another autoscroll capability effected when the user presses
		the <Option> key while initially dragging the Thumb in either scroll
		bar.  I say "initially" because once the user presses the <Option>
		key while clicking on the Thumb, s/he can release the key and continue
		the autoscrolling.  It DOES matter, however, if the mouse wanders far
		from the Scroll Bar while it's still pressed because then the
		autoscrolling ceases.
		
		The user would implement Scroll Bar <Option> scrolling if s/he wished
		to scroll VERY fast through the text, with the speed limited only by
		the speed of the mouse movement.  If s/he wished to scroll only a
		line at a time, then the user would <Option> click directly in the
		text pane itself and NOT in the Scroll Bar.
		
		< See Andy Hertzfeld's "sbarcdev.a" source on the Developer's CD. >
	*/
	
		#define SLOPSIZE	SBARSIZE
		#define ARROWSIZE	SBARSIZE

		typedef long		(*ControlDefFuncPtr) (short varCode,
												  ControlHandle theControl,
												  short	message,
												  long param);
		ControlDefFuncPtr	ctlDefFunc;
		CPStyleScrollPane	*theScrollPane;
		Boolean				optionKeyDown = ( (modifierKeys & optionKey) != 0 );
		register short		whichPart;
		Point				origThumbPt, movedThumbPt;
		LongPt				origThumbPoint, movedThumbPoint;
		LongRect			slopRect;
		Rect				slopR, scrollBarRect, thumbBox;
		RgnHandle			thumbRgn;
		short				topOfThumbToArrow, inThumbOffset;
		long				ctlDefLong;
		short				varCode;
		long				longVH;
		short				origThumbValue, movedThumbValue;
		
	
	/*
		The passed hitPt is in frame coordinates for a CScrollBar since
		"usingLongCoord" = FALSE for this class.  Therefore, I need to
		call FrameToWind to convert the passed Point to one local to the
		window so TestControl works right.
		
		Note that calling QDToLongPt is the same as calling QDToFrame
		since "usingLongCoord" = FALSE.
	*/
	QDToLongPt( hitPt, &origThumbPoint );
	FrameToWind( &origThumbPoint, &origThumbPt );

	whichPart = TestControl( macControl, origThumbPt );
	
	if ( (whichPart >= inThumb) && optionKeyDown )
	{
		/* <Option> drag ... */
	
		PenNormal();

		theScrollPane = (CPStyleScrollPane*) itsEnclosure;

		/*
			Compute the slop and the distance from the top/left
			of the Thumb to the Arrow:
			
			Note that the slop allowed in the length direction
			is 3-times that in the opposite direction in order
			to allow dragging the Thumb to the max.
			
			The distance from the top/left of the Thumb to the
			Arrow is required to prevent scrolling below the
			minimum value when the control CDEV function
			receives a positioning message and subsequently
			calls SetCtlValue.  Such sensitivity does NOT seem
			to exist when scrolling beyond the maximum value ???
			
			By the way, before the control CDEV function calls
			SetCtlValue, it rounds UP the passed position,
			so I need to maintain the "inThumbOffset" value
			AFTER this CDEV function is called.  Otherwise,
			the scrolling Thumb will get out-of-sync with
			the Mouse, that is, may "get ahead" of the Mouse.
		*/
		
		slopRect = frame;
		thumbRgn = (RgnHandle) (**macControl).contrlData;
		thumbBox = (**thumbRgn).rgnBBox;
		scrollBarRect = (**macControl).contrlRect;
		
		if (theOrientation == HORIZONTAL)
		{
			InsetLongRect( &slopRect, - 3*SLOPSIZE, - SLOPSIZE );
			
			topOfThumbToArrow = thumbBox.left - scrollBarRect.left - ARROWSIZE;
			inThumbOffset = origThumbPt.h - thumbBox.left;
		}
		else
		{
			InsetLongRect( &slopRect, - SLOPSIZE, -3*SLOPSIZE );
			
			topOfThumbToArrow = thumbBox.top - scrollBarRect.top - ARROWSIZE;
			inThumbOffset = origThumbPt.v - thumbBox.top;
		}
		FrameToWindR( &slopRect, &slopR );
		QDToLongRect( &slopR, &slopRect );
		
		varCode = GetCVariant( macControl );
		ctlDefLong = (long) (**macControl).contrlDefProc;
		ctlDefLong &= 0x00FFFFFF;		/* Strip variation code. */
		ctlDefFunc = (ControlDefFuncPtr) *((Handle) ctlDefLong);
		
		origThumbValue = GetValue();

		while ( WaitMouseUp() )
		{
			Prepare();
			
			/*
				The "macPort" of the CScrollBar is the same as that
				of its enclosing CScrollPane which is the CWindow.
				CScrollBar is a CControl whose Prepare() sets its
				macPort's origin = {0, 0}.  Therefore, the Point
				returned by GetMouse is already in window coords.
			*/
			GetMouse( &movedThumbPt );
			
			if ( PtInRect(movedThumbPt, &slopR) )
			{
				QDToLongPt( movedThumbPt, &movedThumbPoint );
				
				/*
					Reposition the Thumb and update the Control's value by
					sending a "posCntl" message to the Control Definition
					Function.  Note that for a position control message,
					the returned result = zero and is meaningless.
				*/
								
				if (theOrientation == HORIZONTAL)
				{
					if (movedThumbPoint.h < origThumbPoint.h - topOfThumbToArrow)
					{
						/* Mouse below minimum value: */
						
						movedThumbPoint.h = origThumbPoint.h - topOfThumbToArrow;
					}
					longVH = /* high word = 0 + */
												movedThumbPoint.h - origThumbPoint.h;
					(void) (*ctlDefFunc) ( varCode, macControl, posCntl, longVH );
					
					movedThumbValue = GetValue();
					theScrollPane->DoThumbDrag( movedThumbValue - origThumbValue, 0 );
					
					/*
						Recalculate new position based
						on a newly offset Thumb RgnHandle:
					*/
					thumbBox = (**thumbRgn).rgnBBox;
					topOfThumbToArrow = thumbBox.left - scrollBarRect.left -
																			ARROWSIZE;

					/*
						Update the starting point, but maintain
						the relative position within the Thumb
						as I talked about above:
					*/
					origThumbPoint.h = thumbBox.left + inThumbOffset;
					
				}	/* Horizontal Scroll Bar */

				else
				{
					if (movedThumbPoint.v < origThumbPoint.v - topOfThumbToArrow)
						movedThumbPoint.v = origThumbPoint.v - topOfThumbToArrow;
					longVH = (movedThumbPoint.v - origThumbPoint.v) << 16
																/* + low word = 0 */;
					(void) (*ctlDefFunc) ( varCode, macControl, posCntl, longVH );
					
					movedThumbValue = GetValue();
					theScrollPane->DoThumbDrag( 0, movedThumbValue - origThumbValue );
					
					thumbBox = (**thumbRgn).rgnBBox;
					topOfThumbToArrow = thumbBox.top - scrollBarRect.top - ARROWSIZE;
					origThumbPoint.v = thumbBox.top + inThumbOffset;
				
				}	/* Vertical Scroll Bar */
				
					
			 /*
				CScrollPane's DoThumbDrag calls DoScroll and the
				stuff that Calibrate does is addressed by sending a
				"posCntl" message to the Control Definition Function:
					
				theScrollPane->DoScroll( hDeltaPanUnits, vDeltaPanUnits );
				theScrollPane->Calibrate();
			 */
				 
				/* Update the control value: */
					
				origThumbValue = movedThumbValue;
				
			}	/* PtInRect */
										
		}	/* while-loop */
		
		ForceNextPrepare();

	}	/* <Option> dragging */
	
	else
	{
		CScrollBar::DoClick( hitPt, modifierKeys, when );
	}

}	/* DoClick */



void	CStylePStyleTask::IStylePStyleTask (CPStyleText *aTextPane,
											long aStyleCmd, short firstTaskIndex)
{
	/*
		Called by CPStyleText's MakeStyleTask which was
		called by CAbstractText's DoCommand when we changed:
		
			a) font ID, size or style
			b) text alignment or spacing
	*/


		long	selStart, selEnd;

	
	oldStyles = NULL;
	
	CTextStyleTask::ITextStyleTask( aTextPane, aStyleCmd, firstTaskIndex );
	
	itsTextPane->GetSelection( &selStart, &selEnd );
	this->selStart = selStart;
	this->selEnd = selEnd;
	
}	/* IStylePStyleTask */



/* OVERRIDE */
void	CStylePStyleTask::Dispose (void)
{

	ForgetHandle( oldStyles );
	
	CTextStyleTask::Dispose();

}	/* Dispose */



/* OVERRIDE */
void	CStylePStyleTask::Do (void)
{
	/*
		Called by CAbstractText's DoCommand following
		its call to MakeStyleTask which returns the
		appropriate CTask which is the grandparent
		of my CStylePStyleTask.
	*/
		
		
		CPStyleText		*theStylePane = (CPStyleText*) itsTextPane;
		/*
			Changes in FONTs, font sizes and
			font styles can affect word-wrap:
		*/
		Boolean			rewrap		  = FALSE;
		Str255			itemName;
		long			fontSize;
 	
 	
 	SaveStyle();
 	
	oldAlignCmd = theStylePane->GetAlignCmd();
	oldSpacingCmd = theStylePane->GetSpacingCmd();
	
	/*
		CTextStyleTask's "styleCmd" is saved by CAbstractText's DoCommand
		when it calls CAbstractText::MakeStyleTask.  The latter calls
		ITextStyleTask which stuffs its passed Command into "styleCmd".
		So "styleCmd" is the NEW command to be reckoned with.
	*/
	
	if (styleCmd < 0) 
	{
	
		switch ( HiShort(-styleCmd) ) 
		{
		
			case MENUfont:
				gBartender->GetCmdText( styleCmd, itemName );
				theStylePane->SetFontName( itemName );
				styleAttribute = doFont;
				rewrap = TRUE;
				break;
				
			case MENUsize:
				gBartender->GetCmdText( styleCmd, itemName );
				StringToNum( itemName, &fontSize );
				theStylePane->SetFontSize( fontSize );
				styleAttribute = doSize;
				rewrap = TRUE;
				break;			
		}
		
	}	/* styleCmd < 0 */

	else
	{
		switch	(styleCmd)
		{
			case cmdPlain:
				theStylePane->SetFontStyle( NOTHING );
				styleAttribute = doFace;
				rewrap = TRUE;
				break;
				
			case cmdBold:				/* These toggle <--> ... */
			case cmdItalic:
			case cmdUnderline:
			case cmdOutline:
			case cmdShadow:
			case cmdCondense:
			case cmdExtend:
				theStylePane->SetFontStyle( 1 << (styleCmd - cmdBold) );
				styleAttribute = doFace;
				rewrap = TRUE;
				break;
				
			case cmdJustify:
				theStylePane->SetAlignCmd( styleCmd );
				styleAttribute = doAlign;
				break;
				
			default:
				/*
					For remaining alignments and
					all spacing commands:
				*/
				
				CTextStyleTask::Do();
		}
		
	}	/* styleCmd >= 0 */

	if (rewrap)
	{
		/*
			RecalcLineStarts calls WrapLineStarts,
			followed by SetWholeLines + AdjustBounds:
		*/
		theStylePane->RecalcLineStarts();
	}
		
	theStylePane->ScrollToSelection();

}	/* Do */



/* OVERRIDE */
void	CStylePStyleTask::Undo (void)
{
	/*
		This method does both undo and redo and is
		overridden because changes in FONTs, font
		sizes and font styles can affect word-wrap.
		
		For now, duplicate CTextStyleTask's Undo().
	*/
	
	
		Boolean			rewrap;
	 // StScrpHandle	swapStyles, newStyles;
		TextStyle		swapStyle, newStyle;
		CPStyleText		*theStylePane = (CPStyleText*) itsTextPane;
		
		
	switch (styleAttribute)
	{
		case doFont:
		case doSize:
		case doFace:
			rewrap = TRUE;
			break;
		
		case doAlign:
		case doSpacing:
			rewrap = FALSE;
			break;
	}

 // swapStyles = newStyles = NULL;

	TRY
	{
		if (styleAttribute < doAlign)
		{
			
			theStylePane->Prepare();	
			theStylePane->SetSelection( selStart, selEnd, TRUE );
			
			swapStyle/*s*/ = oldStyle/*s*/;		/* Save the current styling.	 */
		 // oldStyles = NULL;
			
			SaveStyle();
		
			newStyle/*s*/ = oldStyle/*s*/;
			oldStyle/*s*/ = swapStyle/*s*/;		/* Restore the previous styling. */
		 // swapStyles = NULL;
		
			RestoreStyle();
			
		 // ForgetHandle( oldStyles );
			
			oldStyle/*s*/ = newStyle/*s*/;		/* Swap current and previous.	 */
			
			theStylePane->SetSpacingCmd( theStylePane->GetSpacingCmd() );
		 /*
			Postpone till later ...
			
			theStylePane->SetWholeLines( theStylePane->GetWholeLines() );
			theStylePane->AdjustBounds();
		 */
		 
			undone = !undone;

		}
		else
			CTextStyleTask::Undo();
	}
	CATCH
	{
	 // ForgetHandle( oldStyles );
	 // ForgetHandle( newStyles );
	 // ForgetHandle( swapStyles );
	}
	ENDTRY;
	
	if (rewrap)
	{
		/* See comment in Do() method about refreshing: */
		
		theStylePane->RecalcLineStarts();
	}
	else
	{
		/* RecalcLineStarts calls these dudes: */
		
		theStylePane->SetWholeLines( theStylePane->GetWholeLines() );
		theStylePane->AdjustBounds();
	}
	
	theStylePane->ScrollToSelection();
	
}	/* Undo */



/* OVERRIDE */
void	CStylePStyleTask::SaveStyle (void)
{

	oldStyles = NULL;			/* GetStyleScrap may fail. */
	
	oldStyles = ((CPStyleText*) itsTextPane)->GetStyleScrap();
	
}	/* SaveStyle */



/* OVERRIDE */
void	CStylePStyleTask::RestoreStyle (void)
{

	if (styleAttribute < doAlign)
	{
			CPStyleText		*theStylePane = (CPStyleText*) itsTextPane;
		
		
		/* +++ work around problem with redoing cmdPlain on styled text? */
		if ( (styleCmd == cmdPlain) && (undone == TRUE) )
			theStylePane->SetFontStyle( NOTHING );
		else
			theStylePane->SetStyleScrap( selStart, selEnd, oldStyles, TRUE );		
	}
	else
		CTextStyleTask::RestoreStyle();

}	/* RestoreStyle */



/****   G E N E R A L   U T I L I T I E S   ****/


/*
	For now these utilities are NOT called by any class
	function provided in this file.  Rather, I call them
	from within several member functions of CStyleText's
	parent CPane and of other supervising CDocuments that
	are linked to CStyleText's supervisor.  This way, the
	choice of using these utilities is yours, NOT mine.
	
	For example, I could call DirtyDocument from within
	the DoCommand and DoKeyDown methods belonging to the
	parent of my CStyleText CPane.  However, be sure to
	call the inherited methods AFTER DirtyDocument because
	both inherited methods call CDocument's Notify and
	Notify sets the "dirty" instance variable = TRUE.  If
	DirtyDocument is called after the inherited method,
	"dirty" is already TRUE and DirtyWindow is not called.
	
	If I wanted to just call DirtyWindow within DoCommand,
	for example, I would have to include the following:
	
			CWindow		*theWindow = GetWindow();
			Boolean		prevDirty  = ((CDocument*)itsSupervisor)->dirty;
			
     	CPStyleText::DoCommand( theCommand );
    	if ( !prevDirty )	DirtyWindow( theWindow );
*/


void	DirtyWindow (CWindow *theWindow)
{
		Str255			itsDirty	= "\p�",
						oldTitle, dirtyTitle;

	
	if (theWindow)
	{
		theWindow->GetTitle( oldTitle );
		CopyPString( itsDirty, dirtyTitle );
		ConcatPStrings( dirtyTitle, oldTitle );
		theWindow->SetTitle( dirtyTitle );
	}
	
}	/* DirtyWindow */



void	DirtyDocument (CDocument *theDocument)
{
		
	if ( theDocument && !theDocument->dirty )
	{
		theDocument->dirty = TRUE;
		DirtyWindow( theDocument->itsWindow );
	}
	
}	/* DirtyDocument */



void	UndirtyWindow (CWindow *theWindow)
{
		Str255		dirtyTitle;
		short		dirtyLength;

	
	if (theWindow)
	{
		theWindow->GetTitle( dirtyTitle );
		dirtyLength = Length( dirtyTitle );
		dirtyTitle[0] = dirtyLength - 1;
		BlockMove( dirtyTitle + 2, dirtyTitle + 1, dirtyLength - 1L );
		theWindow->SetTitle( dirtyTitle );		/* Actually now a "clean" title. */
	}
	
}	/* UndirtyWindow */



void	UndirtyDocument (CDocument *theDocument)
{
		
	if ( theDocument && theDocument->dirty )
	{
		theDocument->dirty = FALSE;
		UndirtyWindow( theDocument->itsWindow );
	}
	
}	/* UndirtyDocument */




/*	{ end file "CPStyleText.c" }  */
