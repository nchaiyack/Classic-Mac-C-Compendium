/* Strings - Defines the string and error resources of MUDDweller             */

#ifndef __TYPES.R__
#include "Types.r"
#endif

#ifndef __MacAppTypes__
#include "MacAppTypes.r"
#endif

/* -------------------------------------------------------------------------- */

#define kMUDStrings			1002
#define kKeyStrings			1003
#define kCmdStrings			1004

#define kErrReasons			1128

/* -------------------------------------------------------------------------- */

resource 'STR#' (kMUDStrings,
#if qNames
	"kMUDStrings",
#endif
	preload) {
	 {	/* [1] */	"Open Connection";
	 	/* [2] */	"Abort Connection";
	 	/* [3] */	"Connection…";
	 	/* [4] */	"TCP/IP Address…";
	 	/* [5] */	"update ";
	 	/* [6] */	"ed ";
	 	/* [7] */	"w ";
	 	/* [8] */	"1,$d\n";
	 	/* [9] */	"a\n";
	 	/* [10] */	".\n";
	 	/* [11] */	"Q\n";
	 	/* [12] */	":";
	 	/* [13] */	"> ";
	 	/* [14] */	"1p\n";
	 	/* [15] */	".+1p\n";
	 	/* [16] */	".p\n";
	 	/* [17] */	"1,20p\n";
	 	/* [18] */	".+1,.+20p\n";
	 	/* [19] */	".-19,.p\n";
	 	/* [20] */	"Unrecognized or failed command.\n";
	 	/* [21] */	"MPW Shell";
	 	/* [22] */	"0.0.0.0";
	 	/* [23] */	"Log to File…";
	 	/* [24] */	"Close Log";
	 	/* [25] */	"<<<>>> Log";
	 	/* [26] */	"Save log as:";
	 	/* [27] */	"Shift-";
	 	/* [28] */	"Opt-";
	 	/* [29] */	"Ctl-";
	 	/* [30] */	"DEL";
	 	/* [31] */	"Clear";
	 	/* [32] */	"Escape";
	 	/* [33] */	"F1";
	 	/* [34] */	"F2";
	 	/* [35] */	"F3";
	 	/* [36] */	"F4";
	 	/* [37] */	"F5";
	 	/* [38] */	"F6";
	 	/* [39] */	"F7";
	 	/* [40] */	"F8";
	 	/* [41] */	"F9";
	 	/* [42] */	"F10";
	 	/* [43] */	"F11";
	 	/* [44] */	"F12";
	 	/* [45] */	"F13";
	 	/* [46] */	"F14";
	 	/* [47] */	"F15";
	 	/* [48] */	"FwdDel";
	 	/* [49] */	"OFF";
	}
};

/* -------------------------------------------------------------------------- */

resource 'STR#' (kKeyStrings,
#if qNames
	"kKeyStrings",
#endif
	preload) {
	 {	/* [1] */	"NUL";
	 	/* [2] */	"Home";
	 	/* [3] */	"STX";
	 	/* [4] */	"Enter";
	 	/* [5] */	"End";
	 	/* [6] */	"Help";
	 	/* [7] */	"ACK";
	 	/* [8] */	"BEL";
	 	/* [9] */	"Delete";
	 	/* [10] */	"Tab";
	 	/* [11] */	"PgUp";
	 	/* [12] */	"VT";
	 	/* [13] */	"PgDown";
	 	/* [14] */	"Return";
	 	/* [15] */	"SO";
	 	/* [16] */	"SI";
	 	/* [17] */	"DLE";
	 	/* [18] */	"DC1";
	 	/* [19] */	"DC2";
	 	/* [20] */	"DC3";
	 	/* [21] */	"DC4";
	 	/* [22] */	"NAK";
	 	/* [23] */	"SYN";
	 	/* [24] */	"ETB";
	 	/* [25] */	"CAN";
	 	/* [26] */	"EM";
	 	/* [27] */	"SUB";
	 	/* [28] */	"Escape";
	 	/* [29] */	"Left";
	 	/* [30] */	"Right";
	 	/* [31] */	"Up";
	 	/* [32] */	"Down";
	 	/* [33] */	"Space";
	}
};

/* -------------------------------------------------------------------------- */

resource 'STR#' (kCmdStrings,
#if qNames
	"kCmdStrings",
#endif
	preload) {
	 {	/* [1] */	"SEND";
	 	/* [2] */	"MATCH";
	 	/* [3] */	"WAIT";
	 	/* [4] */	"PASSWD";
	 	/* [5] */	"QUIET";
	 	/* [6] */	"ECHO";
	}
};

/* -------------------------------------------------------------------------- */

resource 'errs' (kErrReasons,
#if qNames
	"ErrReasons",
#endif
	purgeable) {
	{	whichList, 0, kErrReasons;
		-23003, -23000, 2;
		-23004, -23004, 3;
		-23005, -23005, 4;
		-23008, -23008, 25;
		-23009, -23009, 5;
		-23012, -23012, 25;
		-23015, -23015, 11;
		-23016, -23016, 6;
		-23033, -23033, 7;
		-23036, -23036, 10;
		-23037, -23037, 8;
		-23041, -23041, 21;
		-23044, -23044, 9;
		-23045, -23045, 22;
		-23046, -23046, 7;
		-23047, -23047, 23;
		-23048, -23048, 10;
		-23048, -23000, 1;
		-25001, -25001, 12;
		-25002, -25002, 13;
		-25003, -25003, 14;
		-25004, -25004, 15;
		-25005, -25005, 16;
		-25006, -25006, 17;
		-25007, -25007, 18;
		-25008, -25008, 19;
		-25009, -25009, 20;
		-25018, -25010, 24;
		-25019, -25019, 26;
	}
};

/* -------------------------------------------------------------------------- */

resource 'STR#' (kErrReasons,
#if qNames
	"app error explanations",
#endif
	purgeable) {
	{	/* [1] */	"of a MacTCP error";
		/* [2] */	"of an error in the MacTCP configuration";
		/* [3] */	"of an error in getting the address";
		/* [4] */	"the connection is closing";
		/* [5] */	"of insufficient resources to perform the request";
		/* [6] */	"the command timed out";
		/* [7] */	"none of the known name servers are responding";
		/* [8] */	"no route could be found";
		/* [9] */	"no nameserver could be found";
		/* [10] */	"there is not enough free memory";
		/* [11] */	"the connection could not be opened";
		/* [12] */	"no communication tool could be found";
		/* [13] */	"of an error while initializing the communication toolbox";
		/* [14] */	"the document has an unknown format";
		/* [15] */	"of a network failure";
		/* [16] */	"of a transmission error";
		/* [17] */	"the network is unreachable";
		/* [18] */	"the host is unreachable";
		/* [19] */	"the receiver process is unreachable";
		/* [20] */	"nobody listens on the specified TCP port";
		/* [21] */	"the host name has a syntax error";
		/* [22] */	"this domain name does not exist";
		/* [23] */	"of an error in the name translation";
		/* [24] */	"of a communication manager error";
		/* [25] */	"the remote host closed the connection";
		/* [26] */	"the connection timed out";
	}
};

/* -------------------------------------------------------------------------- */
