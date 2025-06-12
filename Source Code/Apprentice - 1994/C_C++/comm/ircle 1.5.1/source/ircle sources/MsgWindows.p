{	ircle - Internet Relay Chat client	}
{	File: MsgWindows 	}
{	Copyright © 1992 Olaf Titz (s_titz@ira.uka.de)	}

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

unit MsgWindows;
{ÊDeals with windows for displaying messages }

interface
uses
	MiscGlue, ApplBase;

const
	MW_MAGIC = 'MWIN';

	C_BOLD = 2;			{ Bold: ^B }
	C_INVERSE = 22;			{ Inverse: ^V }
	C_UNDERLINE = 31;		{ Underline: ^_   [ouch!] }
	C_NORMAL = 15;		{ Normal: ^N }

type
	MWHndl = ^MWPtr;	{ This gets stored in the window's refCon }
	MWPtr = ^MWRec;
	MWRec = record
			magic: OSType;			{ for checking }
			w: WindowPtr;			{ the Window }
			whenDone: ProcPtr;		{ called on closing }
			hscr, vscr: ControlHandle;	{ scroll bars }
			t: TEHandle;				{ TextEdit record }
			vislines,					{ # of visible lines }
			NumStyles: integer;		{ used for insertion }
			sty: stScrpHandle;			{ ditto }
		end;

var
	MWActive: MWHndl;	{ MWHndl of active window; or else nil }
	MWdefaultFont, MWdefaultSize: integer;

procedure InitMsgWindows;
{ startup }

function NewMWindow (var title: string; prefSize: Rect; DoWhenDone: ProcPtr): MWHndl;
{ Make a new window. DoWhen done gets called when user wants to }
{ close window; to be declared as: procedure DoWhenDone(w:WindowPtr) }

procedure SetFontSize (window: MWHndl; font, size: integer);
{ Change window font/size }

procedure DeleteMWindow (window: MWHndl);
{ÊDelete window }

procedure MWMessage (window: MWHndl; var msg: string);
{ Display message at bottom of window }

function MWFreeMem: boolean;
{ Tries to free up some memory }

implementation

{$SETC ZOOM=true}

const
	MW_MAXLEN = 20000; 	{ maximum # of chars to store }
	MW_KILLEN = 5000;		{ # of chars to kill if window exceeds MW_MAXLEN }

	STY_0 = sizeof(stScrpRec);
	STY_1 = sizeof(ScrpSTElement);

{$IFC ZOOM}
{ a quick hack to deal with the zoom states }
type
	WStates = array[inZoomIn..inZoomOut] of Rect;
	WStatesPtr = ^WStates;
	WStatesHndl = ^WStatesPtr;
{$ENDC}

var
	glob: MWHndl;
	cornerstone, screensize: rect;
	flood: integer;

function EvtMW (var e: EventRecord): MWHndl;
	var
		w: MWHndl;
	begin
		w := MWHndl(GetWRefCon(WindowPtr(e.message)));
		if w <> nil then
			if w^^.magic <> MW_MAGIC then
				w := nil;
		EvtMW := w
	end;


procedure SetWDimen (win: MWHndl; left, top, width, height, zoomstate: integer);
	var
		r: Rect;
{$IFC ZOOM}
		wp: WindowPeek;
		zs: WStatesHndl;
{$ENDC}
	begin
{$IFC ZOOM}
		wp := WindowPeek(win^^.w);
		zs := WStatesHndl(wp^.dataHandle);
{$ENDC}
		if left + width < 10 then
			left := 10 - width;
		win^^.vislines := (height - 20) div win^^.sty^^.scrpStyleTab[0].scrpHeight;
		height := win^^.vislines * win^^.sty^^.scrpStyleTab[0].scrpHeight + 20;
		SetRect(r, 3, 2, width - 17, height - 17);
		win^^.t^^.destRect := r;
		r.top := r.top + 1;
		win^^.t^^.viewRect := r;
		SetRect(r, left, top, left + width, top + height);
{$IFC ZOOM}
		zs^^[zoomstate] := r;
{$ENDC}
		MoveWindow(win^^.w, left, top, true);
		SizeWindow(win^^.w, width, height, false);
		EraseRect(win^^.w^.portRect);
		MoveControl(win^^.hscr, -1, height - 15);
		SizeControl(win^^.hscr, width - 13, 16);
		MoveControl(win^^.vscr, width - 15, -1);
		SizeControl(win^^.vscr, 16, height - 13);
		TECalText(win^^.t);
		TEPinScroll(0, 32767, win^^.t);
		if win^^.t^^.nlines <= win^^.vislines then
			SetCtlMax(win^^.vscr, 0)
		else
			SetCtlMax(win^^.vscr, win^^.t^^.nlines - win^^.vislines);
		SetCtlValue(win^^.vscr, 0);
		InvalRect(win^^.w^.portRect);
		ShowWindow(win^^.w);
		SelectWindow(win^^.w);
	end;

procedure vscroll (o, n: integer);
	begin
		SetCtlValue(glob^^.vscr, o - n);
		TEPinScroll(0, n * glob^^.sty^^.scrpStyleTab[0].scrpHeight, glob^^.t);
	end;
procedure Vscrolling (cc: ControlHandle; part: integer);
	begin
		case part of
			inPageUp: 
				vscroll(GetCtlValue(glob^^.vscr), glob^^.vislines - 1);
			inPageDown: 
				vscroll(GetCtlValue(glob^^.vscr), 1 - glob^^.vislines);
			inUpButton: 
				vscroll(GetCtlValue(glob^^.vscr), 1);
			inDownButton: 
				vscroll(GetCtlValue(glob^^.vscr), -1);
			otherwise
		end;
	end;

function inContentHandler (var e: EventRecord): boolean;
	var
		p: MWHndl;
		c: ControlHandle;
		pa, i: integer;
	begin
		p := EvtMW(e);
		if p <> nil then begin
			glob := p;
			GlobalToLocal(e.where);
			pa := FindControl(e.where, p^^.w, c);
			case pa of
				inUpButton, inDownButton, inPageUp, inPageDown: 
					if c = p^^.vscr then
						pa := TrackControl(c, e.where, @Vscrolling);
				inThumb: 
					if c = p^^.vscr then begin
						i := GetCtlValue(c);
						pa := TrackControl(c, e.where, nil);
						if pa = inThumb then
							vscroll(i, i - GetCtlValue(c));
					end;
				otherwise
					if PtInRect(e.where, p^^.t^^.viewRect) then
						TEClick(e.where, bitand(e.modifiers, ShiftKey) <> 0, p^^.t);
			end;
			inContentHandler := true;
		end
		else
			inContentHandler := false;
	end;

function inGrowHandler (var e: EventRecord): boolean;
	var
		p: MWHndl;
		r: Rect;
		ii: longint;
	begin
		p := EvtMW(e);
		if p <> nil then begin
			SetRect(r, 32, 32, 32767, 32767);
			ii := GrowWindow(p^^.w, e.where, r);
			inGrowHandler := true;
			if ii <> 0 then
				with p^^.w^.portBits.bounds do
					SetWDimen(p, -left, -top, LoWord(ii), HiWord(ii), inZoomIn);
		end
		else
			inGrowHandler := false;
	end;

procedure XCALL (w: WindowPtr; p: ProcPtr);
inline
	$205F, $4E90;		{ movea.l (a7)+,a0; jsr (a0) }

function inGoAwayHandler (var e: EventRecord): boolean;
	var
		p: MWHndl;
	begin
		p := EvtMW(e);
		if p <> nil then begin
			if TrackGoAway(p^^.w, e.where) then
				if p^^.whenDone <> nil then
					XCALL(p^^.w, p^^.whenDone);
			inGoAwayHandler := true;
		end
		else
			inGoAwayHandler := false;
	end;

{$IFC ZOOM}
function inZoomInOutHandler (var e: EventRecord): boolean;
	var
		p: MWHndl;
		l, t, w, h: integer;
	begin
		p := EvtMW(e);
		if p <> nil then begin
			if TrackBox(p^^.w, e.where, e.what - mouseMsg) then begin
				HideWindow(p^^.w);
				ZoomWindow(p^^.w, e.what - mouseMsg, false);
				with p^^.w^.portBits.bounds do begin
					l := -left;
					t := -top
				end;
				with p^^.w^.portRect do begin
					w := right;
					h := bottom
				end;
				SetWDimen(p, l, t, w, h, e.what - mouseMsg);
				inZoomInOutHandler := true;
			end
		end
		else
			inZoomInOutHandler := false;
	end;
{$ENDC}

function updateHandler (var e: EventRecord): boolean;
	var
		p: MWHndl;
	begin
		flood := 0;
		p := EvtMW(e);
		if p <> nil then begin
			BeginUpdate(p^^.w);
			DrawControls(p^^.w);
			TEUpdate(p^^.w^.portRect, p^^.t);
			DrawGrowIcon(p^^.w);
			EndUpdate(p^^.w);
			updateHandler := true;
		end
		else
			updateHandler := false;
	end;

function activateHandler (var e: EventRecord): boolean;
	var
		p: MWHndl;
		r: Rect;
		i: integer;
	begin
		activateHandler := false;
		p := EvtMW(e);
		if p <> nil then begin
			if odd(e.modifiers) then begin
				HiliteControl(p^^.vscr, 0);
				TEActivate(p^^.t);
				mwActive := p;
			end
			else begin
				HiliteControl(p^^.vscr, 255);
				TEDeactivate(p^^.t);
				mwActive := nil;
			end;
			with p^^.w^.portRect do
				SetRect(r, right - 14, bottom - 14, right, bottom);
			EraseRect(r);
{    InvalRect(p^^.w^.portRect);}
			InvalRect(r);
		end
		else
			mwActive := nil;
	end;

function editHandler (var e: EventRecord): boolean;
	var
		i: integer;
	begin
		if mwActive <> nil then
			if e.message = 4 then begin
				TECopy(mwActive^^.t);
				editHandler := true
			end
			else if e.message = 7 then begin
				TESetSelect(0, 32767, mwActive^^.t);
				SetCtlValue(mwActive^^.vscr, 0);
				editHandler := true
			end
			else
				editHandler := false
		else
			editHandler := false
	end;

procedure ForceUpdate (w: MWHndl);
	var
		ee: EventRecord;
		b: boolean;
		p0: GrafPtr;
	begin
		GetPort(p0);
		SetPort(w^^.w);
		InvalRect(w^^.w^.portRect);
		ee.message := longint(w^^.w);
		b := updateHandler(ee);
		SetPort(p0)
	end;


procedure InitMsgWindows;
	var
		i: integer;
	begin
		SetRect(cornerstone, 2, 40, 500, 40);
		with screenBits.bounds do
			SetRect(screensize, -left + 2, -top + 30, right - left - 4, bottom - top - 35);
		i := ApplTask(@inContentHandler, mouseMsg + inContent);
		i := ApplTask(@inGrowHandler, mouseMsg + inGrow);
		i := ApplTask(@inGoAwayHandler, mouseMsg + inGoAway);
{$IFC ZOOM}
		i := ApplTask(@inZoomInOutHandler, mouseMsg + inZoomIn);
		i := ApplTask(@inZoomInOutHandler, mouseMsg + inZoomOut);
{$ENDC}
		i := ApplTask(@updateHandler, updateEvt);
		i := ApplTask(@activateHandler, activateEvt);
		i := ApplTask(@editHandler, menuMsg + editMenu);
		mwActive := nil;
		flood := 0;
		MWdefaultFont := monaco;
		MWdefaultSize := 9;
	end;

procedure SetFontSize (window: MWHndl; font, size: integer);
	var
		f: FontInfo;
		s: TextStyle;
		s0, s1: integer;
	begin
		TextFont(font);
		TextSize(size);
		GetFontInfo(f);
		with window^^.sty^^.scrpStyleTab[0] do begin
			scrpStartChar := 0;
			scrpHeight := f.descent + f.leading + f.ascent;
			scrpAscent := f.ascent;
			scrpFont := font;
			scrpFace := [];
			scrpSize := size;
			with scrpColor do begin
				red := 0;
				green := 0;
				blue := 0
			end
		end;
		if window^^.vislines = 0 then
			with window^^.w^.portBits.bounds do
				window^^.vislines := (3 * (bottom - top - 45) div 4) div window^^.sty^^.scrpStyleTab[0].scrpHeight;
		with window^^.w^.portBits.bounds do
			SetWDimen(window, -left, -top, 80 * CharWidth('x') + 22, window^^.vislines * window^^.sty^^.scrpStyleTab[0].scrpHeight + 16, inZoomIn);
		with s do begin
			tsFont := font;
			tsFace := [];
			tsSize := size;
			with tsColor do begin
				red := 0;
				green := 0;
				blue := 0
			end
		end;
		with window^^.t^^ do begin
			s0 := selStart;
			s1 := selEnd
		end;
		TEDeactivate(window^^.t);
		TESetSelect(0, 32767, window^^.t);
		TESetStyle(doFont + doSize, s, true, window^^.t);
		TESetSelect(s0, s1, window^^.t);
	end;

function NewMWindow (var Title: string; prefSize: Rect; DoWhenDone: ProcPtr): MWHndl;
	var
		h: MWHndl;
		p0: GrafPtr;
		r: Rect;
		f: FontInfo;
{$IFC ZOOM}
		wp: WindowPeek;
		zs: WStatesHndl;
{$ENDC}
	begin
		if EmptyRect(prefSize) then begin
			r := cornerstone;
			OffsetRect(cornerstone, 8, 16);
		end
		else begin
			r := prefSize;
			if not SectRect(screensize, r, r) then begin
				r := cornerstone;
				OffsetRect(cornerstone, 8, 16);
			end
		end;
		h := MWHndl(NewHandle(sizeof(MWRec)));
		if h <> nil then begin
{$IFC ZOOM}
			h^^.w := NewWindow(nil, r, title, false, 8, WindowPtr(-1), true, longint(h));
{$ELSEC}
			h^^.w := NewWindow(nil, r, title, false, 0, WindowPtr(-1), true, longint(h));
{$ENDC}
			if h^^.w <> nil then begin
				GetPort(p0);
				SetPort(h^^.w);
				TextFont(MWDefaultFont);
				TextSize(MWDefaultSize);
				if EmptyRect(r) then
					h^^.vislines := 0
				else begin
					GetFontInfo(f);
					with f do
						h^^.vislines := (r.bottom - r.top - 16) div (descent + leading + ascent)
				end;
				h^^.magic := MW_MAGIC;
				h^^.hscr := NewControl(h^^.w, r, '', true, 0, 0, 0, 16, 0);
				h^^.vscr := NewControl(h^^.w, r, '', true, 0, 0, 0, 16, 0);
				h^^.whenDone := DoWhenDone;
				h^^.t := TEStylNew(r, r);
				h^^.sty := stScrpHandle(NewHandle(STY_0));
				h^^.NumStyles := 0;
{$IFC ZOOM}
				wp := WindowPeek(h^^.w);
				zs := WStatesHndl(wp^.dataHandle);
				with zs^^[inZoomOut] do begin
					right := right - 64;
					bottom := bottom - 32;
				end;
{$ENDC}
				TEAutoView(false, h^^.t);
				TEActivate(h^^.t);
				SetFontSize(h, MWdefaultFont, MWdefaultSize);
				SetPort(p0)
			end
		end;
		NewMWindow := h
	end;


procedure DeleteMWindow (window: MWHndl);
	begin
		HideWindow(window^^.w);
		TEDispose(window^^.t);
		DisposeControl(window^^.hscr);
		DisposeControl(window^^.vscr);
		DisposeWindow(window^^.w);
		DisposHandle(Handle(window^^.sty));
		DisposHandle(Handle(window));
	end;


procedure FreeMW (window: MWHndl);
	begin
		TESetSelect(0, MW_KILLEN, window^^.t);
		TEDelete(window^^.t);
{    i := Munger(Handle(window^^.t^^.hText), 0, nil, MW_KILLEN, ptr(1), 0);}
{    TECalText(window^^.t);}
{    TESelView(window^^.t);}
{    ForceUpdate(window);}
	end;

procedure MWMessage (window: MWHndl; var msg: string);
	var
		p0: GrafPtr;
		s0, s1, i, c, nsty: integer;
		s: Style;
		e: EventRecord;
		ndelay: boolean;

	procedure AddHunk (y: Style);
		begin
			if y <> s then
				with window^^ do begin
					nsty := nsty + 1;
					if nsty > NumStyles then begin
						SetHandleSize(Handle(sty), STY_0 + nsty * STY_1);
						NumStyles := nsty
					end;
					with sty^^ do begin
						scrpStyleTab[nsty] := scrpStyleTab[nsty - 1];
						scrpStyleTab[nsty].scrpStartChar := i;
						scrpStyleTab[nsty].scrpFace := y;
					end;
					s := y
				end;
		end;

	procedure ToggleHunk (x: StyleItem);
		begin
			if x in s then
				AddHunk(s - [x])
			else
				AddHunk(s + [x])
		end;

	begin
		if window <> nil then begin
			ndelay := EventAvail(everyEvent, e);
			ndelay := (BitAnd(e.modifiers, shiftKey) = 0);
			GetPort(p0);
			SetPort(window^^.w);
			with window^^.t^^ do begin
				s0 := selStart;
				s1 := selEnd
			end;
			TEDeactivate(window^^.t);
			if window^^.t^^.teLength > MW_MAXLEN then begin
				FreeMW(window);
				ndelay := true;
				s0 := s0 - MW_KILLEN;
				if s0 < 0 then
					s0 := 0;
				s1 := s1 - MW_KILLEN;
				if s1 < 0 then
					s1 := 0;
			end;
			if pos(chr(7), msg) > 0 then
				SysBeep(10);
			s := [];
			nsty := 0;
			i := 1;
			while i <= length(msg) do begin
				c := ord(msg[i]);
				if c < 32 then begin
					case c of
						C_BOLD: 
							ToggleHunk(bold);
						C_UNDERLINE: 
							ToggleHunk(underline);
						C_INVERSE: 
							ToggleHunk(outline);
						C_NORMAL: 
							AddHunk([]);
						otherwise
					end;
					delete(msg, i, 1);
				end;
				i := i + 1;
			end;
			insert(chr(13), msg, 1);
			TESetSelect(32767, 32767, window^^.t);
			window^^.sty^^.scrpNStyles := nsty + 1;
			TEStylInsert(@msg[1], length(msg), window^^.sty, window^^.t);
			if ndelay then begin
				TEAutoView(true, window^^.t);
				TESelView(window^^.t);
				TEAutoView(false, window^^.t);
			end;
			TESetSelect(s0, s1, window^^.t);
			if window^^.w = FrontWindow then
				TEActivate(window^^.t);
			i := window^^.t^^.nlines - window^^.vislines + 1;
			if i < 0 then
				i := 0;
			SetCtlMax(window^^.vscr, i);
			if ndelay then
				SetCtlValue(window^^.vscr, i);
{    InvalRect(window^^.t^^.viewRect);}
			SetPort(p0);
		end;
	end;

{ by now it's a dummy :-) }
function MWFreeMem: boolean;
	begin
		MWFreeMem := false;
	end;

end.