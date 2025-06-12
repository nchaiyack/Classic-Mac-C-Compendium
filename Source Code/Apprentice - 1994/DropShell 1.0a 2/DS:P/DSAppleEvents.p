{******************************************************************************
**
**  Project Name:	DropShell
**     File Name:	DSAppleEvents.p
**
**   Description:	Generic AppleEvent handling routines
**					
**					This is the set of routines for handling the required Apple events.
**					You should NEVER have to modify this file!!!
**					Simply add code in DSUserProcs to the routines called by these.
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
**	11/24/91			LDR		Added a handler for 'pdoc' as per DTS recommendation
**								This caused some reorg & userProc routine changes
**								I also created a new common AEVT doc extractor
**								Pass the new userDataHandle to odoc/pdoc routines
**								FailErr now uses the ErrorAlert routine to report probs
**	10/30/91			LDR		Modified USES clause for new include & ifc'ed ThP
**	10/28/91			LDR		Officially renamed DropShell (from QuickShell)
**								Added a bunch of comments for clarification
**	04/09/91	00:04	LDR		Original Version
**
******************************************************************************}

UNIT DSAppleEvents;
INTERFACE

{$IFC THINK_Pascal}
	USES
		AppleEvents, DSGlobals, DSUtils, DSUserProcs;	{just the DropShell files}
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
		DSGlobals, DSUtils, DSUserProcs;
{$ENDC THINK_Pascal}
	
{---------------------}
{Interface Definitions}
{---------------------}

	PROCEDURE InitAEVTStuff;	
	PROCEDURE DoHighLevelEvent( event: EventRecord ) ;	
	
	FUNCTION HandleOAPP( theAppleEvent, reply: AppleEvent; handlerRefcon: LONGINT ): OSErr ; 
	FUNCTION HandleODOC( theAppleEvent, reply: AppleEvent; handlerRefcon: LONGINT ): OSErr ; 
	FUNCTION HandlePDOC( theAppleEvent, reply: AppleEvent; handlerRefcon: LONGINT ): OSErr ; 
	FUNCTION HandleQuit( theAppleEvent, reply: AppleEvent; handlerRefcon: LONGINT ): OSErr ; 

IMPLEMENTATION

	{$S Initialize}
	{ 
		This routine does all initialization for AEM, including the
		creation and then population of the dispatch table.
	}
	PROCEDURE InitAEVTStuff;
	VAR
		aevtErr: OSErr;

	BEGIN
		aevtErr := noErr;

		IF (aevtErr = noErr) THEN
			aevtErr := AEInstallEventHandler(kCoreEventClass, kAEOpenApplication, @HandleOAPP, 0, false);
		IF (aevtErr = noErr) THEN
			aevtErr := AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, @HandleODOC, 0, false);
		IF (aevtErr = noErr) THEN
			aevtErr := AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments, @HandlePDOC, 0, false);
		IF (aevtErr = noErr) THEN
			aevtErr := AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, @HandleQUIT, 0, false);

		IF (aevtErr = noErr) THEN
			InstallOtherEvents;

		IF (aevtErr <> noErr) THEN
			; {Report an error to the user, if you are so inclinded!}
	END;
	
	
	{$S Main}
	{ 
		This routine is a utility routine for checking that all required 
		parameters in the Apple event have been used.
	}
	
	FUNCTION GotRequiredParams(theAppleEvent: AppleEvent): OSErr;
	VAR
		typeCode: DescType;
		actualSize: Size;
		err: OSErr;

	BEGIN
		err := AEGetAttributePtr(theAppleEvent, keyMissedKeywordAttr, typeWildCard, typeCode, NIL, 0, actualSize); 
		IF err = errAEDescNotFound THEN { we got all the required params: all is ok }
			GotRequiredParams := noErr
		ELSE IF err = noErr THEN
			GotRequiredParams := errAEEventNotHandled
		ELSE
			GotRequiredParams := err;
	END; { GotRequiredParams }
	
	{
		This is another routine useful for showing debugging info.
		
		It calls the ErrorAlert routine from DSUtils to put up the 
		error message.
	}
	PROCEDURE FailErr(err: OSErr);
	BEGIN
		IF err <> noErr THEN
			ErrorAlert(kErrStringID, kAEVTErr, err);
	END;
	
	

	{
		This routine is the handler for the oapp (Open Application) event.
		
		It first checks the number of parameters to make sure we got them all 
		(even though we don't want any) and then calls the OpenApp userProc in QSUserProcs.
		Finally it checks to see if the caller wanted a reply & sends one, setting any error.
	}

	FUNCTION HandleOAPP(theAppleEvent, reply: AppleEvent; handlerRefcon: LONGINT): OSErr;
	VAR
		errStr: Str255;

	BEGIN
		FailErr(GotRequiredParams(theAppleEvent));

		{ let's show the user the splash screen }
		ShowWindow(gSplashScreen);

		OpenApp; 	{pass it on to the app specific routine}

		IF reply.dataHandle <> NIL THEN { a reply is sought }
		BEGIN
			errStr := 'Opening';
			HandleOAPP := AEPutParamPtr(reply, 'errs', 'TEXT', Ptr(@errStr[1]), length(errStr));
		END
		ELSE
			HandleOAPP := noErr;
	END;

	{
		This routine is the handler for the quit (Quit Application) event.
		
		It first checks the number of parameters to make sure we got them all 
		(even though we don't want any) and then calls the QuitApp userProc in QSUserProcs.
		Finally it checks to see if the caller wanted a reply & sends one, setting any error.
	}

	FUNCTION HandleQUIT(theAppleEvent, reply: AppleEvent; handlerRefcon: LONGINT): OSErr;
	VAR
		errStr: Str255;

	BEGIN
		FailErr(GotRequiredParams(theAppleEvent));

		QuitApp;	{pass it on to the app specific routine}

		IF reply.dataHandle <> NIL THEN { a reply is sought }
		BEGIN
			errStr := 'Quiting';
			HandleQUIT := AEPutParamPtr(reply, 'errs', 'TEXT', Ptr(@errStr[1]), length(errStr));
		END
		ELSE
			HandleQUIT := noErr;
	END;

	{
		This routine is the low level processing routine for both the 
		odoc (Open Document) and pdoc (Print Document) events.
		
		This routine is the key one, since this is how we get the list of
		files/folders/disks to process.  The first thing to do is the get the
		list of files, and then make sure that's all the parameters (should be!).
		We then send call the PreflightDocs routine (from DSUserProcs), process
		each file in the list by calling OpenDoc (again in DSUserProcs), and finally
		call PostflightDocs (you know where) and setting a return value.
	}

	FUNCTION _HandleDocs(theAppleEvent, reply: AppleEvent; opening: Boolean): OSErr;
	VAR
		myFSS: FSSpec;
		docList: AEDescList;
		index, itemsInList: LONGINT;
		actualSize: Size;
		keywd: AEKeyword;
		typeCode: descType;
		userDataHandle: Handle;

	BEGIN
		FailErr(AEGetParamDesc(theAppleEvent, keyDirectObject, typeAEList, docList));
		FailErr(GotRequiredParams(theAppleEvent));

		IF PreFlightDocs(opening, userDataHandle) THEN {let the app do any preflighting it might need}
		BEGIN
			FailErr(AECountItems(docList, itemsInList));	{count the number of items}
			FOR index := 1 TO itemsInList DO
			BEGIN
				FailErr(AEGetNthPtr(docList, index, typeFSS, keywd, typeCode, @myFSS, sizeof(myFSS), actualSize));
				OpenDoc(@myFSS, opening, userDataHandle);	{call the open routine}
			END;
	
			PostFlightDocs(opening, userDataHandle); {and any cleanup}
	
			_HandleDocs := noErr;
		END
		ELSE
			_HandleDocs := errAEEventNotHandled;	{this tells AEM we didn't handle it}

		FailErr(AEDisposeDesc(docList));
	END; { _HandleDocs }


	{
		This routine is the handler for the odoc (Open Document) event.
		
		The odoc event simply calls the common _HandleDocs routines, which will
		do the dirty work of parsing the AEVT & calling the userProcs.
	}

	FUNCTION HandleODOC(theAppleEvent, reply: AppleEvent; handlerRefcon: LONGINT): OSErr; 
	BEGIN
		HandleODOC := _HandleDocs(theAppleEvent, reply, TRUE);	{call the low level routine}
	END; { HandleODOC }

	{
		This routine is the handler for the pdoc (Print Document) event.
		
		The pdoc event like the odoc simply calls the common _HandleDocs routines
	}

	FUNCTION HandlePDOC(theAppleEvent, reply: AppleEvent; handlerRefcon: LONGINT): OSErr; 
	BEGIN
		HandlePDOC := _HandleDocs(theAppleEvent, reply, FALSE);	{call the low level routine}
	END; { HandlePDOC }

	{
		This is the routine called by the main event loop, when a high level
		event is found.  Since we only deal with Apple events, and not other
		high level events, we just pass everything onto the AEM via AEProcessAppleEvent
	}
	PROCEDURE DoHighLevelEvent(event: EventRecord);
	BEGIN
		FailErr(AEProcessAppleEvent(event));
	END;

END.