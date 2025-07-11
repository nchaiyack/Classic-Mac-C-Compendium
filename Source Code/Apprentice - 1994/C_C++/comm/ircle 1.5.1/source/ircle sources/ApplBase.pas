{	ApplBase - Event dispatcher	}
{	File:	ApplBase, version 2.0.2	}
{	Copyright � 1991-1992 Olaf Titz (s_titz@ira.uka.de)	}

{	This program is free software; you can redistribute it and/or modify	}
{	it under the terms of the GNU General Public License as published by	}
{	the Free Software Foundation; either version 2 of the License, or	}
{	(at your option) any later version.	}

{	This program is distributed in the hope that it will be useful,	}
{	but WITHOUT ANY WARRANTY; without even the implied warranty of	}
{	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	}
{	GNU General Public License for more details.	}

{	You should have received a copy of the GNU General Public License	}
{	along with this program; if not, write to the Free Software	}
{	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.	}

unit ApplBase;

interface

uses
	Coroutines, 

{$IFC DISTRIBUTION }
{$SETC TRACE=FALSE }
{$ENDC }

{$IFC UseTCP}

	TCPTypes, TCPStuff, TCPConnections;

{ If the compiler option UseTCP is set: }
{ This version cooperates with Peter Lewis' TCPConnections package. }
{ ConnectionEvents are treated just like regular events, having a priority }
{ dynamically changing from between key and update to lower. }

{$ENDC}

const
	mouseMsg = 16;		{ new 'events': mouseMsg+x, x=FindWindow result }
	dialogMsg = 26;		{ dialog: message=DialogPtr, item in theItem }
{$IFC UseTCP}
	TCPMsg = 27;		{ Ptr to connectionEventRecord in message }
{$ENDC}
	menuMsg = -226;	{ menuMsg+x, x=menu ID; menuBar=30 }

	menuBar = 256;		{ res ID of MBAR }
	appleMenu = 256;	{ res ID of MENU: apple,file,edit }
	FileMenu = 257;		{ apple and edit menu are mandatory }
	EditMenu = 258;

var
	theItem: integer;				{ item hit in dialog boxes }
	RunningInCoroutine: boolean;	{ if coroutine other than global active }
	HighCoPriority: boolean;		{ if Coroutines should run more often }
	WNETime: integer;			{ Timeout parameter for WaitNextEvent }
{$IFC UseTCP}
	maxTCPFlood: integer;		{ max n of consecutive TCP events }
{$ENDC}

{ If you define the compile option TRACE to true, a file 'ApplBase Trace' }
{ will be generated on each run which records events, context switches etc. }
{ Refer to the source below for details. }

{$IFC TRACE}
	trace: text;
{$ENDC}

procedure ApplEvents (var e: EventRecord);
{ Process given event }

procedure ApplInit;
{ Init the package }

procedure ApplRun;
{ Get event and process it }

procedure ApplExitProc (p: ProcPtr);
{ install exitproc }

procedure ApplExit;
{ call exitprocs & exit program }

function ApplTask (mytask: ProcPtr; message: integer): integer;
{ Enqueue task }

procedure ApplUntask (n: integer);
{ Dequeue task }

{	Task declaration:   function mytask(var e:EventRecord):boolean	}
{	should return true if event completely processed	}
{	The following has been done on task entry: Current grafport saved,	}
{	grafport set to whatever appropriate on events for windows (mouse,update,activ.)	}
{	all processing of DAs (SystemEdit...) and dialog events	}

function ApplCoroutine (mytask: ProcPtr; wspsize: integer): integer;
{ Start Coroutine in background, i.e. on null events }

function ApplWaitPB (pb: ParmBlkPtr): integer;
{ Wait for completion of async FileMgr call in background }


implementation

const
	NMSGS = 40; 		{ this means max. 10 menus }

type
	EvtQuPtr = ^EvtQuRec;	{ Event handler queue }
	EvtQuRec = record
			qLink: EvtQuPtr;
			taskID: integer;
			eProc: ProcPtr
		end;

	BkQuPtr = ^BkQuRec;		{ Background procs queue }
	BkQuRec = record
			qLink: BkQuPtr;
			WSP: handle
		end;

	ExQuPtr = ^ExQuRec;		{�Exitproc queue }
	ExQuRec = record
			qLink: ExQuPtr;
			eProc: ProcPtr
		end;

var
	EvtQuHdr: array[0..NMSGS] of EvtQuPtr;
	EvtTID: integer;
	ThisBack, LastBack: BkQuPtr;
	ExitQuHdr: ExQuPtr;
	applMItems: integer;
	WindowDragRect: Rect;
	univRgn: RgnHandle;
{$IFC UseTCP}
	TCPFlood: integer;
{$ENDC}

function ECALL (var e: EventRecord; p: ProcPtr): boolean;
inline
	$205F, $4E90;		{ movea.l (a7)+,a0; jsr (a0) }


procedure ApplEvents (var e: EventRecord);
	var
		i: integer;
		p, p0: WindowPtr;
		l: EvtQuPtr;
	begin
		GetPort(p0);
		if (e.what = keyDown) and (BitAnd(e.modifiers, cmdKey) <> 0) then begin
			e.message := MenuKey(chr(e.message mod 256));  { Menu shortcut.. }
			e.what := menuMsg
		end
		else if (e.what >= 0) and (e.what <= 15) then
			if IsDialogEvent(e) then begin
{$IFC TRACE}
				write(trace, 'D');
{$ENDC}
				if (e.what = keyDown) and ((BitAnd(loword(e.message), 255) = 3) or (BitAnd(loword(e.message), 255) = 13)) then begin
					e.what := dialogMsg;	{ Pressing Return or Enter in dialog... }
					e.message := longint(FrontWindow);
					theItem := 0			{ is reported as Item #0 }
				end
				else if DialogSelect(e, p, theItem) then begin
					e.what := dialogMsg;		{ Let the Dialog Mgr process the event... }
					e.message := longint(p)		{ and report which dialog and item }
				end
				else if e.what <> 0 then
					exit(ApplEvents);			{ DialogSelect has completely processed }
			end;
		if e.what = mouseDown then begin
			i := FindWindow(e.where, p);
			case i of
				inSysWindow: 
					begin
					if p = FrontWindow then
						SystemClick(e, p)
					else
						SelectWindow(p);
					exit(ApplEvents)
				end;
				inMenuBar: 
					begin
					e.message := MenuSelect(e.where);
					e.what := menuMsg
				end;
				otherwise
					if ((i = inContent) or (i = inGrow)) and (p <> FrontWindow) then begin
						SelectWindow(p);	{ if click in non-active window, select it.. }
						exit(ApplEvents)
					end
					else begin
						SetPort(p);	{ the window where the click is in }
						e.what := mouseMsg + i;
						e.message := longint(p)
					end
			end;
		end;
		if e.what = menuMsg then begin	{ Split the menu message }
{$IFC TRACE}
			write(trace, 'M');
{$ENDC}
			if HiWord(e.message) = 0 then
				e.what := 0
			else begin
				e.what := menuMsg + HiWord(e.message);	{ menuMsg+menuNo }
				e.message := LoWord(e.message);			{ itemNo }
				if e.what = menuMsg + EditMenu then
					if SystemEdit(e.message - 1) then		{ handle edit in DA }
						exit(ApplEvents)
			end
		end;
		if (e.what = updateEvt) or (e.what = activateEvt) then
			SetPort(WindowPtr(e.message));
		l := EvtQuHdr[e.what];
		while l <> nil do
			with l^ do begin	{ Call tasks }
				if ECALL(e, eProc) then
					leave;
				l := qLink
			end;
		HiliteMenu(0);
		SetPort(p0);
	end;



{ --- Standard tasks --- }

function Accessories (var e: EventRecord): boolean; { menuMsg+appleMenu }
	var
		s: str255;
		i: integer;
	begin
		if e.message > applMItems then begin							{ DA item selected? }
			GetItem(GetMHandle(appleMenu), LoWord(e.message), s);	{ Open it }
			i := OpenDeskAcc(s);
		end;
		Accessories := true;
	end;


function AccessoryActivate (var e: EventRecord): boolean; {activateEvt }
	var
		p: WindowPeek;
		m: MenuHandle;
		i: integer;
	begin
		p := WindowPeek(e.message);
		if p^.windowKind < 0 then begin	{ DA window selected? }
			if BitAnd(e.modifiers, activeFlag) <> 0 then begin
				m := GetMHandle(EditMenu);	{ Yes -> enable Edit menu and }
				for i := 0 to 6 do				{ standard Edit options }
					EnableItem(m, i);
				DisableItem(m, 2);				{ but not the separator }
			end
			else
				InitCursor;					{ DA window deselected? -> reset the mouse cursor }
		end;
		AccessoryActivate := true;
	end;


function WindowDragging (var e: EventRecord): boolean;
	begin
		DragWindow(WindowPtr(e.message), e.where, WindowDragRect);
		WindowDragging := true;
	end;


function CancelUpdates (var e: EventRecord): boolean;
	begin
		BeginUpdate(WindowPtr(e.message));	{ Purge update events that have not been processed }
		EndUpdate(WindowPtr(e.message));
		CancelUpdates := true
	end;

function BackgroundRun (var e: EventRecord): boolean;
	begin
		if ThisBack <> nil then begin
			RunningInCoroutine := true;
			Transfer(GlobalProc, ThisBack^.WSP);
			RunningInCoroutine := false;
			if GetHandleSize(ThisBack^.WSP) <= 0 then
				if LastBack = ThisBack then begin
					LastBack := nil;
					dispose(ThisBack);
					ThisBack := nil
				end
				else begin
					LastBack^.qLink := ThisBack^.qLink;
					dispose(ThisBack);
					ThisBack := LastBack^.qLink
				end
			else begin
				LastBack := ThisBack;
				ThisBack := ThisBack^.qLink
			end
		end;
{$IFC TRACE}
		write(trace, 'B');
{$ENDC}
		BackgroundRun := true
	end;


function ApplTask (mytask: ProcPtr; message: integer): integer;
	var
		p: EvtQuPtr;
	begin
		EvtTID := EvtTID + 1;
		new(p);
		if p = nil then begin
{$IFC TRACE}
			write(trace, 'Failed ');
{$ENDC}
			ApplTask := -1
		end
		else
			with p^ do begin
				ApplTask := EvtTID;
				qLink := EvtQuHdr[message];
				taskID := EvtTID;
				eProc := mytask;
				EvtQuHdr[message] := p;
			end;
{$IFC TRACE}
		writeln(trace, 'ApplTask: ', mytask, message, EvtTID);
{$ENDC}
	end;



procedure ApplUntask (n: integer);
	var
		p, p0: EvtQuPtr;
		i: integer;
	begin
{$IFC TRACE}
		writeln(trace, 'ApplUntask ', n);
{$ENDC}
		for i := 0 to NMSGS do begin
			p := EvtQuHdr[i];
			if p <> nil then begin
				if p^.taskID = n then begin
					EvtQuHdr[i] := p^.qLink;
					exit(ApplUntask)
				end
				else
					repeat
						p0 := p;
						p := p0^.qLink;
						if p = nil then
							leave;
						if p^.taskID = n then begin
							p0^.qLink := p^.qLink;
							dispose(p);
							exit(ApplUntask)
						end;
					until false;
			end
		end;
	end;



function ApplCoroutine (mytask: ProcPtr; wspsize: integer): integer;
	var
		h: Handle;
		p: BkQuPtr;
	begin
		h := Newprocess(mytask, wspsize);
		ApplCoroutine := MemError;
{$IFC TRACE}
		writeln(trace, 'ApplCoroutine ', mytask, wspsize, MemError);
{$ENDC}
		if h <> nil then begin
			New(p);
			if p = nil then begin
				DisposHandle(h);
				ApplCoroutine := MemError
			end
			else begin
				if LastBack <> nil then
					LastBack^.qLink := p;
				LastBack := p;
				if ThisBack = nil then
					ThisBack := p;
				p^.qLink := ThisBack;
				p^.WSP := h;
			end
		end
	end;




procedure ApplInit;
	var
		m: MenuHandle;
		h: Handle;
		p: GrafPtr;
		i: integer;
	begin
{$IFC TRACE}
		rewrite(trace, 'ApplBase Trace');
{$ENDC}
		for i := 0 to NMSGS do
			EvtQuHdr[i] := nil;
		ThisBack := nil;
		LastBack := nil;
		ExitQuHdr := nil;
		RunningInCoroutine := false;
		HighCoPriority := false;
		WNETime := 10;
		EvtTID := 0;
{$IFC UseTCP}
		TCPFlood := 0;
		maxTCPFlood := 5;
{$ENDC}
		GetWMgrPort(p);
		WindowDragRect := p^.portRect;
		WindowDragRect.top := WindowDragRect.top + 20;
		InsetRect(WindowDragRect, 4, 4);
		univRgn := NewRgn;
		SetRectRgn(univRgn, -32767, -32767, 32767, 32767);
		h := GetNewMBar(menuBar);
		SetMenuBar(h);
		DrawMenuBar;
		m := GetMHandle(appleMenu);
		applMItems := CountMItems(m);
		AddResMenu(m, 'DRVR');
		i := ApplTask(@BackgroundRun, nullEvent);
		i := ApplTask(@CancelUpdates, updateEvt);
		i := ApplTask(@Accessories, menuMsg + appleMenu);
		i := ApplTask(@AccessoryActivate, activateEvt);
		i := ApplTask(@WindowDragging, mouseMsg + inDrag);
		FlushEvents(everyEvent, 0);
	end;


procedure ApplRun;
	var
		e: EventRecord;
		connEvt: ConnectionEventRecord;
		b: Boolean;
	begin
		if RunningInCoroutine then begin
{$IFC TRACE}
			write(trace, 'G');
{$ENDC}
			Transfer(ThisBack^.WSP, GlobalProc)
		end
		else begin
{$IFC TRACE}
			write(trace, 'W');
{$ENDC}
			b := WaitNextEvent(-1, e, WNETime, univRgn);
{$IFC UseTCP}
			if (e.what = nullEvent) or (e.what = updateEvt) then
				if TCPFlood < maxTCPFlood then
					if GetConnectionEvent(any_connection, connEvt) then begin
{$IFC TRACE}
						write(trace, 'C', ord(connEvt.event) : 2);
{$ENDC}
						e.what := TCPMsg;
						e.message := longint(@ConnEvt);
						TCPFlood := TCPFlood + 1
					end
					else
						TCPFlood := 0
				else
					TCPFlood := 0;
{$ENDC UseTCP}
			ApplEvents(e);
			if HighCoPriority then
				b := BackgroundRun(e);
		end;
	end;


procedure ApplExitProc (p: ProcPtr);
	var
		e: ExQuPtr;
	begin
		new(e);
		e^.qLink := ExitQuHdr;
		e^.eProc := p;
		ExitQuHdr := e;
	end;

procedure CALL (p: ProcPtr);
inline
	$205F, $4E90;		{ movea.l (a7)+,a0; jsr (a0) }

procedure ApplExit;
	var
		ep: ExQuPtr;
	begin
		ExitCoroutines;
		ep := ExitQuHdr;
		while ep <> nil do begin
			CALL(ep^.eProc);
			ep := ep^.qLink
		end;
{$IFC TRACE}
		close(trace);
{$ENDC}
		halt;
	end;

end.