THIS CODE IS UNFINISHED AND UNTESTED

#include <Types.h>
#include <QuickDraw.h>
#include <Events.h>

#include "dotMover.h"

dotMover::dotMover()
{
	GetMouse( &startingpoint);
	current_endpoint = startingpoint;
	visible = false;
}

dotMover::dotMover( const Point &startpoint)
{
	startingpoint    = startpoint;
	current_endpoint = startpoint;
	visible = false;
}

dotMover::~dotMover()
{
	hide();
}

void dotMover::moveto( const Point &newposition)
{
	if( *(long *)&current_endpoint != *(const long *)&newposition)
	{
		if( visible)
		{
			draw_once();
			LineTo( newposition.h, newposition.v);
		}
		current_endpoint = newposition;
	}
}

void dotMover::followMouse()
{
	Point where;
	GetMouse( &where);
	moveto( where);
}

void dotMover::hide()
{
	if( visible)
	{
		visible = false;
		draw_once();
	}
}

void dotMover::show()
{
	if( !visible)
	{
		visible = true;
		draw_once();
	}
}

void dotMover::draw_once() const
{
	PenMode( patXor);
	Rect theRect = { -2, -2, 2, 2};
	OffsetRect( &theRect, current_endpoint.h, current_endpoint.v);
	PaintRect( &theRect);
}
