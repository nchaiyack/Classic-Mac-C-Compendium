program MiniDisplay;

	uses
		TransSkel, TransDisplay;

	const

		fileMenuNum = skelAppleMenuID + 1;

	var

		r: Rect;
		m: MenuHandle;
		w: WindowPtr;
		ignore: Boolean;
		str: Str255;


	procedure DoFileMenu (item: Integer);
	begin
		SkelStopEventLoop;
	end;


begin
	SkelInit(nil);
	SkelApple('', nil);
	m := NewMenu(fileMenuNum, 'File');
	AppendMenu(m, 'Quit/Q');
	ignore := SkelMenu(m, @DoFileMenu, nil, false, true);

	SetRect(r, 100, 75, 420, 275);
	w := NewDWindow(r, 'MiniDisplay', false, WindowPtr(-1), false, 0);

	if (w = nil) then
		begin
			SkelCleanup;
			ExitToShell;
		end;

	DisplayString('This is MiniDisplay, a minimal demonstration of ');
	DisplayString('TransDisplay.  The following types of output may ');
	DisplayString('be written with the built-in output calls:');
	DisplayLn;

	DisplayLn;
	DisplayString('Arbitrary length text: ');
	str := 'Some text';
	DisplayText(Ptr(@str[1]), 9);
	DisplayLn;
	DisplayString('Pascal string: ');
	DisplayString('"\pThis is a Pascal string."');
	DisplayLn;
	DisplayString('C string: ');
	DisplayString('(not available in Pascal demo version)');
	DisplayLn;
	DisplayString('Char: ');
	DisplayChar(chr(120));					{ x }
	DisplayString('    Hex char: ');
	DisplayHexChar(chr(120));				{ x }
	DisplayLn;
	DisplayString('Short: ');
	DisplayShort(1023);
	DisplayString('  Hex short: ');
	DisplayShort(1023);
	DisplayLn;
	DisplayString('Long: ');
	DisplayLong(32768);
	DisplayString('  Hex long: ');
	DisplayHexLong(32768);
	DisplayLn;
	DisplayString('Boolean: ');
	DisplayBoolean(true);
	DisplayString(', ');
	DisplayBoolean(false);
	DisplayLn;
	DisplayString('OS text type: ''');
	DisplayOSType('TEXT');
	DisplayString('''');
	DisplayLn;
	DisplayString('Carriage return.');
	DisplayLn;
	DisplayLn;
	DisplayString('Select Quit from the File menu to exit.');
	SetDWindowPos(w, 0);			{ scroll back to top }
	ShowWindow(w);

	SkelEventLoop;
	SkelCleanup;
end.