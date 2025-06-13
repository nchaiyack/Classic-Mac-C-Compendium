unit WEDemoMenus;

{ WASTE DEMO PROJECT: }
{ Menu Handling }

{ Copyright © 1993-1995 Marco Piovanelli }
{ All Rights Reserved }

interface
	uses
		WEDemoIntf;

	function InitializeMenus: OSErr;
	procedure PrepareMenus;
	procedure DoMenuChoice (menuChoice: LongInt);
	function DoClose (closing: ClosingOption;
									saving: SavingOption;
									window: WindowPtr): OSErr;
	function DoQuit (saving: SavingOption): OSErr;

implementation
	uses
		Aliases, Devices, LowMem, StandardFile, ToolUtils, WEDemoAbout, WEDemoFiles, WEDemoWindows, DialogUtils, WETabs;

	const

{ resource types }

		kTypeMenuColorTable = 'mctb';

	var

		sColors: Handle;			{ handle to the 'mctb' resource for the Color menu }

	function MySFDialogFilter (dialog: DialogPtr;
									var event: EventRecord;
									var item: Integer;
									yourData: Ptr): Boolean;
	begin
		MySFDialogFilter := CallModalFilterProc(dialog, event, item, GetMyStandardDialogFilter);
	end;  { MySFDialogFilter }

	var

{ static variable for exclusive use by GetMySFDialogFilter }

		sMySFDialogFilter: ModalFilterYDUPP;

	function GetMySFDialogFilter: ModalFilterYDUpp;
	begin
		if (sMySFDialogFilter = nil) then
			sMySFDialogFilter := NewModalFilterYDProc(@MySFDialogFilter);
		GetMySFDialogFilter := sMySFDialogFilter;
	end;  { GetMySFDialogFilter }

	procedure SetDefaultDirectory (vRefNum: Integer;
									dirID: LongInt);
	begin
		LMSetCurDirStore(dirID);
		LMSetSFSaveDisk(-vRefNum);
	end;  { SetDefaultDirectory }

	function FindMenuItemText (menu: MenuRef;
									stringToFind: Str255): Integer;
		var
			item: Integer;
			itemText: Str255;
	begin
		for item := CountMItems(menu) downto 1 do
			begin
				GetMenuItemText(menu, item, itemText);
				if EqualString(itemText, stringToFind, false, false) then
					Leave;
			end;
		FindMenuItemText := item;
	end;  { FindMenuItemText }

	function EqualColor (var rgb1, rgb2: RGBColor): Boolean;
	begin
		EqualColor := (rgb1.red = rgb2.red) and (rgb1.green = rgb2.green) and (rgb1.blue = rgb2.blue);
	end;  { EqualColor }

	procedure PrepareMenus;
		var
			window: WindowPtr;
			hWE: WEHandle;
			menu: MenuRef;
			item: Integer;
			itemText: Str255;
			selStart, selEnd: LongInt;
			actionKind: WEActionKind;
			mode: Integer;
			ts: TextStyle;
			temp: Boolean;
	begin

{ get a pointer to the frontmost window, if any }
		window := FrontWindow;

{ get associated WE instance }
		hWE := DocumentPeek(window)^.hWE;

{ *** FILE MENU *** }
		menu := GetMenuHandle(kMenuFile);

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

{ enable Save if the active window is dirty }
				if (WEGetModCount(hWE) > 0) then
					EnableItem(menu, kItemSave);
			end;

{ *** EDIT MENU *** }
		menu := GetMenuHandle(kMenuEdit);

{ first disable all items }
		for item := CountMItems(menu) downto 1 do
			DisableItem(menu, item);

{ by default, the Undo menu item should read "Can't Undo" }
		GetIndString(itemText, kUndoStringsID, 1);
		SetMenuItemText(menu, kItemUndo, itemText);

		if (window <> nil) then
			begin

{ enable Paste if there's anything pasteable on the Clipboard }
				if (WECanPaste(hWE)) then
					EnableItem(menu, kItemPaste);

{ enable Undo if anything can be undone }
				actionKind := WEGetUndoInfo(temp, hWE);
				if (actionKind <> weAKNone) then
					begin
						EnableItem(menu, kItemUndo);

{ change the Undo menu item to "Undo"/"Redo" + name of action to undo }
						GetIndString(itemText, kUndoStringsID, 2 * actionKind + ORD(temp));
						SetMenuItemText(menu, kItemUndo, itemText);
					end;

{ enable Select All if there is anything to select }
				if (WEGetTextLength(hWE) > 0) then
					EnableItem(menu, kItemSelectAll);

{ get the current selection range }
				WEGetSelection(selStart, selEnd, hWE);
				if (selStart <> selEnd) then
					begin

{ enable Cut, Copy and Clear if the selection range is not empty }
						EnableItem(menu, kItemCut);
						EnableItem(menu, kItemCopy);
						EnableItem(menu, kItemClear);
					end;

{ determine which style attributes are continuous over the current selection range }
{ we'll need this information in order to check the Font/Size/Style/Color menus properly }
				mode := weDoAll;		{ query about all attributes }
				temp := WEContinuousStyle(mode, ts, hWE);
			end
		else
			mode := 0;		{ no window; so check no item }

{ *** FONT MENU *** }
		menu := GetMenuHandle(kMenuFont);

{ first remove all check marks }
		for item := CountMItems(menu) downto 1 do
			CheckItem(menu, item, false);

{ if there is a continuous font all over the selection range, }
{ check the corresponding menu item }
		if (BAND(mode, weDoFont) <> 0) then
			begin
				GetFontName(ts.tsFont, itemText);
				CheckItem(menu, FindMenuItemText(menu, itemText), true);
			end;

{ *** SIZE MENU *** }
		menu := GetMenuHandle(kMenuSize);

{ first remove all check marks }
		for item := CountMItems(menu) downto 1 do
			CheckItem(menu, item, false);

{ if there is a continuous font size all over the selection range, }
{ check the corresponding menu item }
		if (BAND(mode, weDoSize) <> 0) then
			begin
				NumToString(ts.tsSize, itemText);
				CheckItem(menu, FindMenuItemText(menu, itemText), true);
			end;

{ *** STYLE MENU *** }
		menu := GetMenuHandle(kMenuStyle);

{ first remove all check marks }
		for item := CountMItems(menu) downto 1 do
			CheckItem(menu, item, false);

{ check the Style menu items corresponding to style attributes }
{ which are continuous over the current selection range }
		if (BAND(mode, weDoFace) <> 0) then
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

{ *** COLOR MENU *** }
		menu := GetMenuHandle(kMenuColor);

{ first remove all check marks }
		for item := CountMItems(menu) downto 1 do
			CheckItem(menu, item, false);

{ if there is a continuous color all over the selection range, }
{ check the corresponding menu item (if any) }
		if (BAND(mode, weDoColor) <> 0) then
			with MenuCRsrcHandle(sColors)^^ do
				for item := numEntries - 1 downto 0 do
					if (EqualColor(ts.tsColor, mcEntryRecs[item].mctRGB2)) then
						CheckItem(menu, mcEntryRecs[item].mctItem, true);

{ *** FEATURES MENU *** }
		menu := GetMenuHandle(kMenuFeatures);

{ first remove all check marks (except the first item, which has a submenu!!) }
		for item := CountMItems(menu) downto 2 do
			CheckItem(menu, item, false);

		if (window <> nil) then
			begin

{ mark each item according to the corresponding feature }
				if (WEIsTabHooks(hWE)) then
					begin
						CheckItem(menu, kItemTabHooks, true);
						DisableItem(menu, kItemAlignment);
					end
				else
					EnableItem(menu, kItemAlignment);

				if Boolean(WEFeatureFlag(weFAutoScroll, weBitTest, hWE)) then
					CheckItem(menu, kItemAutoScroll, true);

				if Boolean(WEFeatureFlag(weFOutlineHilite, weBitTest, hWE)) then
					CheckItem(menu, kItemOutlineHilite, true);

				if Boolean(WEFeatureFlag(weFReadOnly, weBitTest, hWE)) then
					CheckItem(menu, kItemReadOnly, true);

				if Boolean(WEFeatureFlag(weFIntCutAndPaste, weBitTest, hWE)) then
					CheckItem(menu, kItemIntCutAndPaste, true);

				if Boolean(WEFeatureFlag(weFDragAndDrop, weBitTest, hWE)) then
					CheckItem(menu, kItemDragAndDrop, true);

				if Boolean(WEFeatureFlag(weFDrawOffscreen, weBitTest, hWE)) then
					CheckItem(menu, kItemOffscreenDrawing, true);

			end;

{ *** ALIGNMENT MENU *** }
		menu := GetMenuHandle(kMenuAlignment);

{ first remove all check marks }
		for item := CountMItems(menu) downto 1 do
			CheckItem(menu, item, false);

		if (window <> nil) then
			begin

{ find the Alignment menu item corresponding to the current alignment style }
				case WEGetAlignment(hWE) of

					weFlushLeft: 
						item := kItemAlignLeft;

					weFlushRight: 
						item := kItemAlignRight;

					weFlushDefault: 
						item := kItemAlignDefault;

					weCenter: 
						item := kItemCenter;

					weJustify: 
						item := kItemJustify;

					otherwise
						;
				end;  { case }

{ check the menu item }
				CheckItem(menu, item, true);
			end;
	end;  { PrepareMenus }

	procedure DoDeskAcc (menuItem: Integer);
		var
			daName: Str255;
			daNumber: Integer;
	begin
		GetMenuItemText(GetMenuHandle(kMenuApple), menuItem, daName);
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
{$IFC NOT UNDEFINED THINK_PASCAL}
		CustomGetFile(nil, 1, typeList, reply, 0, Point(-1), nil, GetMySFDialogFilter, nil, nil, nil);
{$ELSEC}
		CustomGetFile(nil, 1, @typeList, reply, 0, Point(-1), nil, GetMySFDialogFilter, nil, nil, nil);
{$ENDC}


{ if the user okayed the dialog, create a new window from the specified file }
		if (reply.sfGood) then
			DoOpen := CreateWindow(@reply.sfFile)
		else
			DoOpen := userCanceledErr;

	end;  { DoOpen }

	function SaveWindow (pFileSpec: FSSpecPtr;
									window: WindowPtr): OSErr;
		var
			err: OSErr;
	begin
		ForgetHandle(DocumentPeek(window)^.fileAlias);

{ save the text }
		err := WriteTextFile(pFileSpec, DocumentPeek(window)^.hWE);
		if (err = noErr) then
			begin

{ set the window title to the file name }
				SetWTitle(window, pFileSpec^.name);

{ replace the old window alias (if any) with a new one created from pFileSpec }
				ForgetHandle(DocumentPeek(window)^.fileAlias);
				err := NewAlias(nil, pFileSpec^, AliasHandle(DocumentPeek(window)^.fileAlias));
			end;

		SaveWindow := err;

	end;  { SaveWindow }

	function DoSaveAs (suggestedTarget: FSSpecPtr;
									window: WindowPtr): OSErr;
		var
			hPrompt: StringHandle;
			defaultName: Str255;
			reply: StandardFileReply;
	begin

{ get the prompt string for CustomPutFile from a 'STR ' resource and lock it }
		hPrompt := GetString(kPromptStringID);
		HLockHi(Handle(hPrompt));

{ if a suggested target file is provided, use its name as the default name }
		if (suggestedTarget <> nil) then
			begin
				defaultName := suggestedTarget^.name;
(*SetDefaultDirectory(suggestedTarget^.vRefNum, suggestedTarget^.parID);*)
			end
		else

{ otherwise use the window title as default name for CustomPutFile }
			GetWTitle(window, defaultName);

{ put up the standard Save dialog box }
		CustomPutFile(hPrompt^^, defaultName, reply, 0, Point(-1), nil, GetMySFDialogFilter, nil, nil, nil);

{ unlock the string resource }
		HUnlock(Handle(hPrompt));

{ if the user okayed the dialog, save the window to the specified file }
		if (reply.sfGood) then
			DoSaveAs := SaveWindow(@reply.sfFile, window)
		else
			DoSaveAs := userCanceledErr;

	end;  { DoSaveAs }

	function DoSave (window: WindowPtr): OSErr;
		var
			spec: FSSpec;
			suggestedTarget: FSSpecPtr;
			promptForNewFile, aliasTargetWasChanged: Boolean;
	begin
		suggestedTarget := nil;
		promptForNewFile := true;

{ resolve the alias associated with this window, if any }
		if (DocumentPeek(window)^.fileAlias <> nil) then
			if (ResolveAlias(nil, AliasHandle(DocumentPeek(window)^.fileAlias), spec, aliasTargetWasChanged) = noErr) then
				if (aliasTargetWasChanged) then
					suggestedTarget := @spec
				else
					promptForNewFile := false;

{ if no file has been previously associated with this window, }
{ or if the alias resolution has failed, or if the alias target was changed, }
{ prompt the user for a new destination }
		if (promptForNewFile) then
			DoSave := DoSaveAs(suggestedTarget, window)
		else
			DoSave := SaveWindow(@spec, window);

	end;  { DoSave }

	function DoClose (closing: ClosingOption;
									saving: SavingOption;
									window: WindowPtr): OSErr;
		const
			kButtonSave = 1;
			kButtonCancel = 2;
			kButtonDontSave = 3;
		var
			s: Str255;
			alertResult: Integer;
			err: OSErr;
	begin
		DoClose := noErr;

{ is this window dirty? }
		if (WEGetModCount(DocumentPeek(window)^.hWE) > 0) then
			begin

{ do we have to ask the user whether to save changes? }
				if (saving = savingAsk) then
					begin

{ prepare the parametric strings to be used in the Save Changes alert box }
						GetWTitle(window, s);
						ParamText(s, StringPtr(nil)^, StringPtr(nil)^, StringPtr(nil)^);
						GetIndString(s, kClosingQuittingStringsID, 1 + ORD(closing));
						ParamText(StringPtr(nil)^, s, StringPtr(nil)^, StringPtr(nil)^);

{ put up the Save Changes? alert box }
{$IFC NOT UNDEFINED THINK_PASCAL}
						SetCursor(arrow);
{$ELSEC}
						SetCursor(qd.arrow);
{$ENDC}
						alertResult := Alert(kAlertSaveChanges, GetMyStandardDialogFilter);

{ exit if the user canceled the alert box }
						if (alertResult = kButtonCancel) then
							begin
								DoClose := userCanceledErr;
								Exit(DoClose);
							end;

						if (alertResult = kButtonSave) then
							saving := savingYes
						else
							saving := savingNo;
					end;  { if saving = savingAsk }

				if (saving = savingYes) then
					begin
						err := DoSave(window);
						if (err <> noErr) then
							begin
								DoClose := err;
								Exit(DoClose);
							end;
					end;
			end;  { if window is dirty }

{ destroy the window }
		DestroyWindow(window);

	end;  { DoClose }

	function DoQuit (saving: SavingOption): OSErr;
		var
			window: WindowPtr;
			err: OSErr;
	begin
		DoQuit := noErr;

{ close all open windows }
		repeat
			window := FrontWindow;
			if (window <> nil) then
				begin
					err := DoClose(closingApplication, saving, window);
					if (err <> noErr) then
						begin
							DoQuit := err;
							Exit(DoQuit);
						end;
				end;
		until (window = nil);

{ set a flag so we drop out of the event loop }
		gExiting := true;

	end;  { DoQuit }

	procedure DoAppleChoice (menuItem: Integer);
	begin
		if (menuItem = kItemAbout) then
			DoAboutBox(kDialogAboutBox)
		else
			DoDeskAcc(menuItem);
	end;  { DoAppleChoice }

	procedure DoFileChoice (menuItem: Integer);
		var
			window: WindowPtr;
	begin
		window := FrontWindow;

		case menuItem of

			kItemNew: 
				if (DoNew <> noErr) then
					;

			kItemOpen: 
				if (DoOpen <> noErr) then
					;

			kItemClose: 
				if (DoClose(closingWindow, savingAsk, window) <> noErr) then
					;

			kItemSave: 
				if (DoSave(window) <> noErr) then
					;

			kItemSaveAs:
{ weird as it may seem, using NIL in place of FSSpecPtr(0) }
{ gives me an "internal compiler error" }
				if (DoSaveAs(FSSpecPtr(0), window) <> noErr) then
					;

			kItemQuit: 
				if (DoQuit(savingAsk) <> noErr) then
					;

			otherwise
				;
		end;  { case menuItem }
	end;  { DoFileChoice }

	procedure DoEditChoice (menuItem: Integer);
		var
			window: WindowPtr;
			hWE: WEHandle;
	begin

{ do nothing if no window is active }
		window := FrontWindow;
		if (window = nil) then
			Exit(DoEditChoice);
		hWE := DocumentPeek(window)^.hWE;

		case menuItem of

			kItemUndo: 
				if (WEUndo(hWE) <> noErr) then
					;

			kItemCut: 
				if (WECut(hWE) <> noErr) then
					;

			kItemCopy: 
				if (WECopy(hWE) <> noErr) then
					;

			kItemPaste: 
				if (WEPaste(hWE) <> noErr) then
					;

			kItemClear: 
				if (WEDelete(hWE) <> noErr) then
					;

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
	begin
		window := FrontWindow;
		if (window <> WindowPtr(0)) then
			begin
				GetMenuItemText(GetMenuHandle(kMenuFont), menuItem, fontName);
				GetFNum(fontName, ts.tsFont);
				if (WESetStyle(weDoFont, ts, DocumentPeek(window)^.hWE) <> noErr) then
					;
			end;
	end;  { DoFontChoice }

	procedure DoSizeChoice (menuItem: Integer);
		var
			window: WindowPtr;
			sizeString: Str255;
			longSize: LongInt;
			mode: Integer;
			ts: TextStyle;
	begin
		window := FrontWindow;
		if (window <> nil) then
			begin

				if (menuItem <= kItemLastSize) then
					begin
						GetMenuItemText(GetMenuHandle(kMenuSize), menuItem, sizeString);
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

				if (WESetStyle(mode, ts, DocumentPeek(window)^.hWE) <> noErr) then
					;

			end;
	end;  { DoSizeChoice }

	procedure DoStyleChoice (menuItem: Integer);
		var
			window: WindowPtr;
			ts: TextStyle;
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

				if (WESetStyle(weDoFace + weDoToggleFace, ts, DocumentPeek(window)^.hWE) <> noErr) then
					;

			end;
	end;  { DoStyleChoice }

	procedure DoColorChoice (menuItem: Integer);
		var
			window: WindowPtr;
			i: Integer;
			ts: TextStyle;
	begin

{ do nothing if there is no front window }
		window := FrontWindow;
		if (window = nil) then
			Exit(DoColorChoice);

{ find the color corresponding to the chosen menu item }
		with MenuCRsrcHandle(sColors)^^ do
			for i := numEntries - 1 downto 0 do
				if (mcEntryRecs[i].mctItem = menuItem) then
					begin
						ts.tsColor := mcEntryRecs[i].mctRGB2;
						if (WESetStyle(weDoColor, ts, DocumentPeek(window)^.hWE) <> noErr) then
							;
					end;
	end;  { DoColorChoice }

	procedure DoAlignChoice (menuItem: Integer);
		var
			window: WindowPtr;
			alignment: SignedByte;
	begin
		window := FrontWindow;
		if (window <> nil) then
			begin

				case menuItem of

					kItemAlignDefault: 
						alignment := weFlushDefault;

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

	procedure DoFeatureChoice (menuItem: Integer);
		var
			window: WindowPtr;
			hWE: WEHandle;
			feature, oldSetting: Integer;
	begin
		window := FrontWindow;
		if (window = nil) then
			Exit(DoFeatureChoice);
		hWE := DocumentPeek(window)^.hWE;

		if (menuItem = kItemTabHooks) then
			begin

{ install or remove our custom tab hooks }
				if (WEIsTabHooks(hWE) = false) then
					begin

{ left-align the text (the hooks only work with left-aligned text) }
						WESetAlignment(weFlushLeft, hWE);

{ install tab hooks }
						if (WEInstallTabHooks(hWE) <> noErr) then
							;

					end
				else
					begin

{ remove tab hooks }
						if (WERemoveTabHooks(hWE) <> noErr) then
							;
					end;

{ turn the cursor into a wristwatch }
				SetCursor(GetCursor(watchCursor)^^);

{ recalculate line breaks and redraw the text }
				if (WECalText(hWE) <> noErr) then
					;
			end
		else
			begin
				case menuItem of

					kItemAutoScroll: 
						feature := weFAutoScroll;

					kItemOutlineHilite: 
						feature := weFOutlineHilite;

					kItemReadOnly: 
						feature := weFReadOnly;

					kItemIntCutAndPaste: 
						feature := weFIntCutAndPaste;

					kItemDragAndDrop: 
						feature := weFDragAndDrop;

					kItemOffscreenDrawing: 
						feature := weFDrawOffscreen;

					otherwise
						;
				end;  { case }

{ toggle the specified feature }
				oldSetting := WEFeatureFlag(feature, weBitToggle, hWE);

			end;
	end;  { DoFeatureChoice }

	procedure DoMenuChoice (menuChoice: LongInt);
		var
			menuID, menuItem: Integer;
	begin

{ extract menu ID and menu item from menuChoice }
		menuID := HiWrd(menuChoice);
		menuItem := LoWrd(menuChoice);

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

			kMenuColor: 
				DoColorChoice(menuItem);

			kMenuFeatures: 
				DoFeatureChoice(menuItem);

			kMenuAlignment: 
				DoAlignChoice(menuItem);

			otherwise
				;
		end;  { case menuID }

		HiliteMenu(0);

	end;  { DoMenuChoice }

{$S Init}

	function InitializeMenus: OSErr;
		var
			err: OSErr;
	begin
		InitializeMenus := noErr;

{ build up the whole menu bar from the 'MBAR' resource }
		SetMenuBar(GetNewMBar(kMenuBarID));

{ add names to the Apple and Font menus }
		AppendResMenu(GetMenuHandle(kMenuApple), kTypeDeskAccessory);
		AppendResMenu(GetMenuHandle(kMenuFont), kTypeFont);

{ insert the Alignment submenu into the hierarchical portion of the menu list }
		InsertMenu(GetMenu(kMenuAlignment), -1);

{ disable the "Drag and Drop Editing" item in the Features menu once and for all }
{ if the Drag Manager isn't available }
		if (gHasDragAndDrop = false) then
			DisableItem(GetMenuHandle(kMenuFeatures), kItemDragAndDrop);

{ load the menu color table for the Color menu }
		sColors := GetResource(kTypeMenuColorTable, kMenuColor);
		err := ResError;
		if (err <> noErr) then
			begin
				InitializeMenus := err;
				Exit(InitializeMenus);
			end;
		HNoPurge(sColors);

{ draw the menu bar }
		DrawMenuBar;

	end;  { InitializeMenus }

end.