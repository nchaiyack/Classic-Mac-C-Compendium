/* ==========================================

	path.c - full pathname management
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ========================================== */

#include "multi.h"
#include "nsh.h"
#include "path.h"

#include "cmd_int.proto.h"
#include "drop.proto.h"
#include "flow.proto.h"
#include "fss.proto.h"
#include "nsh.proto.h"
#include "path.proto.h"
#include "pipe.proto.h"
#include "str_utl.proto.h"
#include "utility.proto.h"
#include "vlist.proto.h"

short	g_appl_vol;
long	g_appl_dir;
Str255	g_appl_path;
Str255	g_appl_name;

extern	Str255	g_drop_path;

extern	Boolean	gHasFSSpec;	// true if the calls are available

/* ========== Utilities. ========== */

int path_is_full( StringPtr s )
{
	int	i,limit;
	
	limit = s[0];
	
	if (limit < 1) return(0);
	
	if (s[1] == ':') return(0);
	
	i=2;
	while( i <= limit ) {
		if (s[i] == ':') return(1);
		i++;
		}
			
	return(0);
}

int path_is_partial( StringPtr s )
{
	if (s[0] < 1) return(0);
	
	return(s[1] == ':');
}

int path_append( StringPtr dst, StringPtr new )
{
	int len;
	
	len = dst[0] + new[0];
	
	if (len > 255) return(1);
	
	pStrAppend(dst,new);
	
	return(0);
}

void path_compress( StringPtr s )		// remove things like "::" from path
{
	char	last;
	int 	i,j,count;

	count = 0;

	for ( i = 1 ; i <= s[0] ; i++ )	{		// 'i' runs through the path forwards
	
		if (s[i] == ':')
			count++;						// 'count' counts the ':' chars
			
		if ((s[i] != ':') || (i == s[0])) {
			if (count > 1) {				// if there are multiple ':'s

				j = count;					// 'j' is the number of ':' sets to find

				count = 0;
				
				while ( ( i > 1 ) && ( s[i] != ':' ) ) {
					i--;
					}
				
				last = s[i];
				
				while ((i>1) && j) {		// figure out 'count' chars to remove
					i--;
					if ((s[i] != ':') && (last == ':'))
						j--;
					if (j)
						count++;
					last = s[i];
					}

				// at this point, i=first char to keep, count = chars to remove
				
				if (i==1) {					// figure out first char to copy to
					j = 1;
					count++;				// special case - we ate the whole thing
					}
				else 
					j = i+1;				// typical case - we ate only part

				s[0] = s[0] - count;		// shorten the string
								
				if (s[0])
					for ( ; j <= s[0] ; j++ ) s[j] = s[j + count];

				}
				
			count = 0;
			}
			
		}
}

long path_to_ftype( Str255 pathname )
{
	FInfo	info;
	OSType	type;

	if (GetFInfo( pathname, 0, &info ))
		return(0);
		
	type = info.fdType;
	
	if ( ( type == 'APPL' ) && ( info.fdCreator == DROP_SIGNATURE ) )
		type = SCRIPT_FILE;
	
	return( type );
}

/* ========== find initial volume and directory ids ========== */

int path_is_dir( Str255 pathname )
{
	OSErr		err;
	long		theDir;
	short		theVol;
	Boolean		isDir;
	
	isDir = 0;
	
	err = fss_VRefNum( pathname, &theVol);

	if (!err)
		err = fss_DirID( theVol, pathname, &theDir, &isDir );
		
	if (err)
		return( 0 );
	else
		return( isDir );
}

int path_from_dir(  short vRefNum, long DirID, Str255 s )
{
	CInfoPBRec	block;
	Str255		directoryName;
	int			error;

	*s = 0;
	
	if ( DirID == 1 )
		return( 0 );
	
	block.dirInfo.ioNamePtr = directoryName;
	block.dirInfo.ioDrParID = DirID;
	
	do {
	
		block.dirInfo.ioVRefNum = vRefNum;
		block.dirInfo.ioFDirIndex = -1;
		block.dirInfo.ioDrDirID = block.dirInfo.ioDrParID;

		error = PBGetCatInfo(&block,false);
		
		if (!error) error = path_append(directoryName,"\p:");
		if (!error) error = path_append(directoryName,s);
		
		if (!error) pStrCopy(s,directoryName);

	} while (!error && (block.dirInfo.ioDrDirID != fsRtDirID));

	return(error);
}

/* ========== find initial volume and directory ids ========== */

int path_get_home( void )
{
	FCBPBRec	ourRec;
	int			error;
	short		ourVolID,ourResID;
	long		ourDirID;
	
	error = 0;				// assume success
	g_appl_vol = -1;		// assume failure
	g_appl_dir = -1;		// assume failure

		// Store the refNum of the current resource file
		// (at opening time, this will be the applications
		// resource fork)

	ourResID = CurResFile();

		// Get the volume ID of the resource fork

	if ( error = GetVRefNum(ourResID,&ourVolID) )
		return( error );

		// Put together a 'FCBPBRec' record to ask for the dirID

	ourRec.qLink = 0L;
	ourRec.qType = 0;
	ourRec.ioTrap = 0L;
	ourRec.ioCmdAddr = 0L;
	ourRec.ioCompletion = 0L;
	ourRec.ioFCBIndx = 0;
	ourRec.ioVRefNum = ourVolID;
	ourRec.ioRefNum = ourResID;
	ourRec.ioNamePtr = g_appl_name;

		// Send the record synchronously and store the return value

	error = PBGetFCBInfo(&ourRec,false);

		// Store the values in the variable passed

	if ( !error ) {
		g_appl_vol = ourVolID;
		g_appl_dir = ourRec.ioFCBParID;
		}

		// Return if we were successful

	return( error );
}

/* ========== path_init ========== */

int path_init( void )
{
	int	error;
	
	error = path_get_home();

#if ! FULL_APP	
	if (!error) {
		error = path_from_dir( g_appl_vol, g_appl_dir, g_appl_path );
		drop_get_home();
		}
#endif

	if (!error)
		error = path_from_dir( g_appl_vol, g_appl_dir, g_appl_path );

	if (error) {
	    error_note("\pcould not set initial directory paths");
#if ! FULL_APP
		nsh_quit();
#endif
		}
		
	return( error );
}

/* ========== set default dirs ========== */

int path_defaults( ShellH shell )
{
	int		error;
	Str255	scratch;
	
#if FULL_APP	
	error = vlist_set( shell, "\pPWD", g_appl_path );
#else
	error = vlist_set( shell, "\pPWD", g_drop_path );
#endif
	if (error) return(error);
	
	error = vlist_set( shell, "\pHOME", g_appl_path );
	if (error) return(error);

	pStrCopy( scratch, "\p:/" );
	error = path_append( scratch, g_appl_path );
	if (!error) error = path_append( scratch, "\pbin:" );
	if (!error) error = vlist_set( shell, "\pPATH", scratch );
	if (error) return(error);
	
	pStrCopy( scratch, g_appl_path );
	error = path_append( scratch, "\ptmp:" );
	if (!error) error = vlist_set( shell, "\pTMP", scratch );
	return(error);
}

/* ========== path_update ========== */

#if FULL_APP

int path_update( ShellH shell )
{
	int		any;
	int		error;
	Str255	scratch;
	
	any = 0;
	
	// if the previous PWD is not valid, set to default
	
	error = vlist_env( shell, "\pPWD", scratch );
	
	if (!error)
		error = !path_is_dir( scratch );
		
	if (error) {
		any = 1;
		if (error = vlist_set( shell, "\pPWD", g_appl_path ))
			return(error);
		}
	
	// if the previous HOME is not valid, set to default
	
	error = vlist_env( shell, "\pHOME", scratch );
	
	if (!error)
		error = !path_is_dir( scratch );
		
	if (error) {
		any = 1;
		if (error = vlist_set( shell, "\pHOME", g_appl_path ))
			return(error);
		}

	// if the previous PATH is not valid, set to default
	
	error = vlist_env( shell, "\pPATH", scratch );
	
	if (!error)
		error = !cmd_int_path_check( shell, scratch, 0 );
		
	if (error) {
		any = 1;
		pStrCopy( scratch, "\p:/" );
		error = path_append( scratch, g_appl_path );
		if (!error) error = path_append( scratch, "\pbin:" );
		if (!error) error = vlist_set( shell, "\pPATH", scratch );
		if (error) return(error);
		}
	
	// if the previous TMP is not valid, set to default
	
	error = vlist_env( shell, "\pTMP", scratch );
	
	if (!error)
		error = !path_is_dir( scratch );
		
	if (error) {
		any = 1;
		pStrCopy( scratch, g_appl_path );
		error = path_append( scratch, "\ptmp:" );
		if (!error) error = vlist_set( shell, "\pTMP", scratch );
		}
		
	if (any)
		pipe_putStr_err( shell, "\p\r\rCAUTION: One or more path variables were invalid and were set to defaults.\r" );
		
	return(error);
}

#endif

/* ========== expand to full name ========== */

int path_expand( ShellH shell, Str255 initial )
{
	int			error;
	Str255		final;
	
	error = 0;

	if (!initial[0]) {
		pipe_putStr_err(shell, "\pexpand: Empty filename.\r");
		return( -1 );
		}

	if (!path_is_full(initial)) {

		error = vlist_env( shell, "\pPWD", final );

		if (error)
			pipe_putStr_err(shell, "\pexpand: PWD not found.\r");
		else {
			if (path_is_partial(initial))
				final[0]--;							// remove trailing ':' before adding partial path
			error = path_append(final,initial);
			}

		if (error)
			initial[0] = 0;
		else
			pStrCopy( initial, final );

		}
		
	path_compress( initial );

	return( error );
}

/* ========== follow search path ========== */

int path_which( ShellH shell, Str255 initial )
{
	int		i,start,limit,found,max;
	int		error;
	Str255  final;
	Str255  paths;
	
	error = noErr;
	
	if ( flow_check( initial ) )
		return( which_internal );
	
	if ( cmd_int_check( initial ) )
		return( which_internal );
	
	if (path_is_full(initial))
		switch( path_to_ftype( initial ) ) {
			case SCRIPT_FILE:
				return(which_script);
			case COMMAND_FILE:
				return(which_external);
			default:
				return(which_none);
			}
				
	if (path_is_partial(initial))
		if ( !path_expand( shell, initial ) )
			switch( path_to_ftype( initial ) ) {
				case SCRIPT_FILE:
					return(which_script);
				case COMMAND_FILE:
					return(which_external);
				default:
					return(which_none);
				}

	error = vlist_env( shell, "\pPATH", paths );
	
	if (!error) {
		max = paths[0];
		if (max <1 )
			error = NSHC_ERR_GENERAL;
		}

	start = 1;

	while (!error && ( start <= max )) {

		i = start;
		found = limit = 0;
	
		while (!found && (i <= max))
			if (paths[i++] == '/')
				found = 1;
			else
				limit++;

		final[0] = limit;
		for (i=0; i < limit; i++)
			final[i + 1] = paths[start + i];
			
		if ( final[final[0]] != ':' )
			final[++final[0]] = ':';
		
		if (!error) error = path_append( final, initial );

		if (!error) error = path_expand( shell, final );
		
		if (!error)
			switch( path_to_ftype( final ) ) {
				case SCRIPT_FILE:
					pStrCopy( initial, final );
					return(which_script);
				case COMMAND_FILE:
					pStrCopy( initial, final );
					return(which_external);
				default:;
				}
				
		start = start + limit + 1;
		
		}
	
	if (error)
		return( -1 );
	else
		return( which_none );
}
