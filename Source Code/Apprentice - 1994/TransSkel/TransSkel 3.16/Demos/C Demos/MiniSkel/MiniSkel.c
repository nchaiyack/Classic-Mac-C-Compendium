/*
 * TransSkel demonstration:  Minimal application
 *
 * This program does nothing but put up an Apple menu with desk
 * accessories, and a File menu with a Quit item.  The user can
 * run DA's until Quit is selected or command-Q is typed.
 *
 * The project should include this file, TransSkel.c (or a project
 * built from TransSkel.c), and MacTraps.
 *
 * 21 Apr 88	Paul DuBois
 * 29 Jan 89 Version 1.01
 * - Conversion for TransSkel 2.0.  Integer should be a typedef
 * for compiler 2-byte integer type.
 * 12 Jan 91 Version 1.02
 * - Conversion for TransSkel 3.00.
 * 05 Jun 93 Version 1.03
 * - Conversion for THINK C 6.0.
 * 21 Feb 94
 * - Updated for TransSkel 3.11.
 */

# include	"TransSkel.h"


# define	fileMenuID 	(skelAppleMenuID + 1)


static pascal void
DoFileMenu (short item)
{
	SkelStopEventLoop ();			/* tell SkelEventLoop to quit */
}


int
main (void)
{
MenuHandle	m;

	SkelInit ((SkelInitParamsPtr) nil);	/* initialize TransSkel */
	SkelApple (nil, nil);				/* initialize Apple menu */
	m = NewMenu (fileMenuID, "\pFile");	/* create File menu */
	AppendMenu (m, "\pQuit/Q");
	/* install File menu handler */
	(void) SkelMenu (m, DoFileMenu, nil, false, true);
	SkelEventLoop ();					/* loop 'til Quit selected */
	SkelCleanup ();						/* clean up */
}
