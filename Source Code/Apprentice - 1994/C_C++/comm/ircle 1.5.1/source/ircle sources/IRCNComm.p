{	ircle - Internet Relay Chat client	}
{	File: IRCNComm	}
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

unit IRCNComm;
{ Handles numeric server messages. }

interface
uses
	TCPTypes, TCPStuff, TCPConnections, ApplBase, MiscGlue, MsgWindows, {}
	IRCGlobals, IRCAux, IRCChannels, IRCNotify, IRCCommands;

function NumericComm (comm: integer; var from, target, rest: string): boolean;
{ Handles numeric message comm with arguments from, target, rest }

implementation

function NumericComm (comm: integer; var from, target, rest: string): boolean;
	var
		s: string;
		s1, s2, s3, s4: string[80];
		l: longint;
		i: integer;
		c: char;
	begin
		NumericComm := true;
{ Numerics and their meanings taken originally from IRCII 2.1.something. }
{ Adapted to RFC 1459 with many additionsÉ }
{ Where there are different formats for 2.6/2.7, this will work with 2.7 as tested. }
{ Some 2.8 messages found by experimentation }
{ Here messages are sorted by type }
		case comm of

{ generic line }
			251, 255, { new LUSERS reply }
			305, 306, { AWAY confirmation }
			342, { SUMMON confirmation }
			364, { new LINKS reply }
			371, { new INFO reply }
			372, 375, { new MOTD }
			391, { TIME reply }
			395: { nobody logged in }
				LineMsg(rest);

{ generic line with from field }
			200..209, 261, 212..218, 241..244, { TRACE/STATS info }
			302, {Ênew USERHOST reply }
			382: { Rehash }
				if not flushing then begin
					s := concat(from, ': ', rest);
					LineMsg(s)
				end;

{ generic error or important message }
			1, 3, { 2.8 intro }
			256..259, { new ADMIN reply }
			381, { Operator status }
			401..414, { no such nick/server/channel; cannot send }
			421, { unknown command }
			431, 432, 436, { bogus nick }
			442, { not on channel }
{ 461 need passwd for OPER? }
			461, { need more parameters }
			462, { registration problems }
			467, { Channel key set }
			481..483: { no privileges }
				begin
				if rest[1] = ':' then
					delete(rest, 1, 1);
				s := concat('*** ', rest);
				LineMsg(s);
			end;

{ generic error with from field }
			422..424, { Server config error }
			444..446, { SUMMON/USERS failed }
			465, { Banned }
			471..475, { Cannot join/ set mode }
			491, { No O-line }
			501, 502, { Bad usermode }
			331: { Topic bogosity }
				begin
				s := concat('*** (', from, ') ', rest);
				LineMsg(s)
			end;

{ÊStatus information }
			4: { 2.8 server telling its name, version etc }
				begin
				NextArg(rest, s1);
				CurrentServer := s1;
				NextArg(rest, s1);
				if copy(s1, 1, 3) = '2.8' then
					serverVersion := SV_28
				else begin
					s := concat('*** Server version is unknown to client: ', s1);
					LineMsg(s)
				end;
				NextArg(rest, s1);
				UpdateStatusLine;
				s := concat('Mode flags for users are: ', s1, '; for channels are: ', rest);
				LineMsg(s)
			end;
			301:  { user is away }
				begin
				NextArg(rest, s1);
				s := concat(s1, ' is away (', rest, ')');
				Message(s)
			end;
			324: { Mode }
				begin
				NextArg(rest, s1);
				if length(rest) > 1 then
					if (length(rest) > 2) or (rest[2] <> ' ') then begin
						s := concat('Mode is ', rest);
						ChannelMsg(s1, s);
					end
			end;
			332: { Topic }
				begin
				s := concat('Topic is: ', rest);
				Message(s)
			end;
			221: { user mode }
				begin
				s := concat('User mode is: ', rest);
				Message(s)
			end;


{ Verbose command replies }
			211: { STATS L reply }
				begin
				NextArg(rest, s1);
				s := concat('**', s1, '**');
				LineMsg(s);
				s := '';
				for i := 1 to 5 do begin
					NextArg(rest, s1);
					s := stringof(s, s1 : 12);
				end;
				s := concat(s, ' ', rest);
				LineMsg(s)
			end;
			252..254: { new LUSERS numbers }
				begin
				NextArg(rest, s1);
				s := concat('There are ', s1, ' ', rest);
				LineMsg(s);
			end;
			303: { ISON reply }
				IsonReply(rest);
			352:  { new WHO reply }
				if not flushing then begin
					NextArg(rest, s1);
					if s1 <> 'Channel' then begin
						NextArg(rest, s2);
						NextArg(rest, s3);
						s2 := concat(s2, '@', s3);
						NextArg(rest, s3);
						NextArg(rest, s3);
						NextArg(rest, s4);
						s := StringOf(s1 : 10, ' ', s3 : 9, s4 : 4, '  ', s2, ' (', rest, ')');
						ChannelMsg(s1, s)
					end;
				end;
			353: { new NAMES reply }
				if showNAMES and (not flushing) then begin
					s := copy(rest, 3, 255);
					i := pos(' ', s);
					s1 := copy(s, 1, i - 1);
					ChannelMsg(s1, s)
				end;
			311: { whois name info }
				begin
				NextArg(rest, s1);
				NextArg(rest, s2);
				NextArg(rest, s3);
				s := concat(s1, ' is ', s2, '@', s3, ' (', copy(rest, 4, 255), ')');
				Message(s)
			end;
			314: { whowas name info }
				begin
				NextArg(rest, s1);
				NextArg(rest, s2);
				NextArg(rest, s3);
				s := concat(s1, ' was ', s2, '@', s3, ' (', copy(rest, 4, 255), ')');
				Message(s)
			end;
			313: { whois operator }
				begin
				NextArg(rest, s1);
				s := concat(s1, ' ', rest);
				Message(s)
			end;
			319: { whois channels }
				begin
				NextArg(rest, s1);
				s := concat(s1, ' is on channels ', rest);
				Message(s)
			end;
			312: { whois host/server }
				begin
				NextArg(rest, s1);
				s := concat('On IRC via server ', rest);
				Message(s)
			end;
			317: { whois idle }
				begin
				NextArg(rest, s1);
				NextArg(rest, s2);
				s := concat('idle for ', s2, ' seconds');
				Message(s)
			end;
			321: {ÊLIST header }
				begin
				NextArg(rest, s1);
				NextArg(rest, s2);
				s := StringOf(copy(s1, 1, 12) : 12, s2 : 4, '  ', copy(rest, 1, 60));
				LineMsg(s);
			end;
			322: { LIST entry }
				if not flushing then begin
					NextArg(rest, s1);
					c := s1[1];
					if (listpriv and listglob and (c = '*')) or (listpub and ((listloc and (c = '&')) or (listglob and (c = '#')))) then begin
						NextArg(rest, s2);
						stringtonum(s2, l);
						if (l >= listmin) and (l <= listmax) then begin
							if listtop or (rest <> '') then begin
								s := StringOf(copy(s1, 1, 12) : 12, s2 : 4, '  ', copy(rest, 1, 60));
								LineMsg(s);
								lastWindow := nil; { save from net.terrorists with awfully long topics }
							end
						end
					end
				end;
			341:  { invite confirmation }
				begin
				NextArg(rest, s1);
				s := concat('Inviting ', s1, ' to channel ', rest);
				lastInvite := rest;
				ChannelMsg(rest, s)
			end;
			351: { Server version }
				begin
				NextArg(rest, s1);
				s := concat('Server ', from, ' runs version ', s1);
				Message(s)
			end;
			367: { Ban list }
				begin
				NextArg(rest, s1);
				s := concat(rest, ' is banned on ', s1);
				ChannelMsg(s1, s)
			end;
			392, 393: { USERS reply }
				begin
				NextArg(rest, s1);
				NextArg(rest, s2);
				s := StringOf(copy(s1, 1, 8) : 9, copy(s2, 1, 10) : 12, ' ', rest);
				LineMsg(s);
			end;

{ End of list }
			219, 232, 323, 365: { Display it }
				begin
				flushing := false;
				Message(rest);
				UpdateStatusLine
			end;
			318, 369, 315, 366, 368, 374, 376, 394: { Don't display }
				begin
				flushing := false;
				UpdateStatusLine
			end;

{ Various errors }
			433: { Nick in use }
				begin
				NextArg(rest, s1);
				s := concat('*** Nickname ', s1, ' is in use. You have to choose another.');
				LineMsg(s);
			end;
			441: { Not in channel }
				begin
				NextArg(rest, s1);
				NextArg(rest, s2);
				s := concat('*** ', s1, ' is not on channel ', s2);
				Message(s);
			end;
			443: { unnecessary invitation }
				begin
				NextArg(rest, s1);
				NextArg(rest, s2);
				s := concat('*** ', s1, ' is already on channel ', s2);
				ChannelMsg(s2, s);
			end;
			463, 466: { server refuses connection }
				begin
				s := concat('*** ', from, ' refuses connection: ', rest);
				LineMsg(s)
			end;

{ messages that generate a response }
			451: { not registered }
				begin
				s := concat('*** Registration failed, trying again...');
				LineMsg(s);
				RegUser
			end;
			464: { Need password - to be changed }
				begin
				s := concat('*** ', rest);
				LineMsg(s)
			end;

{ ignored messages }
			2, { duplicated intro }
			300: { dummy }
				begin
			end;

			otherwise
				NumericComm := false
		end;
	end;

end.