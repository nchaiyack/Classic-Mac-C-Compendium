/* ==========================================

	file.c
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ========================================== */

#include <script.h>

#include "file.h"
#include "multi.h"
#include "nsh.h"
#include "resource.h"

#include "file.proto.h"
#include "fss_utl.proto.h"
#include "fss_utl2.proto.h"
#include "str_utl.proto.h"
#include "utility.proto.h"
#include "vlist.proto.h"
#include "multi.proto.h"

static Point SFGwhere = { 90, 82 };
static Point SFPwhere = { 106, 104 };

extern	ShellH		shells[MAXSHELL];

extern	Boolean		gHasFSSpec;			// true if the calls are available

/* ========== These are the menu-level routines. ========== */

int file_open( ShellH shell )
{
	short fRef;
	int error,close;
	int	duplicate;
	int	i,this;

	error = file_openDF( shell, fsRdPerm, &fRef );

	if (!error) {
		error = file_read(shell,fRef);
		close = FSClose(fRef);
		error = error || close;
		}
		
	if (error)
		error_note("\pthe file could not be opened");
	else {
		this = (**shell).ShellNo;
		duplicate = 0;
		for (i=0;i<MAXSHELL;i++)
			if ( (i != this) && shells[i] )
				if ( pStrEqual( (**shell).shell_fss.name, (**shells[i]).shell_fss.name ) )
					duplicate = 1;
					
		if (duplicate)
			switch(Ask("\pA shell with this name is already open.  Should a new name be used?",0)) {
				case askYES:
					multi_set_name( shell );
					(**shell).shell_fss.parID = 0;
					(**shell).shell_fss.vRefNum = 0;
				 	break;
				case askNO:
				 	break;
				 default:
					error = 1;
				 	break;
				}
		}
		
	return(error);
}

int file_save_as( ShellH shell )
{
	short	fRef;
	short	vRef;
	int		error;
	int		close;
	int		ok;
	
	HLock( shell );
	ok = file_sfput( &(**shell).shell_fss );
	HUnlock( shell );

	if ( ok ) {
	
		error = file_create( shell, &fRef);
		
		if ( !error ) {
			error = file_write( shell, fRef );
			close = FSClose(fRef);
			error = error || close;
			}
			
		if ( error )
			error_note("\pthe file could not be saved");
		else
			(**shell).Changed = 0;
	
		}
	else
		error = 1;
				
	return(error);
}

int file_save( ShellH shell )
{
	short fRef;
	int	error,write_protect,close;
	
	write_protect = 0;
	
	error = file_openDF( shell, fsWrPerm, &fRef );
	
	if (error)
		error_note("\pthe file could not be opened");
	else {
		error = file_write( shell, fRef );
		write_protect = error == wPrErr;
		close = FSClose(fRef);
		error = error || close;
		}
	
	if (write_protect)
		error_note("\pthe selected volume is write protected");
	else
		if (error)
			error_note("\pthe file could not be saved");
		else 
			(**shell).Changed = 0;
		
	return(error);
}

/* ========== These are the low-level open/close routines. ========== */

int file_create( ShellH shell, short *theRef)
{
	int error;
	
	HLock( shell );
	error = fss_Create( gHasFSSpec, &(**shell).shell_fss, APPLICATION_SIGNATURE, TRANSCRIPT_FILE, smSystemScript);
	HUnlock( shell );
	
	if (error == dupFNErr) {
		HLock( shell );
		error = fss_Delete( gHasFSSpec, &(**shell).shell_fss );
		HUnlock( shell );
		if (!error) {
			HLock( shell );
			error = fss_Create( gHasFSSpec, &(**shell).shell_fss, APPLICATION_SIGNATURE, TRANSCRIPT_FILE, smSystemScript);
			HUnlock( shell );
			}
		}
		
	if (!error && !gHasFSSpec) {
		HLock( shell );
		error = file_set_type( &(**shell).shell_fss, APPLICATION_SIGNATURE, TRANSCRIPT_FILE );
		HUnlock( shell );
		}

	if (error)
		error_note("\pthe file could not be created");
	else
		error = file_openDF( shell, fsWrPerm, theRef );

	return(error);
}

OSErr file_set_type( const FSSpec *spec, OSType creator, OSType type )
{
	CInfoPBRec		pb;
	OSErr			result;
	
	pb.hFileInfo.ioVRefNum = spec->vRefNum;
	pb.hFileInfo.ioDirID = spec->parID;
	pb.hFileInfo.ioNamePtr = (StringPtr) &(spec->name);
	pb.hFileInfo.ioFDirIndex = 0;
	result = PBGetCatInfoSync(&pb);
	
	if (!result) {
		pb.hFileInfo.ioFlFndrInfo.fdCreator = creator;
		pb.hFileInfo.ioFlFndrInfo.fdType = type;
		pb.hFileInfo.ioDirID = spec->parID;
		result = PBSetCatInfoSync(&pb);
		}
		
	return( result );
}
		
int file_openDF( ShellH shell, char permission, short *fRef )
{
	OSErr error;

	HLock( shell );
	error = fss_OpenDF( gHasFSSpec, &(**shell).shell_fss, permission, fRef );
	HUnlock( shell );
	
	return(error);
}
	
/* ========== These are the low-level read routines. ========== */

int file_read( ShellH shell, short fRef )
{
	char	buffer[256];
	long	remainder;
	long	count;
	int		error;
	int		vars;
	long	version;
	
	TEHandle 		textH;
	
	t_file_header	header;
	t_file_variable	variable;
	
	textH = (**shell).Text;
	
	count = 4;
	if (error = FSRead( fRef, &count, &version ))
		return(error);

	if (version != FILE_VERSION)
		return(-1);
	
	count = sizeof( t_file_header );
	if (error = FSRead( fRef, &count, &header ))
		return(error);
	
	error = SetFPos( fRef, fsFromStart, header.text_start );

	TESetSelect(0, (**textH).teLength, textH);
	TEDelete(textH);

	remainder = header.text_count;

	do {
		count = remainder;
		if (count > 256) count = 256;
		error = FSRead(fRef, &count, &buffer);
		if ((error==noErr) || (error==eofErr))
			TEInsert(&buffer, count, textH);
		remainder -= count;
	} while (remainder && !error);
	
	if (error==eofErr) error = noErr;
		
	vars = header.vars_count;
	count = sizeof( t_file_variable );
	
	while ((vars--) && !error) {
		error = FSRead(fRef, &count, &variable );
		if (!error)
			error = vlist_set( shell, variable.name, variable.value );
		}
	
	return (error);
}

/* ========== These are the low-level write routines. ========== */

int file_write( ShellH shell, short fRef )
{
	int	error;
	long bytes;
	char *text;
	
	TEHandle 		myTE;
	
	long			version_size;
	long			version;
	
	long			header_size;
	t_file_header	header;
		
	myTE = (**shell).Text;
	
	HLock(myTE);
	bytes = (**myTE).teLength;
	text = *(**myTE).hText;
	
	if (bytes)
		if ((text[bytes-3] == RETURN_KEY) &&
		    (text[bytes-2] == '%') &&
		    (text[bytes-1] == ' '))
		    	bytes -=3;

	header_size = sizeof( t_file_header );

	header.text_start = 4 + header_size;	// after version and header
	header.text_count = bytes;
	header.vars_start = header.text_start + bytes;
	header.vars_count = vlist_count( shell );
	
	error = SetEOF( fRef, 0 );
	
	version = FILE_VERSION;
	version_size = sizeof( version );

	if (!error)
		error = FSWrite(fRef, &version_size, &version);

	if (!error)
		error = FSWrite(fRef, &header_size, &header);

	if (!error)
		error = FSWrite(fRef, &bytes, text);

	if (!error)
		error = vlist_write( shell, fRef  );

	HUnlock(myTE);
	
	return(error);
}

/* ========== These are the standard file routines. ========== */

int file_sfput( FSSpec *fss )
{
	SFReply reply;

	SFPutFile(SFPwhere, "\pSave file as", fss->name, 0L, &reply);
	
	if ( reply.good ) {
		pStrCopy(fss->name,reply.fName);
		fss->vRefNum = reply.vRefNum;
		fss->parID = 0;
		}
	
	return(reply.good);
}

int file_sfget( FSSpec *fss )
{
	SFReply		reply;
	SFTypeList	myTypes;
	
	myTypes[0] = TRANSCRIPT_FILE;

	SFGetFile(SFGwhere, "\p", 0L, 1, myTypes, 0L, &reply );

	if ( reply.good ) {
		pStrCopy(fss->name,reply.fName);
		fss->vRefNum = reply.vRefNum;
		fss->parID = 0;
		}
	
	return(reply.good);
}
