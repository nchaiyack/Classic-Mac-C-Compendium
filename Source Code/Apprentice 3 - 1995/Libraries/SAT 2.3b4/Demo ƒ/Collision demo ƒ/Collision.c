
//¥ C translation from Pascal source file: SATcollision.p

//¥ *****************************.
//¥ ********* SAT Collision **********.
//¥ *****************************.

//¥ A SAT demo that is slightly over SATminimal, demonstrating the simplest.
//¥ collision handling based on the kind field (KindCollision)..

//¥ main SATcollision;

#include "SAT.h"
#include "Collision.h"
//, sMrEgghead, sApple;


SpritePtr ignoreSp;
long l;
Point p;

main ()
{
	SATInitToolbox();

//¥ Initialize SAT.
	SATConfigure(true, kVPositionSort, kKindCollision, 32);
	SATInit(128, 129, 512, 322);

//¥ Initialize sprite units.
	InitMrEgghead ();
	InitApple ();

//¥ Make sprites.
	GetMouse(&p); //¥ We get the mouse position in order to put Mr Egghead under it immediately.
	ignoreSp = SATNewSprite(0, p.h, p.v, &SetupMrEgghead);
	ignoreSp = SATNewSprite(0, 0, SATRand(gSAT.offSizeV - 32), &SetupApple);

	HideCursor();

	do
		{
		l = TickCount ();
		SATRun(true); //¥ Run a frame of animation.
		while ( l > TickCount () - 3 ) //¥ Speed limit.
			;

//¥ Start a new apple once in a while.
		if ( SATRand(40) == 1 )
			ignoreSp = SATNewSprite(0, 0, SATRand(gSAT.offSizeV - 32), &SetupApple);
	} while (! Button ());
	ShowCursor ();
	SATSoundShutup (); //¥ Always make sure the sound channel is de-allocated!.
}