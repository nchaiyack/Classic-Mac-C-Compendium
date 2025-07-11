
#include "queue.h"

template<class C> queue<C>::queue( unsigned int numItems) : size( numItems)
{
	theItems = new C[ size];
	F = 0;
	R = 0;
	numInside = 0;
}

template<class C> queue<C>::~queue()
{
	delete theItems;
}

template<class C> void queue<C>::Add( C newItem)
{
	if( !isFull())
	{
		R += 1;
		if( R == size)
		{
			R = 0;
		}
		theItems[ R] = newItem;
		numInside += 1;
	}
}

template<class C> C queue<C>::Remove()
{
	//
	// Note: this code _assumes_ that class C is a pointer type, or at least
	// that zero is a valid value for a 'C' type instance.
	//
	C result = 0;
	if( !isEmpty())
	{	
		F += 1;
		if( F == size)
		{
			F = 0;
		}
		result = theItems[ F];
		numInside -= 1;
	}
	return result;
}
