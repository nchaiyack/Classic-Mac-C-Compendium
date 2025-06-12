{Minimal demo program, with a few sprites moving back and forth.}

program SATminimal;

	uses
		SAT, sMySprite;
	var
		ignoreSp: SpritePtr;
		l: longint;
begin
{Standard Inits are done by Think Pascal.}

{Initialize, using PICTs 128 or 129 as background, Classic-sized drawing area}
	InitSAT(128, 129, 512, 322);

{Initialize sprite unit(s)}
	InitMySprite;

{Show the window and update it.}
	ShowWindow(gSAT.wind);
	SelectWindow(gSAT.wind);
	PeekOffscreen;

{Make some sprites}
	ignoreSp := NewSprite(0, 50, 50, @HandleMysprite, @SetupMySprite, nil);
	ignoreSp := NewSprite(0, 100, 100, @HandleMysprite, @SetupMySprite, nil);
	ignoreSp := NewSprite(0, 125, 120, @HandleMysprite, @SetupMySprite, nil);
	ignoreSp := NewSprite(0, 150, 140, @HandleMysprite, @SetupMySprite, nil);
	ignoreSp := NewSprite(0, 200, 180, @HandleMysprite, @SetupMySprite, nil);
	ignoreSp := NewSprite(0, 250, 200, @HandleMysprite, @SetupMySprite, nil);
	ignoreSp := NewSprite(0, 300, 250, @HandleMysprite, @SetupMySprite, nil);

	repeat
		l := TickCount;
		RunSAT(true); {Run one frame of animation}
		while l > TickCount - 3 do {Maximize speed to 20 fps}
			;
	until Button;

	SATSoundShutUp; {Always de-allocate the sound channel before quitting!}
end.