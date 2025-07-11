unit ICMovableModal;

{ This unit implements a MovableModalDialog routine similar to }
{ the Toolbox routine ModalDialog, to be used for movable modal dialogs }

{ Based on code by Merzwaren <piovanel@dsi.unimi.it> }

interface

	procedure DisableMenuBar (var saved_state: univ Ptr; hmnuID: Integer);
	procedure ReEnableMenuBar (var saved_state: univ Ptr);
	procedure MovableModalDialog (filterProc: ProcPtr; var itemHit: Integer);

implementation

	uses
		GestaltEqu, ICTypes, ICDialogs, ICGlobals;

	const
		kSystemMenuThreshold = -16000;		{ menu IDs <= than this are used by the system }
		kMovableModalEventMask = mDownMask + mUpMask + keyDownMask + keyUpMask + autoKeyMask + updateMask + activMask + app4Mask;

	type
		MenuEntry = record
				hMenu: MenuHandle;
				leftEdge: Integer;
			end;

		MenuList = record
				offsetToLastMenu: Integer;
				rightmostEdge: Integer;
				unused: Integer;
				theMenus: array[0..1000] of MenuEntry;
			end;
		MenuListPtr = ^MenuList;
		MenuListHandle = ^MenuListPtr;

		MenuBarState = record
				mbsBarEnable: LongInt;
				mbsEditEnable: LongInt;
			end;
		MenuBarStatePtr = ^MenuBarState;

	function HasHelpManager: Boolean;
		var
			response: LongInt;
	begin
		HasHelpManager := (Gestalt(gestaltHelpMgrAttr, response) = noErr) & BTST(response, gestaltHelpMgrPresent);
	end;

	function LMGetMenuList: Handle;
	inline
		$2EB8, $0A1C;			{ MOVE.L $0A1C,(SP) }

	function HMSetMenuResID (menuID: integer; resID: integer): OSErr;
	inline
		$303C, $020D, $A830;

	procedure DisableMenuBar (var saved_state: univ Ptr; hmnuID: Integer);
		var
			menuList: MenuListHandle;
			i, nMenus: Integer;
			theMenu: MenuHandle;
			menuID: Integer;
			menuEnable: LongInt;
			theDialog: DialogPtr;
			hasBalloons, needEditMenu: Boolean;
			junk: OSErr;
			state: MenuBarStatePtr;
	begin
		hasBalloons := HasHelpManager;

		theDialog := FrontWindow;
		needEditMenu := (theDialog <> nil) & (SelectedTextItem(theDialog) > 0);

		menuList := MenuListHandle(LMGetMenuList);
		nMenus := menuList^^.offsetToLastMenu div SizeOf(MenuEntry);

		saved_state := NewPtr(SizeOf(MenuBarState));
		state := MenuBarStatePtr(saved_state);

		state^.mbsBarEnable := 0;
		for i := 0 to nMenus - 1 do begin
			theMenu := menuList^^.theMenus[i].hMenu;
			menuID := theMenu^^.menuID;

			if (menuID > kSystemMenuThreshold) then begin { do nothing if this is a system menu }

				if (menuID = M_Edit) then begin
					state^.mbsEditEnable := theMenu^^.enableFlags;
					if needEditMenu then begin
						theMenu^^.enableFlags := 1 + BSL(1, EM_Cut) + BSL(1, EM_Copy) + BSL(1, EM_Paste);
					end
					else begin
						DisableItem(theMenu, 0);
					end;
				end
				else begin { if this menu is enabled, disable it and set the corresponding bit  }
					if (BTST(theMenu^^.enableFlags, 0)) then begin
						BSET(state^.mbsBarEnable, i);
						DisableItem(theMenu, 0);
					end;

					if (hasBalloons) then begin { remap the help strings for this menu }
						junk := HMSetMenuResID(menuID, hmnuID);
					end;
				end;
			end;
		end;

		HiliteMenu(0);
		DrawMenuBar;
	end;

	procedure ReEnableMenuBar (var saved_state: univ Ptr);
		var
			menuList: MenuListHandle;
			i, nMenus: Integer;
			theMenu: MenuHandle;
			menuID: Integer;
			hasBalloons: Boolean;
			err: OSErr;
			state: MenuBarStatePtr;
	begin
		state := MenuBarStatePtr(saved_state);

		hasBalloons := HasHelpManager;

		menuList := MenuListHandle(LMGetMenuList);
		nMenus := menuList^^.offsetToLastMenu div SizeOf(MenuEntry);

		for i := 0 to nMenus - 1 do begin
			theMenu := menuList^^.theMenus[i].hMenu;
			menuID := theMenu^^.menuID;

			if (menuID > kSystemMenuThreshold) then begin
				if (menuID = M_Edit) then begin
					theMenu^^.enableFlags := state^.mbsEditEnable;
				end
				else if (BTST(state^.mbsBarEnable, i)) then begin
					EnableItem(theMenu, 0);
				end;
				if (hasBalloons) then begin
					err := HMSetMenuResID(menuID, -1);
				end;
			end;
		end;

		DisposePtr(saved_state);
		saved_state := nil;
		DrawMenuBar;
	end;

	function CallFilter (dialog: DialogPtr; var event: EventRecord; var item: Integer; filterProc: ProcPtr): Boolean;
	inline
		$205F, $4E90;

	procedure CallBeeper (soundNo: Integer; beeperProc: ProcPtr);
	inline
		$205F, $4E90;

	function LMGetDABeeper: ProcPtr;
	inline
		$2EB8, $0A9C;			{ MOVE.L $0A9C,(SP) }

	function DoMenuChoice (theDialog: DialogPtr; var theEvent: EventRecord; var itemHit: Integer; menuChoice: LongInt): Boolean;
		var
			menuID, menuItem: Integer;
			currentEditField, itemType: Integer;
			junk: OSErr;
	begin
		DoMenuChoice := false;

		menuID := HiWord(menuChoice);
		menuItem := LoWord(menuChoice);

		if (menuID = M_Edit) then begin
			currentEditField := SelectedTextItem(theDialog);
			GetDItemKind(theDialog, currentEditField, itemType);

			if (BAND(itemType, itemDisable) = 0) then begin { if the current edit field is an enabled item, exit from MovableModalDialog loop }
				DoMenuChoice := true;
				itemHit := currentEditField;
			end;

			if (menuItem = EM_Cut) then begin
				DlgCut(theDialog);
				junk := ZeroScrap;
				junk := TEToScrap;
			end
			else if (menuItem = EM_Copy) then begin
				DlgCopy(theDialog);
				junk := ZeroScrap;
				junk := TEToScrap;
			end
			else if (menuItem = EM_Paste) then begin
				DlgPaste(theDialog);
			end;
		end;

		HiliteMenu(0);
	end;

	function HandleMouseDown (theDialog: DialogPtr; var theEvent: EventRecord; var itemHit: Integer): Boolean;
		var
			partCode: Integer;
			wind: WindowPtr;
			beeper: ProcPtr;
			dragRect: Rect;
	begin
		HandleMouseDown := false;

		partCode := FindWindow(theEvent.where, wind);

		if (partCode = inMenuBar) then begin
			HandleMouseDown := DoMenuChoice(theDialog, theEvent, itemHit, MenuSelect(theEvent.where));
		end
		else if (not PtInRgn(theEvent.where, WindowPeek(theDialog)^.strucRgn)) then begin
			beeper := LMGetDABeeper;
			if (beeper <> nil) then begin
				CallBeeper(1, beeper);
			end;
		end
		else if (partCode = inDrag) & (theDialog = wind) then begin { now, we have to handle the only thing DialogSelect doesn't do for us: dragging }
			dragRect := GetGrayRgn^^.rgnBBox;
			DragWindow(wind, theEvent.where, dragRect);
			theEvent.what := nullEvent;
		end;
	end;

	procedure MovableModalDialog (filterProc: ProcPtr; var itemHit: Integer);
		var
			savePort: GrafPtr;
			theDialog, junk_dialog: DialogPtr;
			theEvent: EventRecord;
			gotEvent: Boolean;
	begin
		itemHit := 0;
		theDialog := FrontWindow;
		if (theDialog <> nil) then begin
			SetPort(theDialog);
			repeat
				gotEvent := WaitNextEvent(kMovableModalEventMask, theEvent, 0, nil);
				SetPort(theDialog);

				if (filterProc <> nil) & CallFilter(theDialog, theEvent, itemHit, filterProc) then begin
					Leave;
				end;

				if (theEvent.what = mouseDown) & HandleMouseDown(theDialog, theEvent, itemHit) then begin
					Leave;
				end;

				if (theEvent.what = keyDown) & (BAND(theEvent.modifiers, cmdKey) <> 0) & DoMenuChoice(theDialog, theEvent, itemHit, MenuKey(CHR(BAND(theEvent.message, charCodeMask)))) then begin
					Leave;
				end;

				if IsDialogEvent(theEvent) & DialogSelect(theEvent, junk_dialog, itemHit) then begin
					Leave;
				end;
			until false;
		end;

	end;

end.