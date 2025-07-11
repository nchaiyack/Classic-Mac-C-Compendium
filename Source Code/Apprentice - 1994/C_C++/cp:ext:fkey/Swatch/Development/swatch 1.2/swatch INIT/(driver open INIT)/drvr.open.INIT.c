/**

	open.init.c
	Copyright (c) 1990, joe holt

 **/


/**-----------------------------------------------------------------------------
 **
 **	Compilation Flags
 **
 **/

/**

	MULTI_SEGMENT generates code which is needed to init a multi-segmented
	driver.  Single-segment drivers can be inited also, so there is no logical
	requirement to ever turn this off, except where code size is a concern.

 **/

#define MULTI_SEGMENT	1


/**-----------------------------------------------------------------------------
 **
 **	Headers
 **
 **/

#include <types.h>
#include "ShowINIT.h"


/**-----------------------------------------------------------------------------
 **
 **	Private Constants
 **
 **/

/**

	UNIT_ENTRIES_NEEDED and FIRST_POSSIBLE_UNIT are used when determining the
	driver reference number for our driver.  We assign our driver reference
	number dynamically, looking at FIRST_POSSIBLE_UNIT and on up for an unused
	number.  We increase the unit table size to UNIT_ENTRIES_NEEDED if
	there aren't that many unit numbers, using a technique gleaned from ADSP.
	This occurs on Mac Pluses and Mac SEs, where the unit table defaults to
	forty-eight entries, which doesn't leave any room for custom drivers like
	ourself.

**/

#define UNIT_ENTRIES_NEEDED		(64)
#define FIRST_POSSIBLE_UNIT		(48)

#define INSTALLED_ICN_			(128)
#define NOT_INSTALLED_ICN_		(129)
#define ERROR_INSTALLING_ICN_	(130)


/**-----------------------------------------------------------------------------
 **
 **	Private Functions
 **
 **/

int main(void);

main()
{
	char driver_name[128];
	register Handle h;
	register DCtlHandle *d;
	DCtlPtr dp;
	register int16 the_ID, i, icon;
	int16 res_ID;
	IOParam open_driver_PB;
	KeyMap keymap;

	asm {
				move.l	A4, -(A7)
				movea.l	A0, A4
	}

	/**
	 **   If mouse button or shift key is down, don't install
	 **/

	GetKeys( &keymap );
	if ( Button() || (keymap.Key[1] & 1 /* shift key */) ) {
		icon = NOT_INSTALLED_ICN_;
		goto exit;
	}

	icon = ERROR_INSTALLING_ICN_;
	SetResLoad( FALSE );
	if ( !( h = Get1IndResource( 'DRVR', 1 ) ) )
		goto exit;

	SetResAttrs( h, resSysHeap | resLocked );
	ReleaseResource( h );


/**

	Pick an unused driver reference number for our DRVR and renumber the resource
	to use this new reference number.  We first increase the number of entries
	in the unit table if less than UNIT_ENTRIES_NEEDED, then begin looking at
	FIRST_POSSIBLE_UNIT.  This strategy was copied from the ADSP INIT.

 **/

	SetResLoad( TRUE );
	h = Get1IndResource( 'DRVR', 1 );
	GetResInfo( h, NULL, NULL, driver_name );
	DetachResource( h );
	if ( UnitNtryCnt < UNIT_ENTRIES_NEEDED ) {
		asm {
;
;	Increase the number of available entries in the unit table.
;
				move.w	#UNIT_ENTRIES_NEEDED * 4, D0
				_NewPtr	SYS+CLEAR
				bne		@exit
	
				move	SR, -(A7)
				move	#0x2600, SR
				movea.l	A0, A1
				movea.l	UTableBase, A0
				move.w	UnitNtryCnt, D0
				mulu	#4, D0
				_BlockMove
				_DisposPtr
				move.l	A1, UTableBase
				move.w	#UNIT_ENTRIES_NEEDED, UnitNtryCnt
				move	(A7)+, SR
		}
	}

	d = (DCtlHandle *) UTableBase + FIRST_POSSIBLE_UNIT;
	for ( the_ID = FIRST_POSSIBLE_UNIT; *d && the_ID <= UnitNtryCnt; ++d, ++the_ID );
	if ( the_ID > UnitNtryCnt )
		goto exit;

	asm {
				move.w	the_ID, D0
				not.w	D0
				movea.l	(h), A0
				dc.w	0xA43D				; _DrvrInstall 4
				bne		@exit
	}

	HLock( *d );
	dp = **d;
	dp->dCtlDriver = (Ptr) h;
	dp->dCtlFlags = **(int16 **) h | 0x40;

/**

	Now set the resources which THINK C uses for drivers to load into the System
	Heap as locked, also.  We especially want to lock down any DCOD resources
	because the THINK C segment loader does a MoveHHi() on 'em when first loaded,
	which is not a good idea in the System Heap at INIT time.

 **/

	the_ID <<= 5;

	SetResLoad( FALSE );
	if ( ( h = Get1IndResource( 'DATA', 1 ) ) ) {
		SetResInfo( h, the_ID | 0xC000, NULL );
		SetResAttrs( h, resSysHeap | resLocked );
		ReleaseResource( h );
	}

#if MULTI_SEGMENT
	for ( i = Count1Resources( 'DCOD' ); i; --i ) {
		if ( !( h = Get1IndResource( 'DCOD', i ) ) )
			goto exit;
		GetResInfo( h, &res_ID, NULL, NULL );
		SetResInfo( h, the_ID | (res_ID & 0xF01F), NULL );
		SetResAttrs( h, resSysHeap | resLocked );
		ReleaseResource( h );
	}
#endif

	SetResLoad( TRUE );

	open_driver_PB.ioNamePtr = (StringPtr) driver_name;
	open_driver_PB.ioPermssn = 0;
	if ( !PBOpen( &open_driver_PB, FALSE ) )
		icon = INSTALLED_ICN_;
	
exit:
	SetResLoad( TRUE );
	ShowINIT( icon, -1 );
	asm {
				move.l	(A7)+, A4
	}
}
