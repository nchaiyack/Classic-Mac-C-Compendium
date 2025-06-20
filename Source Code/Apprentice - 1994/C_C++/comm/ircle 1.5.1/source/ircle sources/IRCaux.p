{	ircle - Internet Relay Chat client	}
{	File: IRCaux	}
{	Copyright � 1992 Olaf Titz (s_titz@ira.uka.de)	}

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

unit IRCaux;
{ utilities }

interface
uses
	TCPTypes, TCPStuff, TCPConnections, ApplBase, InputLine, {}
	MiscGlue, MsgWindows, IRCGlobals;

var
	Watch: CursHandle;

procedure ServerOK (status: connectionEvent);
{ Call this with the result of TCP functions }

procedure PutLine (var s: string);
{ Send a line to the server }

function IsChannel (var s: string): boolean;
{ is it a valid channel name? }

procedure MakeChannel (var s: string);
{ insert # to make a channel name }

procedure NextArg (var from, arg: string);
{ get next arg out of 'from' into 'arg' }

procedure OpenConnection;
{ open server connection }

procedure UpdateStatusLine;
{ Draw the IRC status line }

procedure AdjustFontMenu;
{ Set the font menu check marks }

function ValidPrefs: boolean;
{ check whether all settings are correct }

implementation

type
	str8 = string[8];

var
	ip: longint;

procedure ServerOK (status: connectionEvent);
	var
		a, n: integer;
		s: Str255;
	begin
		case status of
			C_SearchFailed: 
				begin
				a := S_OFFLINE;
				n := E_SFAILED;
			end;
			C_NameSearchFailed: 
				begin
				a := S_OFFLINE;
				n := E_NSFAILED;
			end;
			C_FailedToOpen: 
				begin
				a := S_OFFLINE;
				n := E_OFAILED;
			end;
			C_Closing: 
				begin
				a := S_OFFLINE;
				n := E_CLOSING;
			end;
			C_Closed: 
				begin
				a := S_OFFLINE;
				n := 0; { this alert is redundant }
			end;
			otherwise
				begin
				a := serverStatus;
				n := 0;
			end;
		end;
		serverStatus := a;
		if n <> 0 then begin
			GetIndString(s, 256, n);
			ParamText(s, '', '', '');
			InitCursor;
			n := Alert(A_SSTAT, nil);
		end;
		UpdateStatusLine;
	end;

procedure PutLine (var s: string);
	var
		i, n, oe: integer;
		p: TCPConnectionPtr;
	begin
		n := length(s);
		for i := 1 to n do
			s[i] := ISOEncode^^[s[i]];
		s[n + 1] := chr(10);
		GetConnectionTCPC(sSocket, p);
		i := TCPSendAsync(p, @s[1], n + 1, false, @oe);
		if i <> 0 then
			serverStatus := i
		else begin
			repeat
				ApplRun
			until oe <> inProgress;
			serverStatus := oe;
			if oe <> 0 then
				UpdateStatusLine;
		end;
	end;

function IsChannel (var s: string): boolean;
	begin
		IsChannel := (s[1] = '#') or (s[1] = '&'); { RFC 1459 }
	end;

procedure MakeChannel (var s: string);
	begin
		if s[1] <> '#' then
			insert('#', s, 1);
	end;

procedure NextArg (var from, arg: string);
	var
		i: integer;
	begin
		i := pos(' ', from);
		if i = 0 then begin
			arg := from;
			from := ''
		end
		else begin
			arg := copy(from, 1, i - 1);
			while i < length(from) do begin
				if (from[i + 1] <> ':') and (from[i + 1] <> ' ') then
					leave;
				i := i + 1;
			end;
			delete(from, 1, i);
		end
	end;


function watchFound (var e: EventRecord): boolean;
	var
		c: CEPtr;
	begin
		c := CEPtr(e.message);
		if c^.connection <> sSocket then begin
			watchFound := false;
			exit(watchFound)
		end
		else
			watchFound := true;
		if c^.event = C_Found then
			ip := c^.value
		else begin
			ip := -1;
			ServerOk(c^.event);
		end;
	end;

function watchOpen (var e: EventRecord): boolean;
	var
		c: CEPtr;
	begin
		c := CEPtr(e.message);
		if c^.connection <> sSocket then begin
			watchOpen := false;
			exit(watchOpen)
		end
		else
			watchOpen := true;
		if c^.event = C_Established then
			ip := 1
		else begin
			ip := -1;
			ServerOk(c^.event);
		end;
	end;

procedure WaitEvent (p: ProcPtr);
	var
		i: integer;
	begin
		i := ApplTask(p, TCPMsg);
		ip := 0;
		repeat
			ApplRun;
			if flushing then begin
				flushing := false;
				ip := -1
			end
		until ip <> 0;
		ApplUNtask(i);
	end;

procedure OpenConnection;
	var
		e: integer;
	begin
		if (serverStatus <> S_CONN) and ValidPrefs then begin
			CurrentNick := default^^.nick;
			UserRegistered := false;
			serverStatus := S_LOOKUP;
			flushing := false;
			UpdateStatusLine;
			SetCursor(Watch^^);
			e := FindAddress(sSocket, default^^.server, nil);
			if e = 0 then begin
				ip := 0;
				WaitEvent(@watchFound);
				if ip <> -1 then begin
					serverStatus := S_OPENING;
					UpdateStatusLine;
					e := NewActiveConnection(sSocket, 8192, ip, default^^.port, nil);
					if e = 0 then begin
						WaitEvent(@watchOpen);
						if ip <> -1 then begin
							serverStatus := S_CONN;
						end
						else
							serverOk(C_FailedToOpen)
					end
					else
						serverOk(C_FailedToOpen)
				end
			end
			else
				serverOk(C_SearchFailed)
		end;
		UpdateStatusLine;
		InitCursor;
	end;


function two (n: integer): str8;
	var
		s: str8;
	begin
		s := stringof(n + 100 : 3);
		two := copy(s, 2, 2)
	end;

procedure UpdateStatusLine;
	var
		s: string[80];
		s0: string[40];
		sa, s1, s2, s3: string[10];
		d: DateTimeRec;
	begin
		case serverStatus of
			S_OFFLINE: 
				s := '(Offline)';
			S_LOOKUP: 
				s := '(Address lookup)';
			S_OPENING: 
				s := '(opening)';
			S_CONN: 
				s := CurrentServer;
			S_CLOSING, connectionClosing: 
				s := '(closing)';
			otherwise
				s := stringof('Err(', serverStatus : 1, ')');
		end;
		if (serverStatus = S_CONN) or (serverStatus = S_OPENING) or (serverStatus = S_LOOKUP) or (serverStatus = S_CLOSING) then
			DisableItem(GetMHandle(fileMenu), M_F_OPEN)
		else
			EnableItem(GetMHandle(fileMenu), M_F_OPEN);
		if IsAway then
			sa := ' (away)   '
		else
			sa := 'talking to';
		s0 := CurrentTarget;
		if s0 = '' then
			s0 := '(nobody)';
		s0 := concat(s0, '               ');
		if logging then
			s1 := 'Log'
		else
			s1 := '';
		if flushing then
			s2 := 'Flsh'
		else
			s2 := '';
		if NFT > 0 then
			s3 := stringof('FT(', NFT : 1, ')')
		else
			s3 := '';
		GetTime(d);
		s := stringof(CurrentNick : 10, ' talking to', copy(s0, 1, 12) : 13, s : 18, s1 : 4, s2 : 5, s3 : 8, '  ', two(d.hour), ':', two(d.minute), ':', two(d.second));
		StatusLine(s);
	end;

procedure AdjustFontMenu;
	var
		m: MenuHandle;
		i, j: integer;
		s: str255;
	begin
		m := GetMHandle(M_FONT);
		CheckItem(m, 1, (MWdefaultSize = 9));
		CheckItem(m, 2, (MWdefaultSize = 10));
		CheckItem(m, 3, (MWdefaultSize = 12));
		CheckItem(m, 4, (MWdefaultSize = 14));
		for i := 6 to CountMItems(m) do begin
			GetItem(m, i, s);
			GetFNum(s, j);
			CheckItem(m, i, (j = MWdefaultFont));
		end;
	end;

function ValidPrefs: boolean;
	begin
		ValidPrefs := (default^^.server <> '') and (default^^.port <> 0) and (default^^.nick <> '') and (default^^.username <> '') and (default^^.userLoginname <> '')
	end;

end.