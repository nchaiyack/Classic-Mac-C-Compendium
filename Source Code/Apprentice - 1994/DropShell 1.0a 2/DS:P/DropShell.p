{******************************************************************************
**
**  Project Name:	DropShell
**     File Name:	DropShell.p
**
**   Description:	Main application code for the DropShell
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
**	01/25/92			LDR		Added $I- for Think Pascal double-init problems
**								Added Otherwise clauses to all CASE statements 
**									ThP Runtime needs them, for some reason?!?
**	12/09/91			LDR		Added support for new "Select File…" menu item
**								Quit now sends AEVT to self to be politically correct
**								Added support for the new gSplashScreen
**	11/24/91			LDR		Added support for the Apple Menu (duh!)
**								Moved CenterAlert & ErrorAlert to DSUtils
**	10/30/91			LDR		Modified USES clause for new include & ifc'ed ThP
**	10/28/91			LDR		Officially renamed DropShell (from QuickShell)
**								Added a bunch of comments for clarification
**	04/09/91	00:02	LDR		Original Version
**
******************************************************************************}

PROGRAM DropShell;

{$IFC THINK_Pascal}
	USES
		Script, DSGlobals, DSUtils, DSUserProcs, DSAppleEvents;	{just the DropShell files}
{$ELSEC}
	USES
	{ First load standard interface files}
		MemTypes, QuickDraw, 

	{ Now include the stuff from OSIntf }
		OSIntf, 

	{ Now Include the stuff from ToolIntf.p }
		ToolIntf, Packages, GestaltEqu, 

	{ Then any OTHER Toolbox interfaces... }
		Files, Aliases, AppleEvents,

	{ And finally any files from DropShell }
		DSGlobals, DSUtils, DSUserProcs, DSAppleEvents;
{$ENDC THINK_Pascal}

{CODE BEGINS HERE}

{$IFC THINK_Pascal}
{$ELSEC}
	{
		This routine is automatically linked in by the MPW Linker. 
		This external reference to it is done so that we can unload its segment, %A5Init.
	}
	PROCEDURE _DataInit; EXTERNAL;
{$ENDC THINK_Pascal}

	{Simple routine installed by InitDialogs for the Resume button in Bomb boxes}
	PROCEDURE Panic;
	BEGIN
		ExitToShell;
	END;

	{$S Initialize}
	{Nothing special here, just initting the toolbox}
	PROCEDURE InitToolbox;
	BEGIN
{$IFC THINK_Pascal}
{$I-}
{$ELSEC}
		UnloadSeg(@_DataInit);	{we love you MPW!}
{$ENDC THINK_Pascal}

		InitGraf(@thePort);
		InitFonts;
		InitWindows;
		InitMenus;
		TEInit;
		InitDialogs(@Panic);

		InitCursor;
		FlushEvents(everyEvent, 0);
		
		{how about some memory fun!}
		MoreMasters;
		MoreMasters;	{two should be enough!}
	END;
	
	{
		Let's setup those global variables that the DropShell uses.
		
		If you add any globals for your own use,
		init them in the InitUserGlobals routine in DSUserProcs.c
	}
	FUNCTION InitGlobals: Boolean;
	VAR
		aLong: longint;
	BEGIN
		gDone := FALSE;
		gOApped := FALSE;	{Assume it wasn't, since most users wont!}
    	gHasAppleEvents := (Gestalt(gestaltAppleEventsAttr, aLong) = noErr);
		gSplashScreen := NIL;
		
		InitGlobals := InitUserGlobals;	{call the user proc}
	END;
	
	{
		Again, nothing fancy.  Just setting up the menus.
		
		If you add any menus to your DropBox - insert them here!
	}
	PROCEDURE SetUpMenus;
	BEGIN
		gAppleMenu := GetMenu(kAppleNum);
		AddResMenu(gAppleMenu, 'DRVR');
		InsertMenu(gAppleMenu, 0);

		gFileMenu := GetMenu(kFileNum);
		InsertMenu(gFileMenu, 0);
		DrawMenuBar;
	END;
	
	{
		This routine is called during startup to display a splash screen.
		
		This was recommend by the Blue Team HI person, John Sullivan, who
		feels that all apps should display something so that users can easily
		tell what is running, and be able to switch by clicking.  Thanks John!
	}
	PROCEDURE InstallSplashScreen;
	CONST
		windowPicID = 128;
	VAR
		picH: PicHandle;
	BEGIN
		IF (gSplashScreen = NIL) THEN { show the splash screen window }
		BEGIN
			picH := GetPicture(windowPicID);
			IF picH <> NIL THEN
			BEGIN
				gSplashScreen := GetNewWindow(windowPicID, NIL, Pointer(-1));
				IF gSplashScreen <> NIL THEN
				BEGIN
					SetWindowPic(gSplashScreen, picH);
					{ Don't show it here, let it happen during an 'oapp' }
					{ ShowWindow(gSplashScreen); }
				END;
			END;
		END;
	END;

{$S Main}

{** Standard Event Handling routines **}
	PROCEDURE ShowAbout;
	VAR
		itemHit: integer;
	BEGIN
		itemHit := Alert(128, NIL);
	END;
	
	PROCEDURE DoMenu(retVal: longint);
	VAR
		menuID, itemID: integer;
		itemStr: Str255;
		refNum: integer;
	BEGIN
		menuID := HiWord(retVal);
		itemID := LoWord(retVal);
		
		CASE menuID OF
			kAppleNum:
				IF itemID = 1 THEN	{handle the about box}
					ShowAbout	
				ELSE				{get the DA's}
				BEGIN
					GetItem(GetMHandle(kAppleNum), itemID, itemStr);
					refNum := OpenDeskAcc(itemStr);
				END;
			kFileNum:
				IF itemID = 1 THEN	{Select File…}
					SelectFile
				ELSE
					SendQuitToSelf;	{send me a quit event}
			OTHERWISE
				{do nothing}
		END;
		HiliteMenu(0);		{turn it off}
	END;
	
	PROCEDURE DoMouseDown(curEvent:EventRecord);
	VAR
		whichWindow:	WindowPtr;
		whichPart:		integer;
	BEGIN
		whichPart := FindWindow (curEvent.where, whichWindow);
		case whichPart of
			inMenuBar:		DoMenu (MenuSelect (curEvent.where));
			inSysWindow:	SystemClick (curEvent, whichWindow);
			inDrag:			DragWindow(whichWindow, curEvent.where, screenBits.bounds);
			inDesk, inContent, inGrow, inGoAway, inZoomIn, inZoomOut:
					{Do nothing};
			OTHERWISE
					{Do nothing here either}
		end; {case}
	END; {DoMouseDown}
	
	PROCEDURE DoKeyDown(curEvent:EventRecord);
	VAR
		ch:				char;
	BEGIN
		ch := CHR(BitAnd (curEvent.message, charCodeMask));
		
		IF BitAnd (curEvent.modifiers, cmdKey) <> 0 THEN
			DoMenu (MenuKey (ch));
	END; {DoKeyDown}

{Main Program Starts Here}
BEGIN
	InitToolbox;
	IF (InitGlobals) THEN
	BEGIN
		IF gHasAppleEvents THEN
		BEGIN
			InitAEVTStuff;
			SetupMenus;
			InstallSplashScreen;
			
			WHILE NOT gDone DO
			BEGIN
				gWasEvent := WaitNextEvent(everyEvent, gEvent, 0, NIL);
				IF gWasEvent THEN
				BEGIN
					CASE gEvent.what OF
						kHighLevelEvent:	DoHighLevelEvent(gEvent);
						mouseDown:			DoMouseDown(gEvent);
						keyDown, autoKey:	DoKeyDown(gEvent);
						OTHERWISE
								{Do nothing}
					END; {case}
				END;
			END;	
		END
		ELSE
			ErrorAlert(kErrStringID, kCantRunErr, 0);
	END;
END.
