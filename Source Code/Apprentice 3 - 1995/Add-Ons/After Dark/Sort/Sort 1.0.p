{Sort - an After Dark module demonstrating an easy way to write and debug a module}
{in the Think Pascal environment, several sorting algorithms, and while we�re at it,}
{direct to screen drawing and object Pascal in a code resource (including working around}
{a bug in the Think Pascal 4.0.2 libraries).}

{Written and � by Scott Lindhurst, lindhurs@math.wisc.edu, fall 1993 and Nov. 1994.}
{Mail:	1107 Erin St.		Madison, WI 53715-1845}
{or	123 Millwood Dr.		Tonawanda, NY 14150-5513}
{If you use this source in your program, give me credit in the about box and documentation.}



{$SETC ADModule = true}
{To make an After Dark module, set ADModule to True, use RSRCRuntime.Lib in the project,}
{and change the project type to code resource.}
{To develop under the Think Pascal environment, set ADmodule to False, use Runtime.Lib,}
{and change the project type to Application.}



{Known problems and bugs:}
{� Has not ever been tested with multiple monitors. Anyone want to buy me a machine with}
{2 or 3 monitors for testing?}
{� There is a bug in the Quicksort implementation. I don�t know where.}
{� Not all the sort routines have been fully tested lately.}


{$IFC ADModule}
unit SortScreen;
interface
{$ELSEC}
	program SortScreen;
{$ENDC}

	uses
		SortDetails, PixelUtils, GraphicsModuleTypes;


{$IFC ADModule}
	function main (var storage: Handle;
									blankRgn: rgnHandle;
									message: integer;
									params: GMParamBlockPtr): OSErr;
{$ENDC}


	const
		kSortTypeMenu = 0;
		iQuicksort = 1;
		iHeapsort = 2;
		iShellsort = 3;
		iBubblesort = 4;
		iShakersort = 5;
		iBatchersort = 6;
		kBlankFirstControl = 1;
		kDirectPixelsControl = 2;

	type
		StoredStuff = record
				theSortObj: cSorter;
			end;
		StoredStuffP = ^StoredStuff;
		StoredStuffH = ^StoredStuffP;

{$IFC not ADModule}

	var
		gStorage: Handle;				{These are normally filled in by After Dark and passed}
		gBlankRgn: RgnHandle;			{to your module.}
		gParams: GMParamBlock;
		gMonitors: MonitorsInfo;
		gQDGlobals: QDGlobals;

		gModuleErrorMessage: str255;
		gDrawWindow: WindowPtr;
{$ENDC}

{$IFC ADModule}
implementation
{$ENDC}

	function DoInitialize (var storage: Handle;
									blankRgn: rgnHandle;
									params: GMParamBlockPtr): OSErr;
		var
			i: integer;
			device: GDHandle;
			drawRect: Rect;
			quickObj: cQuicksort;
			heapObj: cHeapsort;
			shellObj: cShellsort;
			bubbleObj: cBubbleSort;
			shakerObj: cShakerSort;
			batcherObj: cBatcherSort;
			sorterObj: cSorter;
	begin
		DoInitialize := noErr;
		storage := NewHandle(sizeof(StoredStuff));
		if storage = nil then
			begin
				DoInitialize := ModuleError;
				Exit(DoInitialize);
			end;

{Find global coords of our drawing space, and shrink it down to a square because that's all I can deal with.}
		i := -1;
		repeat		{Pick a rectangle that's actually on some monitor and draw to that one.}
			i := i + 1
		until SectRect(blankRgn^^.rgnBBox, params^.monitors^.monitorList[i].bounds, DrawRect);
{DrawRect should now be in global coordinates.}

		with params^ do
			InitPixelUtils(DrawRect, monitors^.monitorList[i].curDepth, colorQDAvail, controlValues[kDirectPixelsControl]);

		case params^.controlValues[kSortTypeMenu] of
			iQuicksort: 
				begin
					new(quickObj);
					SorterObj := quickObj;
				end;
			iHeapSort: 
				begin
					new(heapObj);
					SorterObj := heapObj;
				end;
			iShellSort: 
				begin
					new(shellObj);
					SorterObj := shellObj;
				end;
			iBubbleSort: 
				begin
					new(bubbleObj);
					SorterObj := bubbleObj;
				end;
			iShakerSort: 
				begin
					new(shakerObj);
					SorterObj := shakerObj;
				end;
			iBatcherSort: 
				begin
					new(batcherObj);
					SorterObj := batcherObj;
				end;
		end;	{case}

		SorterObj.Init(ScreenPixels);
		StoredStuffH(storage)^^.theSortObj := SorterObj;
	end;	{Function DoInitialize}


	function DoBlank (storage: Handle;
									blankRgn: rgnHandle;
									params: GMParamBlockPtr): OSErr;
	begin
{First, erase outside the screen I�m going to draw on.}
		EraseOutsideDrawArea(blankRgn, params);
{Second, randomize the screen I�m going to draw on, if requested.}
		if params^.controlValues[kBlankFirstControl] = 1 then
			begin
				LockForDrawing;
				RandomFillScreen;
				UnlockForDrawing;
			end;
		DoBlank := noErr;
	end;	{function DoBlank}


	function DoDrawFrame (storage: Handle;
									blankRgn: rgnHandle;
									params: GMParamBlockPtr): OSErr;

	begin
		LockForDrawing;
		StoredStuffH(storage)^^.theSortObj.DoALittle;
		if StoredStuffH(storage)^^.theSortObj.Done then
			DoDrawFrame := ImDone
		else
			DoDrawFrame := noErr;
		UnlockForDrawing;
	end;


	function DoClose (storage: Handle;
									blankRgn: RgnHandle;
									params: GMParamBlockPtr): OSErr;
	begin
		StoredStuffH(storage)^^.theSortObj.Free;
		DisposHandle(storage);
		DoClose := noErr;
	end;


	function DoSetup (blankRgn: rgnHandle;
									message: integer;
									params: GMParamBlockPtr): OSErr;
	begin

{This is called when the used clicks on a button in the Control Panel.}

		DoSetup := noErr;

	end;


{$S %_MethTables}
{$Push}
{$N-}
	procedure LoadMethTables;
	begin

	end;
{$Pop}
{$S}


	function main (var storage: Handle;
									blankRgn: rgnHandle;
									message: integer;
									params: GMParamBlockPtr): OSErr;
{Taken from the After Dark programming info files.}
		var
			err: OSErr;
	begin {main}


{$IFC ADModule}
		SetUpA4;				{Works around a bug in the ThP 4.0.2 libraries. Put RememberA4 before}
		RememberA4;		{SetUpA4 if compiling with fixed libraries.}
		LoadMethTables;
{$ENDC}

		err := noErr;
		case message of
			Initialize: 
				err := DoInitialize(storage, blankRgn, params);
			Close: 
				err := DoClose(storage, blankRgn, params);
			Blank: 
				err := DoBlank(storage, blankRgn, params);
			DrawFrame: 
				err := DoDrawFrame(storage, blankRgn, params);
			otherwise
				if (message >= ButtonMessage) then
					err := DoSetup(blankRgn, message, params);
		end;
		main := err;

{$IFC ADModule}
		RestoreA4;
{$ENDC}
	end; {main}



	procedure SelectionSort;
{Selection sort the whole screen.}
		var
			i, j, min: longint;
	begin
		for i := 1 to ScreenPixels - 1 do
			begin
				min := i;
				for j := i + 1 to ScreenPixels do
					if MyGetPixel(j).sortValue < MyGetPixel(min).sortValue then
						min := j;
				SwapPixels(i, min);
			end;
	end;	{procedure SelectionSort}



	procedure RadixExchange (left, right: longint;
									bit: integer);
		var
			i, j, mask: longint;
	begin
		mask := BSL(1, bit);
		if (right > left) and (bit >= 0) then
			begin
				i := left;
				j := right;
				repeat
					while (BAND(mask, MyGetPixel(i).sortValue) = 0) & (i < j) do
						i := i + 1;
					while (BAND(mask, MyGetPixel(j).sortValue) <> 0) & (i < j) do
						j := j - 1;
					SwapPixels(i, j);
				until j = i;
				if BAND(mask, MyGetPixel(right).sortValue) = 0 then
					j := j + 1;
				RadixExchange(left, j - 1, bit - 1);
				RadixExchange(j, right, bit - 1);
			end;
	end;	{procedure RadixExchange}



	procedure SortOneBitScreen;
{Sort the screen, assuming that it is in black and white mode.}
{The screen won't get sorted right if the monitor is not monochrome.}
		var
			i, j: longint;
			pix0, pix1: PixelRec;
	begin
		i := 0;
		j := ScreenPixels + 1;
		pix0.sortValue := 0;
		pix1.sortValue := 1;
		repeat
			repeat
				i := i + 1
			until (MyGetPixel(i).sortValue = 1) | (i >= j);
			repeat
				j := j - 1
			until (MyGetPixel(j).sortValue = 0) | (j <= i);
			MySetPixel(i, pix0);
			MySetPixel(j, pix1);
		until i >= j;
		SwapPixels(i, j);	{undo the extra swap with i=j}
	end;	{procedure SortOneBitScreen}


{$IFC not ADModule}

	procedure RunScreenSaver;
{Call this is run your screen saver as if in the After Dark environment.}
{It will print out some timing information as the program runs.}
{To stop, click or press a key or otherwise create an event.}

		var
			startFrameTicks, measuredTicks, startCallTicks, endCallTicks, ticksThisCall: longint;	{Timing variables}
			numTimesCalled, maxCallTicks: longint;
			err: OSErr;
			theEvent: EventRecord;


	begin
		writeln('Timing data in ticks (1/60 second)');
		ObscureCursor;

		measuredTicks := 0;
		numTimesCalled := 0;
		maxCallTicks := 0;

		startCallTicks := TickCount;

		SetPort(GrafPtr(gDrawWindow));
		err := main(gStorage, gBlankRgn, Initialize, @gParams);
		endCallTicks := TickCount;
		writeln('Initialize time: ', endCallTicks - startCallTicks : 1);

		startCallTicks := endCallTicks;

		if err = noErr then
			begin
				SetPort(GrafPtr(gDrawWindow));
				err := main(gStorage, gBlankRgn, Blank, @gParams);
				endCallTicks := TickCount;
				ticksThisCall := endCallTicks - startCallTicks;
				writeln('Blank time: ', ticksThisCall : 1);

				startFrameTicks := TickCount;

				while (err = noErr) and not OSEventAvail(everyEvent, theEvent) do
					begin
						startCallTicks := TickCount;

						SetPort(GrafPtr(gDrawWindow));
						err := main(gStorage, gBlankRgn, DrawFrame, @gParams);

						endCallTicks := TickCount;
						ticksThisCall := endCallTicks - startCallTicks;
						measuredTicks := measuredTicks + ticksThisCall;
						numTimesCalled := numTimesCalled + 1;
						if (numTimesCalled mod 100) = 0 then
							writeln('DrawFrame #', numTimesCalled : 1, ' time was ', ticksThisCall : 1, ' ticks.');
						if ticksThisCall > maxCallTicks then
							begin
								maxCallTicks := ticksThisCall;
								writeln('DrawFrame #', numTimesCalled : 1, ' took ', ticksThisCall : 1, ' ticks, a new maximum.');
							end;
					end;	{While}

				if (err = noErr) then	{Kill the module because an event happened}
					begin
						startCallTicks := endCallTicks;
						SetPort(GrafPtr(gDrawWindow));
						err := main(gStorage, gBlankRgn, Close, @gParams);
						endCallTicks := TickCount;
						ticksThisCall := endCallTicks - startCallTicks;
						writeln('Close time: ', ticksThisCall : 1);
					end;
			end;	{Main running of the module}

{Write out timing info.}
		writeln;
		write('Module ended ');
		case err of
			noErr: 
				writeln('normally.');
			ModuleError: 
				writeln('with a module error (probably out of memory).');
			RestartMe: 
				writeln('asking to be restarted.');
			ImDone: 
				writeln('because it was done.');
			otherwise
				writeln('with error number ', err : 1);
		end;	{case}

		writeln('Timing summary.');
		ticksThisCall := TickCount - startFrameTicks;	{Total time used}
		writeln('Measured DrawFrame time: ', measuredTicks : 1, ' ticks, or ', measuredTicks div 60 : 1, ' seconds.');
		writeln('Total DrawFrame time: ', ticksThisCall : 1, ' ticks, or ', ticksThisCall div 60 : 1, ' seconds.');
		writeln('DrawFrame was called ', numTimesCalled : 1, ' times.');
		if numTimesCalled > 0 then
			writeln('Average time per frame: ', ticksThisCall / numTimesCalled : 1 : 1, ' ticks.');
		writeln('Maximum time for a single frame: ', maxCallTicks : 1, ' ticks.');

	end;	{procedureRunScreenSaver}



	procedure GeneralImpersonationSetup;
{Fill in the globals containing the After Dark-supplied info the module needs.}
{Not everything is supplied because this isn't really After Dark.}
{In particular, only one monitor, no sound, and systemConfig isn't set.}
		var
			GestaltResult: longint;
	begin
		gStorage := nil;
		with gParams do
			begin
				monitors := @gMonitors;
				if (Gestalt(gestaltQuickDrawVersion, GestaltResult) = noErr) & (GestaltResult >= gestalt8BitQD) then
					colorQDAvail := true
				else
					colorQDAvail := false;
				gMonitors.monitorCount := 0;
				with gMonitors.monitorList[0] do
					begin
						if colorQDAvail then		{Color machine}
							begin
								gDrawWindow := WindowPtr(GetNewCWindow(128, nil, WindowPtr(-1)));
								curDepth := GetMainDevice^^.gdPMap^^.pixelSize
							end
						else		{Monochrome machine}
							begin
								gDrawWindow := GetNewWindow(128, nil, WindowPtr(-1));
								curDepth := 1;
							end;
						bounds := gDrawWindow^.portRect;		{In local coords; (0,0) is the top left of the drawing area}
						synchFlag := true;
					end;	{Setting up monitors info}

				systemConfig := 0;		{I'm not supporting this yet}
				qdGlobalsCopy := @gQdGlobals;
				SetRect(demoRect, 0, 0, 0, 0);
				errorMessage := @gModuleErrorMessage;
				sndChannel := nil;
				adVersion := $0200;
			end;	{Initializing gParams}

		gBlankRgn := NewRgn;
		RectRgn(gBlankRgn, gMonitors.monitorList[0].bounds);

		with gQdGlobals do
			begin
				qdThePort := thePort;
				qdWhite := white;
				qdBlack := black;
				qdGray := gray;
				qdLtGray := ltGray;
				qdDkGray := dkGray;
				qdArrow := arrow;
				qdScreenBits := screenBits;
				qdRandSeed := RandSeed;
			end;
	end;		{Procedure GeneralImpersonationSetup}


	procedure SpecificImpersonationSetup;
{Settings for your module go here. The only thing I can think of to put here is the control values.}
{Change the settings to simulate changing the settings in the control panel.}
	begin
		gParams.controlValues[kSortTypeMenu] := iHeapsort;
		gParams.controlValues[kBlankFirstControl] := 1;		{1 to blank first, 0 to use screen as is}
		gParams.controlValues[kDirectPixelsControl] := 1;	{0 for QD, 1 for direct to screen}
	end;		{procedure SpecificImpersonationSetup}


begin	{Impersonation of After Dark}
	ShowText;

	GeneralImpersonationSetup;
	SpecificImpersonationSetup;
	ObscureCursor;

	SetPort(GrafPtr(gDrawWindow));
	gQdGlobals.qdThePort := thePort;		{Just to be sure it's set up correctly.}


	RunScreenSaver;


{$ENDC}
end.