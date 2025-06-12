UNIT MercutioAPI;

    {xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx}
    {xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx}
    {xxx}
    {xxx     Developer's Programming Interface for Mercutio Menu Definition Function}
    {xxx            ©1992 Ramon M. Felciano, All Rights Reserved}
    {xxx}
    {xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx}
    {xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx}

INTERFACE

	FUNCTION PowerMenuKey (theMessage: longint; theModifiers: integer; hMenu: menuHandle): longint;

	{xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx}
	{x}
	{x	PowerMenuKey is a replacement for the standard toolbox call MenuKey for use with the}
	{x	Mercutio. Given the keypress message and modifiers parameters from a standard event, it }
	{x	checks to see if the keypress is a key-equivalent for a particular menuitem. If you are currently}
	{x	using custom menus (i.e. menus using Mercutio), pass the handle to one of these menus in}
	{x	hMenu. If you are not using custom menus, pass in NIL or another menu, and PowerMenuKey will use the}
	{x	standard MenuKey function to interpret the keypress.}
	{x}
	{x	As with MenuKey, PowerMenuKey returns the menu ID in high word of the result, and the menu}
	{x	item in the low word.}
	{x}
	{xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx}


	FUNCTION GetMDEFCopyright (menu: MenuHandle): str255;
	FUNCTION GetMDEFVersion (menu: MenuHandle): longint;


	{xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx}
	{x}
	{x	These routines allow you to retrieve the Copyright and Version information}
	{x	embedded within the MDEF. The Version information is returned as a longint,}
	{x	which can be typecast to a normal version resource.}
	{x}
	{xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx}






IMPLEMENTATION
	CONST
		customDefProcSig = 'CUST';
		areYouCustomMsg = 128;
		getVersionMsg = 131;
		getCopyrightMsg = 132;
		mMenuKeyMsg = 262;

	TYPE
	{* MENU resource structures *}
		stdItemData = PACKED RECORD
				iconID: byte;
				keyEq: char;
				mark: char;
				textStyle: style;
			END;
		StdItemDataPtr = ^stdItemData;

	{* Menubar structures *}
		MBarRec = PACKED RECORD
				offsetToLastMenu: integer;
				HorizRtEdgeLastMenu: integer;
				dummy: integer;
				menuIDList: ARRAY[1..100] OF PACKED RECORD
						theMenu: menuhandle;
						HorizRtEdge: integer;
					END;
			END;
		MBarRecPtr = ^MBarRec;

	{* MenuKey result structure *}
		menuMatch = PACKED RECORD			{ to get around pascal's typing	}
				CASE boolean OF
					true: (
							L: LongInt;
					);
					false: (
							menuID, itemNum: integer;
					);
			END;




	PROCEDURE CallMDEF (message: integer; theMenu: MenuHandle; VAR menuRect: Rect; hitPt: Point; VAR whichItem: integer; defProc: ProcPtr);
	{xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx}
	{x	Simple inline assembly code to call an MDEF, courtesy of John Cavallino. }
	{xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx}
	INLINE
		$205F,	{ move.l (SP)+,A0 }
		$4E90;	{ jsr (A0) }



	FUNCTION IsCustomMenu (menu: MenuHandle): boolean;
	{xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx}
	{x}
	{x	IsCustomMenu returns true if hMenu is controlled by a custom MDEF. This relies on my}
	{x	convention of returning the customDefProcSig constant in the rect parameter: this obtuse}
	{x	convention should be unique enough that only my custom MDEFs behave this way.}
	{x}
	{xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx}
		VAR
			state: SignedByte;
			proc: handle;
			dummy: rect;
			dummyInt: integer;
	BEGIN
		proc := menu^^.menuProc;
		state := HGetState(proc);
		HLock(proc);
		dummy.topLeft := point(longint(0));
		CallMDEF(areYouCustomMsg, menu, dummy, point(longint(0)), dummyInt, proc^);
		HSetState(proc, state);
		IsCustomMenu := longint(dummy.topLeft) = longint(customDefProcSig);
	END;



	FUNCTION GetMDEFVersion (menu: MenuHandle): longint;
		VAR
			state: SignedByte;
			proc: handle;
			dummy: rect;
			dummyInt: integer;
	BEGIN
		proc := menu^^.menuProc;
		state := HGetState(proc);
		HLock(proc);
		dummy.topLeft := point(longint(0));
		CallMDEF(getVersionMsg, menu, dummy, point(longint(0)), dummyInt, proc^);
		HSetState(proc, state);
		GetMDEFVersion := longint(dummy.topLeft);
	END;

	FUNCTION GetMDEFCopyright (menu: MenuHandle): str255;
		VAR
			state: SignedByte;
			proc: handle;
			dummy: rect;
			dummyInt: integer;
			hCopyright: stringHandle;
	BEGIN
		GetMDEFCopyright := '';
		proc := menu^^.menuProc;
		state := HGetState(proc);
		HLock(proc);
		dummy.topLeft := point(longint(0));
		CallMDEF(getCopyrightMsg, menu, dummy, point(longint(0)), dummyInt, proc^);
		HSetState(proc, state);
		hCopyright := stringHandle(dummy.topLeft);
		IF hCopyright <> NIL THEN
			GetMDEFCopyright := hCopyright^^;
		disposeHandle(handle(hCopyright));
	END;



	FUNCTION PowerMenuKey (theMessage: longint; theModifiers: integer; hMenu: menuHandle): longint;
		VAR
			state: SignedByte;
			proc: handle;
			dummyRect: rect;
			dummyInt: integer;
	BEGIN
		IF ((hMenu = NIL) | (NOT IsCustomMenu(hMenu))) THEN
			PowerMenuKey := MenuKey(char(bitAnd(theMessage, charcodemask)))
		ELSE
			BEGIN
				proc := hMenu^^.menuProc;
				state := HGetState(proc);
				HLock(proc);
				dummyRect.topLeft := point(longint(0));
				CallMDEF(mMenuKeyMsg, hMenu, dummyRect, point(theMessage), themodifiers, proc^);
				HSetState(proc, state);
				PowerMenuKey := longint(dummyRect.topleft);
			END;
	END;

END.