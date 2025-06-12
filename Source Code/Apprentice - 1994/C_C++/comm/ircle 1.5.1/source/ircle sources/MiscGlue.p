unit MiscGlue;

interface

{$IFC OLD_PASCAL}
{ Notification Manager glue - not needed on new compilers }
type
	NMRec = record
			qLink: QElemPtr;   {next queue entry}
			qType: INTEGER;    {queue type -- ORD(nmType) = 8}
			nmFlags: INTEGER;    {reserved}
			nmPrivate: LONGINT;    {reserved}
			nmReserved: INTEGER;    {reserved}
			nmMark: INTEGER;    {item to mark in Apple menu}
			nmSIcon: Handle;     {handle to small icon}
			nmSound: Handle;     {handle to sound record}
			nmStr: StringPtr;  {string to appear in alert}
			nmResp: ProcPtr;    {pointer to response routine}
			nmRefCon: LONGINT;    {for application use}
		end;
	NMPtr = ^NMRec;

function NMInstall (nmReqPtr: QElemPtr): OSErr;
inline
$205F, $A05E, $3E80;

function NMRemove (nmReqPtr: QElemPtr): OSErr;
inline
$205F, $A05F, $3E80;

function SetCurrentA5: longint;
inline
$2E8D, $2A78, $0904;

function SetA5 (newA5: longint): longint;
inline
$2F4D, $0004, $2A5F;


{ (incomplete) Styled TextEdit glue - only needed on obsolete compilers }

const
doFont = 1;
doFace = 2;
doSize = 4;
doColor = 8;
doAll = 15;
addSize = 16;

type
RGBColor = record
		red, green, blue: integer
	end;

ScrpSTElement = record
		scrpStartChar: longint;
		scrpHeight: integer;
		scrpAscent: integer;
		scrpFont: integer;
		scrpFace: Style;
		scrpSize: integer;
		scrpColor: RGBColor
	end;
ScrpSTTable = array[0..0] of scrpSTElement;

StScrpHandle = ^StScrpPtr;
StScrpPtr = ^StScrpRec;
StScrpRec = record
		scrpNStyles: integer;
		scrpStyleTab: ScrpSTTable
	end;

TextStyle = record
		tsFont: integer;
		tsFace: Style;
		tsSize: integer;
		tsColor: RGBColor
	end;

function TEStylNew (destRect, viewRect: Rect): TEHandle;
inline
$A83E;  {Ê_TEStylNew }

function GetStylScrap (hTE: TEHandle): StScrpHandle;
inline
$3F3C, $0006,		{Êmove.w #6, -(sp) }
$A83D; 			{ _TEDispatch }

procedure TEStylInsert (text: Ptr; length: longint; hST: stScrpHandle; hTE: TEHandle);
inline
$3F3C, $0007, 	{ move.w #7,-(sp) }
$A83D; 			{ _TEDispatch }

procedure TESetStyle (mode: integer; newStyle: TextStyle; redraw: boolean; hTE: TEHandle);
inline
$3F3C, $0001, 	{ move.w #1,-(sp) }
$A83D; 			{ _TEDispatch }

{$ENDC}

implementation

end.