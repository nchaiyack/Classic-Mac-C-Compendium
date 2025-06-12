#include "SAT.h"

/* program SATminimal; */

/* MySprite's interface */
extern void InitMySprite();
extern pascal void SetupMySprite (SpritePtr);
extern pascal void HandleMySprite (SpritePtr);

main()
{
		SpritePtr ignoreSp;
		long L;

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

	InitSAT(128, 129, 512, 322);

	InitMySprite(); /* Preload sprite data */

	ShowWindow(gSAT.wind);
	SelectWindow(gSAT.wind); /* Show the window (SATwind and gameWind are the same) */

	PeekOffscreen(); /* Update the window */

/* Make a few sprites */
	ignoreSp = NewSprite(0, 50, 50, HandleMySprite, SetupMySprite, nil);
	ignoreSp = NewSprite(0, 100, 100, HandleMySprite, SetupMySprite, nil);
	ignoreSp = NewSprite(0, 125, 120, HandleMySprite, SetupMySprite, nil);
	ignoreSp = NewSprite(0, 150, 140, HandleMySprite, SetupMySprite, nil);
	ignoreSp = NewSprite(0, 200, 180, HandleMySprite, SetupMySprite, nil);
	ignoreSp = NewSprite(0, 250, 200, HandleMySprite, SetupMySprite, nil);
	ignoreSp = NewSprite(0, 300, 250, HandleMySprite, SetupMySprite, nil);

	while (!Button())
	{
		L = TickCount();
		RunSAT(true); /* Run the animation */
		while (L > TickCount() - 2L) /* Speed limit */
			;
	}
	SATSoundShutup(); /* Always make sure the channel is de-allocated */
}
