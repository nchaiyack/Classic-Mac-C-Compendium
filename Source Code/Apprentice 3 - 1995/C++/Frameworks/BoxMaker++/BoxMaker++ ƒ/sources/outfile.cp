#include <stdlib.h>

#include <Folders.h>

#include "outfile.h"

outfile::outfile( unsigned char *postfix, OSType inWhichFolder)
{
	Str31 fileName;
	makeprefix( fileName);	
	const int append_len = (postfix[ 0] > 17) ? 17 : postfix[ 0];
	fileName[ 0] = append_len + 14;
	for( int i = 1; i <= append_len; i++)
	{
		fileName[ 14 + i] = postfix[ i];
	}

	long foundDirID;
	short foundVRefnum;
	//
	// As I once read somewhere:
	//		'Real programs do error checking here'
	//
	(void)FindFolder( kOnSystemDisk, inWhichFolder, kCreateFolder, &foundVRefnum, &foundDirID);
	(void)FSMakeFSSpec( foundVRefnum, foundDirID, fileName, &theFile);

	(void)FSpCreate( &theFile, 'R*ch', 'TEXT', 0);
	(void)FSpOpenDF( &theFile, fsWrPerm, &fileno);
}
//
// set2chars is a quick hack which puts the ascii representation of the last two
// digits of the number passed in in the two characters pointed to by two_chars.
//
void outfile::set2chars( const int n, unsigned char *two_chars)
{
	const char lastdigit  = n % 10;
	const char firstdigit = ((n - lastdigit) / 10) % 10;
	two_chars[ 0] = firstdigit + '0';
	two_chars[ 1] = lastdigit  + '0';
}
//
// makeprefix sets the first letters of the string specified to the current
// date and time The time format used is "yymmdd hhmmss "
// This format has the property that alphabetical and chronological sorts of
// dates are equal.
//
void outfile::makeprefix( Str31 thename)
{
	DateTimeRec now;	
	GetTime( &now);
	
	set2chars( now.year,   &thename[  1]);
	set2chars( now.month,  &thename[  3]);
	set2chars( now.day,    &thename[  5]);
	thename[ 7] = ' ';
	set2chars( now.hour,    &thename[  8]);
	set2chars( now.minute,  &thename[ 10]);
	set2chars( now.second,  &thename[ 12]);
	thename[ 14] = ' ';
}
