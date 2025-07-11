/* ==========================================

	file.proto.h
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ========================================== */

int file_create( ShellH shell, short *fRef);
int file_open( ShellH shell );
int file_read( ShellH shell, short fRef );
int file_save( ShellH shell );
int file_save_as( ShellH shell );
int file_sfget( FSSpec *fss );
int file_sfput( FSSpec *fss );
int file_write( ShellH shell, short fRef );
int file_openDF( ShellH shell, char permission, short *fRef );

OSErr file_set_type( const FSSpec *spec, OSType creator, OSType type );