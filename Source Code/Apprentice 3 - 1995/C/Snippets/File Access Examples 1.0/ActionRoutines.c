/* ActionRoutines.c
 *
 * This file contains a routine called by the volume scanning routine
 * which simply writes each folder name to the file with the global
 * reference number myFileRef and writes a period (.) to the file
 * for each file found. (The number of periods that follow a folder
 * name won't necessarily be the number of files in that folder. That
 * number also includes files in the parent folder between the current
 * folder and the next folder found.)
 *
 */

#include "FileRoutines.h"		/* uses these routines to write to file */

extern short myFileRef;			/* the global file reference number */


void FolderAction( Str31 folderName, short level )
{
	short x;
	
	/* write a carriage return to the file */
	
		WriteByte( myFileRef, kSequential, '\r' );
	
	/* write the folder name to the file indenting based on */
	/* the level number sent. */
	
		/* 4 spaces for each level */
		
		for( x=1; x<=level; x++ )
			WriteLong( myFileRef, kSequential, '    ' );
		
		WriteBlock( myFileRef, kSequential,
						folderName[0], &folderName[1] );
}


void FileAction( void )
{
	/* Write a period to the file */
	
	WriteByte( myFileRef, kSequential, '.' );
}
