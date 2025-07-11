unit DemoMenus;

{ WASTE DEMO PROJECT: }
{ Menu Handling }

{ Copyright � 1993-1994 Merzwaren }
{ All Rights Reserved }

interface

	function InitializeMenus: OSErr;
	procedure PrepareMenus;
	procedure DoMenuChoice (menuChoice: LongInt);

implementation
	uses
		Script, DemoFiles, DemoWindows;

	function SFDialogFilter (dialog: DialogPtr;
									var event: EventRecord;
									var item: Integer;
									yourData: Ptr): Boolean;
	begin
		SFDialogFilter := DialogFilter(dialog, event, item);
	end;  { SFDialogFilter }

	function FindMenuItem (menu: MenuHandle;
									stringToFind: Str255): Integer;
		var
			item: Integer;
			itemString: Str255;
	begin
		for item := CountMItems(menu) downto 1 do
			begin
				GetItem(menu, item, itemString);
				if EqualString(itemString, stringToFind, false, false) then
					Leave;
			end;
		FindMenuItem := item;
	end;  { FindMenuItem }

	procedure PrepareMenus;
		var
			window: WindowPtr;
			menu: MenuHandle;
			item: Integer;
			itemString: Str255;
			selStart, selEnd: LongInt;
			mode: Integer;
			ts: TextStyle;
			isContinuous: Boolean;
			alignment: SignedByte;
	begin

{ get a pointer to the frontmost window, if any }
		window := FrontWindow;

{ *** FILE MENU *** }
		menu := GetMHandle(kMenuFile);

{ first disable all items }
		for item := CountMItems(menu) downto 1 do
			DisableItem(menu, item);

{ New, Open and Quit are always enabled }
		EnableItem(menu, kItemNew);
		EnableItem(menu, kItemOpen);
		EnableItem(menu, kItemQuit);

{ enable Close and Save As if there is an active window }
		if (window <> nil) then
			begin
				EnableItem(menu, kItemClose);
				EnableItem(menu, kItemSaveAs);
			end;

{ *** EDIT MENU *** }
		menu := GetMHandle(kMenuEdit);

{ first disable all items }
		for item := CountMItems(menu) downto 1 do
			DisableItem(menu, item);

		if (window <> nil) then
			begin

{ enable Paste and Select All if there is an active window }
				EnableItem(menu, kItemPaste);
				EnableItem(menu, kItemSelectAll);

{ get the current selection range }
				WEGetSelection(selStart, selEnd, DocumentPeek(window)^.hWE);
				if (selStart <> selEnd) then
					begin

{ enable Cut, Copy and Clear if the selection range is not empty }
						EnableItem(menu, kItemCut);
						EnableItem(menu, kItemCopy);
						EnableItem(menu, kItemClear);
					end;

{ determine which style attributes are continuous over the current selection range }
{ we'll need this information in order to check the Font/Size/Style menus properly }
				mode := weDoFont + weDoSize + weDoFace;		{ query about these attributes }
				isContinuous := WEContinuousStyle(mode, ts, DocumentPeek(window)^.hWE);
			end
		else
			mode := 0;		{ no window; so check no item }

{ *** FONT MENU *** }
		menu := GetMHandle(kMenuFont);

{ first remove all check marks }
		for item := CountMItems(menu) downto 1 do
			CheckItem(menu, item, false);

{ if there is a continuous font all over the selection range, }
{ check the corresponding menu item }
		if (BitAnd(mode, weDoFont) <> 0) then
			begin
				GetFontName(ts.tsFont, itemString);
				CheckItem(menu, FindMenuItem(menu, itemString), true);
			end;

{ *** SIZE MENU *** }
		menu := GetMHandle(kMenuSize);

{ first remove all check marks }
		for item := CountMItems(menu) downto 1 do
			CheckItem(menu, item, false);

{ if there is a continuous font size all over the selection range, }
{ check the corresponding menu item }
		if (BitAnd(mode, weDoSize) <> 0) then
			begin
				NumToString(ts.tsSize, itemString);
				CheckItem(menu, FindMenuItem(menu, itemString), true);
			end;

{ *** STYLE MENU *** }
		menu := GetMHandle(kMenuStyle);

{ first remove all check marks }
		for item := CountMItems(menu) downto 1 do
			CheckItem(menu, item, false);

{ check the Style menu items corresponding to style attributes }
{ which are continuous over the current selection range }
		if (BitAnd(mode, weDoFace) <> 0) then
			begin

				if (ts.tsFace = []) then
					CheckItem(menu, kItemPlainText, true);

				if (bold in ts.tsFace) then
					CheckItem(menu, kItemBold, true);

				if (italic in ts.tsFace) then
					CheckItem(menu, kItemItalic, true);

				if (underline in ts.tsFace) then
					CheckItem(menu, kItemUnderline, true);

				if (outline in ts.tsFace) then
					CheckItem(menu, kItemOutline, true);

				if (shadow in ts.tsFace) then
					CheckItem(menu, kItemShadow, true);

				if (condense in ts.tsFace) then
					CheckItem(menu, kItemCondensed, true);

				if (extend in ts.tsFace) then
					CheckItem(menu, kItemExtended, true);

			end;

{ *** ALIGNMENT MENU *** }
		menu := GetMHandle(kMenuAlignment);

{ first remove all check marks }
		for item := CountMItems(menu) downto 1 do
			CheckItem(menu, item, false);

		if (window <> nil) then
			begin

{ get the current alignment style }
				alignment := WEGetAlignment(DocumentPeek(window)^.hWE);

{ find the corresponding Alignment menu item }
				case alignment of

					weFlushLeft: 
						item := kItemAlignLeft;

					weFlushRight: 
						item := kItemAlignRight;

					weFlushDefault: 
						if (GetSysJust = 0) then
							item := kItemAlignLeft
						else
							item := kItemAlignRight;

					weCenter: 
						item := kItemCenter;

					weJustify: 
						item := kItemJustify;

				end;  { case }

{ check the menu item }
				CheckItem(menu, item, true);

			end;
	end;  { PrepareMenus }

	procedure DoAboutBox;
		var
			alertResult: Integer;
	begin
		alertResult := Alert(kAlertAboutBox, @DialogFilter);
	end;  { DoAboutBox }

	procedure DoDeskAcc (menuItem: Integer);
		var
			daName: Str255;
			daNumber: Integer;
	begin
		GetItem(GetMHandle(kMenuApple), menuItem, daName);
		daNumber := OpenDeskAcc(daName);
	end;  { DoDeskAcc }

	function DoNew: OSErr;
	begin

{ create a new window from scratch }
		DoNew := CreateWindow(nil);
	end;  { DoNew }

	function DoOpen: OSErr;
		var
			reply: StandardFileReply;
			typeList: SFTypeList;
	begin
		DoOpen := noErr;

{ set up a list of file types we can open for StandardGetFile }
		typeList[0] := kTypeText;

{ put up the standard Open dialog box }
{ (we use CustomGetFile instead of StandardGetFile because we want to provide }
{ our own dialog filter procedure that takes care of updating our windows) }
		CustomGetFile(nil, 1, typeList, reply, 0, Point(-1), nil, @SFDialogFilter, nil, nil, nil);

{ if the user okayed the dialog, create a new window from the specified file }
		if (reply.sfGood) then
			DoOpen := CreateWindow(@reply.sfFile)
		else
			DoOpen := userCanceledErr;

	end;  { DoOpen }

	function DoSaveAs (window: WindowPtr): OSErr;
		var
			hPrompt: StringHandle;
			defaultName: Str255;
			reply: StandardFileReply;
	begin
		DoSaveAs := noErr;

{ get the prompt string for CustomPutFile from a 'STR ' resource and lock it }
		hPrompt := GetString(kPromptStringID);
		HLockHi(Handle(hPrompt));

{ use the window title as default name for CustomPutFile }
		GetWTitle(window, defaultName);

{ put up the standard Save dialog box }
		CustomPutFile(hPrompt^^, defaultName, reply, 0, Point(-1), nil, @SFDialogFilter, nil, nil, nil);

{ unlock the string resource }
		HUnlock(Handle(hPrompt));

{ if the user okayed the dialog, save the window text to the specified file }
		if (reply.sfGood) then
			DoSaveAs := WriteTextFile(@reply.sfFile, DocumentPeek(window)^.hWE)
		else
			DoSaveAs := userCanceledErr;

	end;  { DoSaveAs }

	procedure DoAppleChoice (menuItem: Integer);
	begin
		if (menuItem = kItemAbout) then
			DoAboutBox
		else
			DoDeskAcc(menuItem);
	end;  { DoAppleChoice }

	procedure DoFileChoice (menuItem: Integer);
		var
			err: OSErr;
	begin
		case menuItem of

			kItemNew: 
				err := DoNew;

			kItemOpen: 
				err := DoOpen;

			kItemClose: 
				DestroyWindow(FrontWindow);

			kItemSaveAs: 
				err := DoSaveAs(FrontWindow);

			kItemQuit: 
				gExiting := true;

			otherwise
				;
		end;  { case menuItem }
	end;  { DoFileChoice }

	procedure DoEditChoice (menuItem: Integer);
		var
			window: WindowPtr;
			hWE: WEHandle;
			err: OSErr;
	begin

{ do nothing if no window is active }
		window := FrontWindow;
		if (window = nil) then
			Exit(DoEditChoice);
		hWE := DocumentPeek(window)^.hWE;

		case menuItem of

			kItemCut: 
				err := WECut(hWE);

			kItemCopy: 
				err := WECopy(hWE);

			kItemPaste: 
				err := WEPaste(hWE);

			kItemClear: 
				err := WEDelete(hWE);

			kItemSelectAll: 
				WESetSelection(0, maxLongInt, hWE);

			otherwise
				;
		end;  { case }
	end;  { DoEditChoice }

	procedure DoFontChoice (menuItem: Integer);
		var
			window: WindowPtr;
			fontName: Str255;
			ts: TextStyle;
			err: OSErr;
	begin
		window := FrontWindow;
		if (window <> nil) then
			begin
				GetItem(GetMHandle(kMenuFont), menuItem, fontName);
				GetFNum(fontName, ts.tsFont);
				err := WESetStyle(weDoFont, ts, DocumentPeek(window)^.hWE);
			end;
	end;  { DoFontChoice }

	procedure DoSizeChoice (menuItem: Integer);
		var
			window: WindowPtr;
			sizeString: Str255;
			longSize: LongInt;
			mode: Integer;
			ts: TextStyle;
			err: OSErr;
	begin
		window := FrontWindow;
		if (window <> nil) then
			begin

				if (menuItem <= kItemLastSize) then
					begin
						GetItem(GetMHandle(kMenuSize), menuItem, sizeString);
						StringToNum(sizeString, longSize);
						mode := weDoSize;
						ts.tsSize := longSize;
					end
				else if (menuItem = kItemSmaller) then
					begin
						mode := weDoAddSize;
						ts.tsSize := -1;
					end
				else if (menuItem = kItemLarger) then
					begin
						mode := weDoAddSize;
						ts.tsSize := +1;
					end;

				err := WESetStyle(mode, ts, DocumentPeek(window)^.hWE);

			end;
	end;  { DoSizeChoice }

	procedure DoStyleChoice (menuItem: Integer);
		var
			window: WindowPtr;
			ts: TextStyle;
			err: OSErr;
	begin
		window := FrontWindow;
		if (window <> nil) then
			begin

				case menuItem of

					kItemPlainText: 
						ts.tsFace := [];

					kItemBold: 
						ts.tsFace := [bold];

					kItemItalic: 
						ts.tsFace := [italic];

					kItemUnderline: 
						ts.tsFace := [underline];

					kItemOutline: 
						ts.tsFace := [outline];

					kItemShadow: 
						ts.tsFace := [shadow];

					kItemCondensed: 
						ts.tsFace := [condense];

					kItemExtended: 
						ts.tsFace := [extend];

					otherwise
						Exit(DoStyleChoice);
				end;  { case menuItem }

				err := WESetStyle(weDoFace + weDoToggleFace, ts, DocumentPeek(window)^.hWE);

			end;
	end;  { DoStyleChoice }

	procedure DoAlignChoice (menuItem: Integer);
		var
			window: WindowPtr;
			alignment: SignedByte;
	begin
		window := FrontWindow;
		if (window <> nil) then
			begin

				case menuItem of

					kItemAlignLeft: 
						alignment := weFlushLeft;

					kItemCenter: 
						alignment := weCenter;

					kItemAlignRight: 
						alignment := weFlushRight;

					kItemJustify: 
						alignment := weJustify;

					otherwise
						Exit(DoAlignChoice);
				end;  { case }

{ set the alignment mode (this automatically redraws the text) }
				WESetAlignment(alignment, DocumentPeek(window)^.hWE);

			end;
	end;  { DoAlignChoice }

	procedure DoMenuChoice (menuChoice: LongInt);
		var
			menuID, menuItem: Integer;
	begin

{ extract menu ID and menu item from menuChoice }
		menuID := HiWord(menuChoice);
		menuItem := LoWord(menuChoice);

{ dispatch on menuID }
		case menuID of

			kMenuApple: 
				DoAppleChoice(menuItem);

			kMenuFile: 
				DoFileChoice(menuItem);

			kMenuEdit: 
				DoEditChoice(menuItem);

			kMenuFont: 
				DoFontChoice(menuItem);

			kMenuSize: 
				DoSizeChoice(menuItem);

			kMenuStyle: 
				DoStyleChoice(menuItem);

			kMenuAlignment: 
				DoAlignChoice(menuItem);

			otherwise
				;
		end;  { case menuID }

		HiliteMenu(0);

	end;  { DoMenuChoice }

	function InitializeMenus: OSErr;
	begin
		InitializeMenus := noErr;

{ build up the whole menu bar from the 'MBAR' resource }
		SetMenuBar(GetNewMBar(kMenuBarID));

{ add names to the Apple and Font menus }
		AddResMenu(GetMHandle(kMenuApple), kTypeDeskAccessory);
		AddResMenu(GetMHandle(kMenuFont), kTypeFont);

{ draw the menu bar }
		DrawMenuBar;

	end;  { InitializeMenus }

end.