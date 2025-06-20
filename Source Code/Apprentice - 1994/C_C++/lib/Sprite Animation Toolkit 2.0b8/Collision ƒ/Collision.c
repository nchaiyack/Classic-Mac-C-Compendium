
//� C translation from Pascal source file: SATcollision.p

//� *****************************.
//� ********* SAT Collision **********.
//� *****************************.

//� A SAT demo that is slightly over SATminimal, demonstrating the simplest.
//� collision handling based on the kind field (KindCollision)..

//� main SATcollision;

#include "SAT.h"
#include "Collision.h"
//, sMrEgghead, sApple;


SpritePtr ignoreSp;
long l;
Point p;

main ()
{
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

//� Initialize SAT.
	ConfigureSAT(true, kVPositionSort, kKindCollision, 32);
	InitSAT(128, 129, 512, 322);

//� Initialize sprite units.
	InitMrEgghead ();
	InitApple ();

//� Show and update the window.
	ShowWindow(gSAT.wind);
	SelectWindow(gSAT.wind);
	PeekOffscreen ();

//� Make sprites.
	GetMouse(&p); //� We get the mouse position in order to put Mr Egghead under it immediately.
	ignoreSp = NewSprite(0, p.h, p.v, &HandleMrEgghead, &SetupMrEgghead, 0L);
	ignoreSp = NewSprite(0, 0, Rand(gSAT.offSizeV - 32), &HandleApple, &SetupApple, 0L);

	HideCursor();

	do
		{
		l = TickCount ();
		RunSAT(true); //� Run a frame of animation.
		while ( l > TickCount () - 3 ) //� Speed limit.
			;

//� Start a new apple once in a while.
		if ( Rand(40) == 1 )
			ignoreSp = NewSprite(0, 0, Rand(gSAT.offSizeV - 32), &HandleApple, &SetupApple, 0L);
	} while (! Button ());
	ShowCursor ();
	SATSoundShutup (); //� Always make sure the sound channel is de-allocated!.
}