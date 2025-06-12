/**

	swatch.about.c
	Copyright (c) 1990, Adobe Systems, Inc.

 **/


/**-----------------------------------------------------------------------------
 **
 **	Conditional Compilation Flags
 **
 **/


/**-----------------------------------------------------------------------------
 **
 **	Headers
 **
 **/

#include <Sound.h>

#include "swatch.resources.h"
#include "swatch.h"
#include "swatch.prefs.h"
#include "swatch.about.h"


/**-----------------------------------------------------------------------------
 **
 ** Private Macros
 **
 **/


/**-----------------------------------------------------------------------------
 **
 ** Private Constants
 **
 **/

#define MAX_FLYING			10
#define TICK_FREQ			2
#define CHAR_WIDTH			8
#define CHAR_HEIGHT			12

#define MAX_BITS			14

#define LAUNCH_FREQ			(10)
#define SWIRL_DELAY			(9*60 + 52)
#define BY_DELAY			(15*60 + 30)
#define BIT_DELAY			(18*60 + 40)
#define INTER_BIT_DELAY		(350)
#define INTER_BIT_INC		(40)
#define ERASE2_DELAY		(32*60)
#define FIRST_BEAT_DELAY	(30)
#define CREDITS2_DELAY		(28*60)
#define HEART_FRAMES		(200)


/**-----------------------------------------------------------------------------
 **
 ** Private Variables
 **
 **/

char credits[] = "Swatch 1.2";
char credits2[] = "\pCopyright ©1991, joe holt";


typedef struct {
	Boolean flying;
	Fixed x, y, dx, dy;
	int16 size;
	Rect r;
} Flying_bit;

typedef struct {
	int16 stage;
	char c;
	uns32 next_tick;
	Fixed x, y, dx, dy, ddx, ddy;
	Fixed maxx, maxy, dmax;
	int16 num_bits, num_bits2;
	Flying_bit bit[MAX_BITS];
	Rect r;
} Flyer;


/**-----------------------------------------------------------------------------
 **
 ** Private Functions
 **
 **/

int16 random( int16 max );


/*******************************************************************************
 **
 **	Public Variables
 **
 **/


/*******************************************************************************
 **
 **	Public Functions
 **
 **/


/*******************************************************************************
 ***
 *** prototype
 ***
 *** summary
 ***
 *** History:
 ***
 *** To Do:
 ***
 ***/

void Do_about( WindowPtr the_window )
{
	Flyer *fly;
	register Flyer *f;
	register Flying_bit *b;
	register int16 i, j;
	int16 launch_c;
	uns32 next_launch, ticker, start_ticks, swirl_delay;
	int16 num_active;
	Fixed window_right, window_bottom, sx, sy, byx, byy;
	Boolean got_beat;
	int16 inter_bit_delay;
	Handle snd;
	SndChannelPtr chan;

	fly = (Flyer *)NewPtr( MAX_FLYING * sizeof( Flyer ) );
	if ( !fly )
		return;

	ClipRect( &the_window->portRect );
	snd = Read_riff();

	HideCursor();
	set_back_color( HEAP_UNLOCKED_COLOR );
	set_fore_color( HEAP_LOCKED_COLOR );

	EraseRect( &the_window->portRect );
	window_right = (int32) the_window->portRect.right << 16;
	window_bottom = (int32) the_window->portRect.bottom << 16;

	for ( f = fly, i = MAX_FLYING; i; --i, ++f ) {
		Fixed n, ni, sp;

		f->stage = 0;
		f->num_bits2 = f->num_bits = random( MAX_BITS / 2 ) + MAX_BITS / 2;
		n = FixDiv( (int32) random( 360 ) << 16, 0x00394FEF /* 2pi / 360 */ );
		ni = FixDiv( 0x0006487F /* 2pi */, (int32) f->num_bits << 16 );
		for (  b = f->bit, j = f->num_bits; j; --j, ++b ) {
			b->flying = TRUE;
			SetRect( &b->r, 0, 0, 0, 0 );
			b->size = random( 10 ) + 5;
			sp = (int32) b->size << 16;
			b->size = (15 - b->size) / 5 + 1;
			b->dx = FixMul( Frac2Fix( FracCos( n ) ), sp );
			b->dy = FixMul( Frac2Fix( FracSin( n ) ), sp );
			n += ni;
		}
	}

	if ( snd ) {
		chan = NULL;
		if ( !SndNewChannel( &chan, 0, 0, NULL ) )
			SndPlay( chan, snd, TRUE );
	}
	start_ticks = TickCount();

	sx = FixDiv( window_right, 2L << 16 ) + (21L << 16);
	sy = FixDiv( window_bottom, 2L << 16 ) - (165L << 16);
	byx = FixDiv( window_right, 2L << 16 ) - (65L << 16);
	byy = FixDiv( window_bottom, 2L << 16 ) + (19L << 16);
	swirl_delay = SWIRL_DELAY;
	if ( sy < 0 )
		swirl_delay = swirl_delay - ( FixMul( (175L << 16) + sy, 0x00002800 ) >> 16 );
	next_launch = start_ticks + swirl_delay;
	got_beat = FALSE;
	launch_c = 0;
	num_active = MAX_FLYING;
	inter_bit_delay = 1;
	while ( !Button() && num_active ) {
		for ( f = fly, i = MAX_FLYING; i; --i, ++f ) {

			switch ( f->stage ) {
			case 0:
				if ( got_beat ) {
					f->stage = 2;
					continue;
				}
				if ( TickCount() < start_ticks + FIRST_BEAT_DELAY )
					continue;

				got_beat = TRUE;
				f->r.top = -40;
				f->r.bottom = -40 + CELL_HEIGHT * 2;
				f->r.left = the_window->portRect.right / 2 - 121;
				f->r.right = f->r.left + 242;
				f->stage++;
				f->next_tick = 0;
				break;

			case 1: {
				Rect r;

				if ( TickCount() < f->next_tick )
					continue;

				f->next_tick = TickCount() + TICK_FREQ * 2;
				ScrollRect( &f->r, 0, 1, the_window->clipRgn );
				OffsetRect( &f->r, 0, 1 );

				if ( f->r.top < 1 ) {
					r = f->r;
					set_fore_color_or_pattern( BLACK_COLOR );
					MoveTo( r.left + 20, r.top + CELL_HEIGHT - 2 );
					DrawString( (StringPtr) "\pLocked" );
					MoveTo( r.left + 100, r.top + CELL_HEIGHT - 2 );
					DrawString( (StringPtr) "\pUnlocked" );
					MoveTo( r.left + 180, r.top + CELL_HEIGHT - 2 );
					DrawString( (StringPtr) "\pFree" );

					r.top += CELL_HEIGHT + 2;
					r.bottom -= 2;
					set_fore_color_or_pattern( HEAP_BORDER_COLOR );
					FrameRect( &r );
					InsetRect( &r, 1, 1 );
	
					r.left = r.right - 80;
					set_fore_color_or_pattern( HEAP_FREE_COLOR );
					PaintRect( &r );
					r.left -= 80;
					r.right -= 80;
					set_fore_color_or_pattern( HEAP_UNLOCKED_COLOR );
					PaintRect( &r );
					r.left -= 80;
					r.right -= 80;
					set_fore_color_or_pattern( HEAP_LOCKED_COLOR );
					PaintRect( &r );
				}
				else {
					RgnHandle aRgn;
					
					ClipRect( &the_window->portRect );
					aRgn = NewRgn();
					r = f->r;
					r.top += CELL_HEIGHT + 2;
					r.bottom -= 2;
					RectRgn( aRgn, &r );
					DiffRgn( the_window->clipRgn, aRgn, the_window->clipRgn );

					r.top = f->r.top + 5;
					r.bottom = r.top + CHAR_HEIGHT;
					r.left = f->r.left + 20;
					r.right = r.left + StringWidth( (StringPtr) "\pLocked" );
					RectRgn( aRgn, &r );
					DiffRgn( the_window->clipRgn, aRgn, the_window->clipRgn );

					r.left = f->r.left + 100;
					r.right = r.left + StringWidth( (StringPtr) "\pUnlocked" );
					RectRgn( aRgn, &r );
					DiffRgn( the_window->clipRgn, aRgn, the_window->clipRgn );

					r.left = f->r.left + 180;
					r.right = r.left + StringWidth( (StringPtr) "\pFree" );
					RectRgn( aRgn, &r );
					DiffRgn( the_window->clipRgn, aRgn, the_window->clipRgn );

					DisposeRgn( aRgn );
					f->stage++;
					continue;
				}
				ClipRect( &the_window->portRect );
				break;
			}

			case 2:
				if ( TickCount() < next_launch )
					continue;

				f->stage++;
				if ( launch_c < 20 ) {
					f->maxx = 18L << 16;
					f->maxy = 18L << 16;
					f->dx = -f->maxx;
					f->dy = 0;
					f->x = sx;
					f->y = sy;
					f->ddx = -1L << 16;
					f->ddy = 1L << 16;
					f->dmax = 0x1DFF;
					sx += (int32) CHAR_WIDTH << 16;
					f->next_tick = 0;
				}
				else {
					f->maxx = 0;
					f->maxy = 0;
					f->dx = 0;
					f->dy = 0;
					f->x = byx;
					f->y = byy;
					f->ddx = 0;
					f->ddy = 0;
					f->dmax = 0;
					byx += (int32) CHAR_WIDTH << 16;
					f->next_tick = start_ticks + BY_DELAY + random( 100 );
				}
				f->c = credits[launch_c++];
				SetRect( &f->r, 0, 0, 0, 0 );
				next_launch = TickCount() + LAUNCH_FREQ;
				break;

			case 3:
				if ( TickCount() < f->next_tick )
					continue;

				f->next_tick = TickCount() + TICK_FREQ;
	
				f->x += f->dx;
				f->y += f->dy;
				f->dx += f->ddx;
				if ( f->ddx > 0 && f->dx >= f->maxx ) {
					f->ddx = -f->ddx;
					f->dx = f->maxx;
				}
				else if (f->ddx < 0 && f->dx <= -f->maxx) {
					f->ddx = -f->ddx;
					f->dx = -f->maxx;
				}
				f->dy += f->ddy;
				if ( f->ddy > 0 && f->dy >= f->maxy ) {
					f->ddy = -f->ddy;
					f->dy = f->maxy;
				}
				else if (f->ddy < 0 && f->dy <= -f->maxy) {
					f->ddy = -f->ddy;
					f->dy = -f->maxy;
				}
				f->maxx -= f->dmax;
				f->maxy -= f->dmax;
			
				EraseRect( &f->r );
				f->r.left = f->x >> 16;
				f->r.bottom = (f->y >> 16) + 2;
				f->r.right = f->r.left + CHAR_WIDTH;
				f->r.top = f->r.bottom - CHAR_HEIGHT;
		
				MoveTo( f->r.left, f->r.bottom - 2 );
				DrawChar( f->c );

				if ( f->maxx <= 0 || f->maxy <= 0 ) {
					f->stage++;
					for ( j = f->num_bits, b = f->bit; j; --j, ++b ) {
						b->x = f->x + (CHAR_WIDTH / 2 << 16);
						b->y = f->y - (CHAR_HEIGHT / 2 << 16);
					}
				}
				break;

			case 4:
				if ( TickCount() < start_ticks + BIT_DELAY )
					continue;

				f->next_tick = TickCount() + random( inter_bit_delay );
				if ( inter_bit_delay < INTER_BIT_DELAY )
					inter_bit_delay += INTER_BIT_INC;
				f->stage++;
			/* fall into next */
			case 5:
				if ( TickCount() < f->next_tick )
					continue;

				f->r.left = f->x >> 16;
				f->r.bottom = (f->y >> 16) + 2;
				f->r.right = f->r.left + CHAR_WIDTH;
				f->r.top = f->r.bottom - CHAR_HEIGHT;
				EraseRect( &f->r );
				f->stage++;
			/* fall into next */
			case 6:
				if ( TickCount() < f->next_tick )
					continue;

				for ( j = f->num_bits, b = f->bit; j; --j, ++b ) {
					if ( b->flying ) {
						EraseRect( &b->r );
	
						b->x += b->dx;
						b->y += b->dy;
						if ( b->x < 0 || b->x > window_right ||
								b->y < 0 || b->y > window_bottom ) {
							b->flying = FALSE;
							if ( !--f->num_bits2 ) {
								f->stage++;
								f->next_tick = start_ticks + CREDITS2_DELAY;
								continue;
							}
						}
						else {
							b->r.left = b->x >> 16;
							b->r.right = b->r.left + b->size;
							b->r.top = b->y >> 16;
							b->r.bottom = b->r.top + b->size;
							PaintRect( &b->r );
						}
					}
				}
				f->next_tick = TickCount() + 2;
				break;

			case 7:
				if ( TickCount() >= start_ticks + ERASE2_DELAY ) {
					f->stage++;
					--num_active;
					continue;
				}

				if ( TickCount() < f->next_tick )
					continue;
				MoveTo( ( the_window->portRect.right - StringWidth( (StringPtr) credits2 ) ) / 2,
						the_window->portRect.bottom - 20 );
				DrawString( (StringPtr) credits2 );
				f->next_tick = TickCount() + ERASE2_DELAY; /* never again */
				break;

			default:
				break;
			}
		}

		
	}

	ticker = start_ticks + ERASE2_DELAY;
	while ( !Button() && TickCount() < ticker );

	ClipRect( &the_window->portRect );

	if ( Use_color ) {
		Rect r;

		set_back_color( BACK_COLOR );
		set_fore_color( BLACK_COLOR );

		r = the_window->portRect;
		while ( r.top < the_window->portRect.bottom ) {
			r.bottom = r.top + CELL_HEIGHT;
			EraseRect( &r );
			r.top += CELL_HEIGHT;
			ticker = TickCount() + 3;
			while ( TickCount() < ticker );
		}
	}
	else
		EraseRect( &the_window->portRect );
	InvalRect( &the_window->portRect );

	DisposPtr( (Ptr) fly );
	if ( snd ) {
		SndDisposeChannel( chan, TRUE );
		Dispose_riff( snd );
	}
	FlushEvents( mUpMask | mDownMask | keyDownMask | autoKeyMask | keyUpMask, 0 );
	ShowCursor();
}


/**-----------------------------------------------------------------------------
 **
 **	Private Functions
 **
 **/


int16 random( int16 max )
{
	int16 x;

	x = Random();
	if ( x < 0 )
		x = -x;
	return x % max;
}
