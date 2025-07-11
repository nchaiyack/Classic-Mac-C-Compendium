/*
 * MiniDisplay - TransDisplay Demonstration.  Very simple:  just
 * demonstrates the various output calls.
 *
 * The project should include MiniDisplay.c (this file),
 * TransDisplay.c (or a project made from TransDisplay.c),
 * TransSkel.c (or a project made from TransSkel.c), and MacTraps.
 *
 * 04 Oct 86 Paul DuBois
 * 02 Feb 89 Version 1.01
 * - Changed to work with TransSkel2.0 and TransDisplay2.0. 2-byte
 * and 4-byte integer types are typedef'ed to Integer and Longint to
 * ease porting.
 * 15 Jun 92 Version 1.02
 * - Modified for TransSkel 3.00 and TransDisplay 3.00.  The typedefs
 * are now in Compiler.h.
 * 06 Jun 93 Version 1.03
 * - Conversion for THINK C 6.0.
 * 04 Jan 94
 * - Undid Integer/LongInt type stuff back to short/long.
 * 18 Jan 94
 * - Changed calls to DisplayInt()/DisplayHexInt() into calls to
 * DisplayShort()/DisplayHexShort().  Added calls for DisplayCString()
 * and DisplayOSType().
 * 21 Feb 94
 * - Updated for TransSkel 3.11, TransDisplay 3.05.
 */

# include	"TransSkel.h"

# include	"TransDisplay.h"


# define	fileMenuNum	(skelAppleMenuID+1)

static pascal void
DoFileMenu (short item)
{
	SkelStopEventLoop ();				/* tell SkelMain to quit */
}


int
main (void)
{
Rect		r;
MenuHandle	m;
WindowPtr	w;

	SkelInit ((SkelInitParamsPtr) nil);			/* initialize */
	SkelApple (nil, nil);					/* handle desk accessories */

	m = NewMenu (fileMenuNum, "\pFile");	/* create menu and tell TransSkel */
	AppendMenu (m, "\pQuit/Q");				/* to handle it */
	(void) SkelMenu (m, DoFileMenu, nil, false, true);

	SetRect (&r, 100, 75, 420, 275);
	w = NewDWindow (&r, "\pMiniDisplay", false, (WindowPtr) -1L, false, 0L);

	if (w == nil)
	{
		SkelCleanup ();
		ExitToShell ();
	}

	DisplayString ("\pThis is MiniDisplay, a minimal demonstration of ");
	DisplayString ("\pTransDisplay.  The following types of output may ");
	DisplayString ("\pbe written with the built-in output calls:");
	DisplayLn ();

	DisplayLn ();
	DisplayString ("\pArbitrary length text: ");
	DisplayText ("Some text", 9L);
	DisplayLn ();
	DisplayString ("\pPascal string: ");
	DisplayString ("\p\"\\pThis is a Pascal string.\"");
	DisplayLn ();
	DisplayString ("\pC string: ");
	DisplayCString ("\"This is a C string.\"");
	DisplayLn ();
	DisplayString ("\pChar: '");
	DisplayChar ('x');
	DisplayString ("\p'    Hex char: ");
	DisplayHexChar ('x');
	DisplayLn ();
	DisplayString ("\pShort: ");
	DisplayShort (1023);
	DisplayString ("\p  Hex short: ");
	DisplayHexShort (1023);
	DisplayLn ();
	DisplayString ("\pLong: ");
	DisplayLong (32768L);
	DisplayString ("\p  Hex long: ");
	DisplayHexLong (32768L);
	DisplayLn ();
	DisplayString ("\pBoolean: ");
	DisplayBoolean (true);
	DisplayString ("\p, ");
	DisplayBoolean (false);
	DisplayLn ();
	DisplayString ("\pOS text type: '");
	DisplayOSType ('TEXT');
	DisplayChar ('\'');
	DisplayLn ();
	DisplayString ("\pCarriage return.");
	DisplayLn ();
	DisplayLn ();
	DisplayString ("\pSelect Quit from the File menu to exit.");
	SetDWindowPos (w, 0);	/* scroll back to top */
	ShowWindow (w);

	SkelEventLoop ();				/* loop 'til Quit selected */
	SkelCleanup ();					/* clean up */
}
