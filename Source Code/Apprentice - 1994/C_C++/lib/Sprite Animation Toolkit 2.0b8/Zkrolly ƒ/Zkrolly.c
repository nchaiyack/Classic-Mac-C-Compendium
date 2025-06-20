
//� C translation from Pascal source file: Zkrolly.p

//� main Zkrolly;
#include "SAT.h"

extern void		InitXprite(void);
extern pascal void		SetupXprite(SpritePtr me);
extern pascal void		HandleXprite(SpritePtr me);
extern void		InitZprite(void);
extern pascal void		SetupZprite(SpritePtr me);
extern pascal void		HandleZprite(SpritePtr me);

	
SpritePtr ignoresp, zp;
WindowPtr Zwind;
Rect r;

enum {
	scrollSizeH = 200,
	scrollSizeV = 200
};

pascal Boolean Zyncho()
{
	Point where, dest;
	
	where = zp->position;
	where.h = where.h - scrollSizeH / 2;
	where.v = where.v - scrollSizeV / 2;
	if ( where.h < 0 )
		where.h = 0;
	if ( where.v < 0 )
		where.v = 0;
	if ( where.h + scrollSizeH > gSAT.offSizeH )
		where.h = gSAT.offSizeH - scrollSizeH;
	if ( where.v + scrollSizeV > gSAT.offSizeV )
		where.v = gSAT.offSizeV - scrollSizeV;
	dest.h = 0;
	dest.v = 0;
	
	SATCopyBitsToScreen(gSAT.offScreen, where, dest, scrollSizeH, scrollSizeV, true);

	return true; //� Tell SAT not to draw on-screen: we do that ourselves!.
}

void SetupZwind()
{
	Rect zr;
	SysEnvRec wrld;
//	short scrollSizeH, scrollSizeV;
	
	//� Since SAT hasn't been initialized, we can't use colorFlag but 
	//� have to check environs ourselves.
	if ( noErr != SysEnvirons(1, &wrld))
		;//� ignore errors.
	SetRect(&zr, 20, 30, 20 + scrollSizeV, 30 + scrollSizeH);
	if ( wrld.hasColorQD )
		Zwind = NewCWindow(0L, &zr, "\p", false, plainDBox, (WindowPtr)-1L, false, 0);
	else
		Zwind = NewWindow(0L, &zr, "\p", false, plainDBox, (WindowPtr)-1L, false, 0);
}

main ()
{
/* ThinkC doesn't initialize automatically, so we must do that ourselves? */

	MaxApplZone ();
	FlushEvents (everyEvent - diskMask, 0 );
	InitGraf (&thePort);
	InitFonts ();
	InitWindows ();
	InitMenus ();
	TEInit ();
	InitDialogs (nil);		/* no restart proc */
	InitCursor ();

	MoreMasters ();
	MoreMasters ();

/* End of initializations */

	SetupZwind ();

	SetRect(&r, 0, 0, 510, 340);
	CustomInitSAT(128, 129, &r, Zwind, 0L, false, false, false, true, false);
	InitXprite ();
	InitZprite ();
	ShowWindow(gSAT.wind);
	SelectWindow(gSAT.wind);
	SATInstallSynch(&Zyncho);
	zp = NewSprite(0, 90, 70, &HandleZprite, &SetupZprite, 0L);
	ignoresp = NewSprite(0, 120, 100, &HandleXprite, &SetupXprite, 0L);
	ignoresp = NewSprite(0, 200, 160, &HandleXprite, &SetupXprite, 0L);
	do
	{
		RunSAT(true);
	} while (! Button ());

	//� WARNING! It seems like we mess up the current device somewhere. 
	//� Probably a bug in SAT (where the device setting isn't perfect 
	//� yet). Let's set port and device to something nice and safe!.
	SetPort(gSAT.wind);
	if ( colorFlag )
		SetGDevice(GetMainDevice ());
/* Finally, make sure we dispose of the sound channel. */
	SATSoundShutup();
}

/*To Ken Long:
Errors:
Callback routines should be declared "pascal"
Initializations needed.
Too little memory allocated.
The declaration "short scrollSizeH, scrollSizeV;" in SetupZwind shouldn't be there.
(Partially my fault: a mis-capitalization.)
Zsprite didn't load face 1. ("<" instead of "<=")
In other words, not much.
*/