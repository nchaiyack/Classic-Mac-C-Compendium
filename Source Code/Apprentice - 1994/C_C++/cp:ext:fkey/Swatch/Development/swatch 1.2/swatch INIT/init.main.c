/**

	init.main.c
	Copyright (c) 1990, joe holt

 **/


/**-----------------------------------------------------------------------------
 **
 ** Headers
 **
 **/

#include "init.resources.h"
/*#include "init.types.h"*/

#include "drvr.csControl.h"
#include "drvr.csOpen.h"
#include "init.main.h"


/**-----------------------------------------------------------------------------
 **
 ** Private Functions
 **
 **/

Handle GetAndDetachResource( OSType type, int16 id );
void DisposValidHandle( Handle h );

OSErr main( CntrlParam *cpb, DCtlPtr dce, int16 n );


/*******************************************************************************
 **
 **	Public Functions
 **
 **/

OSErr main( CntrlParam *cpb, DCtlPtr dce, int16 n )
{
	OSErr ret;

	asm {
			move.l	A4, D0
			_StripAddress
			move.l	D0, A4
	}
	ret = noErr;
	switch ( n ) {
	case 0:
		ret = csOpen( cpb, dce );
		break;

	case 2:
		ret = csControl( cpb, dce );
		break;

	case 4:
		ret = closeErr;		/* don't ever close */
		break;

	default:
		break;
	}
	return ret;	
}


#if 0
Handle GetAndDetachResource( OSType type, int16 id )
{
	Handle		res;

	if (!(res = Get1Resource(type, id)) || HandleZone(res) != SysZone)
		return (NULL);
	DetachResource(res);
	return (res);
}


void DisposValidHandle( Handle h )
{
	if (h)
		DisposHandle(h);
}
#endif