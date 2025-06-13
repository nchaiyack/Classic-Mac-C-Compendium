unit IconFamilies;

{  Source:	Macintosh Technical Note #306 by Jim Mensch and David Collins }
{ 	Definitions to deal with icon families under System 7 and later }

interface

{$IFC undefined THINK_Pascal}
	uses
		Memory, QuickDraw, Types;
{$ENDC}

	const
		Large1BitMask = 'ICN#';
		Large4BitData = 'icl4';
		Large8BitData = 'icl8';
		Small1BitMask = 'ics#';
		Small4BitData = 'ics4';
		Small8BitData = 'ics8';
		Mini1BitMask = 'sicn';
		Mini4BitData = 'icm4';
		Mini8BitData = 'icm8';

	const
		ttNone = $0000;
		ttDisabled = $0001;
		ttOffline = $0002;
		ttOpen = $0003;
		ttSelected = $4000;
		ttSelectedDisabled = (ttSelected + ttDisabled);
		ttSelectedOffline = (ttSelected + ttOffline);
		ttSelectedOpen = (ttSelected + ttOpen);

	const
		ttLabel0 = $0000;
		ttLabel1 = $0100;
		ttLabel2 = $0200;
		ttLabel3 = $0300;
		ttLabel4 = $0400;
		ttLabel5 = $0500;
		ttLabel6 = $0600;
		ttLabel7 = $0700;

	const
		atNone = $0000;
		atVerticalCenter = $0001;
		atTop = $0002;
		atBottom = $0003;
		atHorizontalCenter = $0004;
		atLeft = $0008;
		atRight = $000C;

	const
		svLarge1Bit = $00000001;
		svLarge4Bit = $00000002;
		svLarge8Bit = $00000004;
		svSmall1Bit = $00000100;
		svSmall4Bit = $00000200;
		svSmall8Bit = $00000400;
		svMini1Bit = $00010000;
		svMini4Bit = $00020000;
		svMini8Bit = $00040000;
		svAllLargeData = $000000ff;
		svAllSmallData = $0000ff00;
		svAllMiniData = $00ff0000;
		svAll1BitData = (svLarge1Bit + svSmall1Bit + svMini1Bit);
		svAll4BitData = (svLarge4Bit + svSmall4Bit + svMini4Bit);
		svAll8BitData = (svLarge8Bit + svSmall8Bit + svMini8Bit);
		svAllAvailableData = $ffffffff;


	function PlotIconID (var theRect: rect; align, transform, theResID: integer): OSErr;
	inline
		$303C, $0500, $ABC9;

	function NewIconSuite (var theIconSuite: handle): OSErr;
	inline
		$303C, $0207, $ABC9;

	function AddIconToSuite (theIconData, theSuite: handle; theType: ResType): OSErr;
	inline
		$303C, $0608, $ABC9;

	function GetIconFromSuite (var theIconData: handle; theSuite: handle; theType: ResType): OSErr;
	inline
		$303C, $0609, $ABC9;

	function ForEachIconDo (theSuite: handle; selector: longInt; action: ProcPtr; yourDataPtr: ptr): OSErr;
	inline
		$303C, $080A, $ABC9;

	function GetIconSuite (var theIconSuite: handle; theResID: integer; selector: longInt): OSErr;
	inline
		$303C, $0501, $ABC9;

	function DisposeIconSuite (theIconSuite: handle; disposeData: boolean): OSErr;
	inline
		$303C, $0302, $ABC9;

	function PlotIconSuite (var theRect: rect; align, transform: integer; theIconSuite: Handle): OSErr;
	inline
		$303C, $0603, $ABC9;

	function MakeIconCache (var theHandle: Handle; makeIcon: ProcPtr; yourDataPtr: ptr): OSErr;
	inline
		$303C, $0604, $ABC9;

	function LoadIconCache (var theRect: rect; align, transform: integer; theIconCache: Handle): OSErr;
	inline
		$303C, $0606, $ABC9;

	function GetLabel (labelNumber: integer; var labelColor: RGBColor; var labelString: Str255): OSErr;
	inline
		$303c, $050B, $ABC9;

	function PtInIconID (testPt: Point; var iconRect: Rect; alignment, iconID: integer): Boolean;
	inline
		$303c, $060D, $ABC9;

	function PtInIconSuite (testPt: Point; var iconRect: Rect; alignment: integer; theIconSuite: Handle): Boolean;
	inline
		$303c, $070E, $ABC9;

	function RectInIconID (var testRect, iconRect: rect; alignment, iconID: integer): Boolean;
	inline
		$303c, $0610, $ABC9;

	function RectInIconSuite (var testRect, iconRect: rect; alignment: integer; theIconSuite: Handle): Boolean;
	inline
		$303c, $0711, $ABC9;

	function IconIDToRgn (theRgn: RgnHandle; var iconRect: Rect; alignment, iconID: integer): OSErr;
	inline
		$303c, $0613, $ABC9;

	function IconSuiteToRgn (theRgn: RgnHandle; var iconRect: Rect; alignment: integer; theIconSuite: Handle): OSErr;
	inline
		$303c, $0714, $ABC9;

	function SetSuiteLabel (theSuite: Handle; theLabel: integer): OSErr;
	inline
		$303C, $0316, $ABC9;

	function GetSuiteLabel (theSuite: Handle): integer;
	inline
		$303C, $0217, $ABC9;

	function GetIconCacheData (theCache: Handle; var theData: ptr): OSErr;
	inline
		$303C, $0419, $ABC9;

	function SetIconCacheData (theCache: Handle; theData: ptr): OSErr;
	inline
		$303C, $041A, $ABC9;

	function GetIconCacheProc (theCache: Handle; var theProc: ProcPtr): OSErr;
	inline
		$303C, $041B, $ABC9;

	function SetIconCacheProc (theCache: Handle; theProc: ProcPtr): OSErr;
	inline
		$303C, $041C, $ABC9;

	function PlotSICNHandle (var theRect: rect; align, transform: integer; theSICN: Handle): OSErr;
	inline
		$303C, $061E, $ABC9;

	function PlotCIconHandle (var theRect: rect; align, transform: integer; theCIcon: CIconHandle): OSErr;
	inline
		$303C, $061F, $ABC9;

	function SetLabel (labelNumber: integer; var labelColor: RGBColor; var labelString: Str255): OSErr;
	inline
		$303C, $050C, $ABC9;

implementation

end.
