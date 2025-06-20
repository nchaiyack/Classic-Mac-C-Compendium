THIS CODE IS UNFINISHED AND UNTESTED

#include <Types.h>
#include <QuickDraw.h>
#include <Events.h>

#include "rubberband.h"

rubberband::rubberband( const Point &startpoint)
{
	startingpoint    = startpoint;
	current_endpoint = startpoint;
	visible = false;
}

rubberband::~rubberband()
{
	hide();
}

void rubberband::moveto( const Point &newposition)
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

void rubberband::followMouse()
{
	Point where;
	GetMouse( &where);
	moveto( where);
}

void rubberband::hide()
{
	if( visible)
	{
		visible = false;
		draw_once();
	}
}

void rubberband::show()
{
	if( !visible)
	{
		visible = true;
		draw_once();
	}
}

void rubberband::draw_once() const
{
	PenMode( patXor);
	MoveTo( current_endpoint.h, current_endpoint.v);
	LineTo( startingpoint.h, startingpoint.v);
}
