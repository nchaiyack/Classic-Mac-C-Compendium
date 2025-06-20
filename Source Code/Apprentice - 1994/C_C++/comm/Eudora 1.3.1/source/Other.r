resource 'STR#' (1200, purgeable) {
	{"4","24","41","44"}
};

resource 'STR#' (1600, purgeable) {
	{	/* array StringArray: 7 elements */
		/* [1] */
		"To:",
		/* [2] */
		"From:",
		/* [3] */
		"Subject:",
		/* [4] */
		"Cc:",
		/* [5] */
		"Bcc:",
		/* [6] */
		"X-Attachments:",
		/* [7] */
		"Reply-To:",
		/* [8] */
		"X-Priority:"
	}
};

resource 'STR#' (24200, purgeable) {
	{	/* array StringArray: 7 elements */
		/* [1] */
		"To:",
		/* [2] */
		"From:",
		/* [3] */
		"Subject:",
		/* [4] */
		"Cc:",
		/* [5] */
		"Bcc:",
		/* [6] */
		"Attachments:",
		/* [7] */
		"Reply-To:",
		/* [8] */
		"X-Priority:"
	}
};

resource 'STR#' (1800, purgeable) {
	{	/* array StringArray: 12 elements */
		"",
		"",
		"Undo Cut",
		"Undo Copy",
		"Undo Paste",
		"Undo Paste As Quote",
		"Undo Clear",
		"Undo Typing",
		"",
		"",
		"Undo Wrap",
		"Undo Unwrap",
		"",
		"",
		"Redo Cut",
		"Redo Copy",
		"Redo Paste",
		"Redo Paste As Quote",
		"Redo Clear",
		"Redo Typing",
		"",
		"",
		"Redo Wrap",
		"Redo Unwrap"
	}
};

resource 'STR#' (2200, purgeable) {
	{	/* array StringArray: 10 elements */
		/* [1] */
		"The network is presently unreachable.",
		/* [2] */
		"The host is presently unreachable.",
		/* [3] */
		"The protocol is presently unreachable.",
		/* [4] */
		"The port is presently unreachable.",
		/* [5] */
		"A packet could not be sent because it had to be fragmented, and fragmentation is currently not allowed.",
		/* [6] */
		"A packet's source route failed.",
		/* [7] */
		"A packet's time to live was exceeded.",
		/* [8] */
		"Incorrect parameters were supplied to TCP/IP.",
		/* [9] */
		"A required option is missing.",
		/* [10] */
		"An unknown ICMP message was received."
	}
};

resource 'STR#' (3200, purgeable) { /* headers to ignore */
{
  "received",
  "message",
  "resent-message",
  "in-reply",
  "references",
  "return",
  "x400",
	"mail-system-version",
	"mime-version",
	"x-envelope",
	"x-vms",
	"x-ph"
}};

resource 'STR#' (5400, purgeable) { /* headers for replies */
{
  "reply-to: ",
  "resent-reply-to: ",
  "from: ",
  "resent-from: ",
}};

resource 'STR#' (3400, purgeable) { /* headers ALWAYS to ignore */
  {"from "}
};

resource 'STR#' (3600, purgeable) { /* CTB errors */
	{	/* array StringArray: 10 elements */
		/* [1] */
		"It won't say why.",
		/* [2] */
		"(Not really.)",
		/* [3] */
		"The connection was refused.",
		/* [4] */
		"The attempt to make the connection failed.",
		/* [5] */
		"The operation timed out.",
		/* [6] */
		"The connection is not open.",
		/* [7] */
		"The connection is not closed.",
		/* [8] */
		"There is nothing pending.",
		/* [9] */
		"That operation is not supported.",
		/* [10] */
		"No connection tools are available."
	}
};

resource 'STR#' (3800, purgeable) { /* RFC 822 month abbreviations */
  {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"}
};

resource 'STR#' (4000, purgeable) { /* RFC 822 weekday abbreviations */
	{"Sun","Mon","Tue","Wed","Thu","Fri","Sat"}
};

resource 'STR#' (2400, purgeable) {  /* default navin resource */
	{"\n\n\n"}
};


resource 'STR#' (1400, purgeable) {
	{	/* array StringArray: 37 elements */
		/* [1] */
		"An error involving documents has occurred.",
		/* [2] */
		"The directory is full.",
		/* [3] */
		"The disk is full.",
		/* [4] */
		"There is no such volume.",
		/* [5] */
		"An I/O error has occurred.",
		/* [6] */
		"The name is bad.",
		/* [7] */
		"An fnOpnErr has occurred.",
		/* [8] */
		"The end of a file has been unexpectedly reached.",
		/* [9] */
		"Error positioning document pointer.",
		/* [10] */
		"An mFulErr has occurred.",
		/* [11] */
		"There are too many documents open.",
		/* [12] */
		"The document was not found.",
		/* [13] */
		"The disk is locked.",
		/* [14] */
		"The document is locked.",
		/* [15] */
		"The volume is locked.",
		/* [16] */
		"The document is busy.",
		/* [17] */
		"The document already exists.",
		/* [18] */
		"I am unable to open the document for writing.",
		/* [19] */
		"There has been an error in a parameter.",
		/* [20] */
		"The reference number is bad.",
		/* [21] */
		"A gfpErr has occurred.",
		/* [22] */
		"The volume is off-line.",
		/* [23] */
		"You don't have sufficient permissions for that.",
		/* [24] */
		"The volume is on-line.",
		/* [25] */
		"There is no such driver.",
		/* [26] */
		"The disk is not a Macintosh disk.",
		/* [27] */
		"An extFSErr has occurred.",
		/* [28] */
		"A fsRnErr has occurred.",
		/* [29] */
		"A badMDBErr has occurred.",
		/* [30] */
		"You may not write to that.",
		/* [31] */
		"The directory was not found.",
		/* [32] */
		"There are too many working directories open.",
		/* [33] */
		"A badMovErr has occurred.",
		/* [34] */
		"The volume is the wrong type.",
		/* [35] */
		"The volume is gone.",
		/* [36] */
		"An fsDSIntErr has occurred.",
		/* [37] */
		"An unknown error has occurred."
	}
};


resource 'STR#' (800, purgeable) /* password changing */
{{
	"user %p%p","pass %p%p","newpass %p%p","quit%p%p"
}};

resource 'STR#' (600, purgeable) {
	{
		/* [1] */
		"(Highest)",
		/* [2] */
		"(High)",
		/* [3] */
		"(Normal)",
		/* [4] */
		"(Low)",
		/* [5] */
		"(Lowest)",
		"\nThis message is of highest priority.",
		"\nThis message is of high priority.",
		"",
		"\nThis message is of low priority.",
		"\nThis message is not worth the phosphors used to display it."
	}
};

resource 'MENU' (1509) { 509, textMenuProc, allEnabled, enabled,
	"Db",
	{	/* array: 16 elements */
		/* [0] */	"0",									noIcon, noKey, noMark, plain,
		/* [1] */	"MouseRgn",						noIcon, noKey, noMark, plain,
		/* [2] */	"2X TCP Buffer",			noIcon, noKey, noMark, plain,
		/* [3] */	"Leaks",							noIcon, noKey, noMark, plain,
		/* [4] */	"4",								noIcon, noKey, noMark, plain,
		/* [5] */	"5",									noIcon, noKey, noMark, plain,
		/* [6] */	"Heap Check",					noIcon, noKey, noMark, plain,
		/* [7] */	"Purge",							noIcon, noKey, noMark, plain,
		/* [8] */	"Snoop Handles",			noIcon, noKey, noMark, plain,
		/* [9] */	"9",									noIcon, noKey, noMark, plain,
		/* [10]*/	"10",									noIcon, noKey, noMark, plain,
		/* [11]*/	"From Parsing",				noIcon, noKey, noMark, plain,
		/* [12]*/	"ports +10000",				noIcon, noKey, noMark, plain,
		/* [13]*/	"4X TCP Buffer",			noIcon, noKey, noMark, plain,
		/* [14]*/	"14",									noIcon, noKey, noMark, plain,
		/* [15]*/	"15",									noIcon, noKey, noMark, plain
	}
};
