{ Types for After Dark graphics module. }
{ For more information, consult the programmer's section of the manual. }
{ ©1989 Berkeley Systems, Inc. }

unit GraphicsModuleTypes;

interface
	uses
		Sound;

	const

{ These messages are passed to main() by After Dark. }
{ Initialize:		Call DoInitialize().}
{ Close:			Call DoClose().}
{ Blank:			Call DoBlank().}
{ DrawFrame:		Call DoDrawFrame().}
{ ButtonMessage:	Call DoSetup().}

		Initialize = 0;
		Close = 1;
		Blank = 2;
		DrawFrame = 3;
		ModuleSelected = 4;
		DoAbout = 5;
		ButtonMessage = 8;

{ Return Values }

{ ModuleError:		This value is returned to After Dark to indicate a module error }

		ModuleError = -1;

{ These two values can be returned by DoInitialize(), DoClose(), }
{ Call DoBlank(), and DoDrawFrame(). }

{ RestartMe:		Tells After Dark to send an "Initialize" message to main(). }
{ ImDone:			Tells After Dark not to call your function main() and to handle drawing. }

		RestartMe = 1;
		ImDone = 2;

{ This value can be returned by DoSetup(). }
{ RefreshResources:		Tells After Dark to redisplay all controls with the values in the}
{ graphic module's resource fork. }

		RefreshResources = 3;

{ Data structures for After Dark graphics modules. }

	type
		{Description of each monitor on system.}
		MonitorData = packed record
				bounds: Rect;			{ Bounding rectangle in global coords. }
				synchFlag: Boolean; 	{ Flag set by monitor VBL task. }
				curDepth: Byte;		{ Current pixel depth of monitor. }
			end;

		{List Record.}
		MonitorsInfo = record
				monitorCount: integer;	{ Number of monitors in use. }
				monitorList: array[0..0] of MonitorData;	{ List of monitors. }
			end;
		MonitorsInfoPtr = ^MonitorsInfo;

		{Copy of Quickdraw globals.}
		QDGlobals = record
				qdThePort: GrafPtr;	{ Pointer to current grafPort. }
				qdWhite: Pattern;	{ All-white pattern. }
				qdBlack: Pattern;	{ All-black pattern. }
				qdGray: Pattern;	{ 50% gray pattern. }
				qdLtGray: Pattern;	{ 25% gray pattern. }
				qdDkGray: Pattern;	{ 75% gray pattern. }
				qdArrow: Cursor;	{ Standard arrow cursor. }
				qdScreenBits: BitMap;	{ The entire screen. }
				qdRandSeed: longint;	{ Where Random sequence begins. }
			end;
		QDGlobalsPtr = ^QDGlobals;

		{Param Block passed each time to the graphics module.}
		GMParamBlock = record
				controlValues: array[0..3] of integer;	{ The values of the user set controls. }
				monitors: MonitorsInfoPtr;	{ Info about connected monitors. }
				colorQDAvail: Boolean;		{ Whether the Mac has Color QuickDraw. }
				systemConfig: integer;		{ Configuration info about the Mac. }
				qdGlobalsCopy: QDGlobalsPtr;{ A copy of the QuickDraw Globals. }
				brightness: integer;		{ Sets the brightness to this level. }
				demoRect: Rect;				{ The Control Panel rectangle. }
				errorMessage: StringPtr;	{ string to be displayed if error encountered. }
				sndChannel: SndChannelPtr;  { Set if you have a sound channel }
				adVersion: integer;			{ After Dark version in BCD}
			end;
		GMParamBlockPtr = ^GMParamBlock;

implementation

end.