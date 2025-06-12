unit WASTE;

{ Pascal interface to the WASTE text engine: }
{ version 1.0a3 }

{ Copyright © 1993-1994 Merzwaren }
{ All Rights Reserved }

interface
	uses
		LongCoords;

	const

{ alignment styles }

		weFlushLeft = -2;						{ flush left }
		weFlushRight = -1;					{ flush right }
		weFlushDefault = 0;					{ flush according to system direction }
		weCenter = 1;							{ centered }
		weJustify = 2;							{ fully justified }

{ values for the mode parameter in WESetStyle and WEContinuousStyle }

		weDoFont = $0001;					{ set font family number }
		weDoFace = $0002;					{ turn on the styles specified in tsFace }
		weDoSize = $0004;					{ set type size }
		weDoColor = $0008;					{ set color }
		weDoAll = weDoFont + weDoFace + weDoSize + weDoColor;
		weDoAddSize = $0010;				{ adjust type size }
		weDoToggleFace = $0020;			{ turn a style off if it's continuous over the selection }
		weDoReplaceFace = $0040;		{ replace existing styles outright }

{ values for the edge parameter in WEGetOffset etc. }

		kLeadingEdge = -1;					{ offset refers to leading edge of specified character }
		kTrailingEdge = 0;						{ offset refers to trailing edge of previous character }

{ values for WEFeatureFlag feature parameter }

		weFAutoScroll = 0;					{ automatically scroll the selection range into view }
		weFOutlineHilite = 2;				{ frame selection when deactivated }
		weFInhibitRecal = 9;					{ don't recalculate line starts and don't redraw text }
		weFUseTempMem = 10;			{ use temporary memory for main data structures }
		weFDrawOffscreen = 11;			{ draw text offscreen for smoother visual results }

{ values for WENew flags parameter }

		weDoAutoScroll = $0001;
		weDoOutlineHilite = $0004;
		weDoInhibitRecal = $0200;
		weDoUseTempMem = $0400;
		weDoDrawOffscreen = $0800;

{ values for WEFeatureFlag action parameter }

		weBitClear = 0;							{ disables the specified feature }
		weBitSet = 1;							{ enables the specified feature }
		weBitTest = -1;							{ returns the current setting of the specified feature }

{ values for WEGetInfo and WESetInfo selector parameter }

		weClickLoop = 'clik';					{ click loop callback }
		wePort = 'port';							{ graphics port }
		weRefCon = 'refc';						{ reference constant for use by application }
		weScrollProc = 'scrl';					{ scroll callback }
		weText = 'text';							{ handle to the raw text }
		weTSMDocumentID = 'tsmd';		{ Text Services Manager document ID }
		weTSMPreUpdate = 'pre ';			{ Text Services Manager pre-update callback }
		weTSMPostUpdate = 'post';		{ Text Services Manager post-update callback }

	type

{ WASTE data types }

		WERunInfo = record
				runStart: LongInt;			{ byte offset to first character of style run }
				runEnd: LongInt;			{ byte offset past last character of style run }
				runHeight: Integer;		{ line height (ascent + descent + leading) }
				runAscent: Integer;		{ font ascent }
				runStyle: TextStyle;		{ text attributes }
			end;  { WERunInfo }

		WEHandle = Handle;

{ callback prototypes }

{ FUNCTION MyClickLoop(hWE: WEHandle): Boolean; }
		WEClickLoopProcPtr = ProcPtr;

{ PROCEDURE MyScrollProc(hWE: WEHandle); }
		WEScrollProcPtr = ProcPtr;

{ PROCEDURE MyTSMPreUpdate(hWE: WEHandle); }
		WETSMPreUpdateProcPtr = ProcPtr;

{ FUNCTION MyTSMPostUpdate(hWE: WEHandle; fixLength: LongInt; }
{ 		inputAreaStart, inputAreaEnd:LongInt; }
{		pinRangeStart, pinRangeEnd: LongInt); }
		WETSMPostUpdateProcPtr = ProcPtr;

{ WASTE Public Calls }

{ creation and destruction }
	function WENew (destRect, viewRect: LongRect;
									flags: Integer;
									var hWE: WEHandle): OSErr;
	procedure WEDispose (hWE: WEHandle);

{ getting variables }
	function WEGetText (hWE: WEHandle): Handle;
	function WEGetChar (offset: LongInt;
									hWE: WEHandle): Char;
	function WEGetTextLength (hWE: WEHandle): LongInt;
	function WECountLines (hWE: WEHandle): LongInt;
	function WEGetHeight (startLine, endLine: LongInt;
									hWE: WEHandle): LongInt;
	procedure WEGetSelection (var selStart, selEnd: LongInt;
									hWE: WEHandle);
	procedure WEGetDestRect (var destRect: LongRect;
									hWE: WEHandle);
	procedure WEGetViewRect (var viewRect: LongRect;
									hWE: WEHandle);
	function WEIsActive (hWE: WEHandle): Boolean;

{ setting variables }
	procedure WESetSelection (selStart, selEnd: LongInt;
									hWE: WEHandle);
	procedure WESetDestRect (destRect: LongRect;
									hWE: WEHandle);
	procedure WESetViewRect (viewRect: LongRect;
									hWE: WEHandle);

{ accessing style run information }
	function WEContinuousStyle (var mode: Integer;
									var ts: TextStyle;
									hWE: WEHandle): Boolean;
	procedure WEGetRunInfo (offset: LongInt;
									var info: WERunInfo;
									we: Handle);

{ converting byte offsets to screen positions and vice versa }
	function WEGetOffset (thePoint: LongPoint;
									var edge: SignedByte;
									we: Handle): LongInt;
	procedure WEGetPoint (offset: LongInt;
									var thePoint: LongPoint;
									var lineHeight: Integer;
									we: Handle);

{ finding words and lines }
	procedure WEFindWord (offset: LongInt;
									edge: SignedByte;
									var wordStart, wordEnd: LongInt;
									hWE: WEHandle);
	procedure WEFindLine (offset: LongInt;
									edge: SignedByte;
									var lineStart, lineEnd: LongInt;
									hWE: WEHandle);

{ making a copy of a text range }
	function WECopyRange (rangeStart, rangeEnd: LongInt;
									hText: Handle;
									hStyles: StScrpHandle;
									hWE: WEHandle): OSErr;

{ getting and setting the alignment style }
	function WEGetAlignment (hWE: WEHandle): SignedByte;
	procedure WESetAlignment (alignment: SignedByte;
									hWE: WEHandle);

{ recalculating line breaks, drawing and scrolling }
	function WECalText (hWE: WEHandle): OSErr;
	procedure WEUpdate (updateRgn: RgnHandle;
									hWE: WEHandle);
	procedure WEScroll (hOffset, vOffset: LongInt;
									hWE: WEHandle);
	procedure WESelView (hWE: WEHandle);

{ handling activate/deactivate events }
	procedure WEActivate (hWE: WEHandle);
	procedure WEDeactivate (hWE: WEHandle);

{ handling key-down events }
	procedure WEKey (key: Char;
									modifiers: Integer;
									hWE: WEHandle);

{ handling mouse-down events and mouse tracking }
	procedure WEClick (hitPt: Point;
									modifiers: Integer;
									clickTime: LongInt;
									hWE: WEHandle);

{ adjusting the cursor shape }
	function WEAdjustCursor (mouseLoc: Point;
									mouseRgn: RgnHandle;
									hWE: WEHandle): Boolean;

{ blinking the caret }
	procedure WEIdle (var maxSleep: LongInt;
									hWE: WEHandle);

{ modifying the text and the styles }
	function WEInsert (textPtr: Ptr;
									textLength: LongInt;
									styleScrap: STScrpHandle;
									hWE: WEHandle): OSErr;
	function WEDelete (hWE: WEHandle): OSErr;
	function WESetStyle (mode: Integer;
									ts: TextStyle;
									hWE: WEHandle): OSErr;
	function WEUseText (text: Handle;
									hWE: WEHandle): OSErr;
	function WEUseStyleScrap (styleScrap: STScrpHandle;
									hWE: WEHandle): OSErr;

{ Clipboard operations }
	function WECut (hWE: WEHandle): OSErr;
	function WECopy (hWE: WEHandle): OSErr;
	function WEPaste (hWE: WEHandle): OSErr;

{ Script Manager utilities }
	function WECharByte (offset: LongInt;
									hWE: WEHandle): Integer;
	function WECharType (offset: LongInt;
									hWE: WEHandle): Integer;

{ Text Services Manager support }
	function WEInstallTSMHandlers: OSErr;
	procedure WEStopInlineSession (hWE: WEHandle);

{ additional features }
	function WEFeatureFlag (feature: Integer;
									action: Integer;
									hWE: WEHandle): Integer;
	function WEGetInfo (selector: OSType;
									info: univ Ptr;
									hWE: WEHandle): OSErr;
	function WESetInfo (selector: OSType;
									info: univ Ptr;
									hWE: WEHandle): OSErr;

{ long coordinates utility routines }
	procedure WELongPointToPoint (lp: LongPoint;
									var p: Point);
	procedure WEPointToLongPoint (p: Point;
									var lp: LongPoint);
	procedure WESetLongRect (var lr: LongRect;
									left, top, right, bottom: LongInt);
	procedure WELongRectToRect (lr: LongRect;
									var r: Rect);
	procedure WERectToLongRect (r: Rect;
									var lr: LongRect);
	procedure WEOffsetLongRect (var lr: LongRect;
									hOffset, vOffset: LongInt);
	function WELongPointInLongRect (lp: LongPoint;
									lr: LongRect): Boolean;

implementation
end.