/*/
     Project Arashi: VASins.c
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, September 9, 1992, 21:42
     Created: Sunday, February 12, 1989, 22:47

     Copyright � 1989-1992, Juri Munkki
/*/

#define	_VASINS_
#include "VA.h"
#include "VAInternal.h"

#define PI 3.1415926535

int		errno;

#undef	_ERRORCHECK_
#define	_NOERRORCHECK_
#include <Math.h>

void	VAInitSins()
{
	register	int		a;
	
	for(a=0;a<ANGLES;a++)
	{	Sins[a]=sin(a*2*PI/ANGLES)*256;
		Cosins[a]=cos(a*2*PI/ANGLES)*256;
	}
}
