{*****************************}
{********* SAT Collision **********}
{*****************************}

{A SAT demo that is slightly over SATminimal, demonstrating the simplest}
{collision handling based on the kind field (KindCollision).}

program SATcollision;

	uses
		SAT, sMrEgghead, sApple;

	var
		ignoreSp: SpritePtr;
		l: longint;
		p: Point;
begin
{Standard Inits are done by Think Pascal.}

{Initialize SAT}
	ConfigureSAT(true, kVPositionSort, kKindCollision, 32);
	InitSAT(128, 129, 512, 322);

{Initialize sprite units}
	initMrEgghead;
	initApple;

{Show and update the window}
	ShowWindow(gSAT.wind);
	SelectWindow(gSAT.wind);
	PeekOffscreen;

{Make sprites}
	GetMouse(p); {We get the mouse position in order to put Mr Egghead under it immediately}
	ignoreSp := NewSprite(0, p.h, p.v, @HandleMrEgghead, @SetupMrEgghead, nil);
	ignoreSp := NewSprite(0, 0, Rand(gSAT.offSizeV - 32), @HandleApple, @SetupApple, nil);

	HideCursor;

	repeat
		l := TickCount;
		RunSAT(true); {Run a frame of animation}
		while l > TickCount - 3 do {Speed limit}
			;

{Start a new apple once in a while}
		if Rand(40) = 1 then
			ignoreSp := NewSprite(0, 0, Rand(gSAT.offSizeV - 32), @HandleApple, @SetupApple, nil);
	until Button;
	ShowCursor;
	SATSoundShutUp; {Always make sure the sound channel is de-allocated!}
end.