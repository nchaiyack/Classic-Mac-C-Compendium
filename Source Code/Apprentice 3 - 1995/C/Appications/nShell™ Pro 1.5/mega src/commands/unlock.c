/* ========== the commmand file: ==========

	unlock.c
	
	Copyright (c) 1994 Newport Software Development
	
	You may distribute unmodified copies of this file for
	noncommercial purposes.  You may use this file as a
	reference when writing your own nShell(tm) commands.
	
	All other rights are reserved.
	
   ========== the commmand file: ========== */

//
// This code can build to be "lock" or "unlock".
//
// To build "lock", define LOCK below.
//
// To build "unlock", define UNLOCK below.
//
// But only define one!
//

#define  UNLOCK

#ifdef __MWERKS__			// CodeWarrior requires an A4 setup
#include <A4Stuff.h>
#endif

#include <GestaltEqu.h>

#include "nshc.h"

#include "arg_utl.proto.h"
#include "nshc_utl.proto.h"
#include "fss_utl.proto.h"
#include "str_utl.proto.h"

// data definition - this struct is the root of all data

typedef struct {

	int		is_seven;		// 0 if this is a pre-System 7 environment
	int		arg;			// position in arg list

} t_unlock_data;

typedef	t_unlock_data	**unlock_hndl;

/* ======================================== */

// prototypes - utility

int  unlock_is_seven( void );
void unlock_bad( t_nshc_parms *nshc_parms, int code );
void unlock_bad_file( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, StringPtr msg );
void unlock_good( t_nshc_parms *nshc_parms );

// prototypes - file routines

void  unlock_one( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_unlock_data **hData );

// prototypes - state machine

void unlock_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls  );
void unlock_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
void unlock_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );

/* ======================================== */

// return true when OS is system 7 or later

int	unlock_is_seven( void )
{
	long	response;
	
	if ( Gestalt( 'sysv', &response ) )
		return( 1 );
		
	if ( response < 0x700 )
		return( 1 );
	else
		return( 0 );
}

/* ======================================== */

void unlock_bad(  t_nshc_parms *nshc_parms, int code )
{
	nshc_parms->action = nsh_stop;
	nshc_parms->result = code;
}
	
/* ======================================== */

void unlock_bad_file(  t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, StringPtr msg )
{

	nshc_calls->NSH_putStr_err("\p(un)lock: File access error (");
	nshc_calls->NSH_putStr_err(msg);
	nshc_calls->NSH_putStr_err("\p)\r");

	nshc_parms->action = nsh_stop;
	nshc_parms->result = NSHC_ERR_GENERAL;
}
	
/* ======================================== */

void unlock_good(  t_nshc_parms *nshc_parms )
{
	nshc_parms->action = nsh_stop;
	nshc_parms->result = 0;
}

/* ======================================== */

// file access routines

/* ======================================== */

void unlock_one( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, t_unlock_data **hData )
{
	int			result;
	int			dont_worry;
	Str255		pathname;
	FSSpec		fss;
	
	// =====> convert argument to pathname
	
	result = arg_to_str( nshc_parms, nshc_calls, (**hData).arg++, pathname );

	if (result) {
		unlock_bad( nshc_parms, result );
		return;
		}
		
	result = nshc_calls->NSH_path_expand( pathname );

	if (result) {
		unlock_bad_file(  nshc_parms, nshc_calls, pathname );
		return;
		}

	// =====> lock or unlock it

	#ifdef LOCK

		result = SetFLock( pathname, 0 );

	#else

		result = RstFLock( pathname, 0 );

	#endif

	if (!result) {		// try to wake up the parent - but don't worry

		dont_worry = nshc_calls->NSH_path_expand( pathname );

		if (!dont_worry)
			dont_worry = nshc_calls->NSH_path_to_FSSpec( pathname, &fss );

		if (!dont_worry)
			dont_worry = fss_wake_parent( &fss );

		}

	if (result)
		unlock_bad_file(  nshc_parms, nshc_calls, pathname );
}

/* ======================================== */

// state machine - core routines

/* ======================================== */

void unlock_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls  )
{
	unlock_hndl	hData;	// handle to hold our data
	int			one;
	
	// check to see that the developer defined LOCK or UNLOCK, but not both
	
	one = 0;
	
	#ifdef LOCK
		one++;
	#endif
	
	#ifdef UNLOCK
		one++;
	#endif
	
	if ( one != 1 ) {
		nshc_calls->NSH_putStr_err( "\p(un)lock: Your programmer goofed on LOCK or UNLOCK.\r" );
		unlock_bad( nshc_parms, NSHC_ERR_GENERAL );
		return;
		}
		
	// check usage

	if (nshc_parms->argc < 2) {
	
		#ifdef LOCK
		
			nshc_calls->NSH_putStr_err( "\pUsage: lock file [file...]\r" );
			
		#else
		
			nshc_calls->NSH_putStr_err( "\pUsage: unlock file [file...]\r" );
			
		#endif
		
		unlock_bad( nshc_parms, NSHC_ERR_PARMS );
		return;
		}
	
	// ok, set up data
	
	nshc_parms->action = nsh_continue;

	hData = (unlock_hndl)NewHandleClear(sizeof(t_unlock_data));
	
	if (hData) {
	
		(**hData).arg = 1;						// start at the arg = 1 position
		(**hData).is_seven = unlock_is_seven();	// test os revision
		nshc_parms->data = (Handle)hData;		// and file it
		}
	else {
		nshc_calls->NSH_putStr_err( "\p(un)lock: Could not allocate storage.\r" );
		unlock_bad( nshc_parms, NSHC_ERR_MEMORY );
		}
}

/* ======================================== */

void unlock_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	int			i;
	int			arg;
	unlock_hndl	hData;
	
	if (hData = (unlock_hndl)nshc_parms->data) {
	
		arg = (**hData).arg;
		
		if (arg >= nshc_parms->argc)
			unlock_good( nshc_parms );
		else
			unlock_one( nshc_parms, nshc_calls, hData );

		}
}

/* ======================================== */

void unlock_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	if (nshc_parms->data)
		DisposeHandle(nshc_parms->data);
		
	nshc_parms->action = nsh_idle;
}

/* ======================================== */

void main(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls)
{	
#ifdef __MWERKS__
	long oldA4  = SetCurrentA4();
#endif

	if ( !nshc_bad_version( nshc_parms, nshc_calls, NSHC_VERSION ) ) {
	
		switch (nshc_parms->action) {
			case nsh_start:
				unlock_start(nshc_parms, nshc_calls);
				break;
			case nsh_continue:
				unlock_continue(nshc_parms, nshc_calls);
				break;
			case nsh_stop:
				unlock_stop(nshc_parms, nshc_calls);
				break;
			}
		
		}
	
#ifdef __MWERKS__
	SetA4(oldA4);
#endif
}

/* ======================================== */
