{	ircle - Internet Relay Chat client	}
{	File: IRCIgnore	}
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

unit IRCIgnore;
{ Deals with the /ignore command }

interface
uses
	TCPTypes, TCPStuff, TCPConnections, ApplBase, MiscGlue, MsgWindows, {}
	IRCGlobals, IRCAux, IRCChannels;

procedure InitIRCIgnore;
{ Startup }

procedure DoIgnore (var s: string);
{ handles the /ignore command }
{ format: /ignore nick!user@host = ignore nick,user,host}
{    /ignore nick, /ignore user@host do as well}
{     wildcards are ? and * }
{    /ignore +nick!user@host = ignore and give him notice }
{    /ignore = list all ignorations }
{    /ignore -nick!user@host = remove a tag }

procedure GetIgnoreList (var s: string);
{ Returns the ignore list }

function IsIgnored (var s: string; p: boolean): boolean;
{ handles messages from s - returns true if ignored. p=message may be answered }

implementation

const
	DELV = chr(4);	{ mark for rude ignore }
	DELS = chr(5);	{ mark for silent ignore }

var
	ign: CharsHandle;
	maxign: integer;

procedure GetIgnoreList (var s: string);
	var
		i: integer;
	begin
		s := '';
		i := 0;
		while i < maxign do begin
			if ign^^[i] = DELV then
				s := concat(s, '+');
			repeat
				i := succ(i);
				if i >= maxign then
					leave;
				if ord(ign^^[i]) < 32 then
					leave;
				s := concat(s, ign^^[i]);
			until false;
			s := concat(s, ' ');
		end;
	end;

procedure ListIgnores;
	var
		s: string;
	begin
		GetIgnoreList(s);
		s := concat('*** Ignore List: ', s);
		Message(s)
	end;

procedure DoIgnore (var s: string);
	var
		tag: char;
		i: integer;
		p: string;
	begin
		while s <> '' do begin
			NextArg(s, p);
			if pos('@', p) = 0 then
				p := concat(p, '!*@*')
			else if pos('!', p) = 0 then
				p := concat(p[1], '*!', copy(p, 2, 255));
			if p[1] = '-' then begin
				i := Munger(Handle(ign), 0, @p[2], length(p) - 1, Ptr(1), 0);
				if i > 0 then
					i := Munger(Handle(ign), i - 1, nil, 1, Ptr(1), 0);
				maxign := maxign - length(p);
			end
			else begin
				if p[1] = '+' then
					p[1] := DELV
				else
					insert(DELS, p, 1);
				i := length(p);
				if PtrAndHand(@p[1], Handle(ign), i) = 0 then
					maxign := maxign + i;
			end;
		end;
		ListIgnores
	end;

procedure IgnoreBack (var s: string);
	var
		t: string;
		i: integer;
	begin
		if serverStatus = S_CONN then begin
			t := s;
			i := pos('!', t);
			if i > 0 then
				t[0] := chr(i - 1);
			t := concat('NOTICE ', t, ' :You are being ignored');
			PutLine(t);
		end;
	end;

function upc (c: char): char;
	begin
		if (c >= 'a') and (c <= 'z') then
			upc := chr(ord(c) - 32)
		else
			upc := c
	end;

function IsIgnored (var s: string; p: boolean): boolean;
	var
		back: boolean;
		a: integer;
	function matchFrom (i, j: integer): boolean;
		begin
			matchFrom := true;
			repeat
				if (ign^^[i] < ' ') and (s[j] < ' ') then
					exit(matchFrom);	{ completely matched }
				if ign^^[i] = '*' then begin
					i := succ(i);
					if (i >= maxign) or (ign^^[i] < ' ') then
						exit(matchFrom); { pattern ended in * : matched }
					repeat
						while upc(s[j]) <> upc(ign^^[i]) do begin
							j := succ(j);
							if s[j] < ' ' then begin
								matchFrom := false; { pattern ended *x , no match for x }
								exit(matchFrom);
							end;
						end; { pattern *x, , match for x }
						if matchFrom(i, j) then { check for matching rest }
							exit(matchFrom);
						j := succ(j)
					until s[j] < ' '; { backtrack }
					matchFrom := false;
					exit(matchFrom);
				end
				else if (ign^^[i] <> '?') then begin
					if (upc(ign^^[i]) <> upc(s[j])) then begin
						matchFrom := false;	{ Mismatch }
						exit(matchFrom)
					end;
				end;
				i := succ(i);
				j := succ(j);
			until false;
		end;
	begin
		s[length(s) + 1] := chr(0);
		a := 0;
		while a < maxign do begin
			back := (ign^^[a] = DELV);
			a := succ(a);
			if matchFrom(a, 1) then begin
				IsIgnored := true;
				if p and back then
					IgnoreBack(s);
				exit(IsIgnored)
			end;
			repeat
				a := succ(a);
				if a >= maxign then
					leave;
			until ign^^[a] < ' ';
		end;
		IsIgnored := false;
	end;


procedure InitIRCIgnore;
	begin
		ign := CharsHandle(NewHandle(0));
		maxign := 0;
	end;

end.