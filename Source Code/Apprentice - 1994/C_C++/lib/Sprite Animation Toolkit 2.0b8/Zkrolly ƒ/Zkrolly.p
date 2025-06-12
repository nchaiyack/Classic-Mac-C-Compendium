program Zkrolly;
	uses
		SAT, sXprite, sZprite;

	var
		ignoresp, zp: SpritePtr;
		zWind: WindowPtr;
		r: Rect;

	const
		scrollSizeH = 200;
		scrollSizeV = 200;

	function Zyncho: Boolean;
		var
			where, dest: Point;
	begin
		where := zp^.position;
		where.h := where.h - scrollsizeH div 2;
		where.v := where.v - scrollsizeV div 2;
		if where.h < 0 then
			where.h := 0;
		if where.v < 0 then
			where.v := 0;
		if where.h + scrollsizeH > gSAT.offSizeH then
			where.h := gSAT.offSizeH - scrollSizeH;
		if where.v + scrollsizeV > gSAT.offSizeV then
			where.v := gSAT.offSizeV - scrollSizeV;
		dest.h := 0;
		dest.v := 0;

		SATCopyBitsToScreen(gSAT.offScreen, where, dest, scrollSizeH, scrollSizeV, true);

		Zyncho := true; {Tell SAT not to draw on-screen: we do that ourselves!}
	end;

	procedure SetupZwind;
		var
			zr: Rect;
			wrld: SysEnvRec;
	begin
{Since SAT hasn't been initialized, we can't use colorFlag but have to check environs ourselves.}
		if noErr <> SysEnvirons(1, wrld) then
			; {ignore errors}
		SetRect(zr, 20, 30, 20 + scrollSizeV, 30 + scrollSizeH);
		if wrld.hasColorQD then
			Zwind := NewCWindow(nil, zr, '', false, plainDBox, WindowPtr(-1), false, 0)
		else
			Zwind := NewWindow(nil, zr, '', false, plainDBox, WindowPtr(-1), false, 0);
	end;

begin
	SetupZwind;

	SetRect(r, 0, 0, 510, 340);
	CustomInitSAT(128, 129, r, zwind, nil, false, false, false, true, false);
	InitXprite;
	InitZprite;
	ShowWindow(gSAT.wind);
	SelectWindow(gSAT.wind);
	SATInstallSynch(@Zyncho);
	zp := NewSprite(0, 90, 70, @HandleZprite, @SetupZprite, nil);
	ignoresp := NewSprite(0, 120, 100, @HandleXprite, @SetupXprite, nil);
	ignoresp := NewSprite(0, 200, 160, @HandleXprite, @SetupXprite, nil);
	repeat
		RunSAT(true);
	until Button;

{WARNING! It seems like we mess up the current device somewhere. Probably a bug in SAT}
{(where the device setting isn't perfect yet). Let's set port and device to something nice}
{and safe!}
	SetPort(gSAT.wind);
	if colorFlag then
		SetGDevice(GetMainDevice);
{ Finally, make sure we dispose of the sound channel. }
	SATSoundShutup;
end.