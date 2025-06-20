/*************************************************************************
 *																		 *
 *  Module:		unix main.c												 *
 *	Programmer:	Steve Adams												 *
 *																		 *
 *  (C) Copyright 1985, THINK Technologies, Inc.  All rights reserved.   *
 *																		 *
 *  Alternate main program to handle Unix command lines under Lightspeed *
 *  C.  The user is prompted for the command line on program entry.  The *
 *  line is disected and placed in an "argv" array up to a maximum of    *
 *  MAX_ARGS entries.  I/O redirection is also supported, as follows:    *
 *																		 *
 *		> 		redirects stdout to following file name					 *
 *		<		redirects stdin to following file name					 *
 *		>>		redirects stderr to following file name					 *
 *																		 *
 *  File names following a redirection may be immediately after the      *
 *  redirector, or at what appears to be the next argument.  If a file   *
 *  open error occurs, then the program calls "SysBeep" and exits to the *
 *  shell.																 *
 *																		 *
 *  TO USE: change the "main" function in you application to "_main" and *
 *  include this file in your project.									 *
 *																		 *
 *************************************************************************/


#include <stdio.h>
#include <ctype.h>

#define	MAX_ARGS					 50

#ifndef	true
#define	true						  1
#define false						  0
#endif

static	int			argc			= 1;		/* final argument count  */
static	char		*argv[MAX_ARGS]	= { "" };	/* array of pointers	 */
static	char		command[256];				/* input line buffer	 */
static	int			filename 		= false;	/* TRUE iff file name	 */


/*************************************************************************
 *																		 *
 *  Local routine to make a "beep" and exit to the shell.				 *
 *																		 *
 *************************************************************************/

static void punt()
	{
	SysBeep( 5L );
	ExitToShell();
	}


/*************************************************************************
 *																		 *
 *  Local routine to open a file in argv[--argc] after closing it's		 *
 *  previous existance.													 *
 *																		 *
 *************************************************************************/


static void openfile( file, mode )
char				*mode;						/* mode for file open	 */
FILE				*file;						/* file pointer to use	 */
	{

	if ( (file = freopen( argv[--argc], mode, file ) ) <= (FILE *) NULL)
		punt();
	filename = false;
	}


/*************************************************************************
 *																		 *
 *  New main routine.  Prompts for command line then calls user's main   *
 * now called "_main" with the argument list and redirected I/O.		 *
 *																		 *
 *************************************************************************/


void main()
	{
	char			c;							/* temp for EOLN check	 */
	register char	*cp;						/* index in command line */
	char			*mode;						/* local file mode		 */
	FILE			*file;						/* file to change		 */
	int i;
	
	printf(	"LZSS.C File Compression Utility - by Haruhiko Okumura (6-Apr-89)\n"
			"adapted from MS-DOS to Macintosh by Rob Elliott (27-May-89)\n\n"
			"EXPERIMENTAL: See 'About MacLZSS'!\n"
			"Commands: 'e file1 file2' encodes file1 into file2.\n"
			"          'd file2 file1' decodes file2 into file1.\n\n"
			"Enter command line:\n");

	gets( command );							/* allow user to edit	 */
	cp = &command[0];							/* start of buffer		 */
	argv[0] = "";								/* program name is NULL  */
	while (argc < MAX_ARGS)
		{ /* up to MAX_ARGS entries */
		while (isspace( *cp++ ));
		if ( !*--cp )
			break;
		else if ( *cp == '<' )
			{ /* redirect stdin */
			cp++;
			file = stdin;
			mode = "r";
			filename = true;
			}
		else if ( *cp == '>' )
			{
			mode = "w";
			filename = true;
			if (*++cp == '>')
				{
				file = stderr;
				cp++;
				}
			else
				file = stdout;
			}
		else
			{ /* either an argument or a filename */
			argv[argc++] = cp;
			while ( *++cp && !isspace( *cp ) );
			c = *cp;
			*cp++ = '\0';
			if (filename)
				openfile( file, mode );
			if (!c)
				break;
			}
		}
	_main( argc, argv );
	}
