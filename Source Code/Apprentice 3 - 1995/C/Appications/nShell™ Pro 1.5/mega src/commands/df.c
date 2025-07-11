/* ========== the commmand file: ==========

	df.c
		
	Copyright (c) 1994 Newport Software Development
	
	You may distribute unmodified copies of this file for
	noncommercial purposes.  You may use this file as a
	reference when writing your own nShell(tm) commands.
	
	All other rights are reserved.
	
   ========== the commmand file: ========== */

#ifdef __MWERKS__			// CodeWarrior requires an A4 setup
#include <A4Stuff.h>
#endif

#include "nshc.h"

#include "arg_utl.proto.h"
#include "str_utl.proto.h"
#include "nshc_utl.proto.h"

/* ======================================== */

// data definition - this struct is the root of all data

typedef struct {

	int		arg;			// position in arg list (used when args are given)
	short	volumeIndex;	// position in volume list (used when no args are given)
	
} t_df_data;

typedef	t_df_data	**df_hndl;

/* ======================================== */

// prototypes

// prototypes - utility

void df_bad( t_nshc_parms *nshc_parms, int code );
void df_good( t_nshc_parms *nshc_parms );
int  df_match( StringPtr from_os, StringPtr from_user );

// prototypes - file routines

void df_display( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, Str255 name, HParamBlockRec	*hfsParams);
int  df_scan( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, short volumeIndex  );
int  df_find( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, Str255 match  );

// state machine - custom routines

void df_by_arg( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, df_hndl hData );
void df_by_vol( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, df_hndl hData );

// prototypes - state machine

void df_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls  );
void df_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
void df_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );

/* ======================================== */

// utility routines

/* ======================================== */

void df_bad(  t_nshc_parms *nshc_parms, int code )
{
	nshc_parms->action = nsh_stop;
	nshc_parms->result = code;
}

void df_good(  t_nshc_parms *nshc_parms )
{
	nshc_parms->action = nsh_stop;
	nshc_parms->result = 0;
}

int df_match( StringPtr from_os, StringPtr from_user )
{
	int len;
	
	len = from_user[0];
	
	if ( from_user[len] == ':' )
		len--;
	
	if (len != from_os[0])
		return( 0 );
		
	while (len) {
		if ( from_os[len] != from_user[len] )
			return(0);
		len--;
		}
		
	return(1);
}

/* ======================================== */

// file access routines

/* ======================================== */

void df_display( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, Str255 name, HParamBlockRec *hfsParams)
{
	int		i;
	long	kbytes, used, avail;
	int		capacity;
	
	i = name[0];
	
	while (i < 27)
		name[++i] = ' ';
		
	name[0] = i;
	
	kbytes = hfsParams->volumeParam.ioVNmAlBlks;
	kbytes *= hfsParams->volumeParam.ioVAlBlkSiz;
	kbytes /= 1024;
	
	avail = hfsParams->volumeParam.ioVFrBlk;
	avail *= hfsParams->volumeParam.ioVAlBlkSiz;
	avail /= 1024;
	
	used = kbytes - avail;
	
	capacity = ( used * 100 ) / kbytes;
	
	nshc_calls->NSH_putStr( name );
	nshc_calls->NSH_printf( " %8ld %8ld %8ld", kbytes, used, avail );
	nshc_calls->NSH_printf( " %3d%%\r", capacity );
}

/* ======================================== */

int df_scan( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, short volumeIndex  )
{			
	HParamBlockRec	hfsParams;
	OSErr			myErr;
	Str255			name;

	hfsParams.volumeParam.ioNamePtr = name;
	hfsParams.volumeParam.ioVRefNum = 0;
	hfsParams.volumeParam.ioVolIndex = volumeIndex;
	
	myErr = PBHGetVInfoSync(&hfsParams);
	
	// a nsvErr indicates that the current pass is over
	
	if (myErr)
		return( 0 );
	
	// process data
	
	df_display( nshc_parms, nshc_calls, name, &hfsParams );

	return( 1 );
}

/* ======================================== */

int df_find( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, Str255 match  )
{
	HParamBlockRec	hfsParams;
	short			volumeIndex;
	OSErr			myErr;
	Str255			name;
	
	volumeIndex = 0;
	
	for (;;) {
	
		volumeIndex++;
			
		// convert the volumeIndex into a vRefNum
		
		hfsParams.volumeParam.ioNamePtr = name;
		hfsParams.volumeParam.ioVRefNum = 0;
		hfsParams.volumeParam.ioVolIndex = volumeIndex;
		
		myErr = PBHGetVInfoSync(&hfsParams);
		
		// a nsvErr indicates that the current pass is over
		
		if (myErr)
			return( 0 );
		
		// process data
		
		if ( df_match( name, match) ) {
			df_display( nshc_parms, nshc_calls, name, &hfsParams );
			return( 1 );
			}
			
		}
}

/* ======================================== */

// state machine - custom routines

/* ======================================== */

void df_by_arg( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, df_hndl hData )
{
	Str255	vName;

	if ((**hData).arg >= nshc_parms->argc)
	
		df_good( nshc_parms );
		
	else {
	
		if (!arg_to_str( nshc_parms, nshc_calls, (**hData).arg, vName ))
			if (!df_find( nshc_parms, nshc_calls, vName )) {
				nshc_calls->NSH_putStr_err( "\pdf: Volume not found = ");
				nshc_calls->NSH_putStr_err( vName );
				nshc_calls->NSH_putchar_err( '\r' );
				}
				
		(**hData).arg++;
				
		}
}

/* ======================================== */

void df_by_vol( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, df_hndl hData )
{

	if (!df_scan( nshc_parms, nshc_calls, (**hData).volumeIndex ))
		df_good(nshc_parms);
			
	(**hData).volumeIndex++;
}

/* ======================================== */

// state machine - core routines

/* ======================================== */

void df_start( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls  )
{
	df_hndl	hData;	// handle to hold our data
	
	nshc_parms->action = nsh_continue;

	hData = (df_hndl)NewHandleClear(sizeof(t_df_data));
	
	if (hData) {
		(**hData).arg = 1;					// start at the arg = 1 position
		(**hData).volumeIndex = 1;			// start at the volume = 1 position
		nshc_parms->data = (Handle)hData;
		nshc_calls->NSH_putStr( "\pfilesystem                    kbytes     used    avail  capacity\r");
		}
	else {
		nshc_calls->NSH_putStr_err( "\pdf: Could not allocate storage.\r" );
		df_bad( nshc_parms, NSHC_ERR_MEMORY );
		}
}

/* ======================================== */

void df_continue( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	df_hndl	hData;
	
	if (hData = (df_hndl)nshc_parms->data) {
	
		if ( nshc_parms->argc > 1 )		
			df_by_arg( nshc_parms, nshc_calls, hData );	// we are searching by name
		else 								
			df_by_vol( nshc_parms, nshc_calls, hData ); // we are scanning all volumes
			
		}
	else
		df_bad( nshc_parms, NSHC_ERR_MEMORY );

}

/* ======================================== */

void df_stop( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	df_hndl	hData;
	
	if (hData = (df_hndl)nshc_parms->data)
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
				df_start(nshc_parms, nshc_calls);
				break;
			case nsh_continue:
				df_continue(nshc_parms, nshc_calls);
				break;
			case nsh_stop:
				df_stop(nshc_parms, nshc_calls);
				break;
			}
			
		}
	
#ifdef __MWERKS__
	SetA4(oldA4);
#endif
}

/* ======================================== */

