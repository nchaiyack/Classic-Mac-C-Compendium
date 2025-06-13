/**********************************************************************\
*                                                                      *
* Patterns MDEF.c                                                      *
*                                                                      *
* This module implements a Menu Definition Procedure for the Spion     *
* World Editor.  It is used as a popup menu which allows the user to   *
* select the desired background pattern.                               *
\**********************************************************************/


#define	PATTERN_WIDTH	16
#define	PATTERN_HEIGHT	16
#define	HORIZ_PATTERNS	16
#define	VERT_PATTERNS	16

#define	MENU_PICT		128

/********************************* Protypes *****************************/

pascal void main (int message, MenuHandle the_menu,
					Rect *the_rect, Point hit_pt, int *which_item);
void highlight_item (int which_item, Rect *menu_rect, Boolean hilight);



/*****************************************************************************\
* procedure main                                                              *
*                                                                             *
* Purpose: This procedure is called whenever there is an event involving this *
*          popup menu.  It processes the event.                               *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 17, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/


pascal void main (int message, MenuHandle the_menu,
					Rect *the_rect, Point hit_pt, int *which_item)
{
	short		item_hit;			/* Which pattern is being selected */
	short		pop_up_offset_h;	/* Offset into menu of this pattern, horizontally */
	short		pop_up_offset_v;	/* Offset into menu of this pattern, vertically */
	Rect		pattern_rect;		/* Rectangle containing this pattern */
	short		pattern_row;		/* Row of this pattern */
	short		pattern_column;		/* Column of this pattern */
	PicHandle	menu_picture;
			
	switch (message)
		{
		case mDrawMsg:
			
			/* Copy the Menu PICT to the menu */
			menu_picture = GetPicture(MENU_PICT);
			DrawPicture(menu_picture, the_rect);
				
			break;
			
		case mChooseMsg:
			if (PtInRect (hit_pt, the_rect))
				{
				/* compute which pattern mouse is currently in */
				pattern_row = (hit_pt.v - the_rect->top)/(PATTERN_HEIGHT + 1);
				pattern_column = (hit_pt.h - the_rect->left)/(PATTERN_WIDTH + 1);
				
				/* Constrain to acceptable row and column numbers */
				if (pattern_row >= VERT_PATTERNS)
					pattern_row = VERT_PATTERNS - 1;
				if (pattern_column >= HORIZ_PATTERNS)
					pattern_column = HORIZ_PATTERNS - 1;

				/* Find the item number */
				item_hit = pattern_row * HORIZ_PATTERNS + pattern_column + 1;
				
				/* If mouse is on a different item than before, deselect the old
					and select the new */
				if (item_hit != *which_item)
					{
					highlight_item (*which_item, the_rect, FALSE);
					highlight_item (item_hit, the_rect, TRUE);
					*which_item = item_hit;
					}
				}
			
			/* If there was an item selected, but now the mouse is outside of the
				menu, deselect the item */
			else if (*which_item)
				{
				highlight_item (*which_item, the_rect, FALSE);
				*which_item = 0;
				}
			
			break;
			
		case mSizeMsg:
			(*the_menu)->menuWidth = (PATTERN_WIDTH + 1) * HORIZ_PATTERNS + 1;
			(*the_menu)->menuHeight = (PATTERN_HEIGHT + 1) * VERT_PATTERNS + 1;
			break;
		
		case 3:	/* popup */
			pop_up_offset_v = ((*which_item-1) / VERT_PATTERNS) * (PATTERN_HEIGHT + 1);
			pop_up_offset_h = ((*which_item-1) % VERT_PATTERNS) * (PATTERN_WIDTH + 1);
			
			the_rect->left = hit_pt.v - pop_up_offset_h - PATTERN_WIDTH/2;
			the_rect->top = hit_pt.h - pop_up_offset_v - PATTERN_HEIGHT/2;
			the_rect->right = the_rect->left + (PATTERN_HEIGHT + 1) * VERT_PATTERNS + 1;
			the_rect->bottom = the_rect->top + (PATTERN_WIDTH + 1) * HORIZ_PATTERNS + 1;
			
			break;			
			
		default:
			break;

		}

}	/* main() */



/*****************************************************************************\
* procedure highlight_item                                                    *
*                                                                             *
* Purpose: This procedure highlights an item in the patterns menu by drawing  *
*           a box around it.                                                  *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 17, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void highlight_item (int which_item, Rect *menu_rect, Boolean hilight)
{

	RGBColor	rgb_black = {0x0000, 0x0000, 0x0000};
	RGBColor	rgb_white = {0xFFFF, 0xFFFF, 0xFFFF};
	Rect 		bound_rect = {0, 0, PATTERN_HEIGHT+1, PATTERN_WIDTH+1};

	/* Don't draw illegal values */
	if ((which_item > VERT_PATTERNS*HORIZ_PATTERNS) || (which_item < 1))
		return;

	/* Find the rectangle enclosing this selection */
	bound_rect.left = menu_rect->left + ((which_item-1) % VERT_PATTERNS) * (PATTERN_WIDTH + 1);
	bound_rect.top = menu_rect->top + ((which_item-1) / VERT_PATTERNS) * (PATTERN_HEIGHT + 1);
	bound_rect.right = bound_rect.left + PATTERN_WIDTH + 2;
	bound_rect.bottom = bound_rect.top + PATTERN_HEIGHT + 2;

	/* Invert the black border */
	PenMode(patXor);
	FrameRect (&bound_rect);
	PenMode(patCopy);
	
}	/* highlight_item() */