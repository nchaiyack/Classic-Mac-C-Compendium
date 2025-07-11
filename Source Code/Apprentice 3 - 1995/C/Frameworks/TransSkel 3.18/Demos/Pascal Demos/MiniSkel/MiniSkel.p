program MiniSkel;

	uses
		TransSkel;

	const
		fileMenuNum = skelAppleMenuID + 1;

	procedure DoFileMenu (item: Integer);
	begin
		SkelStopEventLoop;
	end;

	procedure main;
		var
			m: MenuHandle;
			ignore: Boolean;
	begin
		SkelInit(nil);
		SkelApple('', nil);
		m := NewMenu(fileMenuNum, 'File');
		AppendMenu(m, 'Quit/Q');
		ignore := SkelMenu(m, @DoFileMenu, nil, false, true);
		SkelEventLoop;
		SkelCleanup;
	end;

begin
	main;
end.