{	ircle - Internet Relay Chat client	}
{	File: IRCPreferences	}
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

unit IRCPreferences;
{ deals with the Preferences settings and files }

interface
uses
	TCPTypes, TCPStuff, TCPConnections, ApplBase, MiscGlue, MsgWindows, {}
	IRCGlobals, IRCaux, IRCChannels;

procedure InitIRCPreferences;
{ startup }

procedure GetPrefs;
{ run prefs inquire dialog }

procedure GetShortcuts;
{ inquire new shortcuts }

procedure SetShortcutsMenu;
{ set the shortcuts menu }

implementation

{$SETC DEBUG=false}

{$IFC DISTRIBUTION }
{$SETC DEBUG=FALSE }
{$ENDC}

{ÊThis module is ugly. I know that. -ot }

const
	MAXMENU = 16;

var
	shcd: DialogPtr;

function NewClearHandle (nbytes: longint): Handle;
inline
	$201F,		{	MOVE.L (A7)+, D0		}
	$A322,		{	_NewHandle,Clear		}
	$2E88,		{	MOVE.L A0,(A7)		}
	$31C0, $0220;	{	MOVE.W D0,MemError	}

procedure defTODia (d: DialogPtr);
	var
		h: Handle;
		i, j: integer;
		l: longint;
		s: Str255;
		r: Rect;
	begin
		GetDItem(d, 3, i, h, r);
		SetIText(h, default^^.Server);
		GetDItem(d, 4, i, h, r);
		NumToString(default^^.Port, s);
		SetIText(h, s);
		GetDItem(d, 5, i, h, r);
		SetIText(h, default^^.Nick);
		GetDItem(d, 6, i, h, r);
		SetIText(h, default^^.userLoginName);
		GetDItem(d, 7, i, h, r);
		SetIText(h, default^^.userName);
		GetDItem(d, 8, i, h, r);
{    SetIText(h, default^^.autoExec); }
		for i := 1 to 4 do begin
			GetDItem(d, i + 8, j, h, r);
			SetCtlValue(ControlHandle(h), ord(default^^.notify[i]))
		end;
	end;

procedure defFROMDia (d: DialogPtr);
	var
		h: Handle;
		i, j: integer;
		l: longint;
		s: Str255;
		r: Rect;
	begin
		GetDItem(d, 3, i, h, r);
		GetIText(h, default^^.Server);
		GetDItem(d, 4, i, h, r);
		GetIText(h, s);
		StringToNum(s, l);
		default^^.Port := loword(l);
		GetDItem(d, 5, i, h, r);
		GetIText(h, default^^.Nick);
		GetDItem(d, 6, i, h, r);
		GetIText(h, default^^.userLoginName);
		GetDItem(d, 7, i, h, r);
		GetIText(h, default^^.userName);
		GetDItem(d, 8, i, h, r);
{     GetIText(h, default^^.autoExec); }
		for i := 1 to 4 do begin
			GetDItem(d, i + 8, j, h, r);
			default^^.notify[i] := boolean(GetCtlValue(ControlHandle(h)))
		end;
	end;

procedure SetShortcutsMenu;
	var
		m: MenuHandle;
		i: integer;
		s: str255;
	begin
		m := GetMHandle(261);
		for i := 1 to 10 do begin
			if Shortcuts^^[i] = '' then
				s := ' '
			else if length(Shortcuts^^[i]) > MAXMENU then
				s := concat(copy(Shortcuts^^[i], 1, MAXMENU), '...')
			else
				s := Shortcuts^^[i];
			SetItem(m, i + 2, s);
		end;
	end;

function CloseShortcuts (var e: eventRecord): boolean;
	var
		i, j: integer;
		h: Handle;
		r: Rect;
		s: str255;
	begin
		if shcd <> nil then
			if DialogPtr(e.message) = shcd then
				if TrackGoAway(shcd, e.where) then begin
					for i := 1 to 10 do begin
						GetDItem(shcd, i, j, h, r);
						GetIText(h, s);
						Shortcuts^^[i] := s;
					end;
					SetShortcutsMenu;
					DisposDialog(shcd);
					shcd := nil;
					dirtyPrefs := true;
					CloseShortcuts := true;
					exit(CloseShortcuts)
				end;
		CloseShortcuts := false
	end;


procedure GetOldstylePrefs;
	var
		i: integer;
		olds: boolean;
	begin
		olds := false;
		default := defaultHndl(GetResource('Pref', 256));
		if default = nil then begin
			default := defaultHndl(NewClearHandle(sizeof(defaultType)));
			default^^.port := 6667
		end
		else begin
			DetachResource(Handle(default));
			olds := true
		end;
		if gethandlesize(handle(default)) < sizeof(defaultType) then begin
			sethandlesize(handle(default), sizeof(defaultType));
			for i := 1 to 4 do
				default^^.notify[i] := false
		end;
		shortcuts := shortcutsHndl(GetResource('Pref', 257));
		if shortcuts = nil then begin
			shortcuts := shortcutsHndl(NewClearHandle(sizeof(ShortcutsType)));
		end
		else begin
			DetachResource(Handle(shortcuts));
			olds := true
		end;
		SetShortcutsMenu;
		defFont := FSHndl(GetResource('Pref', 258));
		if defFont <> nil then begin
			with defFont^^ do begin
				MWdefaultFont := fnt;
				MWdefaultSize := siz
			end;
			ReleaseResource(Handle(defFont));
			olds := true
		end;
		if olds then
			i := Alert(A_CONVERT, nil);
	end;


procedure GetPrefs;
	var
		d: DialogPtr;
		h: Handle;
		i, j: integer;
		r: Rect;
		s: str255;
	begin
		d := GetNewDialog(D_PREFS, nil, WindowPtr(-1));
		defTODia(d);
		SelIText(d, 3, 0, 32767);
		InitCursor;
		repeat
			ModalDialog(nil, i);
			if i = 2 then begin
				DisposDialog(d);
				exit(GetPrefs)
			end;
			if (i >= 9) and (i <= 12) then begin
				GetDItem(d, i, j, h, r);
				default^^.notify[i - 8] := not default^^.notify[i - 8];
				SetCtlValue(ControlHandle(h), ord(default^^.notify[i - 8]));
			end;
		until i = 1;
		defFROMDia(d);
		DisposDialog(d);
		default^^.nick := copy(default^^.nick, 1, 9);
		dirtyPrefs := true;
	end;

procedure GetShortcuts;
	var
		m: MenuHandle;
		i, j: integer;
		h: Handle;
		r: Rect;
	begin
		if shcd = nil then begin
			shcd := GetNewDialog(D_SHCUTS, nil, WindowPtr(-1));
			m := GetMHandle(261);
			for i := 1 to 10 do begin
				GetDItem(shcd, i, j, h, r);
				SetIText(h, Shortcuts^^[i]);
			end;
		end
		else
			SelectWindow(shcd);
	end;


procedure InitIRCPreferences;
	var
		i, j: integer;
		a: AppFile;
		s: str255;
	procedure GetNewstylePrefs;
		begin
			default := defaultHndl(NewClearHandle(sizeof(defaultType)));
			shortcuts := shortcutsHndl(NewClearHandle(sizeof(ShortcutsType)));
			reset(initFile, a.fname);  { actual loading occurs later in IRCInit }
			readPrefs := true;
		end;
	begin
		shcd := nil;
		default := nil;
		shortcuts := nil;
		readPrefs := false;
		dirtyPrefs := false;
		CountAppFiles(i, j);
		if j = 0 then begin
			default := defaultHndl(NewClearHandle(sizeof(defaultType)));
			default^^.port := 6667;
			shortcuts := shortcutsHndl(NewClearHandle(sizeof(ShortcutsType)));
			GetPrefs
		end
		else begin
			GetAppFiles(1, a);
			j := SetVol(nil, a.vrefnum);
			if j = 0 then begin
				i := OpenResFile(a.fname);
				j := ResError
			end;
			if (j = resFNotFound) or (j = EOFErr) then
				GetNewstylePrefs
			else if j = 0 then begin
				if Count1Resources('Pref') > 0 then
					GetOldstylePrefs
				else
					GetNewstylePrefs;
				CloseResFile(i);
			end
			else begin
				NumtoString(j, s);
				ParamText(s, '', '', '');
				i := Alert(A_FERR, nil);
				ApplExit
			end;
		end;
		if default = nil then
			debugstr('default=nil');
		if shortcuts = nil then
			debugstr('shortcuts=nil');
		i := ApplTask(@CloseShortcuts, mouseMsg + inGoAway);
	end;

end.