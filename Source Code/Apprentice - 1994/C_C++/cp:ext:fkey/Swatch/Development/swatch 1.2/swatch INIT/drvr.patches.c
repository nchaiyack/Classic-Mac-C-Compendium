/**

	drvr.patches.c
	Copyright (c) 1990, Adobe Systems, Inc.

 **/


/**-----------------------------------------------------------------------------
 **
 **	Headers
 **
 **/

#include "drvr.globals.h"
#include "drvr.patches.h"


/**-----------------------------------------------------------------------------
 **
 ** Private Constants
 **
 **/

#define TRAP_OpenResFile	0xA997
#define TRAP_CloseResFile	0xA99A


/**-----------------------------------------------------------------------------
 **
 ** Private Variables
 **
 **/

void *OpenResFile_orig_addr;
void *CloseResFile_orig_addr;


/**-----------------------------------------------------------------------------
 **
 ** Private Variables
 **
 **/

void OpenResFile_patch( void );
void CloseResFile_patch( void );


/*******************************************************************************
 **
 **	Public Functions
 **
 **/

/*******************************************************************************
 ***
 *** OSErr Install_patches( void );
 ***
 *** summary
 ***
 *** History:
 ***
 ***/

OSErr Install_patches( void )
{
	register App_stat_t *a;
	register int16 i;

	asm {
				lea		@swatch_A4, A0
				move.l	A4, (A0)
	}
	Globals.version = VERSION;
	Globals.max_apps = MAX_APPS;
	Globals.zero = 0;
	Globals.num_apps = 0;
	Globals.apps_need_updating = FALSE;
	if ( !( a = (App_stat_t *) NewPtr( (int32) sizeof( App_stat_t ) * MAX_APPS ) ) )
		return MemError();
	Globals.apps = a;
	for ( i = MAX_APPS; i; ++a, --i )
		a->free = TRUE;

	OpenResFile_orig_addr = (void *) NGetTrapAddress( TRAP_OpenResFile, ToolTrap );
	NSetTrapAddress( OpenResFile_patch, TRAP_OpenResFile, ToolTrap );

	CloseResFile_orig_addr = (void *) NGetTrapAddress( TRAP_CloseResFile, ToolTrap );
	NSetTrapAddress( CloseResFile_patch, TRAP_CloseResFile, ToolTrap );

	return noErr;

	asm {
swatch_A4:		dc.l	0

extern OpenResFile_patch:
				movem.l	A0/A1/A4, -(A7)
				lea		@swatch_A4, A4			; get our app's globals
				movea.l	(A4), A4

				movea.l	0x10(A7), A0			; get name of res file
				lea		CurApName, A1			; are we opening the current app?
				moveq	#0, D0
				move.b	(A0), D0
@open2:			cmpm.b	(A0)+, (A1)+
				dbne	D0, @open2

				bne.s	@open9

				move.w	#MAX_APPS, D0			; yes, find avail slot and fill it in
				cmp.w	Globals.num_apps, D0
				beq.s	@open9					; app table full

				subq.w	#1, D0
				movea.l	Globals.apps, A1
@open4:			tst.b	OFFSET( App_stat_t, free )(A1)
				bne.s	@open5					; found a free slot

				adda.w	#sizeof( App_stat_t ), A1
				dbra	D0, @open4

				bra.s	@open9					; should never happen...

@open5:			clr.b	OFFSET( App_stat_t, free )(A1)		; set up stats
				move.l	ApplZone, OFFSET( App_stat_t, zone )(A1)
				lea		CurApName, A0
				lea		OFFSET( App_stat_t, appname )(A1), A1
				moveq	#0, D0
				move.b	(A0), D0
@open6:			move.b	(A0)+, (A1)+
				dbra	D0, @open6

				addq.w	#1, Globals.num_apps
				move.b	#TRUE, Globals.apps_need_updating

@open9:			move.l	OpenResFile_orig_addr, D0	; do the _OpenResFile
				movem.l	(A7)+, A0/A1/A4
				move.l	D0, -(A7)
				rts


extern CloseResFile_patch:
				movem.l	D1/A0-A2/A4, -(A7)
				lea		@swatch_A4, A4			; get our app's globals
				movea.l	(A4), A4

				move.w	CurApRefNum, D0
				cmp.w	0x18(A7), D0			; are we closing the current app?
				bne.s	@close9

				move.w	#MAX_APPS-1, D0			; yes, find slot and free it
				movea.l	Globals.apps, A1
@close1:		tst.b	OFFSET( App_stat_t, free )(A1)
				bne.s	@close3

				lea		CurApName, A0			; is this the app?
				lea		OFFSET( App_stat_t, appname )(A1), A2
				moveq	#0, D1
				move.b	(A0), D1
@close2			cmpm.b	(A0)+, (A2)+
				dbne	D1, @close2

				bne.s	@close3					; no, keep looking

				move.b	#TRUE, OFFSET( App_stat_t, free )(A1)	; yes, make free
				subq.w	#1, Globals.num_apps
				move.b	#TRUE, Globals.apps_need_updating
				bra.s	@close9

@close3:		adda.w	#sizeof( App_stat_t ), A1
				dbra	D0, @close1
				
@close9:		move.l	CloseResFile_orig_addr, D0	; do the _CloseResFile
				movem.l	(A7)+, D1/A0-A2/A4
				move.l	D0, -(A7)
				rts
	}
}
