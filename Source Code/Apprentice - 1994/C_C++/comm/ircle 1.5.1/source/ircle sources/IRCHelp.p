{	ircle - Internet Relay Chat client	}
{	File: IRCHelp	}
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

unit IRCHelp;
{ About and Help functions }

interface
uses
	TCPTypes, TCPStuff, TCPConnections, ApplBase, MiscGlue, MsgWindows, {}
	IRCGlobals, IRCChannels;

procedure InitIRCHelp;
{ Startup }

procedure ShowHelp;
{ Show HELP Window }

implementation

var
	dia: DialogPtr;	{ About dialog box }
	hel: MWHndl;	{ Help window }

procedure closehelp (w: WindowPtr);
	begin
		DeleteMWindow(hel);
		hel := nil
	end;

procedure GetVersion (var s: string);
	type
		vers = record
				v, st, l: integer;
			end;
		versPtr = ^vers;
		versHndl = ^versPtr;
	var
		vh: VersHndl;
	begin
		vh := VersHndl(GetResource('vers', 1));
		if vh <> nil then begin
			with vh^^ do begin
				s := concat(char(v div 256 + 48), '.', chr((v div 16) mod 16 + 48));
				if (v mod 16) <> 0 then
					s := concat(s, '.', chr(v mod 16 + 48));
			end;
			case (vh^^.st div 8192) of
				1: 
					s := concat(s, ' (development)');
				2: 
					s := concat(s, ' (alpha)');
				3: 
					s := concat(s, ' (beta)');
				4: 
					;
			end;
			ReleaseResource(Handle(vh));
		end
	end;

procedure ShowHelp;
	var
		st: str255;
		ht, hs: Handle;
		r: rect;
	begin
		if hel = nil then begin
{ The Help function consists of just loading the complete text into a window }
{ Sorry, no fancy section selector or hyperlinks. }
{ These are the things I am too lazy to write }
			ht := GetResource('TEXT', 128);
			hs := GetResource('styl', 128);
			if (ht <> nil) and (hs <> nil) then begin
				st := 'Help';
				SetRect(r, 0, 0, 0, 0);
				hel := NewMWindow(st, r, @closehelp);
				if hel <> nil then begin
					HLock(ht);
					TEStylInsert(ht^, GetHandleSize(ht), stScrpHandle(hs), hel^^.t);
					HUnlock(ht);
					ReleaseResource(ht);
					ReleaseResource(hs);
					SetCtlMax(hel^^.vscr, hel^^.t^^.nlines - hel^^.vislines);
					exit(ShowHelp)
				end
			end;
			StatusMsg(E_NOHELP);
		end
		else
			SelectWindow(hel^^.w);
	end;


function Menu256 (var e: EventRecord): boolean;
	var
		i: integer;
	begin
		if e.message = 1 then begin
			ParamText(CL_VERSION, '', '', '');
			SetDAFont(courier);
			i := Alert(A_INFO, nil);
			SetDAFont(0);
			if i = 2 then begin
				ShowHelp;
				Menu256 := true
			end
			else
				Menu256 := false;
		end;
	end;

procedure InitIRCHelp;
	var
		i: integer;
	begin
		i := ApplTask(@Menu256, menuMsg + 256);
		dia := nil;
		hel := nil;
		GetVersion(CL_VERSION)
	end;

end.