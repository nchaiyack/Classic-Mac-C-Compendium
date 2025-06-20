/*/
     Project Arashi: Lenses.c
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, September 9, 1992, 21:44
     Created: Tuesday, January 3, 1989, 22:50

     Copyright � 1989-1992, Juri Munkki
/*/

#include <SerialDvr.h>

/*
>>	SetLenses sets the status of the lenses.
*/
void	SetGlasses(port,left,right)
int		port,left,right;
{	
	port=-6-port-port;
	
	if(right)
		Control(port,17,0);
	else
		Control(port,18,0);

	if(left)
		SerSetBrk(port);
	else
		SerClrBrk(port);
}

void	InitGlasses(port)	/*	0 for modem, 1 for printer			*/
int		port;
{
	RAMSDOpen(port);
	SetGlasses(port,1,1);
}

void	CloseGlasses(port)
{
	SetGlasses(port,1,1);
	RAMSDClose(port);
}
