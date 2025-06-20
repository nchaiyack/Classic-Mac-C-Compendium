/****************************************************************************************
 *
 *	File:		DirentTest.c
 *	Created:	7/3/93		By:	George T. Talbot
 *	Purpose:	This program tests the Mac dirent stuff.
 *
 *	Modifications:
 *
 *	Notes:
 *			1) These routines will NOT work under A/UX.
 *			2) WD = working directory
 *			3) CD = change directory
 *			4) FS = file system
 *			5) Mac filesystems allow spaces as part of pathnames!
 *			6) All routines which return a path use the default Macintosh path separator,
 *			   a colon (":").
 *
 ****************************************************************************************/

#include <stdio.h>
#include "dirent.h"

/****************************************************************************************
 *
 *	The main program opens the parent of the current working directory, prints the path
 *	to this parent and the path to the current working directory, and prints the contents
 *	of the opened directory.
 *
 *	Calls:			opendir(), printf(), fprintf(), print_cwd(), print_dpath(), telldir(),
 *					readdir(), rewinddir(), seekdir(), closedir()
 *	Called By:		main()
 *	Globals Used:	none
 *	Parameters:		none
 *	Returns:		nothing
 *
 ****************************************************************************************/

void	main()
	{
	DIR				*d;
	struct dirent	*de;
	char			path[MAXPATHLEN];

	/*	Open the current working directory's parent directory	*/
	d	= opendir("..");

	if (d == nil)
		{
		fprintf(stderr, "Couldn't open. err = %d\n", dd_errno);
		return;
		}

	/*	Print paths to both parent and current WD	*/
	printf("Current WD: ");
	
	if (getwd(path) == nil)
		fprintf(stderr, "Couldn't get working directory. err = %d\n", dd_errno);
	else
		printf("%s\n", path);

	printf("opendir WD: ");
	if (pathdir(d, path) == nil)
		fprintf(stderr, "Couldn't get path to opened directory. err = %d\n", dd_errno);
	else
		printf("%s\n", path);

	/*	Iterate through the entire directory opened above	*/
	do	{
		int	dirnum;
		
		/*	Get the index of this directory entry.  This index will be equal to the
		 *	ioFDirIndex of the entry minus 1 to maintain POSIX similarity.
		 */
		dirnum	= telldir(d);

		/*	Read the directory entry	*/
		de		= readdir(d);
		
		/*	These two calls are redundant.  They are here to test if rewinddir() and
		 *	seekdir() actually work.  These may be omitted and the program will print
		 *	identical output.
		 */
		rewinddir(d);
		seekdir(d, dirnum+1);

		/*	If the directory entry actually was read, print the name and index	*/
		if (de != nil)
			printf("  %d \"%s\"\n", dirnum, de->d_name);
		}
	/*	Until we get done reading entries or an error occurred	*/
	while (de != nil);

	if (dd_errno)
		fprintf(stderr, "Error while reading directory. err = %d\n", dd_errno);

	/*	Close the directory opened above	*/
	closedir(d);
	
	/*	CD to the parent directory	*/
	printf("Attempting chdir(\"..\")...\n");

	if (chdir(".."))
		fprintf(stderr, "Couldn't change to parent directory. err = %d\n", dd_errno);
	else
		{
		printf("New WD: ");
		if (getwd(path) == nil)
			fprintf(stderr, "Couldn't get working directory. err = %d\n", dd_errno);
		else
			printf("%s\n", path);
		}
	}
