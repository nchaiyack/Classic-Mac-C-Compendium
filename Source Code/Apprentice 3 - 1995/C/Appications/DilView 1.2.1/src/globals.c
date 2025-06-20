/* globals.c */

#include "globals.h"
#include "menus.h"
#include "structs.h"
#include "error.h"

// some globals shared between files.. note that there are also local globals in some src files!

//еееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееее
//
// The Scale of the window, as #defined in "menus.h" (ie i50scale, i75scale, or i100scale)
// 

short g_cur_scale = i50scale;

short GetScale( void )
{
	return( g_cur_scale );
}

float GetScalePercent( void )
{
	switch( g_cur_scale )
	{
		case i50scale:
			return( 0.50 );
			break;
		case i75scale:
			return( 0.75 );
			break;
		case i100scale:
			return( 1.00 );
			break;
		default:
			myError( "Programmer Error: invalid scale!", true );
			break;
	}
}

void SetScale( short new_scale )
{
	g_cur_scale = new_scale;
}

//еееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееее
//
// cur_dil, the current dil
//

dil_rec *cur_dil = nil;

dil_rec *GetCurDil( void )
{
	return( cur_dil );
}

void SetCurDil( dil_rec *dil )
{
	cur_dil = dil;
}

//еееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееее
