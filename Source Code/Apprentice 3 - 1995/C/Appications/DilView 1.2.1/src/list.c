/* list.c */

#include <QDOffScreen.h>
#include "list.h"
#include "globals.h"
#include "windows.h"
#include "error.h"
#include "structs.h"
#include "constants.h"
#include "controls.h"
#include "nyi.h"

GWorldPtr g_list_buffer;
dil_rec *head = nil, *tail = nil;
Boolean g_marked_flag = false;

GWorldPtr GetGListPort( void )
{
	return( g_list_buffer );
}

void SetMarkedFlag( Boolean value )
{
	g_marked_flag = value;
}

Boolean GetMarkedFlag( void )
{
	return( g_marked_flag );
}

void myInitList( void )
{
	Rect rect;
	QDErr err;

	// the full size offscreen buffer
	SetRect( &rect, 0, 0, 1, 1 );
	err = NewGWorld( &g_list_buffer, k_list_buffer_depth, &rect, nil, nil, 0 );
	if( err != noErr ) myError( "Couldn't create offscreen list buffer.  Try increasing memory allocation", true );
}

void myDisposeList( void )
{
	DisposeGWorld( g_list_buffer );
}

void DrawList( void )
{
	static dil_rec *last_dil = nil;
	static short last_scale = 0, last_length = 0;
	Boolean reDraw = false;
	dil_rec *cur_dil, *dil;
	short cur_scale, cur_length;
	Rect rect, screenRect, eraseRect;
	short numRows, numCols, curRow, curCol;
	short rectBottomsDelta;
	
	cur_dil = GetCurDil();
	cur_scale = GetScale();
	cur_length = GetNumListItems();
	
	numRows = GetNumListRows();
	numCols = GetNumListCols();
	
	// the the dil being displayed has changed, we need to redraw so as to highlight the new one
	if( last_dil != cur_dil )
	{
		reDraw = true;
		last_dil = cur_dil;
	}
	
	// if the scale has changed we need to resize the list buffer, and redraw
	if( (last_scale != cur_scale) || (last_length != cur_length) )
	{	
		reDraw = true;
		last_scale = cur_scale;
		last_length = cur_length;
		
		// figure the ideal size
		GetListRect( &screenRect );
		SetRect( &rect, 0, 0, screenRect.right - screenRect.left, numRows * k_list_row_height );
			
		// make sure the rect is at least one pixel high (so empty lists don't mess things up)
		if( rect.bottom == 0 )
			rect.bottom = 1;
			
		// finally reallocate a properly sized offscreen buffer
		UpdateGWorld( &g_list_buffer, k_list_buffer_depth, &rect, nil, nil, 0 );
	}
	
	// if any marks/unmarks have been made, we need to redraw 
	if( GetMarkedFlag() == true )
	{
		reDraw = true;
		SetMarkedFlag( false );
	}
		
	if( reDraw )
	{	
		SetPortToGWorld( g_list_buffer );
		EraseRect( &(g_list_buffer->portRect) );
		dil = GetFirstDil();
		
		for( curRow = 0 ; curRow < numRows ; curRow++ )
		{
			for( curCol = 0 ; curCol < numCols ; curCol++ )
			{
				if( dil != nil )
				{
					GetCheckBoxRect( &rect, curRow, curCol );
					FrameRect( &rect );
					
					if( dil->marked )
					{
						MoveTo( rect.left, rect.top );
						LineTo( rect.right-1, rect.bottom-1 );
						MoveTo( rect.right-1, rect.top );
						LineTo( rect.left, rect.bottom-1 );
					}
					
					GetItemRect( &rect, curRow, curCol );
					MoveTo( rect.left + k_list_item_offset_h, rect.top + k_list_item_offset_v );
					DrawString( dil->fname );
					if( dil == GetCurDil() )
						InvertRect( &rect );
					dil = GetNextDil( dil );
				}
			}
		}	
		RestorePort();
	}
	
	// now copy the appropriate part of the list_buffer onscreen..
	GetListRect( &screenRect );
	SetRect( &rect, 0, GetListBufferOffsetFromControl(), 
		screenRect.right - screenRect.left,
		GetListBufferOffsetFromControl() + (screenRect.bottom - screenRect.top) );
		
	// if the window stretches farther down than the list, truncate and erase the extra area
	rectBottomsDelta = (g_list_buffer->portRect).bottom - rect.bottom;
	if( rectBottomsDelta < 0 )
	{
		// truncate the rect to be copied from the offscreen buffer
		rect.bottom += rectBottomsDelta;
		if( rect.bottom < rect.top )
			rect.bottom = rect.top;
			
		// truncate the rect we're copying to
		screenRect.bottom += rectBottomsDelta;
		if( screenRect.bottom < screenRect.top )
			screenRect.bottom = screenRect.top;
			
		// erase the extra stuff (oldRect - newRect)
		GetListRect( &eraseRect );
		eraseRect.top = screenRect.bottom;
		if( eraseRect.bottom < eraseRect.top )
			eraseRect.bottom = eraseRect.top;
		SetPortToWindow();
		EraseRect( &eraseRect );
	}

	CopyGWorldToWindow( g_list_buffer, &rect, &screenRect, srcCopy );

	// erase the frame!
	SetPortToWindow();
	GetListRect( &screenRect );
	InsetRect( &screenRect, -1, -1 );
	ForeColor( whiteColor );
	FrameRect( &screenRect );
	ForeColor( blackColor );
}

short GetNumListCols( void )
//
// return the number of columns, key: 50% = 3, 75% = 4, 100% = 5
//
{
	return( (4 * GetScalePercent()) + 1 );
}

short GetNumListRows( void )
{
	short numItems, numCols, numRows;

	numItems = GetNumListItems();
	numCols = GetNumListCols();

	// figure the number of full rows
	numRows = numItems / numCols;
	
	// if there are any left over, add another row for them (division truncates..)
	if( (numItems % numCols) != 0 )
		numRows++;
		
	return( numRows );
}

short GetNumListItems( void )
{
	short count = 0;
	dil_rec *dil = head;
		
	while( dil != nil )
	{
		count++;
		dil = dil->next;
	}
	return( count );
}

Rect *GetCheckBoxRect( Rect *rect, short row, short col )
{	
	Rect item_rect;

	GetItemRect( &item_rect, row, col );
	
	SetRect( rect, item_rect.left + 2,
					item_rect.top + 2,
					item_rect.left + 2 + (item_rect.bottom - item_rect.top - 4),
					item_rect.bottom - 2 );
	
	return( rect );
}

Rect *GetItemRect( Rect *rect, short row, short col )
{
	static short last_scale, col_width;
	short cur_scale;
	
	cur_scale = GetScale();
		
	if( cur_scale != last_scale )
	{
		last_scale = cur_scale;
		
		GetListRect( rect );
		col_width = (rect->right - rect->left) / GetNumListCols();
	}
	
	SetRect( rect, col * col_width,
					row * k_list_row_height,
					(col + 1) * col_width,
					(row +1 ) * k_list_row_height );
	
	return( rect );
}

Rect *GetListRect( Rect *rect )
{
	Rect win_rect;
	
	GetWinRect( &win_rect );

	SetRect( rect, k_list_frame_height, 
		(k_dil_height * GetScalePercent()) + k_seperator_height + k_list_frame_height,
		win_rect.right - k_scrollbar_width - k_list_frame_height,
		win_rect.bottom - k_scrollbar_width - k_list_frame_height );
		
	return( rect );
}

dil_rec *GetFirstDil( void )
//
// ADT.. get the first dil record in the linked list
//
{
	return( head );
}

dil_rec *GetNextDil( dil_rec *dil )
//
// ADT.. get the next dil record from the linked list
//
{
	if( dil == nil )
		return( nil );
	else
		return( dil->next );
}

dil_rec *GetNextMarkedDil( dil_rec *dil )
//
// ADT.. get the next marked dil from the linked list, after the one passed.
//
{
	dil_rec *temp;

	// defensive programming..
	if( dil == nil )
		return( nil );

	temp = dil->next;

	// loop until we either find a marked dil or hit the end of the linked list, in which case 
	// return nil
	while( temp != nil )
	{
		if( temp->marked )
			return( temp );
		temp = temp->next;
	}
	return( nil );	
}

dil_rec *GetPrevDil( dil_rec *dil )
//
// ADT.. get the dil occuring _before_ 'dil' in the list.
//
{
	dil_rec *temp;
	
	temp = GetFirstDil();
	
	// if asking for previous to first, return nil.
	if( temp == dil )
		return( nil );
	
	// step through the list until we run over or find the one previous to our mark.
	while( temp != nil )
	{
		if( temp->next == dil )
			return( temp );
		temp = temp->next;
	}
	return( nil );
}

void AddDilToList( dil_rec *dil )
{
	// if the list is empty, insert it at front, else append after tail
	if( head == nil )
		head = dil;
	else
		tail->next = dil;
	
	// in any case, the newly added item is the new tail
	tail = dil;
}

void ListClick( Point localPt )
{
	Rect check_rect, item_rect;
	dil_rec *dil;
	short curRow, curCol, numRows, numCols;
	
	numRows = GetNumListRows();
	numCols = GetNumListCols();
	
	localPt.v -= (k_dil_height * GetScalePercent()) + k_seperator_height + k_list_frame_height -
		GetListBufferOffsetFromControl();

	dil = GetFirstDil();

	for( curRow = 0 ; curRow < numRows ; curRow++ )
	{
		for( curCol = 0 ; curCol < numCols ; curCol++ )
		{
			if( dil != nil )
			{
				GetCheckBoxRect( &check_rect, curRow, curCol );
				GetItemRect( &item_rect, curRow, curCol );
				
				if( PtInRect( localPt, &check_rect ))
				{
					dil->marked = !(dil->marked);
					SetMarkedFlag( true );
				}
				else if( PtInRect( localPt, &item_rect ))
					SetCurDil( dil );
				
				dil = GetNextDil( dil );
			}
		}
	}
	myUpdate(false);
}

void SetCurToHead( void )
{
	SetCurDil( GetFirstDil() );
}