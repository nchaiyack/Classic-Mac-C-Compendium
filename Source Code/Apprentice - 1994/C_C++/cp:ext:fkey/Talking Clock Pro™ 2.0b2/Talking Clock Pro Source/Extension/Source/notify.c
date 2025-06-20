/*
 * Notify.c
 */

#include <Types.h>

#include "notify.h"
#include "str.h"

/*
 * We use ONE block of memory containing the following:
 *
 * Response procedure (14 bytes)
 *	this procedure removes the NMRec and disposes the memory
 * Pointer to NMRec (not really needed since the rec is in the block...)
 * String to show in the notification alert
 * Notification record (NMRec)
 *
 * This is so we can put up the notification (in the system heap) and then
 * quit the application and still leave no memory undisposed when the
 * notification is answered by the user
 */

#include <Memory.h>
#include <Notification.h>

#define LEN 14

unsigned short Dummy [ ] = { 0x2f08 , 0x2068 , 0x000e , 0xa05f , 0x205f , 0xa01f , 0x4e75 } ;
#if 0
static void
Dummy ( void ) {
	asm {
		move.l	a0,-(a7)
		move.l	LEN(a0),a0
		dc.w		0xA05F		; _NMRemove
		move.l	(a7)+,a0
		dc.w		0xa01f		; _DisposPtr
	}
}
#endif


void
NotifyStr ( unsigned char * str ) {

char * ptr = NewPtrSysClear ( 265 + LEN + sizeof ( void * ) + sizeof ( NMRec ) ) ;
NMRecPtr recPtr = ( NMRecPtr ) ( ptr + 256 + LEN + sizeof ( void * ) ) ;

	BlockMove ( & recPtr , ptr + LEN , sizeof ( void * ) ) ;
	BlockMove ( Dummy , ptr , LEN ) ; /* Yeah, really portable for sure --- NOT! */
	recPtr -> qType = nmType ;
	recPtr -> nmStr = ( unsigned char * ) ptr + LEN + sizeof ( void * ) ;
	recPtr -> nmResp = ( void * ) ptr ;
	CopyPString ( str , recPtr -> nmStr ) ;
	( void ) NMInstall ( recPtr ) ;
}
