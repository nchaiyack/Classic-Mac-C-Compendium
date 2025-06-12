{	ircle - Internet Relay Chat client	}
{	File: IRCNotify	}
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

unit IRCNotify;
{ Deals with the /notify command }

interface

uses
	TCPTypes, TCPStuff, TCPConnections,{}
	Coroutines, ApplBase, MiscGlue, MsgWindows, InputLine, {}
	IRCGlobals, IRCaux, IRCPreferences, IRCChannels;

procedure InitIRCNotify;
{ Startup }

procedure DoNotify (var s: string);
{ Processes the NOTIFY command }

procedure IsonReply (var s: string);
{ Processes ISON replies from server }

procedure OneNotify (var s: string; ison: boolean);
{ Processes explicit signon/off messages }

procedure GetNotifyList (var s: string);
{ Returns the notification list }

procedure RunNotify;
{ Does periodic queries }

implementation

var
	doit, pending: boolean;
	themap: set of byte;
	noti: Handle;

procedure InitIRCNotify;
	begin
		doit := false;
		pending := false;
		themap := [];
		noti := NewHandle(0);
	end;

procedure RunNotify0;
	var
		s: string;
		n: integer;
	begin
		if serverStatus = S_CONN then begin
			n := GetHandleSize(noti);
			BlockMove(noti^, @s[1], n);
			s[0] := chr(n);
			insert('ISON :', s, 1);
			PutLine(s);
		end;
	end;

procedure GetNotifyList (var s: string);
	var
		i: integer;
	begin
		i := GetHandleSize(noti);
		BlockMove(noti^, @s[1], i);
		s[0] := chr(i);
	end;

procedure DoNotify (var s: string);
	var
		i, n: integer;
		j: byte;
		olddoit: boolean;
		p: string;
	begin
		if s = '' then begin
			pending := false;
			RunNotify0;
		end;
		while s <> '' do begin
			NextArg(s, p);
			insert(' ', p, 255);
			if p[1] = '-' then begin
				n := length(p) - 1;
				i := Munger(noti, 0, @p[2], n, Ptr(1), 0);
				if i > 0 then begin
					for j := i to 255 - n do
						if j + n in themap then
							themap := themap + [j]
						else
							themap := themap - [j];
					for j := 255 - n + 1 to 255 do
						themap := themap - [j];
				end
			end
			else begin
				if length(p) + GetHandleSize(noti) < 250 then
					i := PtrAndHand(@p[1], noti, length(p));
			end;
		end;
		pending := false;
		i := GetHandleSize(noti);
		olddoit := doit;
		doit := (i > 0);
		if doit then begin
			GetNotifyList(s);
			insert('*** Notify List: ', s, 1);
			LineMsg(s);
			if not olddoit then
				RunNotify;
		end;
	end;


procedure onestring (i: integer; var s: string);
	var
		n: integer;
		no: CharsHandle;
	begin
		n := GetHandleSize(noti);
		if i > n then
			s := ''
		else begin
			no := CharsHandle(noti);
			BlockMove(@no^^[i], @s[1], n - i + 1);
			s[0] := chr(n - i + 1);
			i := pos(' ', s);
			if i > 0 then
				s[0] := chr(i - 1);
		end;
	end;


procedure OneNotify (var s: string; ison: boolean);
	var
		i: integer;
	begin
		i := Munger(noti, 0, @s[1], length(s), nil, 0);
		if i >= 0 then begin
			if ison then
				themap := themap + [i]
			else
				themap := themap - [i];
		end;
	end;


procedure IsonReply (var s: string);
	var
		p: string;
		i: integer;
		j: byte;
		newmap: set of byte;
	begin
		if pending then begin
			newmap := [];
			while s <> '' do begin
				NextArg(s, p);
				i := Munger(noti, 0, @p[1], length(p), nil, 0);
				if i >= 0 then
					newmap := newmap + [i];
			end;
			for j := 0 to 255 do
				if (j in themap) and (not (j in newmap)) then begin
					onestring(j, p);
					insert('*** Signoff: ', p, 1);
					Message(p);
				end
				else if (not (j in themap)) and (j in newmap) then begin
					onestring(j, p);
					insert('*** Signon: ', p, 1);
					Message(p);
				end;
			themap := newmap;
			pending := false
		end
		else begin
			insert('*** Signed on: ', s, 1);
			LineMsg(s)
		end
	end;


procedure RunNotify; { Does periodic queries }
	begin
		if doit and (not pending) then begin
			pending := true;
			RunNotify0
		end
	end;

end.