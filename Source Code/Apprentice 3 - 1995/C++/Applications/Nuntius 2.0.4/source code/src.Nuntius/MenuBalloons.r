// Copyright � 1993-1995 Peter Speck (speck@dat.ruc.dk).  All rights reserved.
// MenuBalloons.r

#ifndef __BALLOONTYPES.R__
#include "BalloonTypes.r"
#endif

resource 'hmnu' (mApple, "mApple", purgeable) {
	HelpMgrVersion, hmDefaultOptions, 0, 0, HMSkipItem { }, {
		HMStringItem {
			"Displayes information about Nuntius and its lack of shareware fee.",
			"The about box can not be displayed at this time", "", ""
		},
}};
//-------------------
resource 'hmnu' (mFile, "mFile", purgeable) {
	HelpMgrVersion, hmDefaultOptions, 0, 0, HMSkipItem { }, {
		HMStringItem {
			"File menu.\n\nUse this menu to open, close, save, and print Nuntius documents, "
				"and to quit Nuntius.",
			"", "", ""
		},	
		HMStringItem {
			"Creates a new empty list for groups. You can drag groups you often read "
				"to this list.",
			"Not available at this time.", "", ""
		},
		HMStringItem {
			"Displays a dialog box that allows you to select an existing group list "
				"document to open.",
			"", "", ""
		},
		HMSkipItem {
		},
		HMStringItem {
			"Opens the list of all groups.", "", "", ""
		},
		HMSkipItem {
		},
		HMStringItem {
			"Closes the active window.",
			"Closes the active window.  Not available at this time because there is no "
				"active window, or it is not closeable.",
			"", ""
		},
		HMStringItem {
			"Saves the active document.",
			"Saves the active document. Not available at this time because there is no active "
				"document, or it is not changed.", "", ""
		},
		HMStringItem {
			"Displays a dialog box in which you can assign a name to the document and indicate "
				"where to save it.",
			"Saves the active document. Not available at this time because there is no active "
				"document, or it is not changed.", "", ""
		},
		HMStringItem {
			"Displays a dialog box in which you can assign a name to the copy of the document "
				"and indicate where to save the copy.",
			"Saves a copy of the active document. Not available at this time because there "
				"is no active document.", "", ""
		},
		HMStringItem {
			"Reverts the document to the lastest version saved.",
			"Reverts the document to the lastest version saved. Not available at this time "
				"because there is no active document, or it is not changed.", "", ""
		},
		HMSkipItem {
		},
		//++++
//		HMCompareItem {
//			"Extract binaries",
			HMStringItem {
				"Extract binaries.\n"
					"\n"
					"In list of threads:\n"
						"If you have selected multiple threads, they will be extracted as one "
						"document.\n"
//						"Hold down the option key to extract them as separate documents.\n"
					"\n"
					"In list of articles:\n"
						"Hold down the option key to extract binaries from selected articles only.\n"
				"",
				"Extract binaries. Not available as no thread or article are selected.",
				"", ""
			},
//		},
//		HMStringItem {
//			"Extract binaries. If you have selected multiple threads, "
//				"they will be extracted as multiple documents.",
//			"", "", ""
//		},
		//----
		HMSkipItem {
		},
		HMStringItem {
			"Displays a dialog box in which you can select paper size, orientation, "
				"and other printing options.",
			"Not available because there is no active window, or it is not printable.",
			"", ""
		},
		HMStringItem {
			"Prints the contents of the active window.",
			"Prints the contents of the active window.  Not available at this time "
				"because printing is not supported for the active window, "
				"or there is no active window.", "", ""
		},
		HMSkipItem {
		},
		HMStringItem {
			"Quits the Nuntius application.  If you have not saved changes you made "
				"to an open document, you will be asked whether you want to save changes.",
			"Nuntius is not able to quit now. Hit \"Cancel\" in the progress window.", "", ""
		},
}};
//-------------------
resource 'hmnu' (mGroupLists, "mGroupLists", purgeable) {
	HelpMgrVersion, hmDefaultOptions, 0, 0, 
		HMStringItem {
			"Opens this group list. The group list is shown here because it is located in "
				"the same folder as the \"Nuntius Preferences\" document.",
			"Groups lists located in the same folder as the \"Nuntius Preferences\" document "
				"are shown here for easy access.",
			"",
			""
		},
		{
}};
//-------------------
resource 'hmnu' (mEdit, "mEdit", purgeable) {
	HelpMgrVersion, hmDefaultOptions, 0, 0, HMSkipItem { }, {
		HMStringItem {
			"Edit menu.\n\nUse this menu to manipulate text, to select the entire "
				"contents of a document, and to show what's on the Clipboard.",
			"", "", ""
		},	
		HMStringItem {
			"Undoes your last action.",
			"Undoes your last action. Not available now because your last action "
				"cannot be undone.", "", ""
		},
		HMSkipItem {
		},
		HMStringItem {
			"Removes the selected text and places it temporarily into a storage "
				"area called the Clipboard.",
			"Removes the selected text and places it temporarily into a storage "
				"area called the Clipboard. Not available now because nothing is selected.",
				"", ""
		},
		HMStringItem {
			"Copies the selected text.  The original selection remains where it is. "
				"The copy is placed temporarily into a storage area called the Clipboard.\n",
			"Copies the selected text.  The original selection remains where it is. "
				"The copy is placed temporarily into a storage area called the Clipboard. "
				"The command is not available now because nothing is selected.", "", ""
		},
		HMStringItem {
			"Copies the selected text, and inserts \">\" at start of each line to indicate it's a quote. "
			  "The original selection remains where it is. "
				"The copy is placed temporarily into a storage area called the Clipboard.\n",
			"Copies the selected text.  The original selection remains where it is. "
				"The copy is placed temporarily into a storage area called the Clipboard. "
				"The command is not available now because nothing is selected.", "", ""
		},
		HMStringItem {
			"Inserts the contents of the Clipboard at the location of the insertion point.",
			"Inserts the contents of the Clipboard at the location of the insertion point. "
				"Not available because there is nothing on the Clipboard or because the contents "
				"of the Clipboard cannot be inserted here", "", ""
		},
		HMStringItem {
			"Removes the selected text or graphics without storing it on the Clipboard.",
			"Removes the selected text or graphics without storing it on the Clipboard. "
				"Not available now because nothing is selected.", "", ""
		},
		HMSkipItem {
		},
		HMStringItem {
			"Selects the entire contents of the open document.",
			"Selects the entire contents of the open document. "
				"Not available now because no document is open or because the open "
				"document is empty.", "", ""
		},
		HMSkipItem {
		},
		HMCompareItem {
			"Show Clipboard",
			HMStringItem {
				"Displays the contents of the Clipboard (a storage area for the last "
					"item cut or copied).",
				"", "", ""
			},
		},
		HMCompareItem {
			"Hide Clipboard",
			HMStringItem {
				"Closes the Clipboard window.",
				"", "", ""
			},
		},
		HMSkipItem {
		},
}};
//-------------------
resource 'hmnu' (mDiscussions, "mDiscussions", purgeable) {
	HelpMgrVersion, hmDefaultOptions, 0, 0, HMSkipItem { }, {
		HMStringItem {
			"Threads menu.\n\nUse this menu to manipulate threads: "
				"specify which threads to show, and "
				"to change the status of threads.",
			"Threads menu.\n\nUse this menu to manipulate threads: "
				"specify which threads to show, and "
				"to change the status of threads. "
				"Not available now as the front most window is not a list of threads.",
			 "", ""
		},	
		HMStringItem {
			"Creates a new for for use when you want to start a new discussion.",
			"Creates a new thread. Not available now, as the front most window "
				"is not a list of threads", "", ""
		},
		HMSkipItem {
		},
}};
//-------------------
resource 'hmnu' (mArticles, "mArticles", purgeable) {
	HelpMgrVersion, hmDefaultOptions, 0, 0, HMSkipItem { }, {
		HMStringItem {
			"Articles menu.\n\nUse this menu to manipulate articles: "
				"specify which articles to show, and "
				"to post follow-up articles.",
			"Articles menu.\n\nUse this menu to manipulate articles: "
				"specify which articles to show, and "
				"to post follow-up articles."
				"Not available now as the front most window is not a list of articles.",
			"", ""
		},	
		HMStringItem {
			"Prepares and posts a new article as a response to existing articles.",
			"Posts a new articles. Not available now, as the front most window "
				"is not a list of articles, or you have not selected an article "
				"(by clicking in its text).",
			"", ""
		},
		HMStringItem {
			"Creates a new letter (in Eudora) to the author of the selected articles.",
			"Creates a new letter. Not available now, as the front most window "
				"is not a list of articles, or you have not selected an article "
				"(by clicking in its text).", 
			"", ""
		},
		HMSkipItem {
		},
		HMSkipItem {
		},
		HMSkipItem {
		},
		HMSkipItem {
		},
		HMSkipItem {
		},
		HMSkipItem {
		},
		HMSkipItem {
		},
		HMSkipItem {
		},
		HMSkipItem {
		},
		HMStringItem {
			"Shows the headers. Not very interesting mostly.",
			"Shows the headers. Not available now, as the front most window "
				"is not a list of articles.", "", ""
		},
}};
//-------------------
resource 'hmnu' (mPreferences, "mPreferences", purgeable) {
	HelpMgrVersion, hmDefaultOptions, 0, 0, HMSkipItem { }, {
		HMStringItem {
			"Preferences menu.\n\nUse this menu to specify your name etc, "
				"font, options for extracting binaries, editing articles for posting.",
			"", "", ""
		},	
		HMSkipItem {
		},
		HMSkipItem {
		},
		HMSkipItem {
		},
		HMStringItem {
			"Choose this to specify your full name, email address, and name of your "
				"organization.",
			"", "", ""
		},
		HMStringItem {
			"Choose this to specify various options for extracting binaries.",
			"", "", ""
		},
		HMStringItem {
			"Choose this to specify various options for editing and posting articles, eg "
				"which word processor to use, folder for articles, and signature file.",
			"", "", ""
		},
		HMStringItem {
			"Choose this to specify various options for your mail application.",
			"", "", ""
		},
		HMStringItem {
			"Choose this to specify various options for the news server.",
			"", "", ""
		},
		HMStringItem {
			"Choose this to specify misc options.",
			"", "", ""
		},
		HMStringItem {
			"Choose this to specify options for how to manage the database of articles.",
			"", "", ""
		},
		HMSkipItem {
		},
		HMStringItem {
			"Choose this to update the list of all groups. If new groups have been created, "
				"they will be shown in a new group list.",
			"", "", ""
		},
		HMStringItem {
			"Choose this to rebuild the list of all groups from scratch.",
			"", "", ""
		},
		HMStringItem {
			"Choose this to check for new articles in the groups in your list.",
			"Checks for new articles in a list of group. Not available as the active"
				"window is not a list of groups.",
			"", ""
		},
		HMStringItem {
			"Choose this to make Nuntius very forgetfull about your password.",
			"Gets Nuntius to forget your password. Not available, "
				"as Nuntius doesn't know your password.\n"
				"Nuntius will ask for your password when (if) needed.",
			"", ""
		},
}};
