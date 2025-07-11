/* ========================================

	walk_utl.c
	
	Copyright (c) 1994,1995 Newport Software Development
	
   ======================================== */
   
#include "nshc.h"
#include "walk_utl.h"
#include "walk_utl.proto.h"
#include "str_utl.proto.h"

/* ======================================== */

void walk_copy( FSSpec *dest_fss, FSSpec *src_fss )
{
	dest_fss->vRefNum = src_fss->vRefNum;
	dest_fss->parID = src_fss->parID;
	pStrCopy( dest_fss->name, src_fss->name );
}

/* ======================================== */

t_walk_hndl walk_init( FSSpec *start_fss )
{
	t_walk_hndl	wData;
	
	wData = (t_walk_hndl)NewHandleClear( sizeof( t_walk_data ) );
	
	// start with current fsspec as level 0
	
	if ( wData ) {
		walk_copy( &(**wData).levels[0].fss, start_fss );
		(**wData).levels[0].index = 0;
		(**wData).level = 0;
		}
		
	return( wData );
}

/* ======================================== */

OSErr walk_next( t_walk_hndl wData, FSSpec *fss, short *level, short *isDir )
{
	short		this_level;
	short		this_index;
	short		this_vol;
	long		this_dir;
	Str32		this_name;
	
	short		new_level;

	OSErr		error;
	CInfoPBRec	cipbr;
	HFileInfo	*fpb;
	short		idx;
	long		foundDir;
	
	*level = 0;
	
	this_level = (**wData).level;

	do {	// find the next item (in this or a parent directory)
	
		// fill local variables
		
		this_index = (**wData).levels[this_level].index;
		this_vol = (**wData).levels[this_level].fss.vRefNum;
		this_dir = (**wData).levels[this_level].fss.parID;
		pStrCopy( this_name, (**wData).levels[this_level].fss.name );
		
		// prepare for PBGetCatInfo
		
		fpb = (HFileInfo *)&cipbr;
		fpb->ioVRefNum = this_vol;
		fpb->ioDirID = this_dir;
		fpb->ioFDirIndex = this_index;
		fpb->ioNamePtr = this_name;
		
		// see what the current item is
		
		error = PBGetCatInfo( &cipbr, FALSE );
		
		if ( fpb->ioFlAttrib & 16 )
			*isDir = 1;
		else 
			*isDir = 0;
		
		(**wData).levels[this_level].index++;
	
		if ( error )
			if ( this_level > 1 )
				this_level--;
			else
				return( error );
					
	} while ( error );
		
	// if it is a directory, set up a new level
	
	if ( *isDir ) {
	
		new_level = this_level + 1;
		
		if ( new_level >= MAX_WALK_LEVELS )
			return( tmfoErr );
			
		(**wData).levels[new_level].fss.vRefNum = fpb->ioVRefNum;
		(**wData).levels[new_level].fss.parID = fpb->ioDirID;
		pStrCopy( (**wData).levels[new_level].fss.name, this_name );
		(**wData).levels[new_level].index = 1;
		
		(**wData).level = new_level;
		
		}

	// and set up return values
	
	fss->vRefNum = this_vol;
	fss->parID = this_dir;
	pStrCopy( fss->name, this_name );
	*level = this_level;
	
	return( 0 );
}

/* ======================================== */
