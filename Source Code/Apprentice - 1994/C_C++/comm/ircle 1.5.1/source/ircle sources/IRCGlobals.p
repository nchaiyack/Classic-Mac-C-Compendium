{	ircle - Internet Relay Chat client	}
{	File: IRCGlobals	}
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

unit IRCGlobals;
{ Contains global constants and variables }

interface
uses
	TCPTypes, TCPStuff, TCPConnections, MiscGlue, MsgWindows;

type
	defaultType = record
			Port: integer;
			Server, Nick, username, userLoginName, autoExec: str255;
			notify: array[1..4] of boolean;
		end;
	defaultPtr = ^defaultType;
	defaultHndl = ^defaultPtr;		{ Preferences }

	shortcutsType = array[1..10] of str255;
	shortcutsPtr = ^shortcutsType;
	shortcutsHndl = ^shortcutsPtr;	{ Shortcuts }

	FSType = record
			fnt, siz: integer
		end;
	FSPtr = ^FSType;
	FSHndl = ^FSPtr;				{ default font/size }

	Table = packed array[char] of char;
	TablePtr = ^Table;
	TableHndl = ^TablePtr;		{ charset translation table }

	str20 = string[20];
	CEPtr = ^ConnectionEventRecord;


const

	CREATOR_IRCLE = 'pr¾C';
	TYPE_PREFS = 'Pref';

	COSPACE = 10000;		{ Workspace to allocate for Coroutines }
	LOFREEMEM = 20000;		{ Memory checking lower (critical) bound }
	HIFREEMEM = 30000;		{ Memory checking higher (uncritical) bound }
	MEMTIME = 30;			{ Memory checking timeout (seconds) }

	DCC_CHAT_PREFIX = '=';
	DCC_CHAT_POSTFIX = '=';
	INACTIVE_PREFIX = '(';
	INACTIVE_POSTFIX = ')';

{ Error messages (STR#256 indexes) }
	E_NOSERVER = 1;		{ Not connected }
	E_NOTARGET = 2;		{ Not talking to a target }
	E_SFAILED = 3;			{ Corresponding with ConnectionEvents }
	E_NSFAILED = 4;
	E_OFAILED = 5;
	E_CLOSING = 6;
	E_CLOSED = 7;
	E_OPEN = 8;			{ Attempting to re-open }
	E_NOPREFS = 9;			{ÊDon't have valid prefs }
	E_NOHELP = 10;			{ÊThis means the app file is broken... }

{ Connection status }
	S_CONN = 0;			{ connected }
	S_OFFLINE = 1;			{ offline }
	S_LOOKUP = 2;			{ DNS lookup }
	S_OPENING = 3;			{ connecting }
	S_CLOSING = 4;			{ closing }
{ Server versions }
	SV_27 = 27;
	SV_28 = 28;

{ Alerts }
	A_INFO = 128;			{ About box }
	A_TCPERR = 129;		{ Cannot open TCP driver }
	A_QUIT = 131;			{ Ask whether to quit }
	A_SAVE = 132;			{ Ask whether to save set }
	A_CONVERT = 133;		{ÊWarning about oldstyle prefs file }
	A_LOWMEM = 259;		{ Memory shortage }
	A_OPKILL = 260;		{ Operator kill }
	A_SSTAT = 261;		{ Server connection status }
	A_FERR = 262;			{ File system error }

{ Dialogs }
	D_INFOPOP = 130;		{ Popup status window }
	D_PREFS = 256;			{ Preferences }
	D_SHCUTS = 257;		{ Shortcuts }

{ Menus }
	M_F_OPEN = 1;
	M_F_CLOSE = 2;
	M_F_LOAD = 4;
	M_F_SAVE = 5;
	M_F_PREFS = 6;
	M_F_LOG = 8;
	M_F_FLUSH = 9;
	M_F_QUIT = 11;
	M_COMMANDS = 259;		{ Commands menu }
	M_CO_JOIN = 1;
	M_CO_PART = 2;
	M_CO_LIST = 4;
	M_CO_WHO = 5;
	M_CO_QUERY = 7;
	M_CO_WHOIS = 8;
	M_CO_INVITE = 9;
	M_CO_KICK = 10;
	M_CO_AWAY = 12;
	M_CO_MSG = 13;
	M_SHCUTS = 261;		{ Shortcuts menu }
	M_SH_DEFINE = 1;
	M_SH_FIRST = 2;
	M_FONT = 262;			{ Fonts menu }
	M_FO_9 = 1;
	M_FO_10 = 2;
	M_FO_12 = 3;
	M_FO_14 = 4;
	M_FO_FIRST = 5;
	M_WINDOWS = 263;		{ Windows menu }
	M_WI_CYCLE = 1;
	M_WI_MAIN = 2;
	M_WI_FIRST = 3;


var
{ general status vars }
	CL_VERSION: string;		{ Version of this program }
	serverVersion, 			{ Version of server }
	serverStatus: integer;	{ Connection status }
	dirtyPrefs,				{ Need to save prefs }
	readPrefs,				{ Need to read prefs from file }
	UserRegistered, 			{ USER command already done }
	QuitRequest,			{ Will quit }
	flushing, 				{ Ignore server output up to end of list }
	logging,				{ Logging to file }
	IsAway,				{ AWAY status set }
	inBackground,			{ Running in background }
	notified				{ User has got notfication from background }
	: boolean;
	sSocket: connectionIndex;	{ Server connection }
	lastWindow: MWHndl;		{ window last notice went in }
	readTimeout,			{ timeout for TCP reading }
	idleTime: longint;		{ time when last command sent }
	NFT: integer;			{ number of DCC filetransfers in progress }

{ user vars }
	CmdChar: char;			{ Command prefix }
	CurrentServer, 			{ server currently connected to }
	CurrentTarget,			{ current target to talk to (corresponds to active window) }
	lastInvite				{ Channel to which got last invited }
	: str255;
	CurrentNick, 			{ Nickname }
	lastMsg				{ Nick who sent last MSG }
	: str20;
	initFile, 				{ File to read init commands from }
	logfile: text;			{ File to log to }

{ user display status }
	showJOIN, showPART, showQUIT, showWALLOPS, showTOPIC, {}
	showINVITE, showNICK, showMODE, showKICK, showNAMES: boolean;

{ user preferences }
	default: defaultHndl;
	Shortcuts: shortcutsHndl;
	defFont: FSHndl;

	ISOEncode, ISODecode: TableHndl;


implementation
{ Global variables get initialized in IRCInit }
end.