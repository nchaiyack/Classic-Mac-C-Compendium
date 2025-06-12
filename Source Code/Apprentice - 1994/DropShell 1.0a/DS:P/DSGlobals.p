{******************************************************************************
**
**  Project Name:	DropShell
**     File Name:	DSGlobals.p
**
**   Description:	Globals used by DropShell
**
*******************************************************************************
**                       A U T H O R   I D E N T I T Y
*******************************************************************************
**
**	Initials	Name
**	--------	-----------------------------------------------
**	LDR			Leonard Rosenthol
**
*******************************************************************************
**                      R E V I S I O N   H I S T O R Y
*******************************************************************************
**
**	  Date		Time	Author	Description
**	--------	-----	------	---------------------------------------------
**	11/24/91			LDR		Added some new consts for error handling
**	10/30/91			LDR		Modified USES clause for new include & ifc'ed ThP
**	10/28/91			LDR		Officially renamed DropShell (from QuickShell)
**								Added a bunch of comments for clarification
**	04/09/91	00:03	LDR		Original Version
**
******************************************************************************}

UNIT DSGlobals;

INTERFACE

{$IFC THINK_Pascal}
{$ELSEC}
	USES
	{ First load standard interface files}
		MemTypes, QuickDraw, 

	{ Now include the stuff from OSIntf }
		OSIntf, 

	{ Now Include the stuff from ToolIntf.p }
		ToolIntf, Packages, GestaltEqu, 

	{ Then any OTHER Toolbox interfaces... }
		Files, Aliases, AppleEvents;
{$ENDC THINK_Pascal}

	CONST
		kAppleNum = 128;
		kFileNum = 129;
		
		kErrStringID = 100;
		kCantRunErr = 1;
		kAEVTErr = 2;
		
	VAR
		gDone, gOApped, gHasAppleEvents, gWasEvent: Boolean;
		gEvent: EventRecord;
		gAppleMenu, gFileMenu: MenuHandle;
		gSplashScreen: WindowPtr;

IMPLEMENTATION

END.
