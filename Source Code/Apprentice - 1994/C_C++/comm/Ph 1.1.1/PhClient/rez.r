/*_____________________________________________________________________

	rez.r	-  Resources for the Ph Application
 _____________________________________________________________________*/


#include "Types.r"
#include "SysTypes.r"

include	"Movable-Modal WDEF 1.01";

/* Edit Window. */

resource 'WIND' (128, "Edit") {
	{0, 0, 283, 504},
	zoomDocProc, 
	inVisible, 
	goAway, 
	0x0, 
	""
};

resource 'CNTL' (128, "Edit Scroll Bar") {
	{28, 489, 269, 505},
	0, 
	visible, 
	0, 
	0, 
	scrollBarProc, 
	0, 
	""
};

resource 'CNTL' (129, "Show Field Info Check Box") {
	{6, 5, 19, 120},
	0,
	visible,
	1,
	0,
	checkBoxProcUseWFont,
	0,
	"Show Field Info"
};

resource 'CNTL' (130, "Revert Field Btn") {
	{6, 135, 19, 225},
	0,
	visible,
	0,
	0,
	pushButProcUseWFont,
	0,
	"Revert Field"
};

resource 'CNTL' (131, "Revert All Fields Btn") {
	{6, 250, 19, 365},
	0,
	visible,
	0,
	0,
	pushButProcUseWFont,
	0,
	"Revert All Fields"
};

/* Query Window. */

resource 'WIND' (129, "Query", preload) {
	{0, 0, 283, 504},
	zoomDocProc,
	inVisible,
	goAway,
	0x0,
	""
};

resource 'CNTL' (132, "Query Doit Btn") {
	{42, 430, 62, 490},
	0,
	visible,
	1,
	0,
	pushButProc,
	0,
	"Doit"
};

resource 'CNTL' (133, "Query Scroll Bar") {
	{75, 489, 269, 505},
	0,
	visible,
	0,
	0,
	scrollBarProc,
	0, 
	""
};

/* Help Window. */

resource 'WIND' (130, "Help", preload) {
	{0, 0, 283, 504},
	zoomDocProc,
	inVisible,
	goAway,
	0x0,
	""
};

resource 'CNTL' (134, "Help Scroll Bar") {
	{-1, 489, 269, 505},
	0,
	visible,
	0,
	0,
	scrollBarProc,
	0, 
	""
};
   
/* Menus. */
	
#define AllItems		0b1111111111111111111111111111111	/* 31 flags */
#define mi1		0b0000000000000001
#define mi2		0b0000000000000010
#define mi3		0b0000000000000100
#define mi4		0b0000000000001000
#define mi5		0b0000000000010000
#define mi6		0b0000000000100000
#define mi7		0b0000000001000000
#define mi8		0b0000000010000000
#define mi9		0b0000000100000000
#define mi10	0b0000001000000000
#define mi11	0b0000010000000000
#define mi12	0b0000100000000000
#define mi13	0b0001000000000000
#define mi14	0b0010000000000000
#define mi15	0b0100000000000000
#define mi16	0b1000000000000000

resource 'MBAR' (128, preload) {
	{128, 129, 130, 131, 132, 133}
};

resource 'MENU' (128, "Apple", preload) {
	1, 
	textMenuProc,
	AllItems & ~mi2,
	enabled, 
	apple,
	{
		"About Ph…",
			noicon, nokey, nomark, plain;
		"-",
			noicon, nokey, nomark, plain
	}
};

resource 'MENU' (129, "File", preload) {
	2, 
	textMenuProc,
	mi1 | mi7 | mi12 | mi13 | mi15,
	enabled, 
	"File",
	{
		"New Query Window",
			noicon, "N", nomark, plain;
		"New Ph Record…",
			noicon, "R", nomark, plain;
		"Open Ph Record…",
			noicon, "O", nomark, plain;
		"Close",
			noIcon, "W", nomark, plain;
		"Save Ph Record",
			noicon, "S", nomark, plain;
		"-",
			noicon, nokey, nomark, plain;
		"Login…",
			noicon, "L", nomark, plain;
		"Logout",
			noicon, nokey, nomark, plain;
		"-",
			noicon, nokey, nomark, plain;
		"Change Password…",
			noicon, "K", nomark, plain;
		"Delete Ph Record…",
			noicon, "D", nomark, plain;
		"Update Site List",
			noicon, nokey, nomark, plain;
		"Change Default Server…",
			noicon, nokey, nomark, plain;
		"-",
			noicon, nokey, nomark, plain;
		"Quit",
			noicon, "Q", nomark, plain
	}
};

resource 'MENU' (130, "Edit", preload) {
	3, 
	textMenuProc,
	0,
	disabled, 
	"Edit",
	 {
		"Undo",
			noicon, "Z", nomark, plain;
		"-",
			noicon, nokey, nomark, plain;
		"Cut",
			noicon, "X", nomark, plain;
		"Copy",
			noicon, "C", nomark, plain;
		"Paste",
			noicon, "V", nomark, plain;
		"Clear",
			noicon, nokey, nomark, plain;
		"-",
			noicon, nokey, nomark, plain;
		"Select All",
			noicon, "A", nomark, plain;
	}
};

resource 'MENU' (131, "Proxy", preload) {
	4,
	textMenuProc,
	allItems,
	disabled,
	"Proxy",
	{
	}
};

resource 'MENU' (132, "Windows", preload) {
	5,
	textMenuProc,
	allItems,
	enabled,
	"Windows",
	{
	}
};

resource 'MENU' (133, "Help", preload) {
	6,
	textMenuProc,
	allItems,
	enabled,
	"Help",
	{
		"Change Ph Help Server…",
			noicon, nokey, nomark, plain;
		"-",
			noicon, nokey, nomark, plain;
	}
};

/* Strings. */

resource 'STR#' (128, "Server Field Names", preload) {
	{
/* 1*/	"name",
/* 2*/	"type",
/* 3*/	"hero",
/* 4*/	"alias",
/* 5*/	"proxy",
/* 6*/	"password",
/* 7*/	"email",
	}
};

resource 'STR#' (129, "Server Commands", preload) {
	{
/* 1*/	"fields",
/* 2*/	"login \"^0\"",
/* 3*/	"answer ",
/* 4*/	"query ^0 return alias",
/* 5*/	"query alias=\"^0\" return all",
/* 6*/	"query ^0 return all",
/* 7*/	"change alias=\"^0\" make ^1=\"",
/* 8*/	"change alias=\"^0\" make password=",
/* 9*/	"query proxy=\"^0\" return alias",
/*10*/	"add alias=\"^0\"",
/*11*/	"delete alias=\"^0\"",
/*12*/	"status",
/*13*/	"quit",
/*14*/	"query ",
/*15*/	"help macph ",
/*16*/	"help native ",
/*17*/	"query ns-servers type=serverlist return text",
/*18*/	"query id=\"^0\" return all",
	}
};

resource 'STR#' (130, "Server Field Attributes", preload) {
	{
/* 1*/	"Change",
/* 2*/	"Encrypt",
	}
};

resource 'STR#' (131, "MacTCP Error Messages", preload) {
	{
/* 1*/	"MacTCP is improperly configured.",
/* 2*/	"MacTCP is not installed, or it is improperly configured.",
/* 3*/	"The domain name server is not responding.",
/* 4*/	"The host name you specified for the Ph server does not exist.",
/* 5*/	"Unable to locate a domain name server for the host name "
			"you specified for the Ph server. MacTCP may be improperly "
			"configured.",
/* 6*/	"The Ph server is not responding.",
/* 7*/	"The Ph server went down.",
/* 8*/	"None of the known domain name servers are responding.",
/* 9*/	"MacTCP initialization error: error in getting address from a "
			"server or the address is already in use by another machine.",
/*10*/	"Domain name server timeout.",
/*11*/	"Unexpected error number ^0.",
/*12*/	"The domain name server has returned an error.",
/*13*/	"Syntax error in Ph server host domain name.",
/*14*/	"MacTCP is improperly configured. "
			"No domain name servers are defined.",
	}
};

resource 'STR#' (132, "Server Error Messages", preload) {
	{
/* 1*/	"One of the values you specified was not accepted by the server.",
/* 2*/	"You must specify a name.",
/* 3*/	"Unexpected Ph server response code ^0.",
/* 4*/	"The alias or password was not accepted by the Ph server.",
/* 5*/	"The name does not uniquely identify a Ph server record (more "
			"than one Ph server record matches the name.)",
/* 6*/	"Sorry, the server database is currently read only (^0).",
/* 7*/	"The server database is busy. Try again later.",
/* 8*/	"There is no Ph server record matching that alias, id or name.",
/* 9*/	"^0",
/*10*/	"This field has a maximum size of ^0 characters.",
/*11*/	"You must specify a server.",
/*12*/	"You must specify an alias or name.",
/*13*/	"You must specify a password.",
/*14*/	"You must specify an alias.",
/*15*/	"You must specify a type.",
/*16*/	"The two passwords are not identical.",
/*17*/	"Only standard printable ASCII characters are legal here.",
/*18*/	"You are not authorized to make changes to the Ph record for “^0”.",
/*19*/	"The Proxy menu only works with specially modified Ph servers.",
/*20*/	"Unexpected syntax error in server response.",
/*21*/	"The Ph help text has not been installed on the server.",
/*22*/	"The value you specified for the “^0” field was not accepted by "
			"the server: ^1",
/*23*/	"The password you specified was not accepted by the server: ^0",
/*24*/	"You cannot login to a record which has no alias.",
/*25*/	"You cannot open a record which has no alias.",
/*26*/	"You cannot save a record with an empty alias field.",
/*27*/	"You must specify an alias, id or name."
	}
};

resource 'STR#' (133, "Miscellaneous Strings", preload) {
	{
/* 1*/	"• = Field Changed",
/* 2*/	"Max Size = ",
/* 3*/	"Passwords are never displayed.  To change the password, "
			"use\nthe “Change Password” command in the “File” menu.",
/* 4*/	"Encrypted values cannot be displayed or edited.",
/* 5*/	"¬ = Can’t Edit Field",
/* 6*/	"Sending new password to server. Please wait.",
/* 7*/	"Getting Ph record from server. Please wait.",
/* 8*/	"Sending changed fields to server. Please wait.",
/* 9*/	"Creating server Ph record. Please wait.",
/*10*/	"Deleting server Ph record. Please wait.",
/*11*/	"Querying server. Please wait.",
/*12*/	"Getting help text from server. Please wait.",
/*13*/	"Getting help topic list from server ^0. Please wait.",
/*14*/	"Getting site list from server ^0. Please wait.",
/*15*/	"You are logged in to server “^0” as “^1”. ^2",
/*16*/	"You are a hero!",
/*17*/	"You are not logged in to a server.",
/*18*/	"^0^1 (login)",
/*19*/	"^0^1 (hero)",
/*20*/	"^0^1",
/*21*/	"Save changes for alias “^0” before ^1?",
/*22*/	"quitting",
/*23*/	"closing window",
/*24*/	"Why?…",
/*25*/	"Do you really want to delete the Ph record for alias “^0”?",
/*26*/	"You can’t delete the Ph record for alias “^0”, because "
			"it is the current login window, or because it was used as "
			"the current login window for some other open window.",
/*27*/	"Query ^0",
/*28*/	"site",
/*29*/	"server",
/*30*/	"domain",
/*31*/	"Ph Prefs",
/*32*/	"You are getting low on memory and should close some windows.",
/*33*/	"----- The query reply was too long. It was truncated.",
/*34*/	"Please select your default Ph server from the popup menu, "
			"or type the domain name of your default Ph server in the field "
			"provided. The default server should normally be your local campus "
			"or organization Ph server.",
/*35*/	"Please type the domain name of your default Ph server in the "
			"field provided. The default server should normally be your local "
			"campus or organization Ph server.",
/*36*/	"Ph requires the Monaco font.",
/*37*/	"Ph requires System 6.0.5 or later.",
/*38*/	"Please select a Ph help server from the popup menu, "
			"or type the domain name of a Ph help server in the field "
			"provided. The help server should normally be your local campus "
			"or organization Ph server.",
/*39*/	"Please type the domain name of a Ph help server in the "
			"field provided. The help server should normally be your local "
			"campus or organization Ph server.",
	}
};

resource 'STR#' (134, "About Box Text") {
	{
"A Mac Ph client, for use with the UIUC CSO nameserver.",
" ",
"Copyright ©1991, 1992, Northwestern University.",
" ",
"John Norstad",
"Academic Computing and Network Services",
"Northwestern University",
"j-norstad@nwu.edu",
" ",
"This program is free. Use it in good health.",
" ",
"Thanks to Steve Dorner for designing and writing the Ph system and",
"for his advice. Thanks also to Albert Lunde for the icon.",
" ",
"MPW C source code is available via anonymous FTP from",
"ftp.acns.nwu.edu.",
	}
};

resource 'STR ' (128, "Default Default Server", preload) {
	"ns.nwu.edu"
};

resource 'STR ' (129, "Server Port Number", preload) {
	"105"
};

/* About Box Dialog. */

resource 'DLOG' (128, "About Box") {
	{0, 0, 300, 500},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	128,
	""
};

resource 'DITL' (128, "About Box") {
	{	
		/* [1] - Icon. */
		{10, 23, 42, 55},
		Icon {
			enabled,
			128
		},
		/* [2] - Status info. */
		{20, 140, 52, 485},
		UserItem {
			enabled
		},
		/* [3] - Ph version number. */
		{17, 60, 35, 135},
		StaticText {
			enabled,
			"Ph 1.1.1"
		},
		/* [4] - About box text in Monaco 9. */
		{80, 15, 290, 485},
		UserItem {
			enabled
		},
		/* [5] - About box horizontal rule. */
		{55, 10, 56, 490},
		UserItem {
			enabled
		},
		/* [6] - Hot rectangle covering entire dialog. */
		{0, 0, 300, 500},
		UserItem {
			enabled
		},
		/* [7] - Balloon help. */
		{0, 0, 0, 0},
		HelpItem {
			disabled,
			HMScanhdlg {128}
		},
	}
};

/* Login Dialog. */

resource 'DLOG' (129, "Login") {
	{0, 0, 205, 450},
	2053,
	invisible,
	noGoAway,
	0x0,
	129,
	""
};

resource 'DITL' (129, "Login") {
	{	/* array DITLarray: 9 elements */
		/* [1] */
		{172, 377, 192, 437},
		Button {
			enabled,
			"Login"
		},
		/* [2] */
		{172, 304, 192, 364},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{96, 20, 112, 115},
		StaticText {
			disabled,
			"Alias or Name"
		},
		/* [4] */
		{96, 122, 112, 437},
		EditText {
			disabled,
			""
		},
		/* [5] */
		{134, 47, 150, 115},
		StaticText {
			disabled,
			"Password"
		},
		/* [6] */
		{134, 122, 150, 437},
		EditText {
			disabled,
			""
		},
		/* [7] */
		{58, 48, 74, 115},
		StaticText {
			disabled,
			"Ph Server"
		},
		/* [8] */
		{58, 122, 74, 437},
		EditText {
			disabled,
			""
		},
		/* [9] */
		{20, 438, 36, 437},
		UserItem {
			enabled,
		},
		/* [10] - Balloon help. */
		{0, 0, 0, 0},
		HelpItem {
			disabled,
			HMScanhdlg {129}
		},
	}
};

/* Change Password Dialog. */

resource 'DLOG' (130, "Change Password") {
	{0, 0, 165, 450},
	2053,
	invisible,
	noGoAway,
	0x0,
	130,
	""
};

resource 'DITL' (130, "Change Password") {
	{	/* array DITLarray: 7 elements */
		/* [1] */
		{132, 377, 152, 437},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{132, 304, 152, 364},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{56, 14, 72, 115},
		StaticText {
			disabled,
			"New Password"
		},
		/* [4] */
		{56, 122, 72, 437},
		EditText {
			disabled,
			""
		},
		/* [5] */
		{94, 13, 110, 114},
		StaticText {
			disabled,
			"New Password"
		},
		/* [6] */
		{94, 122, 110, 437},
		EditText {
			disabled,
			""
		},
		/* [7] */
		{15, 95, 31, 355},
		StaticText {
			disabled,
			"Please type your new password twice."
		},
		/* [8] - Balloon help. */
		{0, 0, 0, 0},
		HelpItem {
			disabled,
			HMScanhdlg {130}
		},
	}
};

/* Server Transaction Wait Dialog. */

resource 'DLOG' (131, "Wait", preload) {
	{0, 0, 88, 360},
	2053,
	invisible,
	noGoAway,
	0x0,
	131,
	""
};

resource 'DITL' (131, "Wait", preload) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{55, 287, 75, 347},
		Button {
			enabled,
			"Abort"
		},
		/* [2] */
		{10, 10, 42, 347},
		StaticText {
			disabled,
			""
		},
		/* [3] - Balloon help. */
		{0, 0, 0, 0},
		HelpItem {
			disabled,
			HMScanhdlg {131}
		},
	}
};

/* Open Dialog. */

resource 'DLOG' (132, "Open") {
	{0, 0, 91, 450},
	2053,
	invisible,
	noGoAway,
	0x0,
	132,
	""
};

resource 'DITL' (132, "Open") {
	{	/* array DITLarray: 4 elements */
		/* [1] */
		{58, 377, 78, 437},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{58, 304, 78, 364},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{20, 20, 36, 135},
		StaticText {
			disabled,
			"Alias, Id or Name"
		},
		/* [4] */
		{20, 142, 36, 437},
		EditText {
			disabled,
			""
		},
		/* [5] - Balloon help. */
		{0, 0, 0, 0},
		HelpItem {
			disabled,
			HMScanhdlg {132}
		},
	}
};

/* Error Message Dialog. */

resource 'DLOG' (133, "Error Message") {
	{0, 0, 139, 341},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	133,
	""
};

resource 'DITL' (133, "Error Message") {
	{	/* array DITLarray: 4 elements */
		/* [1] */
		{106, 268, 126, 328},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{13, 23, 45, 55},
		Icon {
			disabled,
			0
		},
		/* [3] */
		{13, 78, 93, 328},
		StaticText {
			disabled,
			"^0"
		},
		/* [4] - to outline ok button. */
		{0, 0, 0, 0},
		UserItem {
			disabled
		},
		/* [5] - Balloon help. */
		{0, 0, 0, 0},
		HelpItem {
			disabled,
			HMScanhdlg {133}
		},
	}
};

/* Save Changes Dialog. */

resource 'DLOG' (134, "Save Changes") {
	{0, 0, 139, 361},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	134,
	""
};

resource 'DITL' (134, "Save Changes") {
	{	/* array DITLarray: 6 elements */
		/* [1] */
		{106, 288, 126, 348},
		Button {
			enabled,
			"Save"
		},
		/* [2] */
		{106, 215, 126, 275},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{106, 78, 126, 161},
		Button {
			enabled,
			"Don’t Save"
		},
		/* [4] */
		{13, 23, 45, 55},
		Icon {
			disabled,
			2
		},
		/* [5] */
		{13, 78, 93, 348},
		StaticText {
			disabled,
			"^0"
		},
		/* [6] */
		{0, 0, 0, 0},
		UserItem {
			disabled
		},
		/* [7] - Balloon help. */
		{0, 0, 0, 0},
		HelpItem {
			disabled,
			HMScanhdlg {134}
		},
	}
};

/* New Dialog. */

resource 'DLOG' (135, "New") {
	{0, 0, 243, 422},
	2053,
	invisible,
	noGoAway,
	0x0,
	135,
	""
};

resource 'DITL' (135, "New") {
	{	/* array DITLarray: 12 elements */
		/* [1] */
		{210, 349, 230, 409},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{210, 276, 230, 336},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{20, 53, 36, 87},
		StaticText {
			disabled,
			"Alias"
		},
		/* [4] */
		{20, 94, 36, 409},
		EditText {
			disabled,
			""
		},
		/* [5] */
		{58, 45, 74, 87},
		StaticText {
			disabled,
			"Name"
		},
		/* [6] */
		{58, 94, 74, 409},
		EditText {
			disabled,
			""
		},
		/* [7] */
		{96, 53, 112, 87},
		StaticText {
			disabled,
			"Type"
		},
		/* [8] */
		{96, 94, 112, 409},
		EditText {
			disabled,
			""
		},
		/* [9] */
		{134, 19, 150, 87},
		StaticText {
			disabled,
			"Password"
		},
		/* [10] */
		{134, 94, 150, 409},
		EditText {
			disabled,
			""
		},
		/* [11] */
		{172, 19, 188, 87},
		StaticText {
			disabled,
			"Password"
		},
		/* [12] */
		{172, 94, 188, 409},
		EditText {
			disabled,
			""
		},
		/* [13] - Balloon help. */
		{0, 0, 0, 0},
		HelpItem {
			disabled,
			HMScanhdlg {135}
		},
	}
};

/* Really Delete Dialog. */

resource 'DLOG' (136, "Really Delete") {
	{0, 0, 139, 361},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	136,
	""
};

resource 'DITL' (136, "Really Delete") {
	{	/* array DITLarray: 5 elements */
		/* [1] */
		{106, 288, 126, 348},
		Button {
			enabled,
			"Delete"
		},
		/* [2] */
		{106, 215, 126, 275},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{13, 23, 45, 55},
		Icon {
			disabled,
			0
		},
		/* [4] */
		{13, 78, 93, 348},
		StaticText {
			disabled,
			"^0"
		},
		/* [5] */
		{0, 0, 0, 0},
		UserItem {
			disabled
		},
		/* [6] - Balloon help. */
		{0, 0, 0, 0},
		HelpItem {
			disabled,
			HMScanhdlg {136}
		},
	}
};

/* Site Dialog. */

resource 'DLOG' (137, "Site") {
	{0, 0, 210, 475},
	2053,
	invisible,
	noGoAway,
	0x0,
	137,
	""
};

resource 'DITL' (137, "Site") {
	{	/* array DITLarray: 5 elements */
		/* [1] */
		{177, 402, 197, 462},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{177, 329, 197, 389},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{135, 100, 151, 375},
		EditText {
			disabled,
			""
		},
		/* [4] */
		{13, 13, 77, 462},
		StaticText {
			disabled,
			"^0"
		},
		/* [5] */
		{98, 13, 114, 462},
		UserItem {
			enabled
		},
		/* [6] - Balloon help. */
		{0, 0, 0, 0},
		HelpItem {
			disabled,
			HMScanhdlg {137}
		},
	}
};

/* Out of memory alert. */

resource 'ALRT' (138, preload) {
	{0, 0, 69, 293},
	138,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (138, preload) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{36, 220, 56, 280},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{7, 74, 23, 280},
		StaticText {
			disabled,
			"Out of memory."
		}
	}
};

/* Cursors. */

resource 'CURS' (128, "BeachBall 1", preload) {
	$"07C0 1F30 3F08 7F04 7F04 FF02 FF02 FFFE"
	$"81FE 81FE 41FC 41FC 21F8 19F0 07C0",
	$"07C0 1FF0 3FF8 7FFC 7FFC FFFE FFFE FFFE"
	$"FFFE FFFE 7FFC 7FFC 3FF8 1FF0 07C0",
	{7, 7}
};

resource 'CURS' (129, "BeachBall 2", preload) {
	$"07C0 1FF0 3FF8 5FF4 4FE4 87C2 8382 8102"
	$"8382 87C2 4FE4 5FF4 3FF8 1FF0 07C0",
	$"07C0 1FF0 3FF8 7FFC 7FFC FFFE FFFE FFFE"
	$"FFFE FFFE 7FFC 7FFC 3FF8 1FF0 07C0",
	{7, 7}
};

resource 'CURS' (130, "BeachBall 3", preload) {
	$"07C0 19F0 21F8 41FC 41FC 81FE 81FE FFFE"
	$"FF02 FF02 7F04 7F04 3F08 1F30 07C0",
	$"07C0 1FF0 3FF8 7FFC 7FFC FFFE FFFE FFFE"
	$"FFFE FFFE 7FFC 7FFC 3FF8 1FF0 07C0",
	{7, 7}
};

resource 'CURS' (131, "BeachBall 4", preload) {
	$"07C0 1830 2008 701C 783C FC7E FEFE FFFE"
	$"FEFE FC7E 783C 701C 2008 1830 07C0",
	$"07C0 1FF0 3FF8 7FFC 7FFC FFFE FFFE FFFE"
	$"FFFE FFFE 7FFC 7FFC 3FF8 1FF0 07C0",
	{7, 7}
};

/* Bundle. */

resource 'BNDL' (128) {
	'PHED',
	0,
	{	/* array TypeArray: 2 elements */
		/* [1] */
		'FREF',
		{	/* array IDArray: 2 elements */
			/* [1] */
			0, 128,
			/* [2] */
			1, 129
		},
		/* [2] */
		'ICN#',
		{	/* array IDArray: 2 elements */
			/* [1] */
			0, 128,
			/* [2] */
			1, 129
		}
	}
};


type 'PHED' as 'STR ';

resource 'PHED' (0) {
	"Ph 1.1.1. Copyright Northwestern University 1991, 1992"
};

resource 'FREF' (128) {
	'APPL',
	0,
	""
};

resource 'FREF' (129) {
	'PHEP',
	1,
	""
};

resource 'ICON' (128) {
		$"0000 7800 000F 8E00 00F0 0B80 1F00 0540"
		$"600F E540 23FF F540 27FF 7540 277F 02A0"
		$"2773 02A0 1061 82A0 1073 C2A0 10FF E2B0"
		$"10FF F150 11FF F150 09FC F150 09E0 0150"
		$"0800 1FA8 0800 20A8 0C00 4068 0400 9828"
		$"0401 241E 040F FF8E 0401 180E 0200 800E"
		$"0200 400E 0200 300E 0201 FFFE 023E 0F1E"
		$"01C1 F078 009E 0F80 0061 F000 003E 0000",
};

resource 'ICN#' (128) {
	{	/* array: 2 elements */
		/* [1] */
		$"0000 7800 000F 8E00 00F0 0B80 1F00 0540"
		$"600F E540 23FF F540 27FF 7540 277F 02A0"
		$"2773 02A0 1061 82A0 1073 C2A0 10FF E2B0"
		$"10FF F150 11FF F150 09FC F150 09E0 0150"
		$"0800 1FA8 0800 20A8 0C00 4068 0400 9828"
		$"0401 241E 040F FF8E 0401 180E 0200 800E"
		$"0200 400E 0200 300E 0201 FFFE 023E 0F1E"
		$"01C1 F078 009E 0F80 0061 F000 003E",
		/* [2] */
		$"0000 7800 000F FE00 00FF FF80 1FFF FFC0"
		$"7FFF FFC0 3FFF FFC0 3FFF FFC0 3FFF FFE0"
		$"3FFF FFE0 1FFF FFE0 1FFF FFE0 1FFF FFF0"
		$"1FFF FFF0 1FFF FFF0 0FFF FFF0 0FFF FFF0"
		$"0FFF FFF0 0FFF FFF8 0FFF FFF8 07FF FFF8"
		$"07FF FFF8 07FF FFFE 07FF FFFE 03FF FFFE"
		$"03FF FFFE 03FF FFFE 03FF FFFE 03FF FFFE"
		$"03FF FFFE 01FF FFF8 00FF FF80 007F F0"
	}
};

resource 'ICN#' (129) {
	{	/* array: 2 elements */
		/* [1] */
		$"0000 7800 000F 8E00 00F0 0B80 1F00 0540"
		$"600F E540 23FF F540 27FF 7540 277F 02A0"
		$"2773 02A0 1061 82A0 1073 C2A0 10FF E2B0"
		$"10FF F150 11FF F150 09FC F150 09E0 0150"
		$"0800 0128 0800 00A8 0C00 00A8 0400 00A8"
		$"0400 00A8 0400 00A8 0400 0054 0200 0054"
		$"0200 00D4 0200 0F14 0201 F0F4 023E 0F04"
		$"01C1 F07C 009E 0F80 0061 F000 003E",
		/* [2] */
		$"0000 7800 000F FE00 00FF FF80 1FFF FFC0"
		$"7FFF FFC0 3FFF FFC0 3FFF FFC0 3FFF FFE0"
		$"3FFF FFE0 1FFF FFE0 1FFF FFE0 1FFF FFF0"
		$"1FFF FFF0 1FFF FFF0 0FFF FFF0 0FFF FFF0"
		$"0FFF FFF0 0FFF FFF8 0FFF FFF8 07FF FFF8"
		$"07FF FFF8 07FF FFF8 07FF FFF8 03FF FFFC"
		$"03FF FFFC 03FF FFFC 03FF FFFC 03FF FFFC"
		$"03FF FFFC 01FF FFFC 00FF FF80 007F F0"
	}
};

resource 'ics#' (128) {
	{	/* array: 2 elements */
		/* [1] */
		$"0060 03B0 1C18 E3D8 8F98 5F98 56CC 47EC"
		$"476C 261E 2023 20FB 2023 107C 13F8 1F80",
		/* [2] */
		$"0060 03E0 1FF0 FFF0 FFF0 7FF0 7FF8 7FF8"
		$"7FF8 3FFC 3FFF 3FFF 3FFF 1FFC 1F80 1C"
	}
};

resource 'ics#' (129) {
	{	/* array: 2 elements */
		/* [1] */
		$"0060 03B0 1C18 E3D8 8F98 5F98 56CC 47EC"
		$"476E 2606 2006 2003 200F 107C 13F0 1F80",
		/* [2] */
		$"0060 03F0 1FF0 FFF8 FFF8 7FF8 7FFC 7FFC"
		$"7FFC 3FFE 3FFE 3FFE 3FFC 1FF0 1F80 1C"
	}
};

resource 'ics8' (128) {
	$"0000 0000 0000 0000 00FF FF00 0000 0000"
	$"0000 0000 0000 FFFF FF00 FFFF 0000 0000"
	$"0000 00FF FFFF 0000 0000 00FF FF00 0000"
	$"FFFF FF00 0000 ECEC ECEC 00FF FF00 0000"
	$"FF00 0000 ECEC ECEC EC00 00FF FF00 0000"
	$"00FF 00EC ECEC ECEC EC00 00FF FF00 0000"
	$"00FF 00EC 00EC EC00 ECEC 0000 FFFF 0000"
	$"00FF 0000 00EC ECEC ECEC EC00 FFFF 0000"
	$"00FF 0000 00EC ECEC 00EC EC00 FFFF 0000"
	$"0000 FF00 00EC EC00 0000 00FF FFFF FF00"
	$"0000 FF00 0000 0000 0000 FF08 0808 FFFF"
	$"0000 FF00 0000 0000 FFFF FFFF FF08 FFFF"
	$"0000 FF00 0000 0000 0000 FF08 0808 FFFF"
	$"0000 00FF 0000 0000 00FF FFFF FFFF 0000"
	$"0000 00FF 0000 FFFF FFFF FFFF FF00 0000"
	$"0000 00FF FFFF FFFF FF"
};

resource 'ics8' (129) {
	$"0000 0000 0000 0000 00FF FF00 0000 0000"
	$"0000 0000 0000 FFFF FF00 FFFF 0000 0000"
	$"0000 00FF FFFF 0000 0000 00FF FF00 0000"
	$"FFFF FF00 0000 ECEC ECEC 00FF FF00 0000"
	$"FF00 0000 ECEC ECEC EC00 00FF FF00 0000"
	$"00FF 00EC ECEC ECEC EC00 00FF FF00 0000"
	$"00FF 00EC 00EC EC00 ECEC 0000 FFFF 0000"
	$"00FF 0000 00EC ECEC ECEC EC00 FFFF 0000"
	$"00FF 0000 00EC ECEC 00EC EC00 FFFF FF00"
	$"0000 FF00 00EC EC00 0000 0000 00FF FF00"
	$"0000 FF00 0000 0000 0000 0000 00FF FF00"
	$"0000 FF00 0000 0000 0000 0000 0000 FFFF"
	$"0000 FF00 0000 0000 0000 0000 FFFF FFFF"
	$"0000 00FF 0000 0000 00FF FFFF FFFF 0000"
	$"0000 00FF 0000 FFFF FFFF FFFF 0000 0000"
	$"0000 00FF FFFF FFFF FF"
};

resource 'ics4' (128) {
	$"0000 0000 0FF0 0000 0000 00FF F0FF 0000"
	$"000F FF00 000F F000 FFF0 0066 660F F000"
	$"F000 6666 600F F000 0F06 6666 600F F000"
	$"0F06 0660 6600 FF00 0F00 0666 6660 FF00"
	$"0F00 0666 0660 FF00 00F0 0660 000F FFF0"
	$"00F0 0000 00FC CCFF 00F0 0000 FFFF FCFF"
	$"00F0 0000 00FC CCFF 000F 0000 0FFF FF00"
	$"000F 00FF FFFF F000 000F FFFF F0"
};

resource 'ics4' (129) {
	$"0000 0000 0FF0 0000 0000 00FF F0FF 0000"
	$"000F FF00 000F F000 FFF0 0066 660F F000"
	$"F000 6666 600F F000 0F06 6666 600F F000"
	$"0F06 0660 6600 FF00 0F00 0666 6660 FF00"
	$"0F00 0666 0660 FFF0 00F0 0660 0000 0FF0"
	$"00F0 0000 0000 0FF0 00F0 0000 0000 00FF"
	$"00F0 0000 0000 FFFF 000F 0000 0FFF FF00"
	$"000F 00FF FFFF 0000 000F FFFF F0"
};

resource 'icl8' (128) {
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"00FF FFFF FF00 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 FFFF FFFF"
	$"FF00 0000 FFFF FF00 0000 0000 0000 0000"
	$"0000 0000 0000 0000 FFFF FFFF 0000 0000"
	$"0000 0000 FFF8 FFFF FF00 0000 0000 0000"
	$"0000 00FF FFFF FFFF 0000 0000 0000 0000"
	$"0000 0000 00FF F8FF F8FF 0000 0000 0000"
	$"00FF FF00 0000 0000 0000 0000 ECEC ECEC"
	$"ECEC EC00 00FF F8FF F8FF 0000 0000 0000"
	$"0000 FF00 0000 ECEC ECEC ECEC ECEC ECEC"
	$"ECEC ECEC 00FF F8FF F8FF 0000 0000 0000"
	$"0000 FF00 00EC ECEC ECEC ECEC ECEC ECEC"
	$"00EC ECEC 00FF F8FF F8FF 0000 0000 0000"
	$"0000 FF00 00EC ECEC 00EC ECEC ECEC ECEC"
	$"0000 0000 0000 FFF8 FFF8 FF00 0000 0000"
	$"0000 FF00 00EC ECEC 00EC ECEC 0000 ECEC"
	$"0000 0000 0000 FFF8 FFF8 FF00 0000 0000"
	$"0000 00FF 0000 0000 00EC EC00 0000 00EC"
	$"EC00 0000 0000 FFF8 FFF8 FF00 0000 0000"
	$"0000 00FF 0000 0000 00EC ECEC 0000 ECEC"
	$"ECEC 0000 0000 FFF8 FFF8 FF00 0000 0000"
	$"0000 00FF 0000 0000 ECEC ECEC ECEC ECEC"
	$"ECEC EC00 0000 FFF8 FFF8 FFFF 0000 0000"
	$"0000 00FF 0000 0000 ECEC ECEC ECEC ECEC"
	$"ECEC ECEC 0000 00FF F8FF F8FF 0000 0000"
	$"0000 00FF 0000 00EC ECEC ECEC ECEC ECEC"
	$"ECEC ECEC 0000 00FF F8FF F8FF 0000 0000"
	$"0000 0000 FF00 00EC ECEC ECEC ECEC 0000"
	$"ECEC ECEC 0000 00FF F8FF F8FF 0000 0000"
	$"0000 0000 FF00 00EC ECEC EC00 0000 0000"
	$"0000 0000 0000 00FF F8FF F8FF 0000 0000"
	$"0000 0000 FF00 0000 0000 0000 0000 0000"
	$"0000 00FF FFFF FFFF FFF8 FFF8 FF00 0000"
	$"0000 0000 FF00 0000 0000 0000 0000 0000"
	$"0000 FF08 0808 0808 FFF8 FFF8 FF00 0000"
	$"0000 0000 FFFF 0000 0000 0000 0000 0000"
	$"00FF 0808 0808 0808 08FF FFF8 FF00 0000"
	$"0000 0000 00FF 0000 0000 0000 0000 0000"
	$"FF08 08FF FF08 0808 0808 FFF8 FF00 0000"
	$"0000 0000 00FF 0000 0000 0000 0000 00FF"
	$"0808 FF00 00FF 0808 0808 08FF FFFF FF00"
	$"0000 0000 00FF 0000 0000 0000 FFFF FFFF"
	$"FFFF FFFF FFFF FFFF FF08 0808 FFFF FF00"
	$"0000 0000 00FF 0000 0000 0000 0000 00FF"
	$"0808 08FF FF08 0808 0808 0808 FFFF FF00"
	$"0000 0000 0000 FF00 0000 0000 0000 0000"
	$"FF08 0808 0808 0808 0808 0808 FFFF FF00"
	$"0000 0000 0000 FF00 0000 0000 0000 0000"
	$"00FF 0808 0808 0808 0808 0808 FFFF FF00"
	$"0000 0000 0000 FF00 0000 0000 0000 0000"
	$"0000 FFFF 0808 0808 0808 0808 FFFF FF00"
	$"0000 0000 0000 FF00 0000 0000 0000 00FF"
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FF00"
	$"0000 0000 0000 FF00 0000 FFFF FFFF FFF8"
	$"F8F8 F8F8 FFFF FFFF F8F8 F8FF FFFF FF00"
	$"0000 0000 0000 00FF FFFF F8F8 F8F8 F8FF"
	$"FFFF FFFF F8F8 F8F8 F8FF FFFF FF00 0000"
	$"0000 0000 0000 0000 FFF8 F8FF FFFF FFF8"
	$"F8F8 F8F8 FFFF FFFF FF00 0000 0000 0000"
	$"0000 0000 0000 0000 00FF FFF8 F8F8 F8FF"
	$"FFFF FFFF 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 FFFF FFFF FF"
};

resource 'icl8' (129) {
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"00FF FFFF FF00 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 FFFF FFFF"
	$"FF00 0000 FFFF FF00 0000 0000 0000 0000"
	$"0000 0000 0000 0000 FFFF FFFF 0000 0000"
	$"0000 0000 FFF8 FFFF FF00 0000 0000 0000"
	$"0000 00FF FFFF FFFF 0000 0000 0000 0000"
	$"0000 0000 00FF F8FF F8FF 0000 0000 0000"
	$"00FF FF00 0000 0000 0000 0000 ECEC ECEC"
	$"ECEC EC00 00FF F8FF F8FF 0000 0000 0000"
	$"0000 FF00 0000 ECEC ECEC ECEC ECEC ECEC"
	$"ECEC ECEC 00FF F8FF F8FF 0000 0000 0000"
	$"0000 FF00 00EC ECEC ECEC ECEC ECEC ECEC"
	$"00EC ECEC 00FF F8FF F8FF 0000 0000 0000"
	$"0000 FF00 00EC ECEC 00EC ECEC ECEC ECEC"
	$"0000 0000 0000 FFF8 FFF8 FF00 0000 0000"
	$"0000 FF00 00EC ECEC 00EC ECEC 0000 ECEC"
	$"0000 0000 0000 FFF8 FFF8 FF00 0000 0000"
	$"0000 00FF 0000 0000 00EC EC00 0000 00EC"
	$"EC00 0000 0000 FFF8 FFF8 FF00 0000 0000"
	$"0000 00FF 0000 0000 00EC ECEC 0000 ECEC"
	$"ECEC 0000 0000 FFF8 FFF8 FF00 0000 0000"
	$"0000 00FF 0000 0000 ECEC ECEC ECEC ECEC"
	$"ECEC EC00 0000 FFF8 FFF8 FFFF 0000 0000"
	$"0000 00FF 0000 0000 ECEC ECEC ECEC ECEC"
	$"ECEC ECEC 0000 00FF F8FF F8FF 0000 0000"
	$"0000 00FF 0000 00EC ECEC ECEC ECEC ECEC"
	$"ECEC ECEC 0000 00FF F8FF F8FF 0000 0000"
	$"0000 0000 FF00 00EC ECEC ECEC ECEC 0000"
	$"ECEC ECEC 0000 00FF F8FF F8FF 0000 0000"
	$"0000 0000 FF00 00EC ECEC EC00 0000 0000"
	$"0000 0000 0000 00FF F8FF F8FF 0000 0000"
	$"0000 0000 FF00 0000 0000 0000 0000 0000"
	$"0000 0000 0000 00FF F8F8 FFF8 FF00 0000"
	$"0000 0000 FF00 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 FFF8 FFF8 FF00 0000"
	$"0000 0000 FFFF 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 FFF8 FFF8 FF00 0000"
	$"0000 0000 00FF 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 FFF8 FFF8 FF00 0000"
	$"0000 0000 00FF 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 FFF8 FFF8 FF00 0000"
	$"0000 0000 00FF 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 FFF8 FFF8 FF00 0000"
	$"0000 0000 00FF 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 00FF F8FF F8FF 0000"
	$"0000 0000 0000 FF00 0000 0000 0000 0000"
	$"0000 0000 0000 0000 00FF F8FF F8FF 0000"
	$"0000 0000 0000 FF00 0000 0000 0000 0000"
	$"0000 0000 0000 0000 FFFF F8FF F8FF 0000"
	$"0000 0000 0000 FF00 0000 0000 0000 0000"
	$"0000 0000 FFFF FFFF F8F8 F8FF F8FF 0000"
	$"0000 0000 0000 FF00 0000 0000 0000 00FF"
	$"FFFF FFFF F8F8 F8F8 FFFF FFFF F8FF 0000"
	$"0000 0000 0000 FF00 0000 FFFF FFFF FFF8"
	$"F8F8 F8F8 FFFF FFFF F8F8 F8F8 F8FF 0000"
	$"0000 0000 0000 00FF FFFF F8F8 F8F8 F8FF"
	$"FFFF FFFF F8F8 F8F8 F8FF FFFF FFFF 0000"
	$"0000 0000 0000 0000 FFF8 F8FF FFFF FFF8"
	$"F8F8 F8F8 FFFF FFFF FF00 0000 0000 0000"
	$"0000 0000 0000 0000 00FF FFF8 F8F8 F8FF"
	$"FFFF FFFF 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 FFFF FFFF FF"
};

resource 'icl4' (128) {
	$"0000 0000 0000 0000 0FFF F000 0000 0000"
	$"0000 0000 0000 FFFF F000 FFF0 0000 0000"
	$"0000 0000 FFFF 0000 0000 FDFF F000 0000"
	$"000F FFFF 0000 0000 0000 0FDF DF00 0000"
	$"0FF0 0000 0000 6666 6660 0FDF DF00 0000"
	$"00F0 0066 6666 6666 6666 0FDF DF00 0000"
	$"00F0 0666 6666 6666 0666 0FDF DF00 0000"
	$"00F0 0666 0666 6666 0000 00FD FDF0 0000"
	$"00F0 0666 0666 0066 0000 00FD FDF0 0000"
	$"000F 0000 0660 0006 6000 00FD FDF0 0000"
	$"000F 0000 0666 0066 6600 00FD FDF0 0000"
	$"000F 0000 6666 6666 6660 00FD FDFF 0000"
	$"000F 0000 6666 6666 6666 000F DFDF 0000"
	$"000F 0006 6666 6666 6666 000F DFDF 0000"
	$"0000 F006 6666 6600 6666 000F DFDF 0000"
	$"0000 F006 6660 0000 0000 000F DFDF 0000"
	$"0000 F000 0000 0000 000F FFFF FDFD F000"
	$"0000 F000 0000 0000 00FC CCCC FDFD F000"
	$"0000 FF00 0000 0000 0FCC CCCC CFFD F000"
	$"0000 0F00 0000 0000 FCCF FCCC CCFD F000"
	$"0000 0F00 0000 000F CCF0 0FCC CCCF FFF0"
	$"0000 0F00 0000 FFFF FFFF FFFF FCCC FFF0"
	$"0000 0F00 0000 000F CCCF FCCC CCCC FFF0"
	$"0000 00F0 0000 0000 FCCC CCCC CCCC FFF0"
	$"0000 00F0 0000 0000 0FCC CCCC CCCC FFF0"
	$"0000 00F0 0000 0000 00FF CCCC CCCC FFF0"
	$"0000 00F0 0000 000F FFFF FFFF FFFF FFF0"
	$"0000 00F0 00FF FFFD DDDD FFFF DDDF FFF0"
	$"0000 000F FFDD DDDF FFFF DDDD DFFF F000"
	$"0000 0000 FDDF FFFD DDDD FFFF F000 0000"
	$"0000 0000 0FFD DDDF FFFF 0000 0000 0000"
	$"0000 0000 00FF FFF0"
};

resource 'icl4' (129) {
	$"0000 0000 0000 0000 0FFF F000 0000 0000"
	$"0000 0000 0000 FFFF F000 FFF0 0000 0000"
	$"0000 0000 FFFF 0000 0000 FDFF F000 0000"
	$"000F FFFF 0000 0000 0000 0FDF DF00 0000"
	$"0FF0 0000 0000 6666 6660 0FDF DF00 0000"
	$"00F0 0066 6666 6666 6666 0FDF DF00 0000"
	$"00F0 0666 6666 6666 0666 0FDF DF00 0000"
	$"00F0 0666 0666 6666 0000 00FD FDF0 0000"
	$"00F0 0666 0666 0066 0000 00FD FDF0 0000"
	$"000F 0000 0660 0006 6000 00FD FDF0 0000"
	$"000F 0000 0666 0066 6600 00FD FDF0 0000"
	$"000F 0000 6666 6666 6660 00FD FDFF 0000"
	$"000F 0000 6666 6666 6666 000F DFDF 0000"
	$"000F 0006 6666 6666 6666 000F DFDF 0000"
	$"0000 F006 6666 6600 6666 000F DFDF 0000"
	$"0000 F006 6660 0000 0000 000F DFDF 0000"
	$"0000 F000 0000 0000 0000 000F DDFD F000"
	$"0000 F000 0000 0000 0000 0000 FDFD F000"
	$"0000 FF00 0000 0000 0000 0000 FDFD F000"
	$"0000 0F00 0000 0000 0000 0000 FDFD F000"
	$"0000 0F00 0000 0000 0000 0000 FDFD F000"
	$"0000 0F00 0000 0000 0000 0000 FDFD F000"
	$"0000 0F00 0000 0000 0000 0000 0FDF DF00"
	$"0000 00F0 0000 0000 0000 0000 0FDF DF00"
	$"0000 00F0 0000 0000 0000 0000 FFDF DF00"
	$"0000 00F0 0000 0000 0000 FFFF DDDF DF00"
	$"0000 00F0 0000 000F FFFF DDDD FFFF DF00"
	$"0000 00F0 00FF FFFD DDDD FFFF DDDD DF00"
	$"0000 000F FFDD DDDF FFFF DDDD DFFF FF00"
	$"0000 0000 FDDF FFFD DDDD FFFF F000 0000"
	$"0000 0000 0FFD DDDF FFFF 0000 0000 0000"
	$"0000 0000 00FF FFF0"
};

resource 'SICN' (128) {
	{	/* array: 1 element */
		/* [1] */
		$"0060 03B0 1C18 E3D8 8F98 5F98 56CC 47EC"
		$"476C 261E 2023 20FB 2023 107C 13F8 1F80",
	}
};

/* Version. */

resource 'vers' (1) {
	0x01, 0x11, release, 0x00, verUS,      /* 1.1.1 */
	"1.1.1",
	"1.1.1, Copyright Northwestern University 1991, 1992"
};

resource 'vers' (2) {
	0x01, 0x11, release, 0x00, verUS,      /* 1.1.1 */
	"1.1.1",
	"Ph 1.1.1"
};

/* Size. */

resource 'SIZE' (-1) {
	reserved,
	acceptSuspendResumeEvents,
	reserved,
	canBackground,
	doesActivateOnFGSwitch,
	backgroundAndForeground,
	dontGetFrontClicks,
	ignoreAppDiedEvents,
	is32BitCompatible,
	isHighLevelEventAware,
	onlyLocalHLEvents,
	notStationeryAware,
	dontUseTextEditServices,
	reserved,
	reserved,
	reserved,
	400*1024,
	400*1024,
};
