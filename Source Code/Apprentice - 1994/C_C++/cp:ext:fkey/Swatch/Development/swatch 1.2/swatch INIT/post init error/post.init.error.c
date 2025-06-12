/**

	post.init.error.c
	Copyright (c) 1990, joe holt

 **/


/**-------------------------------------------------------------------------
 **
 ** Headers
 **
 **/

#include "post.init.error.h"


/**-------------------------------------------------------------------------
 **
 ** Private Macros
 **
 **/

/**

	TRAP_NMInstall and TRAP_Unimplemented are Mac toolbox trap numbers used
	to test for the existence of the Notification Manager.

 **/

#define TRAP_NMInstall			0xA05E
#define TRAP_Unimplemented		0xA89F

/**

	INIT_ERROR_STR_ is the resource ID of the STR# containing the
	error message corresponding to the error number passed to
	Post_init_error().

	RESPONSE_CODE is the resource ID of the code resource compiled
	separately and stuck in your INIT's resources.

 **/

#define INIT_ERROR_STR_		1000
#define INIT_RESPONSE_CODE	1000


/***************************************************************************
 ***
 *** void Post_init_error( int16 error_strx );
 ***
 *** When your INIT runs into a problem, clean things up, show the X'ed
 *** version of your icon and call Post_init_error() with an error number
 *** corresponding to the message you want displayed.
 ***
 *** History:
 ***   jhh 18 jun 90 -- response to news posting
 ***
 ***/

void Post_init_error( int16 error_strx )
{
	register NMRec *note;
	register Handle response_code;
	register long size;
	register THz svZone;
	Str255 error_text;

/**

	Make sure we've got a Notification Manager.

 **/

	if ( NGetTrapAddress( TRAP_NMInstall, OSTrap ) !=
			NGetTrapAddress( TRAP_Unimplemented, ToolTrap ) ) {

/**

	All of the memory we allocate from here on out is in the System
	Heap.  First create a Notification Manager record and fill it in.
	Note that you can expand this notification method with sounds and
	icons by adding the appropriate code.  See the Notification
	Manager technote #184 for details.

 **/

		svZone = TheZone;
		TheZone = SysZone;
		note = (NMRec *) NewPtr( sizeof( NMRec ) );
		if ( !note )
			goto exit;
		note->qType = nmType;
		note->nmMark = 0;
		note->nmSIcon = 0L;
		note->nmSound = (Handle) -1;

/**

	Get the error message corresponding to the error number given.
	For maximum performance, the STR# resource should be tagged
	"Preload" and not "System Heap".  This way, you can be sure
	the messages will be there even if memory space is the cause of
	the error.

	We create a pointer in the System Heap just big enough for the
	string and copy the string into it.  Point the NM record at this
	string.

 **/

		GetIndString( error_text, INIT_ERROR_STR_, error_strx );
		size = *(unsigned char *) error_text + 1;
		note->nmStr = (StringPtr) NewPtr( size );
		if ( !note->nmStr ) {
			DisposPtr( note );
			goto exit;
		}
		BlockMove( error_text, note->nmStr, size );

/**

	The response procedure also must be in a pointer in the System
	Heap.  You need to include the compiled code resource in your
	INIT's resources of type 'CODE'.

	Create a pointer just big enough for it and point the NM record
	at it, also.

 **/

		response_code = GetResource( 'CODE', INIT_RESPONSE_CODE );
		if ( !response_code ) {
			DisposPtr( note->nmStr );
			DisposPtr( note );
			goto exit;
		}
		size = GetHandleSize( response_code );
		note->nmResp = (ProcPtr) NewPtr( size );
		if ( !note->nmResp ) {
			DisposPtr( note->nmStr );
			DisposPtr( note );
			goto exit;
		}
		BlockMove( *response_code, note->nmResp, size );

/**

	Now post the note.  As soon as startup is complete, the NM
	will display the note for the user's edification.  Hurrah.

 **/

		NMInstall( note );
exit:
		TheZone = svZone;
	}
}