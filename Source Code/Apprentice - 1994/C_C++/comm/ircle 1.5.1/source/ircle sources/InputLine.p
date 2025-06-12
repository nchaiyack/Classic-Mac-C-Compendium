{	ircle - Internet Relay Chat client	}
{	File: InputLine	}
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

unit InputLine;
{ Provides a small window with status and input lines. }
{ All keystrokes go into the input line. Implements a command history.}

interface
uses
	MiscGlue, ApplBase, MsgWindows;

procedure InitInputLine;
{ Startup }

procedure OpenInputLine (process: ProcPtr);
{ Open the input line window }
{ process(var s:string) gets called whenever Return was pressed }

procedure SetInputLine (var s: string);
{ Preset the input line }

procedure InsertInputLine (var s: string; select: boolean);
{ Insert a string into the input line }

procedure StatusLine (var s: string);
{ Set the status line }

procedure CloseInputLine;
{ Close the window }

implementation

const
	MAXHIST = 5000;	{ Maximum # of chars to store in command history }
	MAXLINE = 240;		{ Maximum length of input line }

var
	iw: WindowPtr;
	Hact, Hupd, Hmouse, Hkey, Hakey, Hidle, Hpaste: integer;
	status: string[80];
	lineh: TEHandle;
	procs: ProcPtr;
	line1, line2, letterw: integer;
	ReturnHit: boolean;
	hist: CharsHandle;
	hpos: integer;

procedure initInputLine;
	var
		i: integer;
	begin
		iw := nil;
		lineh := nil;
		hist := CharsHandle(NewHandle(1));
		hist^^[0] := chr(0);
		hpos := 0;
	end;

procedure DoRedraw (l: integer);
	var
		p0: GrafPtr;
	begin
		GetPort(p0);
		SetPort(iw);
		if l = 1 then begin
			MoveTo(1, line1);
			DrawString(status);
		end
		else begin
			TEUpdate(lineh^^.viewRect, lineh);
		end;
		SetPort(p0);
	end;

procedure StackupLine (var line: string);
	var
		i: integer;
	begin
		if gethandlesize(Handle(hist)) > MAXHIST then begin
			i := 1;
			while hist^^[i] <> chr(0) do
				i := succ(i);
			i := Munger(Handle(hist), 0, nil, i, ptr(1), 0);
		end;
		i := length(line) + 1;
		if i > 1 then begin
			line[i] := chr(0);
			i := PtrAndHand(@line[1], Handle(hist), i);
			hpos := gethandlesize(Handle(hist)) - 1;
		end
	end;

procedure RecallLine (p: integer);
	var
		i: integer;
		s: string;
	begin
		hpos := p;
		i := 0;
		repeat
			p := succ(p);
			i := succ(i);
			s[i] := hist^^[p];
		until s[i] = chr(0);
		s[0] := chr(i - 1);
		SetInputLine(s);
	end;

procedure RecallLineUp;
	var
		i: integer;
	begin
		i := hpos;
		if i > 0 then
			repeat
				i := pred(i)
			until hist^^[i] = chr(0);
		RecallLine(i);
	end;

procedure RecallLineDown;
	var
		i: integer;
	begin
		i := hpos;
		if i < gethandlesize(handle(hist)) - 1 then begin
			repeat
				i := succ(i)
			until hist^^[i] = chr(0);
			if i < gethandlesize(handle(hist)) - 1 then
				RecallLine(i)
		end
	end;


procedure SetCursor (n: integer);
	begin
		if n < 1 then
			n := 1
		else if n > MAXLINE then
			n := MAXLINE;
		TESetSelect(n, n, lineh);
		DoRedraw(2);
	end;


function Activate (var e: EventRecord): boolean;
	begin
		if iw <> nil then
			if bitand(e.message, 1) = 1 then begin
				ShowWindow(iw);
				TEActivate(lineh)
			end
			else begin
				TEDeActivate(lineh);
				HideWindow(iw);
			end;
		Activate := false
	end;

function Update (var e: EventRecord): boolean;
	begin
		if WindowPtr(e.message) = iw then begin
			BeginUpdate(iw);
			MoveTo(1, line1);
			DrawString(status);
			TEUpdate(lineh^^.viewRect, lineh);
			EndUpdate(iw);
			Update := true
		end
		else
			Update := false
	end;

function Mouse (var e: EventRecord): boolean;
	begin
		if WindowPtr(e.message) = iw then begin
			GlobalToLocal(e.where);
			if e.where.v < 11 then begin
				e.what := mouseMsg + inDrag;
				LocalToGlobal(e.where);
				ApplEvents(e);  { Let ApplBase do the dragging }
			end
			else begin
				TEClick(e.where, false, lineh);
			end;
			Mouse := true
		end
		else
			Mouse := false;
	end;

procedure SCALL (var s: string; p: ProcPtr);
inline
	$205F, $4E90;		{ movea.l (a7)+,a0; jsr (a0) }

procedure GotLine;
	var
		i: integer;
		c: string[1];
		line: string;
	begin
		ReturnHit := true;
		i := lineh^^.teLength;
		if i > 255 then
			i := 255;
		BlockMove(lineh^^.htext^, @line[1], i);
		while (i > 0) and (line[i] = ' ') do
			i := pred(i);
		line[0] := chr(i);
		StackupLine(line);
		c := '';
		SetInputLine(c);
		SCALL(line, procs);
		ReturnHit := false;
	end;


function Key (var e: EventRecord): boolean;
	var
		c: char;
		i: integer;
		p0: GrafPtr;
	begin
		if iw = nil then
			Key := false
		else if not ReturnHit then begin
			getPort(p0);
			SetPort(iw);
			c := chr(e.message mod 256);
			case ord(c) of
				13: 
					GotLine;
				21:  { ^U }
					begin
					TEKey(chr(C_UNDERLINE), lineh);
					TESelView(lineh)
				end;
				30: 
					RecallLineUp;
				31: 
					RecallLineDown;
				otherwise
					begin
					TEKey(c, lineh);
					TESelView(lineh)
				end;
			end;
			SetPort(p0);
		end;
		Key := true;
	end;

function AKey (var e: EventRecord): boolean;
	begin
		AKey := Key(e)
	end;

function Idle (var e: EventRecord): boolean;
	begin
		TEIdle(lineh);
		Idle := false;
	end;

function Paste (var e: EventRecord): boolean;
	var
		h: CharsHandle;
		i, n, c: integer;
		f: EventRecord;
		b: boolean;
	begin
		if e.message = 5 then begin
			i := TEFromScrap;
			h := CharsHandle(TEScrapHandle);
			n := TEGetScrapLen;
			for i := 0 to n - 1 do begin
				c := ord(h^^[i]);
				f.message := c;
				b := Key(f);
				if c = 13 then
					repeat
						ApplRun
					until not ReturnHit;
			end;
			Paste := true
		end
		else
			Paste := false
	end;

procedure OpenInputLine (process: ProcPtr);
	var
		p0: GrafPtr;
		fi: FontInfo;
		r: Rect;
		i: integer;
	begin
		if iw = nil then begin
			for i := 1 to 80 do begin
				Status[i] := ' ';
			end;
			ReturnHit := false;
			Status[0] := chr(80);
			SetRect(r, 0, 0, 16, 16);
			iw := NewWindow(nil, r, '', false, 3, WindowPtr(-1), false, 0);
			if iw <> nil then begin
				GetPort(p0);
				SetPort(iw);
				SetOrigin(-2, -2);
				penMode(patXor);
				TextFont(monaco);
				TextSize(9);
				TextFace([]);
				TextMode(srcCopy);
				GetFontInfo(fi);
				line1 := fi.ascent + fi.leading;
				line2 := line1 + fi.descent + fi.leading + fi.ascent + 1;
				letterw := fi.widMax;
				SizeWindow(iw, 80 * letterw + 4, line2 + fi.descent + fi.leading + 6, true);
				with screenBits.bounds do
					MoveWindow(iw, (right - left - iw^.portRect.right + 2) div 2 - 1, bottom - iw^.portRect.bottom - 5, true);
				SetRect(r, 0, line1 + fi.descent + 1, 80 * letterw, line2 + fi.descent);
				lineh := TENew(r, r);
				if lineh <> nil then begin
					Hact := ApplTask(@Activate, app4Evt);
					Hupd := ApplTask(@Update, updateEvt);
					Hmouse := ApplTask(@Mouse, mouseMsg + inContent);
					Hkey := ApplTask(@Key, keyDown);
					Hakey := ApplTask(@Akey, autoKey);
					Hidle := ApplTask(@Idle, nullEvent);
					Hpaste := ApplTask(@Paste, menuMsg + editMenu);
					SetPort(p0);
					ShowWindow(iw);
					TEAutoView(true, lineh);
					TEActivate(lineh);
					procs := process;
				end;
			end
		end;
	end;

procedure StatusLine (var s: string);
	begin
		status := s;
		DoRedraw(1);
	end;


procedure InsertInputLine (var s: string; select: boolean);
	var
		i, k: integer;
		f: EventRecord;
		b: boolean;
	begin
		repeat
			i := pos(chr(13), s);
			if i = 0 then begin
				k := lineh^^.selStart;
				TEInsert(@s[1], length(s), lineh);
				if select then
					TESetSelect(k, k + length(s), lineh);
				TESelView(lineh);
				exit(InsertInputLine);
			end;
			TEInsert(@s[1], i - 1, lineh);
			GotLine; { this might cause reentrance! }
			delete(s, 1, i);
		until s = '';
	end;

procedure SetInputLine (var s: string);
	begin
		if iw <> nil then begin
			TEDeactivate(lineh);
			TESetSelect(0, 32767, lineh);
			TEDelete(lineh);
			InsertInputLine(s, false);
			SetCursor(length(s) + 1);
			TESelView(lineh);
			TEActivate(lineh);
		end
	end;

procedure CloseInputLine;
	begin
		ApplUntask(Hact);
		ApplUntask(Hupd);
		ApplUntask(Hmouse);
		ApplUntask(Hkey);
		ApplUntask(Hakey);
		ApplUntask(Hidle);
		ApplUntask(Hpaste);
		DisposeWindow(iw);
		iw := nil
	end;

end.