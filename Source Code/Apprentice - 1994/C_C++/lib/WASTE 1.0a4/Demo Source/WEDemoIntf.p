unit DemoIntf;

{ WASTE DEMO PROJECT: }
{ Global interface: constant, type and class declarations used in most other units }

{ Copyright © 1993-1994 Merzwaren }
{ All Rights Reserved }

interface
	uses
		DialogUtils, TextServices, WASTE;

	const

{ WASTE demo signature }

		kAppSignature = 'OEDE';

{ resource types, clipboard types and file types }

		kTypeDeskAccessory = 'DRVR';
		kTypeFont = 'FONT';
		kTypeStyleScrap = 'styl';
		kTypeText = 'TEXT';

{ menu IDs }

		kMenuApple = 1;
		kMenuFile = 2;
		kMenuEdit = 3;
		kMenuFont = 4;
		kMenuSize = 5;
		kMenuStyle = 6;
		kMenuAlignment = 7;

{ Apple menu items }

		kItemAbout = 1;

{ File menu items }

		kItemNew = 1;
		kItemOpen = 2;
		kItemClose = 4;
		kItemSaveAs = 5;
		kItemQuit = 7;

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

{ Alignment menu items }

		kItemAlignLeft = 1;
		kItemCenter = 2;
		kItemAlignRight = 3;
		kItemJustify = 4;

{ Alert template resource IDs }

		kAlertNeedSys7 = 128;
		kAlertAboutBox = 129;
		kAlertGenError = 130;

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

{ values for HiliteControl hiliteState parameter }

		kCtlActive = 0;					{ active control }
		kCtlHilited = 1;					{ highlighted active control }
		kCtlBackground = 254;		{ inactive control in an inactive window }
		kCtlInactive = 255;			{ inactive control in the active window }

{ other commonly used constants }

		kBarWidth = 16;				{ width of a scroll bar }
		kTitleHeight = 20;				{ usual height of a window title bar }
		kTextMargin = 3;				{ indent of text rect from window port rect }
		kScrollDelta = 11;				{ pixels to scroll when the scroll bar arrow is clicked }

	type

{ a ScrollBarPair is just a pair of control handles }

		ScrollBarPair = record
				case Integer of
					0: (
							v, h: ControlHandle;
					);
					1: (
							vh: array[VHSelect] of ControlHandle;
					);
			end;  { ScrollBarPair }

{ a DocumentRecord is a window record with additional fields }

		DocumentRecord = record
				window: WindowRecord;					{ the window }
				scrollBars: ScrollBarPair;					{ its scroll bars }
				hWE: WEHandle;								{ its WASTE instance }
			end;  { DocumentRec }
		DocumentPeek = ^DocumentRecord;

	var

{ global variables }

		gHasColorQD: Boolean;			{ TRUE if Color QuickDraw is available }
		gHasTextServices: Boolean;	{ TRUE if the Text Services Manager is available }
		gExiting: Boolean;					{ set this variable to drop out of event loop and quit }

{ general purpose utility routines }

	procedure ErrorAlert (err: OSErr);
	function NewHandleTemp (blockSize: Size;
									var h: univ Handle): OSErr;

implementation

	procedure ErrorAlert (err: OSErr);
		var
			errString: Str255;
			alertResult: Integer;
	begin
		NumToString(err, errString);
		ParamText(errString, '', '', '');
		alertResult := Alert(kAlertGenError, @DialogFilter);
	end;  { ErrorAlert }

	function NewHandleTemp (blockSize: Size;
									var h: univ Handle): OSErr;

{ allocate a new relocatable block from temporary memory or, }
{ if that fails, from the current heap }

	begin

{ first try tapping temporary memory }
		h := TempNewHandle(blockSize, NewHandleTemp);

{ in case of failure, try with current heap }
		if (h = nil) then
			begin
				h := NewHandle(blockSize);
				NewHandleTemp := MemError;
			end;

	end;  { NewHandleTemp }

end.