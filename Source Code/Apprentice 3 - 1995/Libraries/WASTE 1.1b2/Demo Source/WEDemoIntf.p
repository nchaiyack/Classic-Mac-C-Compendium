unit WEDemoIntf;

{ WASTE DEMO PROJECT: }
{ Global interface: constant, type and class declarations used in most other units }

{ Copyright © 1993-1995 Marco Piovanelli }
{ All Rights Reserved }

interface
	uses
		Types, Resources, Quickdraw, Menus, Windows, Controls, WASTE;

	const

{ WASTE demo signature }

		kAppSignature = 'OEDE';

{ resource types, clipboard types and file types }

		kTypeDeskAccessory = 'DRVR';
		kTypeFont = 'FONT';
		kTypePicture = 'PICT';
		kTypeSound = 'snd ';
		kTypeSoup = 'SOUP';
		kTypeStyles = 'styl';
		kTypeText = 'TEXT';

{ menu IDs }

		kMenuApple = 1;
		kMenuFile = 2;
		kMenuEdit = 3;
		kMenuFont = 4;
		kMenuSize = 5;
		kMenuStyle = 6;
		kMenuColor = 7;
		kMenuFeatures = 8;
		kMenuAlignment = 9;

{ Apple menu items }

		kItemAbout = 1;

{ File menu items }

		kItemNew = 1;
		kItemOpen = 2;
		kItemClose = 4;
		kItemSave = 5;
		kItemSaveAs = 6;
		kItemQuit = 8;

{ Edit menu items }

		kItemUndo = 1;
		kItemCut = 3;
		kItemCopy = 4;
		kItemPaste = 5;
		kItemClear = 6;
		kItemSelectAll = 7;

{ Size menu items }

		kItemLastSize = 6;
		kItemSmaller = 8;
		kItemLarger = 9;

{ Style menu items }

		kItemPlainText = 1;
		kItemBold = 2;
		kItemItalic = 3;
		kItemUnderline = 4;
		kItemOutline = 5;
		kItemShadow = 6;
		kItemCondensed = 7;
		kItemExtended = 8;

{ Color menu items }

		kItemBlack = 1;
		kItemRed = 2;
		kItemGreen = 3;
		kItemBlue = 4;
		kItemCyan = 5;
		kItemMagenta = 6;
		kItemYellow = 7;

{ Alignment menu items }

		kItemAlignDefault = 1;
		kItemAlignLeft = 3;
		kItemCenter = 4;
		kItemAlignRight = 5;
		kItemJustify = 6;

{ Features menu item }

		kItemAlignment = 1;
		kItemTabHooks = 2;
		kItemAutoScroll = 4;
		kItemOutlineHilite = 5;
		kItemReadOnly = 6;
		kItemIntCutAndPaste = 7;
		kItemDragAndDrop = 8;
		kItemOffscreenDrawing = 9;

{ Alert & dialog template resource IDs }

		kAlertNeedSys7 = 128;
		kAlertGenError = 130;
		kAlertSaveChanges = 131;
		kDialogAboutBox = 256;

{ String list resource IDs }

		kUndoStringsID = 128;
		kClosingQuittingStringsID = 129;

{ miscellaneous resource IDs }

		kMenuBarID = 128;
		kWindowTemplateID = 128;
		kScrollBarTemplateID = 128;
		kPromptStringID = 128;

{ virtual key codes for navigation keys found on extended keyboards }

		keyPgUp = $74;
		keyPgDn = $79;
		keyHome = $73;
		keyEnd = $77;

{ other commonly used constants }

		kBarWidth = 16;				{ width of a scroll bar }
		kTitleHeight = 20;				{ usual height of a window title bar }
		kTextMargin = 3;				{ indent of text rect from window port rect }
		kScrollDelta = 11;				{ pixels to scroll when the scroll bar arrow is clicked }

	type

{ enumeration types used for closing a window and/or quitting the application }

		ClosingOption = (closingWindow, closingApplication);
		SavingOption = (savingYes, savingNo, savingAsk);

{ a ScrollBarPair is just a pair of control handles }

		ScrollBarPair = record
				case Integer of
					0: (
							v, h: ControlRef;
					);
					1: (
{$IFC NOT UNDEFINED THINK_PASCAL}
							vh: array[VHSelect] of ControlRef;
{$ELSEC}
							vh: array[0..1] of ControlRef;
{$ENDC}
					);
			end;  { ScrollBarPair }

{ a DocumentRecord is a window record with additional fields }

		DocumentRecord = record
				window: WindowRecord;					{ the window }
				scrollBars: ScrollBarPair;					{ its scroll bars }
				hWE: WEHandle;								{ its WASTE instance }
				fileAlias: Handle;								{ alias to associated file }
			end;  { DocumentRec }
		DocumentPeek = ^DocumentRecord;

	var

{ global variables }

		gHasColorQD: Boolean;			{ TRUE if Color QuickDraw is available }
		gHasDragAndDrop: Boolean;	{ TRUE if the Drag Manager is available }
		gHasTextServices: Boolean;	{ TRUE if the Text Services Manager is available }
		gExiting: Boolean;					{ set this variable to drop out of event loop and quit }

{ general purpose utility routines }

	procedure ErrorAlert (err: OSErr);
	procedure ForgetHandle (var h: univ Handle);
	procedure ForgetResource (var h: univ Handle);
	function NewHandleTemp (blockSize: Size;
									var h: univ Handle): OSErr;

implementation
	uses
		DialogUtils;

	procedure ErrorAlert (err: OSErr);
		var
			errString: Str255;
			alertResult: Integer;
	begin
		NumToString(err, errString);
		ParamText(errString, '', '', '');
{$IFC NOT UNDEFINED THINK_PASCAL}
		SetCursor(arrow);
{$ELSEC}
		SetCursor(qd.arrow);
{$ENDC}
		alertResult := Alert(kAlertGenError, GetMyStandardDialogFilter);
	end;  { ErrorAlert }

	procedure ForgetHandle (var h: univ Handle);
		var
			theHandle: Handle;
	begin
		theHandle := h;
		if (theHandle <> nil) then
			begin
				h := nil;
				DisposeHandle(theHandle);
			end;
	end;  { ForgetHandle }

	procedure ForgetResource (var h: univ Handle);
		var
			theHandle: Handle;
	begin
		theHandle := h;
		if (theHandle <> nil) then
			begin
				h := nil;
				ReleaseResource(theHandle);
			end;
	end;  { ForgetResource }

	function NewHandleTemp (blockSize: Size;
									var h: univ Handle): OSErr;

{ allocate a new relocatable block from temporary memory or, }
{ if that fails, from the current heap }

		var
			err: OSErr;
	begin

{ first try tapping temporary memory }
		h := TempNewHandle(blockSize, err);
		NewHandleTemp := err;

{ in case of failure, try with current heap }
		if (h = nil) then
			begin
				h := NewHandle(blockSize);
				NewHandleTemp := MemError;
			end;

	end;  { NewHandleTemp }

end.