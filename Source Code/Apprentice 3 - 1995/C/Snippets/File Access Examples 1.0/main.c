/* main.c
 *
 * A short example program which uses some of the functions in
 * FileRoutines.c and the scanning routine in Scanner.c. In this
 * example, I ignore the Boolean return values of the file access
 * routines. You shouldn't do this in your programs.
 */

#include "FileRoutines.h"

#include "Scanner.h"

short myFileRef;	/* global file reference number */

void main( void )
{
	StandardFileReply	myReply;
	OSErr				myErr;
	char				x;

	/* Initialize the Macintosh toolbox */
	
		InitGraf( &qd.thePort );
		InitFonts();
		InitWindows();
		InitMenus();
		TEInit();
		InitDialogs( 0L );
		InitCursor();
		FlushEvents( everyEvent, 0 );

	/* Ask user for file spec to create new file */
	
		StandardPutFile( "\pSave as:", "\p", &myReply );
	
	/* If cancelled, exit here */
	
		if ( !myReply.sfGood )
			ExitToShell();
			
	/* If we're replacing an existing file, delete the old one */
	
		if ( myReply.sfReplacing )
			myErr = FSpDelete( &myReply.sfFile );

	/* Now create a new file (a TeachText file) */
	
		myErr = FSpCreate( &myReply.sfFile, 'ttxt', 'TEXT',
											myReply.sfScript );

	/* Now open the file for reading and writing */
	/* storing the reference number in global myFileRef */
	
		myErr = FSpOpenDF( &myReply.sfFile,
									fsRdWrPerm, &myFileRef );

	/* Write the characters 'A' through 'Z' to the file */
	/* using the WriteByte routine sequentially */
	
		for ( x=65; x<=90; x++ )
			WriteByte( myFileRef, kSequential, x );
	
	/* Now, use direct access method to replace some of */
	/* those letters with something that suits me better. */
	
		WriteByte( myFileRef, 11, 'e' );
		WriteByte( myFileRef, 12, 'n' );
		WriteByte( myFileRef, 13, ' ' );
	
	/* Put the file mark back at the end of the file */
	
		SetCurrentRecord( myFileRef,
					RecordsInFile( myFileRef, 1 ), 1 );
		
	/* Write a carriage return to the file */
	
		WriteByte( myFileRef, kSequential, '\r' );
	
	/* Call ScanVolumes to scan all mounted volumes and */
	/* record all folder names in the file */
	
		ScanVolumes();
	
	/* Now close the file */
	
		FSClose( myFileRef );
	
	/* And we're done! */
	
		ExitToShell();
}