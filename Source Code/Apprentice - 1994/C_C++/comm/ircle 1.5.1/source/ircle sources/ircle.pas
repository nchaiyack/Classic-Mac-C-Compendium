{	ircle - Internet Relay Chat client	}
{	File: ircle	}
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

program ircle;
{ This is a small IRC client for the Macintosh. }
{ Written by Olaf Titz (s_titz@ira.uka.de), Karlsruhe, July/August 1992. }
{ The TCP interface code written  by Peter N.Lewis and Harry Chesley/Apple Computer Inc. }
{ Parts of the command protocol handling, and many ideas }
{   derived from IRCII by Michael Sandrof }

uses
	TCPTypes, TCPStuff, TCPConnections,{}
	Coroutines, ApplBase, MiscGlue, MsgWindows, InputLine, {}
	IRCGlobals, IRCaux, IRCPreferences, IRCInput, {}
	IRCCommands, IRCChannels, IRCNotify, IRCSComm, IRCHelp, IRCInit;

const
	NOTIFY_INTERVAL = 30; { Seconds between notify checks }

var
	i, notify: integer;
	fmem, lofmem, t0: longint;
	purged: boolean;

{$SETC autoopen=false}

{$IFC DISTRIBUTION }
{$SETC autoopen=true }
{$ENDC}

{ CheckMem gets called once in each run through the main event loop. }
{ If free memory runs low, first memory is compacted, then an alert is displayed }
{ and subsequent memory checking disabled for 30 seconds, or until more memory gets freed, }
{ e.g. by closing a window. }
procedure CheckMem;
	var
		i: longint;
	begin
		if fmem < 0 then begin
			getdatetime(i);
			if (abs(i - lofmem) > MEMTIME) or (freemem > HIFREEMEM) then
				fmem := LOFREEMEM;
		end
		else if freemem < fmem then begin
			if purged then begin
				fmem := -1;
				getdatetime(lofmem);
				if not MWFreeMem then
					i := Alert(A_LOWMEM, nil)
			end
			else begin
				PurgeMem(maxSize);
				purged := true
			end
		end
		else
			purged := false;
	end;

function Clock (var e: EventRecord): boolean;
	begin
		CheckMem;
{    if GetWRefCon(FrontWindow) = 0 then}
{    DisableItem(FMenu, M_F_CLOSE)}
{    else}
{    EnableItem(FMenu, M_F_CLOSE);}
		if abs(e.when - t0) >= 60 then begin
			t0 := e.when;
			UpdateStatusLine;
			notify := notify + 1;
			if notify >= NOTIFY_INTERVAL then begin
				notify := 0;
				if not IsAway then
					RunNotify
			end
		end;
		Clock := false
	end;

procedure ReadInitFile;
	var
		s: string;
		b1, b2: boolean;
	begin
		while not eof(initFile) do begin
			readln(initFile, s);
			b1 := (copy(s, 1, 6) = 'SERVER');
			b2 := (copy(s, 1, 4) = 'JOIN');
			HandleCommand(s);
			if b1 then { on SERVER command, wait for connection starting up }
				begin
				flushing := true;
				repeat
					ApplRun
				until not flushing;
			end;
			if b2 then
				repeat
					ApplRun
				until EmptyRect(windowarg);
		end;
		close(initFile);
		SetShortcutsMenu;
	end;

procedure ExitTCP;
	var
		i: integer;
	begin
		if logging then
			close(logfile);
		FinishEverything
	end;

begin
	serverStatus := S_OFFLINE;
	fmem := LOFREEMEM;
	purged := false;
	if IRCInitAll then begin
		notify := 0;
		i := ApplTask(@Clock, nullEvent);
		ApplExitproc(@ExitTCP);
		UnloadSeg(@IRCInitAll);
		t0 := -maxlongint;
		if readPrefs then
			ReadInitFile;
{$IFC autoopen}
		OpenConnection;
		if serverStatus = S_CONN then
			RegUser;
{$ENDC}
		InitCursor;
		repeat
			ApplRun;
		until QuitRequest;
		ApplExit;
	end;
end.