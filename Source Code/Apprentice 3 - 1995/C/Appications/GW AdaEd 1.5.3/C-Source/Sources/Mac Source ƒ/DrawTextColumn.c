/*** DrawTextColumn.c
	Name: DrawTextColumn
	Version: 1.0
	Category: functions
	Contributor: Andrew Gilmartin <Andrew_Gilmartin@Brown.Edu> 
	Date: 92-04
	TCL-Version: 1.1.2
	TCL-Language: C
	Copyright: Copyright (C) 1993 by Brown University. All rights reserved.
***/
/*
 *  DrawTextColumn.c
 *
 *  DrawTextColumn() and DrawStringColumn() draw text within a given 
 *  width. If all the text wont fit using the current style, condensed is 
 *  next tried. If the text still doesn't fit then the left n characters 
 *  that will fit are drawn. To indicated to the user that not all the 
 *  text is being displated an ellipsis is drawn an the end of the 
 *  truncated text.
 * 
 *  Copyright C) 1992 by Brown University. All rights reserved.
 *
 *  Permission is granted to any individual or institution to use, copy,
 *  or redistribute the binary version of this software and its
 *  documentation provided this notice and the copyright notices are
 *  retained.  Permission is granted to any individual or non-profit
 *  institution to use, copy, modify, or redistribute the source files
 *  of this software provided this notice and the copyright notices are
 *  retained.  This software may not be distributed for profit, either
 *  in original form or in derivative works, nor can the source be
 *  distributed to other than an individual or a non-profit institution.
 *  Any  individual or group interested in seeing and/or using these
 *  source files but who are prevented from doing so by the above
 *  constraints should contact Don Wolfe, Assistant Vice-President for
 *  Computer Systems at Brown University, (401) 863-7250, for possible
 *  software licensing of the source developed at Brown.
 *
 *  Brown University and Andrew James Gilmartin make no representations
 *  about the suitability of this software for any purpose.
 *
 *  BROWN UNIVERSITY AND ANDREW JAMES GILMARTIN GIVE NO WARRANTY, EITHER
 *  EXPRESS OR IMPLIED, FOR THE PROGRAM AND/OR DOCUMENTATION PROVIDED,
 *  INCLUDING, WITHOUT LIMITATION, WARRANTY OF MERCHANTABILITY AND
 *  WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE.
 *
 *  MODIFIED: 92-04-11
 */

// MODIFIED: 1994 [Fabrizio Oddone]

#include "DrawTextColumn.h"


#define kEllipsisChar 0xC9


void DrawStringColumn( ConstStr255Param string, unsigned short columnWidth )
{
DrawTextColumn( (Ptr) string, 1, StrLength(string), columnWidth );
} /* DrawStringColumn */


void DrawTextColumn(const Ptr theText, unsigned short firstByte, unsigned short textLength, unsigned short columnWidth )
{
Style textStyle;
unsigned short textWidth;
unsigned short ellipsisWidth;
unsigned short left;
unsigned short right;
unsigned short index;

	/* Does the text at current style fit column width? */
	
textWidth = TextWidth( theText, firstByte, textLength );
if ( textWidth <= columnWidth )
{
		/* If so, draw the text */
		
	DrawText( theText, firstByte, textLength );
	
		/* Done */
		
	return;
}

	/* Does the text fit using condensed? */
	
textStyle = qd.thePort->txFace;
TextFace( textStyle | condense );
textWidth = TextWidth( theText, firstByte, textLength );	
if ( textWidth <= columnWidth )
{
		/* if so, draw the text */
		
	DrawText( theText, firstByte, textLength );
	
		/* Reset the style */
		
	TextFace( textStyle );
	
		/* Done */
		
	return;	
}

	/* 
	 * Since the text wont fit completly within the column width, we need to
	 * find the maximum number of characters that will fit. We do this by
	 * bisecting the text until we find that number of characters with just
	 * the right width.
	 */
	 
	 /* End the truncated text with an ellipsis */
	 
ellipsisWidth = CharWidth( kEllipsisChar );

	/* Determine the number of characters that can be drawn */
	
left = 1;
right = textLength;

do
{
	index = left + right;
	index >>= 1; // shift instead of / to help stupid compilers [Fabrizio Oddone]
	textWidth = TextWidth( theText, firstByte, index ) + ellipsisWidth;

	if ( textWidth <= columnWidth )
		left = index + 1;
	else
		right = index - 1;

} while( left < right );

	/* Draw some of the text... */
	
DrawText( theText, firstByte, right - 1 );
DrawChar( kEllipsisChar );

	/* Reset the text style */
	
TextFace( textStyle );


} /* DrawTextColumn */

