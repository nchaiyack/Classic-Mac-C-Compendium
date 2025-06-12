unit TextServices;

{ Pascal interface to the Macintosh Libraries: }
{ Text Services Manager }

{ Based on material copyrighted by Apple Computer, Inc. }
{ ANTI© 1993 Merzwaren }

interface
	uses
		AppleEvents, Components;

	const

		gestaltTSMgrVersion = 'tsmv';			{ Text Services Manager gestalt selector }

		kTSMVersion = 1;								{ version of Text Services Manager }
		kTextService = 'tsvc';						{ component type }
		kInputMethodService = 'inpm';			{ component subtype }

		bTakeActiveEvent = 15;					{ bit set if component takes activate events }
		bScriptMask = $00007F00;
		bLanguageMask = $000000FF;
		bScriptLanguageMask = bScriptMask + bLanguageMask;

{ hilite styles }

		kCaretPosition = 1;							{ specify caret position }
		kRawText = 2;									{ specify range of raw text }
		kSelectedRawText = 3;						{ specify range of selected raw text }
		kConvertedText = 4;							{ specify range of converted text }
		kSelectedConvertedText = 5;				{ specify range of selected converted text }

{ Apple Event constants }

		kTextServiceClass = kTextService;	{ event class }
		kUpdateActiveInputArea = 'updt';	{ update active inline area }
		kPos2Offset = 'p2st';							{ convert global coordinates to text offset }
		kOffset2Pos = 'st2p';							{ convert text offset to global coordinates }
		kShowHideInputWindow = 'shiw';	{ show or hide the input window }

{ event keywords }

		keyAETSMDocumentRefCon = 'refc';	{ TSM document refcon }
		keyAEServerInstance = 'srvi';			{ server instance }
		keyAETheData = 'kdat';						{ text }
		keyAEScriptTag = 'sclg';					{ script & language tags }
		keyAEFixLength = 'fixl';					{ length of confirmed text }
		keyAEHiliteRange = 'hrng';				{ hilite range array }
		keyAEUpdateRange = 'udng';			{ update range array }
		keyAEClauseOffsets = 'clau';				{ clause offsets array }
		keyAECurrentPoint = 'cpos';				{ current point }
		keyAEDragging = 'bool';					{ dragging flag }
		keyAEOffset = 'ofst';							{ text offset }
		keyAERegionClass = 'rgnc';				{ region class }
		keyAEPoint = 'gpos';							{ current point }
		keyAEBufferSize = 'buff';					{ buffer size to get the text }
		keyAERequestedType = 'rtyp';			{ requested text type }
		keyAEMoveView = 'mvvw';				{ move view flag }
		keyAELength = 'leng';						{ length }
		keyAENextBody = 'nxbd';					{ next or previous body }

{ optional keywords for Offset2Pos }

		keyAETextFont = 'ktxf';
		keyAETextPointSize = 'ktps';
		keyAETextLineHeight = 'ktlh';
		keyAETextLineAscent = 'ktas';
		keyAEAngle = 'kang';

{ optional keyword for Pos2Offset }

		keyAELeftSide = 'klef';

{ optional keyword for ShowHideInputWindow }

		keyAEShowHideInputWindow = 'shiw';

{ keyword for PinRange }

		keyAEPinRange = 'pnrg';

{ descriptor types }

		typeComponentInstance = 'cmpi';
		typeTextRange = 'txrn';
		typeTextRangeArray = 'tray';
		typeOffsetArray = 'ofay';
		typeIntlWritingCode = 'intl';
		typeQDPoint = 'QDpt';
		typeAEText = 'tTXT';
		typeText = 'TEXT';

{ descriptor type constants }

		kTSMOutsideOfBody = 1;
		kTSMInsideOfBody = 2;
		kTSMInsideOfActiveInputArea = 3;

		kNextBody = 1;
		kPreviousBody = 2;

{ Apple Event error constants }

		errOffsetInvalid = -1800;
		errOffsetIsOutsideOfView = -1801;
		errTopOfDocument = -1810;
		errTopOfBody = -1811;
		errEndOfDocument = -1812;
		errEndOfBody = -1813;

{ Text Services Manager result codes }

		tsmComponentNoErr = noErr;
		tsmUnsupScriptLanguageErr = -2500;
		tsmInputMethodNotFoundErr = -2501;
		tsmNotAnAppErr = -2502;
		tsmAlreadyRegisteredErr = -2503;
		tsmNeverRegisteredErr = -2504;
		tsmInvalidDocIDErr = -2505;
		tsmTSMDocBusyErr = -2506;
		tsmDocNotActiveErr = -2507;
		tsmNoOpenTSErr = -2508;
		tsmCantOpenComponentErr = -2509;
		tsmTextServiceNotFoundErr = -2510;
		tsmDocumentOpenErr = -2511;
		tsmUseInputWindowErr = -2512;
		tsmTSHasNoMenuErr = -2513;
		tsmTSNotOpenErr = -2514;
		tsmComponentAlreadyOpenErr = -2515;
		tsmInputMethodIsOldErr = -2516;
		tsmScriptHasNoIMErr = -2517;
		tsmUnsupportedTypeErr = -2518;
		tsmUnknownErr = -2519;

{ data types }

	type

		TextRange = record
				fStart: LongInt;
				fEnd: LongInt;
				fHiliteStyle: Integer;
			end;
		TextRangePtr = ^TextRange;
		TextRangeHandle = ^TextRangePtr;

		TextRangeArray = record
				fNumOfRanges: Integer;
				fRange: array[0..0] of TextRange;
			end;
		TextRangeArrayPtr = ^TextRangeArray;
		TextRangeArrayHandle = ^TextRangeArrayPtr;

		OffsetArray = record
				fNumOfOffsets: Integer;
				fOffset: array[0..0] of LongInt;
			end;
		OffsetArrayPtr = ^OffsetArray;
		OffsetArrayHandle = ^OffsetArrayPtr;

		TextServiceInfo = record
				fComponent: Component;
				fItemName: Str255;
			end;
		TextServiceInfoPtr = ^TextServiceInfo;

		TextServiceList = record
				fTextServiceCount: Integer;
				fServices: array[0..0] of TextServiceInfo;
			end;
		TextServiceListPtr = ^TextServiceList;
		TextServiceListHandle = ^TextServiceListPtr;

		ScriptLanguageRecord = record
				fScript: ScriptCode;
				fLanguage: LangCode;
			end;

		ScriptLanguageSupport = record
				fScriptLanguageCount: Integer;
				fScriptLanguageArray: array[0..0] of ScriptLanguageRecord;
			end;
		ScriptLanguageSupportPtr = ^ScriptLanguageSupport;
		ScriptLanguageSupportHandle = ^ScriptLanguageSupportPtr;

		InterfaceTypeList = array[0..0] of OSType;

		TSMDocumentID = Ptr;

{ Text Services Manager Routines for Client Applications }

	function InitTSMAwareApplication: OSErr;
	inline
		$7014, $AA54;
	function CloseTSMAwareApplication: OSErr;
	inline
		$7015, $AA54;
	function NewTSMDocument (numOfInterface: Integer;
									var supportedInterfaceTypes: InterfaceTypeList;
									var idocID: TSMDocumentID;
									refCon: LongInt): OSErr;
	inline
		$7000, $AA54;
	function DeleteTSMDocument (idocID: TSMDocumentID): OSErr;
	inline
		$7001, $AA54;
	function ActivateTSMDocument (idocID: TSMDocumentID): OSErr;
	inline
		$7002, $AA54;
	function DeactivateTSMDocument (idocID: TSMDocumentID): OSErr;
	inline
		$7003, $AA54;
	function TSMEvent (var event: EventRecord): Boolean;
	inline
		$7004, $AA54;
	function TSMMenuSelect (menuResult: LongInt): Boolean;
	inline
		$7005, $AA54;
	function SetTSMCursor (mousePos: Point): Boolean;
	inline
		$7006, $AA54;
	function FixTSMDocument (idocID: TSMDocumentID): OSErr;
	inline
		$7007, $AA54;
	function GetServiceList (numOfInterfaceTypes: Integer;
									var supportedInterfaceTypes: InterfaceTypeList;
									var serviceInfo: TextServiceListHandle;
									var seedValue: LongInt): OSErr;
	inline
		$7008, $AA54;
	function OpenTextService (idocID: TSMDocumentID;
									aComponent: Component;
									var aComponentInstance: ComponentInstance): OSErr;
	inline
		$7009, $AA54;
	function CloseTextService (idocID: TSMDocumentID;
									aComponentInstance: ComponentInstance): OSErr;
	inline
		$700A, $AA54;
	function UseInputWindow (idocID: TSMDocumentID;
									useWindow: Boolean): OSErr;
	inline
		$7010, $AA54;

{ Utilities -- called by the Script Manager }

	function SetDefaultInputMethod (ts: Component;
									var slRecord: ScriptLanguageRecord): OSErr;
	inline
		$700C, $AA54;
	function GetDefaultInputMethod (var ts: Component;
									var slRecord: ScriptLanguageRecord): OSErr;
	inline
		$700D, $AA54;
	function SetTextServiceLanguage (var slRecord: ScriptLanguageRecord): OSErr;
	inline
		$700E, $AA54;
	function GetTextServiceLanguage (var slRecord: ScriptLanguageRecord): OSErr;
	inline
		$700F, $AA54;

{ Text Services Manager Routines for Components }

	function SendAEFromTSMComponent (var theAppleEvent: AppleEvent;
									var reply: AppleEvent;
									sendMode: AESendMode;
									sendPriority: AESendPriority;
									timeOutInTicks: LongInt;
									idleProc: IdleProcPtr;
									filterProc: EventFilterProcPtr): OSErr;
	inline
		$700B, $AA54;
	function NewServiceWindow (wStorage: Ptr;
									boundsRect: Rect;
									title: Str255;
									visible: Boolean;
									theProc: Integer;
									behind: WindowPtr;
									goAwayFlag: Boolean;
									ts: ComponentInstance;
									var window: WindowPtr): OSErr;
	inline
		$7011, $AA54;
	function CloseServiceWindow (window: WindowPtr): OSErr;
	inline
		$7012, $AA54;
	function GetFrontServiceWindow (var window: WindowPtr): OSErr;
	inline
		$7013, $AA54;
	function FindServiceWindow (thePoint: Point;
									var theWindow: WindowPtr): Integer;
	inline
		$7017, $AA54;

implementation
end.