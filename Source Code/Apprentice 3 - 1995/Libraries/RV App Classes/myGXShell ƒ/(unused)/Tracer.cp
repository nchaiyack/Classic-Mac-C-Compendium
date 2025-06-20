THIS CODE IS UNFINISHED AND UNTESTED

#include "Tracer.h"

Tracer::Tracer()
{
	GetMouse( &dotLocation);
	PenMode( patXor);
	PenSize( 5, 5);
	InvertDot();
}

Tracer::Tracer( const Point startLocation)
{
	PenMode( patXor);
	PenSize( 5, 5);
	dotLocation = startLocation;
	InvertDot();
}

Tracer::~Tracer()
{
	InvertDot();
}

void Tracer::Move()
{
	Point newLocation;
	GetMouse( &newLocation);
	Move( newLocation);
}

void Tracer::Move( const Point newLocation)
{
	if( *(long *)&dotLocation != *(long *)&newLocation)
	{
		InvertDot();
		dotLocation = newLocation;
		InvertDot();
	}
}

Point Tracer::operator()() const
{
	return dotLocation;
}

void Tracer::InvertDot()
{
	MoveTo( dotLocation.h - 2, dotLocation.v - 2);
	Line( 0, 0);
}
