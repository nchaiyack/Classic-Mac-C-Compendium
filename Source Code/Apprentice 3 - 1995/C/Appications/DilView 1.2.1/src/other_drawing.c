/* other_drawing.c */

#include "other_drawing.h"
#include "constants.h"
#include "windows.h"
#include "globals.h"

void OtherDrawing( void )
// 
// draw the seperator bar
//
{
	Rect seperatorRect;
	
	GetSeperatorRect( &seperatorRect );
	SetPortToWindow();
	PaintRect( &seperatorRect );
}

Rect *GetSeperatorRect( Rect *rect )
{
	float scale;
	
	scale = GetScalePercent();

	SetRect( rect, 0, k_dil_height * scale, k_dil_width * scale,
				k_dil_height * scale + k_seperator_height );
	
	return( rect );
}