/* ========== the commmand file: ==========

	man.c - where normal Macintosh stuff is handled.
	
	Copyright (c) 1993,1994 Newport Software Development
	
	You may distribute unmodified copies of this file for
	noncommercial purposes.  You may use this file as a
	reference when writing your own nShell(tm) commands.
	
	All other rights are reserved.
	
   ========== the commmand file: ========== */

#ifdef __MWERKS__			// CodeWarrior requires an A4 setup
#include <A4Stuff.h>
#endif

#define		MAN_ERR_FILE	1
#define		MAN_ERR_RSRC	2

#include "nshc.h"
					
#include "str_utl.proto.h"
#include "nshc_utl.proto.h"

extern short ResErr;

/* ========== Utility Functions. ========== */

void man_display( t_nshc_calls *nshc_calls, char **text );

void man_display( t_nshc_calls *nshc_calls, char **text )
{
	long	i;
	char	*p;
	
	i = SizeResource(text);
	SetHandleSize(text,i+2);
	HLock(text);
	p=*text;
	if ( p[i-1] != '\r' ) {
		p[i]='\r';
		p[i+1]=0;
		}
	else
		p[i]=0;
	nshc_calls->NSH_puts(p);
	HUnlock(text);
	ReleaseResource(text);
}

/* ========== find the resource in the specified file ========== */

int man_get( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, int arg );

int man_get( t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls, int arg )
{
	char	**text;
	Str255	rsrcName;
	Str255	fileName;
	short	iFileRef;
	OSErr	error;
	
	error = 0;

	pStrFromC( fileName, &nshc_parms->arg_buf[nshc_parms->argv[arg]] );
	
	iFileRef = -1;
	
	if ( !nshc_calls->NSH_path_which( fileName ) )
		iFileRef = OpenResFile( fileName );
		
	if ( iFileRef < 0 )
		return( MAN_ERR_FILE );

	pStrFromC( rsrcName, "man ");
	
	if (arg == 0) {
		pStrAppendC( rsrcName, &nshc_parms->arg_buf[nshc_parms->argv[1]]);
		pStrAppendC( rsrcName, " " );
		}
		
	if (nshc_parms->argc > 2)
		pStrAppendC( rsrcName, &nshc_parms->arg_buf[nshc_parms->argv[2]] );
	else
		pStrAppendC( rsrcName, "general");
	
	text = (char **)GetNamedResource( 'TEXT', rsrcName);
	
	if ( !text )
		error = MAN_ERR_RSRC;
	else
		man_display( nshc_calls, text );

	if (iFileRef)
		CloseResFile(iFileRef);
		
	return( error );
}

/* ========== the command. ========== */

void main(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls)
{
	int	result;
	int	argc;

#ifdef __MWERKS__
	long oldA4  = SetCurrentA4();
#endif
	
	nshc_parms->action = nsh_idle;		// assume completion
	result = 1;							// assume failure (resource not found)

	if ( !nshc_bad_version( nshc_parms, nshc_calls, NSHC_VERSION ) ) {

		argc = nshc_parms->argc;
	
		if ((argc <2) || (argc >3)) {
			nshc_calls->NSH_putStr_err("\pUsage: man command [section].\r");
			result = NSHC_ERR_PARMS;
			}
		else
			if ( result = man_get( nshc_parms, nshc_calls, 1 ) )
				if ( result = man_get( nshc_parms, nshc_calls, 0 ) )
					nshc_calls->NSH_putStr_err("\pman: Manual Page Not Found.\r");
		
		nshc_parms->result = result;
		nshc_parms->action = nsh_idle;
		
		}
	
#ifdef __MWERKS__
	SetA4(oldA4);
#endif
}
