/* ========== the commmand file: ==========

	sfput.c
	
	Copyright (c) 1994 Newport Software Development
	
	You may distribute unmodified copies of this file for
	noncommercial purposes.  You may use this file as a
	reference when writing your own nShell(tm) commands.
	
	All other rights are reserved.
	
   ========== the commmand file: ========== */

#ifdef __MWERKS__			// Get the appropriate A4 stuff
#include <A4Stuff.h>
#else
#include <SetUpA4.h>
#endif

#include <GestaltEqu.h>
#include <Folders.h>

#include "nshc.h"
#include "str_utl.proto.h"
#include "nshc_utl.proto.h"

/* ======================================== */

// globals

Str32	gVarName;
Str255	gPath;
short	gFileRef;
Str255	gPrompt;

/* ======================================== */

// prototypes - for local use only

int     sfput_open_rsrc(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls);
void    sfput_close_rsrc(void);
int     sfput( void );
int     sfput_init(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls );
int     sfput_path( StandardFileReply *sfReply );
int     sfput_pre7( void );

/* ======================================== */

// utility

int	sfput_pre7( void )
{
	long	response;
	
	if ( Gestalt( 'sysv', &response ) )
		return( 1 );
		
	if ( response < 0x700 )
		return( 1 );
	else
		return( 0 );
}

int sfput_path( StandardFileReply *sfReply )
{
	CInfoPBRec	block;
	int			error;
	Str255		temp;

	error = 0;
	gPath[0] = 0;
	temp[0] = 0;
	block.dirInfo.ioNamePtr = temp;
	block.dirInfo.ioDrParID = sfReply->sfFile.parID;
	
	if ( sfReply->sfFile.parID != 1)
		do {
		
			block.dirInfo.ioVRefNum = sfReply->sfFile.vRefNum;
			block.dirInfo.ioFDirIndex = -1;
			block.dirInfo.ioDrDirID = block.dirInfo.ioDrParID;
	
			error = PBGetCatInfo(&block,false);
			
			if ( temp[0] + gPath[0] + 1 > 255 )
				error = 1;
			
			if (!error) {
				temp[ ++temp[0] ] = ':';
				pStrAppend(temp,gPath);
				pStrCopy(gPath,temp);
				}
	
		} while (!error && (block.dirInfo.ioDrDirID != fsRtDirID));

	if ( error )
		return( error );

	if ( gPath[0] + sfReply->sfFile.name[0] > 255 )
		error = 1;
	else
		pStrAppend( gPath, sfReply->sfFile.name );

	if ( sfReply->sfFile.parID == 1 )
		if ( gPath[0] < 255 )
			gPath[ ++gPath[0] ] = ':';
		else
			error = 1;

	return( error );
}

/* ======================================== */

// sfput

int sfput( void )
{
	StandardFileReply	sfReply;
	int					my_err;

	StandardPutFile(gPrompt,gPath,&sfReply);
	
	if (sfReply.sfGood)
		my_err = sfput_path( &sfReply );
	else
		my_err = 1;
		
	return( my_err );
}

/* ======================================== */

int sfput_init(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls )
{
	int		usage;
	int		prompt;
	int		var;
	int		argc;
	int		error;
	int		i;
	char	c;
	char	*p;
	char	*q;
	
	// return if bad include file version
		
	if (nshc_bad_version( nshc_parms, nshc_calls, NSHC_VERSION ))
		return( 0 );
	
	// return if pre system 7 os
		
	if ( sfput_pre7() ) {
		nshc_calls->NSH_putStr_err("\psfput: This command requires System 7.\r");
		nshc_parms->result = NSHC_ERR_GENERAL;
		nshc_parms->action = nsh_idle;
		return( 0 );
		}

	// return if bad usage
		
	usage = 0;
	argc = nshc_parms->argc;
	prompt = nshc_got_option(nshc_parms, 'p');

	if ((argc != 2) && (argc != 4))
		usage = 1;
		
	if ( (argc == 4) && ( prompt != 1 ) && ( prompt != 2 ) )
		usage = 1;
	
	if ( usage ) {
		nshc_calls->NSH_putStr_err("\pUsage: sfput variable_name [-p \"prompt string\"].\r");
		nshc_parms->result = NSHC_ERR_PARMS;
		nshc_parms->action = nsh_idle;
		return( 0 );
		}
		
	// return if bad prompt string
	
	if (prompt) {
	
		p = &nshc_parms->arg_buf[nshc_parms->argv[prompt + 1]];
			
		if ( cStrLen( p ) > 255 ) {
			nshc_calls->NSH_putStr_err("\psfput: Prompt string is too long.");
			nshc_parms->result = NSHC_ERR_PARMS;
			nshc_parms->action = nsh_idle;
			return( 0 );
			}
		else
			pStrFromC( gPrompt, p );
			
		}
	else
		pStrCopy( gPrompt, "\pSave as:");
	
	// return if bad variable name
	
	if ( prompt == 1 )
		var = 3;
	else
		var = 1;
		
	p = q = &nshc_parms->arg_buf[ nshc_parms->argv[ var ] ];
	i = 0;
	while (c = *p++) {
		error = 1;
		if ( c == '_' ) error = 0; else
		if ( ( c >= 'a' ) && ( c <= 'z' ) ) error = 0; else
		if ( ( c >= 'A' ) && ( c <= 'Z' ) ) error = 0; else
		if ( ( c >= '0' ) && ( c <= '9' ) ) error = 0;
		if (error) {
			nshc_calls->NSH_putStr_err( "\psfput: Invalid variable name = " );
			nshc_calls->NSH_puts_err( q );
			nshc_calls->NSH_putchar_err( '\r' );
			nshc_parms->result = NSHC_ERR_PARMS;
			nshc_parms->action = nsh_idle;
			return( 0 );
			}
		gVarName[++i] = c;
		}
		
	if ( i > 31 ) {
		nshc_calls->NSH_putStr_err( "\psfput: Variable name too long = " );
		nshc_calls->NSH_puts_err( q );
		nshc_calls->NSH_putchar_err( '\r' );
		nshc_parms->result = NSHC_ERR_PARMS;
		nshc_parms->action = nsh_idle;
		return( 0 );
		}
	else
		gVarName[0] = i;
		
	gPath[0] = 0;
	
	return( 1 );
}

/* ======================================== */

int sfput_open_rsrc(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls)
{
	Str255	fileName;

	pStrFromC( fileName, &nshc_parms->arg_buf[nshc_parms->argv[0]] );
	
	gFileRef = -1;
	
	if ( !nshc_calls->NSH_path_which( fileName ) )
		gFileRef = OpenResFile( fileName );
		
	if ( gFileRef < 0 ) {
		nshc_calls->NSH_putStr_err("\psfput: Could not open resource file.\r");
		nshc_parms->result = NSHC_ERR_PARMS;
		nshc_parms->action = nsh_idle;
		return( 1 );
		}
	else
		return( 0 );
}

void sfput_close_rsrc(void)
{
	if (gFileRef)
		CloseResFile(gFileRef);
}

/* ======================================== */

void main(t_nshc_parms *nshc_parms, t_nshc_calls *nshc_calls)
{
	int		error;
	
#ifdef __MWERKS__
	long oldA4  = SetCurrentA4();
#else
	RememberA0();
	SetUpA4();
#endif
	
	if ( sfput_init( nshc_parms, nshc_calls ) ) {
	
		error = sfput_open_rsrc( nshc_parms, nshc_calls );
	
		if ( !error )
			error = sfput();
			
		sfput_close_rsrc();
		
		if ( !error )
			nshc_calls->NSH_var_set( gVarName, gPath );

 		nshc_parms->action = nsh_idle;
		nshc_parms->result = error;
			
		}

#ifdef __MWERKS__
	SetA4(oldA4);
#else
	RestoreA4();
#endif
}
