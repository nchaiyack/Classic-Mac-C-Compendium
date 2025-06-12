{***************************}
{********* myPlatform **********}
{***************************}

{This demo is a hack I made, testing if we can use faceless sprites to make stationary}
{obstacles. That worked pretty nicely, so I went on and made some moving platforms too.}
{Take it for what it is: a test hack that suggests one way to make this kind of games.}
{There are many other ways. The controls can be improved a lot, but it is a start.}

program myPlatform;

	uses
		SAT, sPlayerSprite, sPlatform, sMovPlatForm, sHMovPlatForm;

	var
{gameWind: WindowPtr;}
		ignoreSp: SpritePtr;
		l: longint;
		p: Point;
	var
		thepat: SATPatHandle;

	procedure DrawInfo;
		var
			r: Rect;
	begin
		SetPort(gSAT.backScreen);
		SetRect(r, 100, 50, 300, 100);
		EraseRect(r);
		FrameRect(r);
		MoveTo(110, 70);
		DrawString('SAT Platform demo');
		MoveTo(110, 90);
		DrawString('Move with "," "." and space');
		SATBackChanged(r);
	end;

begin
{Standard Inits are done by Think Pascal.}

	ConfigureSAT(true, kLayerSort, kBackwardCollision, 32);
	InitSAT(0, 0, 512, 322); {No PICTs}

{Use a background pattern (instead of PICTs - just to demo that too)}
	SetPort(gSAT.backScreen);
	thepat := SATGetPat(128);
	SATPenPat(thepat);
	PaintRect(0, 0, gSAT.offsizeV, gSAT.offsizeH);
{The following CopyBits would have been faster if we had used SATSetPortBackScreen}
	CopyBits(gSAT.backScreen^.portBits, gSAT.offScreen^.portBits, gSAT.offScreen^.portrect, gSAT.offscreen^.portrect, srcCopy, nil);

	DrawInfo;

{Initialize all sprite units}
	initPlayerSprite;
	initPlatform;
	initMovPlatform;
	initHMovPlatform;

{Show the game window (SATwind) and update it.}
	ShowWindow(gSAT.wind);
	SelectWindow(gSAT.wind);
	PeekOffscreen;

{Make the sprites.}
	GetMouse(p);
	ignoreSp := NewSprite(1, p.h, p.v, @HandlePlayerSprite, @SetupPlayerSprite, @HitPlayerSprite);
	ignoreSp := NewSprite(0, 50, 300, @HandlePlatform, @SetupPlatform, @HitPlatform);
	ignoreSp := NewSprite(0, 150, 200, @HandlePlatform, @SetupPlatform, @HitPlatform);
	ignoreSp := NewSprite(0, 250, 100, @HandlePlatform, @SetupPlatform, @HitPlatform);
	ignoreSp := NewSprite(0, 350, 50, @HandlePlatform, @SetupPlatform, @HitPlatform);

	ignoreSp := NewSprite(0, 350, 300, @HandleMovPlatform, @SetupMovPlatform, @HitMovPlatform);
	ignoreSp := NewSprite(0, 50, 200, @HandleMovPlatform, @SetupMovPlatform, @HitMovPlatform);

	ignoreSp := NewSprite(0, 200, 150, @HandleHMovPlatform, @SetupHMovPlatform, @HitHMovPlatform);

	HideCursor;
	done := false;
	repeat
		l := TickCount;
		RunSAT(true); {Run one frame of animation.}
		while l > TickCount - 2 do {Maximize speed to 30 fps}
			;
	until done or Button;
	ShowCursor;
	SATSoundShutUp; {Always make sure the sound channel is de-allocated!}
end.