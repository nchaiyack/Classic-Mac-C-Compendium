/*_____________________________________________________________________

	balloons.r	-  Balloon Help Resources for the Ph Application
 _____________________________________________________________________*/


#include "Types.r"
#include "SysTypes.r"
#include "BalloonTypes.r"

resource 'hfdr' (-5696, "Application Icon Help", purgeable) {
	HelpMgrVersion, hmDefaultOptions, 0, 0,	/* header info */
	{
		HMStringResItem {1000, 48},
	},
};

resource 'hmnu' (1, "Apple Menu Help", purgeable) {
	HelpMgrVersion, hmDefaultOptions, 0, 0,	/* header info */
	HMSkipItem {},
	{
		HMStringResItem {		/* Menu title */
			0, 0, 1000, 26, 0, 0, 0, 0 },
		HMStringResItem {		/* About Ph */
			1000, 1, 0, 0, 0, 0, 0, 0 },
	},
};

resource 'hmnu' (2, "File Menu Help", purgeable) {
	HelpMgrVersion, hmDefaultOptions, 0, 0,	/* header info */
	HMSkipItem {},
	{
		HMStringResItem {		/* Menu title */
			1000, 2, 0, 0, 1000, 26, 0, 0 },
		HMStringResItem {		/* New Query Window */
			1000, 3, 1000, 15, 0, 0, 0, 0 },
		HMStringResItem {		/* New Ph Record */
			1000, 4, 1000, 16, 0, 0, 0, 0 },
		HMStringResItem {		/* Open Ph Record */
			1000, 5, 1000, 17, 0, 0, 0, 0 },
		HMStringResItem {		/* Close */
			1000, 6, 1000, 18, 0, 0, 0, 0 },
		HMStringResItem {		/* Save Ph Record */
			1000, 7, 1000, 19, 0, 0, 0, 0 },
		HMSkipItem {},			/* separator */
		HMStringResItem {		/* Login */
			1000, 8, 1000, 20, 0, 0, 0, 0 },
		HMStringResItem {		/* Logout */
			1000, 9, 1000, 21, 0, 0, 0, 0 },
		HMSkipItem {},			/* separator */
		HMStringResItem {		/* Change Password */
			1000, 10, 1000, 22, 0, 0, 0, 0 },
		HMStringResItem {		/* Delete Ph Record */
			1000, 11, 1000, 23, 0, 0, 0, 0 },
		HMStringResItem {		/* Update Site List */
			1000, 12, 1000, 24, 0, 0, 0, 0 },
		HMStringResItem {		/* Change Default Server */
			1000, 13, 1000, 25, 0, 0, 0, 0 },
		HMSkipItem {},			/* separator */
		HMStringResItem {		/* Quit */
			1000, 14, 0, 0, 0, 0, 0, 0 },
	},
};

resource 'hmnu' (3, "Edit Menu Help", purgeable) {
	HelpMgrVersion, hmDefaultOptions, 0, 0,	/* header info */
	HMSkipItem {},
	{
		HMStringResItem {		/* Menu title */
			1000, 27, 1000, 34, 1000, 26, 0, 0 },
		HMStringResItem {		/* Undo */
			1000, 28, 1000, 35, 0, 0, 0, 0 },
		HMSkipItem {},			/* separator */
		HMStringResItem {		/* Cut */
			1000, 29, 1000, 36, 0, 0, 0, 0 },
		HMStringResItem {		/* Copy */
			1000, 30, 0, 0, 0, 0, 0, 0 },
		HMStringResItem {		/* Paste */
			1000, 31, 1000, 37, 0, 0, 0, 0 },
		HMStringResItem {		/* Clear */
			1000, 32, 1000, 38, 0, 0, 0, 0 },
		HMSkipItem {},			/* separator */
		HMStringResItem {		/* Select All */
			1000, 33, 0, 0, 0, 0, 0, 0 },
	},
};

resource 'hmnu' (4, "Proxy Menu Help", purgeable) {
	HelpMgrVersion, hmDefaultOptions, 0, 0,	/* header info */
	HMStringResItem {		/* Missing items */
		1000, 41, 0, 0, 0, 0, 0, 0 },
	{
		HMStringResItem {		/* Menu title */
			1000, 39, 1000, 40, 1000, 26, 0, 0 },
	},
};

resource 'hmnu' (5, "Windows Menu Help", purgeable) {
	HelpMgrVersion, hmDefaultOptions, 0, 0,	/* header info */
	HMStringResItem {		/* Missing items */
		1000, 44, 0, 0, 1000, 45, 0, 0 },
	{
		HMStringResItem {		/* Menu title */
			1000, 42, 1000, 43, 1000, 26, 0, 0 },
	},
};

resource 'hmnu' (kHMHelpMenuID, "Help Menu Help", purgeable) {
	HelpMgrVersion, hmDefaultOptions, 0, 0,	/* header info */
	HMStringResItem {		/* Missing items */
		1000, 46, 1000, 49, 0, 0, 0, 0 },
	{
		HMStringResItem {		/* Change Help Server */
			1000, 47, 1000, 50, 0, 0, 0, 0},
	},
};

resource 'hdlg' (128, "About Box Help", purgeable) {
	HelpMgrVersion, 0, hmDefaultOptions, 0, 0,	/* header info */
	HMSkipItem {},
	{
		HMStringResItem {		/* 1 - icon */
			{0, 0}, {0, 0, 0, 0},
			1000, 51, 0, 0, 0, 0, 0, 0},
		HMStringResItem {		/* 2 - status info */
			{16, 100}, {0, 0, 0, 0},
			1000, 52, 0, 0, 0, 0, 0, 0},
	},
};

resource 'hdlg' (129, "Login Dialog Help", purgeable) {
	HelpMgrVersion, 0, hmDefaultOptions, 0, 0,	/* header info */
	HMSkipItem {},
	{
		HMStringResItem {		/* 1 - Login button */
			{0, 0}, {0, 0, 0, 0},
			1000, 53, 0, 0, 0, 0, 0, 0},
		HMStringResItem {		/* 2 - Cancel button */
			{0, 0}, {0, 0, 0, 0},
			1000, 54, 0, 0, 0, 0, 0, 0},
		HMStringResItem {		/* 3 - Static text */
			{0, 0}, {0, 0, 0, 0},
			0, 0, 0, 0, 0, 0, 0, 0},
		HMStringResItem {		/* 4 - Alias or Name field */
			{0, 0}, {0, 0, 0, 0},
			0, 0, 1000, 55, 0, 0, 0, 0},
		HMStringResItem {		/* 5 - Static text */
			{0, 0}, {0, 0, 0, 0},
			0, 0, 0, 0, 0, 0, 0, 0},
		HMStringResItem {		/* 6 - Password field */
			{0, 0}, {0, 0, 0, 0},
			0, 0, 1000, 56, 0, 0, 0, 0},
		HMStringResItem {		/* 7 - Static text */
			{0, 0}, {0, 0, 0, 0},
			0, 0, 0, 0, 0, 0, 0, 0},
		HMStringResItem {		/* 8 - Ph server domain name */
			{0, 0}, {0, 0, 0, 0},
			0, 0, 1000, 57, 0, 0, 0, 0},
		HMStringResItem {		/* 9 - Ph server popup menu */
			{0, 0}, {0, 0, 0, 0},
			1000, 58, 0, 0, 0, 0, 0, 0},
	},
};

resource 'hdlg' (130, "Change Password Dialog Help", purgeable) {
	HelpMgrVersion, 0, hmDefaultOptions, 0, 0,	/* header info */
	HMSkipItem {},
	{
		HMStringResItem {		/* 1 - OK button */
			{0, 0}, {0, 0, 0, 0},
			1000, 59, 0, 0, 0, 0, 0, 0},
		HMStringResItem {		/* 2 - Cancel button */
			{0, 0}, {0, 0, 0, 0},
			1000, 60, 0, 0, 0, 0, 0, 0},
		HMStringResItem {		/* 3 - Static text */
			{0, 0}, {0, 0, 0, 0},
			0, 0, 0, 0, 0, 0, 0, 0},
		HMStringResItem {		/* 4 - First password field */
			{0, 0}, {0, 0, 0, 0},
			0, 0, 1000, 61, 0, 0, 0, 0},
		HMStringResItem {		/* 5 - Static text */
			{0, 0}, {0, 0, 0, 0},
			0, 0, 0, 0, 0, 0, 0, 0},
		HMStringResItem {		/* 6 - Second password field */
			{0, 0}, {0, 0, 0, 0},
			0, 0, 1000, 61, 0, 0, 0, 0},
		HMStringResItem {		/* 7 - Static text */
			{0, 0}, {0, 0, 0, 0},
			0, 0, 0, 0, 0, 0, 0, 0},
	},
};

resource 'hdlg' (131, "Server Wait Dialog Help", purgeable) {
	HelpMgrVersion, 0, hmDefaultOptions, 0, 0,	/* header info */
	HMSkipItem {},
	{
		HMStringResItem {		/* 1 - Abort button */
			{0, 0}, {0, 0, 0, 0},
			1000, 62, 0, 0, 0, 0, 0, 0},
	},
};

resource 'hdlg' (132, "Open Dialog Help", purgeable) {
	HelpMgrVersion, 0, hmDefaultOptions, 0, 0,	/* header info */
	HMSkipItem {},
	{
		HMStringResItem {		/* 1 - OK button */
			{0, 0}, {0, 0, 0, 0},
			1000, 63, 0, 0, 0, 0, 0, 0},
		HMStringResItem {		/* 2 - Cancel button */
			{0, 0}, {0, 0, 0, 0},
			1000, 64, 0, 0, 0, 0, 0, 0},
		HMStringResItem {		/* 3 - Static text */
			{0, 0}, {0, 0, 0, 0},
			0, 0, 0, 0, 0, 0, 0, 0},
		HMStringResItem {		/* 4 - Alias, Id or name field */
			{0, 0}, {0, 0, 0, 0},
			0, 0, 1000, 65, 0, 0, 0, 0},
	},
};

resource 'hdlg' (133, "Error Message Alert Help", purgeable) {
	HelpMgrVersion, 0, hmDefaultOptions, 0, 0,	/* header info */
	HMSkipItem {},
	{
		HMStringResItem {		/* 1 - OK button */
			{0, 0}, {0, 0, 0, 0},
			1000, 66, 0, 0, 0, 0, 0, 0},
	},
};

resource 'hdlg' (134, "Save Changes Dialog Help", purgeable) {
	HelpMgrVersion, 0, hmDefaultOptions, 0, 0,	/* header info */
	HMSkipItem {},
	{
		HMStringResItem {		/* 1 - Save button */
			{0, 0}, {0, 0, 0, 0},
			1000, 67, 0, 0, 0, 0, 0, 0},
		HMStringResItem {		/* 2 - Cancel button */
			{0, 0}, {0, 0, 0, 0},
			1000, 68, 0, 0, 0, 0, 0, 0},
		HMStringResItem {		/* 3 - Don't Save button */
			{0, 0}, {0, 0, 0, 0},
			1000, 69, 0, 0, 0, 0, 0, 0},
	},
};

resource 'hdlg' (135, "New Record Dialog Help", purgeable) {
	HelpMgrVersion, 0, hmDefaultOptions, 0, 0,	/* header info */
	HMSkipItem {},
	{
		HMStringResItem {		/* 1 - OK button */
			{0, 0}, {0, 0, 0, 0},
			1000, 70, 0, 0, 0, 0, 0, 0},
		HMStringResItem {		/* 2 - Cancel button */
			{0, 0}, {0, 0, 0, 0},
			1000, 71, 0, 0, 0, 0, 0, 0},
		HMStringResItem {		/* 3 - Static text */
			{0, 0}, {0, 0, 0, 0},
			0, 0, 0, 0, 0, 0, 0, 0},
		HMStringResItem {		/* 4 - Alias field */
			{0, 0}, {0, 0, 0, 0},
			0, 0, 1000, 72, 0, 0, 0, 0},
		HMStringResItem {		/* 5 - Static text */
			{0, 0}, {0, 0, 0, 0},
			0, 0, 0, 0, 0, 0, 0, 0},
		HMStringResItem {		/* 6 - Name field */
			{0, 0}, {0, 0, 0, 0},
			0, 0, 1000, 73, 0, 0, 0, 0},
		HMStringResItem {		/* 7 - Static text */
			{0, 0}, {0, 0, 0, 0},
			0, 0, 0, 0, 0, 0, 0, 0},
		HMStringResItem {		/* 8 - Type field */
			{0, 0}, {0, 0, 0, 0},
			0, 0, 1000, 74, 0, 0, 0, 0},
		HMStringResItem {		/* 9 - Static text */
			{0, 0}, {0, 0, 0, 0},
			0, 0, 0, 0, 0, 0, 0, 0},
		HMStringResItem {		/* 10 - First password field */
			{0, 0}, {0, 0, 0, 0},
			0, 0, 1000, 75, 0, 0, 0, 0},
		HMStringResItem {		/* 11 - Static text */
			{0, 0}, {0, 0, 0, 0},
			0, 0, 0, 0, 0, 0, 0, 0},
		HMStringResItem {		/* 12 - Second password field */
			{0, 0}, {0, 0, 0, 0},
			0, 0, 1000, 75, 0, 0, 0, 0},
	},
};

resource 'hdlg' (136, "Really Delete Dialog Help", purgeable) {
	HelpMgrVersion, 0, hmDefaultOptions, 0, 0,	/* header info */
	HMSkipItem {},
	{
		HMStringResItem {		/* 1 - Delete button */
			{0, 0}, {0, 0, 0, 0},
			1000, 76, 0, 0, 0, 0, 0, 0},
		HMStringResItem {		/* 2 - Cancel button */
			{0, 0}, {0, 0, 0, 0},
			1000, 77, 0, 0, 0, 0, 0, 0},
	},
};

resource 'hdlg' (137, "Site Dialog Help", purgeable) {
	HelpMgrVersion, 0, hmDefaultOptions, 0, 0,	/* header info */
	HMSkipItem {},
	{
		HMStringResItem {		/* 1 - OK button */
			{0, 0}, {0, 0, 0, 0},
			1000, 78, 0, 0, 0, 0, 0, 0},
		HMStringResItem {		/* 2 - Cancel button */
			{0, 0}, {0, 0, 0, 0},
			1000, 79, 0, 0, 0, 0, 0, 0},
		HMStringResItem {		/* 3 - Ph server domain name */
			{0, 0}, {0, 0, 0, 0},
			0, 0, 1000, 57, 0, 0, 0, 0},
		HMStringResItem {		/* 4 - Static text */
			{0, 0}, {0, 0, 0, 0},
			0, 0, 0, 0, 0, 0, 0, 0},
		HMStringResItem {		/* 5 - Ph server popup menu */
			{0, 0}, {0, 0, 0, 0},
			1000, 58, 0, 0, 0, 0, 0, 0},
	},
};

resource 'STR#' (1000, "Help Balloon Strings", purgeable) {
	{
/*  1*/	"Displays information about this version of the Ph application.",
/*  2*/	"File menu\n\nUse this menu to open and close windows, "
			"manipulate Ph records, quit Ph, and perform other miscellaneous "
			"tasks.",
/*  3*/	"Opens a new query window.",
/*  4*/	"Creates a new Ph record.",
/*  5*/	"Opens some other person’s Ph record for editing.",
/*  6*/	"Closes the active window.",
/*  7*/	"Saves the changes made to the active Ph record.",
/*  8*/	"Logs in to a Ph server and opens your Ph record for editing.",
/*  9*/	"Logs out of the Ph server and closes all editing windows.",
/* 10*/	"Changes the password for the active Ph record.",
/* 11*/	"Deletes the active Ph record.",
/* 12*/	"Updates the list of known Ph server sites.",
/* 13*/	"Changes the default Ph server.",
/* 14*/	"Quits the Ph program. If you have not saved changes you made "
			"to open Ph records, you will be asked whether you want to "
			"save changes.",
/* 15*/	"Opens a new query window. Not available now because there is "
			"a dialog box on your screen.",
/* 16*/	"Creates a new Ph record. Not available now because you are not "
			"logged in as a hero, or because there is a dialog box on your "
			"screen. Only "
			"heros (database administrators) are permitted to create new Ph "
			"records.",
/* 17*/	"Opens some other person’s Ph record for editing. Not "
			"available now because you are not logged in to a server, or "
			"because there is a dialog box on your screen.",
/* 18*/	"Closes the active window. Not available now because there are no "
			"Ph windows open on your screen, or because there is a dialog box on "
			"your screen.",
/* 19*/	"Saves the changes made to the active Ph record. Not available "
			"now because a Ph edit window is not active, or because you "
			"have not made any changes to the window, or because there is "
			"a dialog box on your screen.",
/* 20*/	"Logs in to a Ph server and opens your Ph record for editing. "
			"Not available now because there is a dialog box on your screen.",
/* 21*/	"Logs out of the Ph server and closes all editing windows. "
			"Not available now because you are not logged in to a server, or "
			"because there is a dialog box on your screen.",
/* 22*/	"Changes the password for the active Ph record. Not available now "
			"because a Ph edit window is not active, or because there is a "
			"dialog box on your screen.",
/* 23*/	"Deletes the active Ph record. Not available now because "
			"a Ph edit window is not active, or because you are not logged in "
			"as a hero, or because there is a dialog box on your screen. "
			"Only heros "
			"(database administrators) are permitted to delete Ph records.",
/* 24*/	"Updates the list of known Ph server sites. Not available now "
			"because there is a dialog box on your screen.",
/* 25*/	"Changes the default Ph server. Not available now because there "
			"is a dialog box on your screen.",
/* 26*/	"This menu is not available because it cannot be used with the "
			"dialog box on your screen.",
/* 27*/	"Edit menu\n\nUse this menu to manipulate text.",
/* 28*/	"Undoes your last action.",
/* 29*/	"Removes the selected text and places it temporarily into a storage "
			"area called the Clipboard.",
/* 30*/	"Copies the selected text. The original selection remains where it is. "
			"The copy is placed temporarily into a storage area called the "
			"Clipboard.",
/* 31*/	"Inserts the contents of the Clipboard at the location of the "
			"insertion point.",
/* 32*/	"Removes the selected text without storing it in the Clipboard.",
/* 33*/	"Selects the entire contents of the current field.",
/* 34*/	"Edit menu\n\nUse this menu to manipulate text. Not available now "
			"because there are no Ph windows open on your screen, or because "
			"it cannot be used with the dialog box on your screen.",
/* 35*/	"Undoes your last action. Not available now because Ph does not "
			"currently support the Undo command. This command is present only "
			"for use by desk accessories under System 6.",
/* 36*/	"Removes the selected text and places it temporarily into a storage "
			"area called the Clipboard. Not available now because you are not "
			"permitted to make changes to the current field.",
/* 37*/	"Inserts the contents of the Clipboard at the location of the "
			"insertion point. Not available now because you are not permitted to "
			"make changes to the current field.",
/* 38*/	"Removes the selected text without storing it in the Clipboard. Not "
			"available now because you are not permitted to make changes to the "
			"current field.",
/* 39*/	"Proxy menu\n\nUse this menu to open Ph records for people who have "
			"permitted you to edit their records.",
/* 40*/	"Proxy menu\n\nUse this menu to open Ph records for people who have "
			"permitted you to edit their records. Not available now because you "
			"are not logged in, or because nobody has granted you "
			"proxy permission, or because there is a dialog box on your screen.",
/* 41*/	"Opens the Ph record for the person with this alias. This person has "
			"granted you permission to edit his or her Ph record.",
/* 42*/	"Windows menu\n\nUse this menu to make Ph windows active (bring them "
			"to the front).",
/* 43*/	"Windows menu\n\nUse this menu to make Ph windows active (bring them "
			"to the front). Not available now because because there are no Ph "
			"windows open on your screen, or because it cannot be used with "
			"the dialog box on your screen.",
/* 44*/	"Activates (brings to the front) the Ph window with this name.",
/* 45*/	"Activates (brings to the front) the Ph window with this name. This "
			"window is already active, as indicated by the checkmark next to its "
			"name.",
/* 46*/	"Gets and displays Ph server help text for this topic.",
/* 47*/	"Changes the Ph help server and rebuilds the help topic list. This "
			"command can sometimes be useful to get information about some other "
			"site’s Ph system.",
/* 48*/	"Ph\n\nUse the Ph application to look up directory information. You "
			"can also use Ph to make changes to directory entries.",
/* 49*/	"Gets and displays Ph server help text for this topic. Not available "
			"now because there is a dialog box on your screen.",
/* 50*/	"Changes the Ph help server and rebuilds the help topic list. This "
			"command can sometimes be useful to get information about some other "
			"site’s Ph system. Not available now because there is a dialog box "
			"on your screen.",
/* 51*/	"We don’t need no stinkin’ X.500!",
/* 52*/	"This status message tells whether or not you are logged in "
			"to a server. If "
			"you are logged in, it gives the server name and login alias, and "
			"also tells if you are a hero.",
/* 53*/	"To log in using the values you specified, click this button.",
/* 54*/	"To cancel the login, click this button.",
/* 55*/	"Type your alias or your name in this field.",
/* 56*/	"Type your Ph password in this field. For security, the password is "
			"not displayed in clear text as you enter it, so type carefully!",
/* 57*/	"Type the domain name of the Ph server in this field, or use the "
			"popup menu above (if it’s present). It’s usually easier to use "
			"the popup menu.",
/* 58*/	"Select the Ph server from this popup menu, or type its domain name "
			"in the field below. It’s usually easier to use this popup menu.",
/* 59*/	"To change your password to the new value you specified, click "
			"this button.",
/* 60*/	"To cancel the change password operation, click this button.",
/* 61*/	"Type your new Ph password in this field. For security, the password "
			"is not displayed in clear text as you enter it, so type carefully! "
			"To protect against typing errors, you are asked to enter your new "
			"password twice.",
/* 62*/	"To abort the current transaction with the Ph server, click this button.",
/* 63*/	"To open the Ph record you specified, click this button.",
/* 64*/	"To cancel the open operation, click this button.",
/* 65*/	"Type the alias, id or name of the record you wish to open in this field.",
/* 66*/	"To close the alert box after reading the message, click this button.",
/* 67*/	"To send the changed fields to the Ph server, click this button.",
/* 68*/	"To cancel the current operation, click this button. The changed fields "
			"will not be sent to the Ph server, and the window containing the changed "
			"fields will not be closed.",
/* 69*/	"To continue the current operation without sending the changed fields "
			"to the Ph server, click this button. All changes you have made to fields "
			"in the Ph record will be lost.",
/* 70*/	"To create a new Ph record with the values you specified, click this "
			"button.",
/* 71*/	"To cancel the creation of a new Ph record, click this button.",
/* 72*/	"Type the alias of the new record in this field.",
/* 73*/	"Type the name of the new record in this field.",
/* 74*/	"Enter the record type in this field (e.g., “person,” etc.)",
/* 75*/	"Type the password for the new record in this field. For security, the "
			"password is not displayed in clear text as you enter it, so type "
			"carefully! To protect against typing errors, you are asked to enter the "
			"password twice.",
/* 76*/	"To delete the Ph record, click this button.",
/* 77*/	"To cancel the delete operation, click this button.",
/* 78*/	"To select the specified server, click this button.",
/* 79*/	"To cancel the server selection, click this button.",
/* 80*/	"Type the domain name of the Ph server you wish to query in this field, "
			"or use the popup menu to the left (if it’s present). It’s usually easier "
			"to use the popup menu.",
/* 81*/	"Type your query here. The simplest query is a person’s name. For example, "
			"type “John Doe” to get information about John Doe. It is not necessary to "
			"type the word “query” as part of your query.",
/* 82*/	"The result of your query, if any, is displayed here, along with any error "
			"messages which may be returned by the Ph server.",
/* 83*/	"Select the Ph server you wish to query from this popup menu, or type its "
			"domain name in the field to the right. It’s usually easier to use this "
			"popup menu.",
/* 84*/	"To send your query to the server, click this button.",
/* 85*/	"To send your query to the server, click this button. Not available now "
			"because you have not yet typed a query, or because you have not specified "
			"a Ph server.",
/* 86*/	"Scroll bar\n\nTo scroll a line at a time, press one of the "
			"scroll arrows. To scroll by the windowfull, click in the gray bar. "
			"To scroll to another part of the window, drag the scroll box.",
/* 87*/	"Scroll bar\n\nUse the scroll bar to see information that is out of "
			"view. This scroll bar is not available because this window "
			"contains no information above or below the window’s borders.",
/* 88*/	"Size box\n\nTo change the height of the window, drag the "
			"size box.",
/* 89*/	"The requested Ph server help text is displayed here.",
/* 90*/	"The Ph record is displayed and edited here.",
/* 91*/	"To show detailed information about each field, click this box.",
/* 92*/	"To hide the detailed field information, click this box.",
/* 93*/	"To restore the original value of the current field, click "
			"this button.",
/* 94*/	"To restore the original value of the current field, click "
			"this button. "
			"Not available now because you haven’t made any changes to the "
			"current field.",
/* 95*/	"To restore the original values of all the fields, click "
			"this button.",
/* 96*/	"To restore the original values of all the fields, click "
			"this button. "
			"Not available now because you haven’t changed any fields.",
	}
};