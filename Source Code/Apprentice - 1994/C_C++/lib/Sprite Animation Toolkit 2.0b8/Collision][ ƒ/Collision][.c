/********************************/
/********** SAT Collision ][ ***********/
/********************************/

/*A more advanced version of SAT Collision, demonstrating the use of callback routines*/
/*(hit task) for collision handling. What has happened is that the apples switch between*/
/*good and bad, and we are not supposed to eat one when it's bad.*/

/*program CollisionII;*/

#include "SAT.h"
#include "Collision][.h"


	SpritePtr	ignoreSp;
	long		l;
	Point		p;

main()
{
/*Standard Inits.*/
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


	ConfigureSAT(true, kVPositionSort, kBackwardCollision, 32);
	InitSAT(128, 129, 512, 322);

	InitMrEgghead();
	InitApple();

	ShowWindow(gSAT.wind);
	SelectWindow(gSAT.wind);
	PeekOffscreen();

	GetMouse(&p);
	ignoreSp = NewSprite(0, p.h, p.v, &HandleMrEgghead, &SetupMrEgghead, &HitMrEgghead);
	ignoreSp = NewSprite(0, 0, Rand(gSAT.offSizeV - 32), &HandleApple, &SetupApple, &HitApple);

	HideCursor();

	while (!Button()) {
		l = TickCount();
		RunSAT(true);
/*Start a new sprite once in a while.*/
		if (Rand(40) == 1)
			ignoreSp = NewSprite(0, 0, Rand(gSAT.offSizeV - 32), &HandleApple, &SetupApple, &HitApple);
		while (l > TickCount() - 2L) /*Maximize speed to 30 fps*/
			;
	};
	ShowCursor();
	SATSoundShutup(); /*Always make sure the sound channel is de-allocated!*/
}