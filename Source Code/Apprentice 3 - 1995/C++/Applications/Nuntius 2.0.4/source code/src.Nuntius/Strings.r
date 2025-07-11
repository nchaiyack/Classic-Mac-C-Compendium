// Copyright � 1992-1995 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// Strings.r

// From {RIncludes}Types.r
type 'STR#' {
		integer = $$Countof(StringArray);
		array StringArray {
				pstring;										/* String				*/
		};
};
#include "RsrcGlobals.h"
#include "MenuGlobals.h"

/* ===============================   strings   ================================== */
resource 'STR#' (kGroupTreeStrings, "kGroupTreeStrings", purgeable) {{
	"All Groups";
}};

resource 'STR#' (kDiscListStatusStrings, "kDiscListStatusStrings", purgeable) {{
	"�";
	"�";
	"";
	"n.a.";
	"?";
}};

resource 'STR#' (kGroupListNAStrings, "kGroupListNAStrings", purgeable) {{
	"?";
	"";
	"�";
}};

resource 'STR#' (kArticleStatusStrings, "ArticleStatusStrings", purgeable) {{
	"You have read all the articles in this thread.";
	"You'll have to wait a bit for this article.";
	"Sorry, but the server does not have this article anymore.";
	"This article is not shown.";
	"This article could not be displayed due to an error.";
}};

resource 'STR#' (kArticleStrings, "ArticleStrings", purgeable) {{
	"(cannot get the subject)";
	"Subject: ";
	"\n";
	"From: ";
	"\n";
	"Date: ";
	"\n";
	"----------------------------------------------------------------------";
	", ";
	"In article �msgid� �realname�, �email� writes:"; // �msgid�, �realname�, �email�
	"In article �msgid�, you write:"; // �msgid�, �realname�, �email�
	"�realname�: �subject�";
	"--------------------------------------------------------------------------\n"
		"From �email� �mailboxdate�\n"
		"Excerpt from �realname�, �email� in �msgid�, " // �msgid�, �realname�, �email�, �date�
		"�date�:\n\n";
	"n.a.";
	"> �line�";
	"Page �pagenum� of �pagecount�";
}};

resource 'STR#' (kSaveArticlesStrings, "kSaveArticlesStrings", purgeable) {{
	"Save selected articles as";
	"Save binary extraction as";
	"Saving articles";
	"Extracting articles";
	" (part ";
	"---------------------------------------------------------------------------\n";
}};

resource 'STR#' (kNntpStrings, "kNntpStrings", purgeable) {{
	"Opening connection";
	"too many user";
}};

resource 'STR#' (kCheckServerGroupsStrings, "kCheckServerGroupsStrings", purgeable) {{
	"soc.motss";
	"comp.sys.mac.apps";
	"comp.sys.mac.programmer";
	"misc";
	"test";
	"news.admin";
	"news.future";
	"comp.unix.admin";
	"comp.unix.programmer";
	"control";
}};

resource 'STR#' (kNewServerStrings, "kNewServerStrings", purgeable) {{
	"Obtaining address of server";
	"Performing basic test of server";
}};

resource 'STR#' (kUpdGroupTreeStrings, "kUpdGroupTreeStrings", purgeable) {{
	"Fetching list of groups";
	"Building new list of all groups";
	"Updating list of all groups";
	"Saving list on disk";
	"Updating list of all groups";
	"New groups";
	"Fetching descriptions of groups";
	"Processing descriptions of groups";
	"Fetching list of new groups";
}};

resource 'STR#' (kPreferencesStrings, "kPreferencesStrings", purgeable) {{
	"Nuntius Preferences";
	"Nuntius Prefs �num�";
	"Nuntius public files";
	"Nuntius personal files";
	"Untitled document";
	"List of All Groups";
	"Obsolete Nuntius files";
	"Description of fatal error";
}};


resource 'STR#' (kGroupListDocStrings, "kGroupListDocStrings", purgeable) {{
	"Untitled group list <<<>>>";
	"Do you want to replace them with the groups you are moving?";
	"Do you want to replace them with the groups you are copying?";
}};

resource 'STR#' (kHelpMenuStrings, "kHelpMenuStrings", purgeable) {{
	"Frequently Asked Questions�";
	"Show Nuntius www page�";
	"Show Nuntius ftp homesite�";
}};

resource 'STR#' (kOpenGroupStrings, "kOpenGroupStrings", purgeable) {{
	"Reading group database from disk";
	"Checking for new articles";
	"Deleting inactive threads";
	"Updating database (estimate: �num� new articles)";
	"Building database (estimate: �num� articles)";
	"Saving updated database to disk";
}};

resource 'STR#' (kStartupStrings, "kStartupStrings", purgeable) {{
	"Starting up";
	"";
	"Reading preferences";
	"Initializes application";
	"Reading list of all groups";
}};

resource 'STR#' (kNewsServerStrings, "kNewsServerStrings", purgeable) {{
	"Name: ";
	"";
	"IP = ";
	"";
}};
resource 'STR#' (kPostArticleStrings, "kPostArticleStrings", purgeable) {{
	"Preparing article for posting";
	"Sending article to server";
	"Waiting for acknowledgement";
	"Sending cancel request";
	"Cancel request";
	"ISO 8859-1";
	"ISO 8859-1";
}};
resource 'STR#' (kPostArticleWeekDayNames, "kPostArticleWeekDayNames", purgeable) {{
	 "Sun"; "Mon"; "Tue"; "Wed"; "Thu"; "Fri"; "Sat"
}};
resource 'STR#' (kPostArticleMonthNames, "kPostArticleMonthNames", purgeable) {{
	"Jan"; "Feb"; "Mar"; "Apr"; "May"; "Jun";
	"Jul"; "Aug"; "Sep"; "Oct"; "Nov"; "Dec";
}};

resource 'STR#' (kMailStrings, "kMailStrings", purgeable) {{
	"Re: ";
	"";
	"Mail Script";
	"ReplyByMail";
	"URL script";
	"OpenURL";
}};

resource 'STR#' (kAskFileFolderStrings, "kAskFileFolderStrings", purgeable) {{
	"TeachText";
	"ttxt";
	"Select a folder:";
}};
	
resource 'STR#' (kMiscStrings, "kMiscStrings", purgeable) {{
	"�"; // password dot
	"no documents found";
	"Select \"";
	"\"";
	" �";
	"Close idle NNTP connections (�num�)";
	"68K";
	"PPC";
	"ftp://ftp.ruc.dk/pub/nuntius/";
	"http://guru.med.cornell.edu/~aaron/nuntius/nuntius.html";
	"Version �vers� ";
}};

resource 'STR#' (kFatalErrorStrings, "kFatalErrorStrings", preload) {{
	"Please send this error description to speck@dat.ruc.dk\nPlease don't expect any personal reply to the bug report\n\n";
	"\n"; //"\nDetails:\n\n";
	""; // End of description
	"Nuntius version: �version� (�cputype�), language = �language�";
	""; // version 2
	"US";
	"Mac model: �macmodel�";
	"System version: �sysversion�";
	"\nError:";
	_DATETIME_;
	"Compiled: "_DATESTR_"\n";
}};

resource 'STR#' (kEnglishShortWeekdayStrings, "kEnglishShortWeekdayStrings", purgeable) {{
	"Sun";	"Mon";	"Tue";	"Wed";	"Thu";	"Fri";	"Sat";
}};
resource 'STR#' (kEnglishShortMonthStrings, "kEnglishShortMonthStrings", purgeable) {{
	"Jan";	"Feb";	"Mar";	"Apr";	"May";	"Jun";
	"Jul";	"Aug";	"Sep";	"Oct";	"Nov";	"Dec";
}};

resource 'STR#' (kValidURLSchemeStrings, "kValidURLSchemeStrings", purgeable) {{
	"ftp://", "http://", "gopher:/", "mailto:", "news:", "nntp:", "telnet:/", "wais:/",
	"file://", "prospero:",
	// "about", NetScape special type, doesn't make sense in articles
 "bolo://"; // Suggested by Stuart Cheshire <cheshire@DSG.Stanford.EDU>
}};

resource 'STR#' (kSuffixMappings, "kSuffixMappings", purgeable) {{
	"GIFf", "JVWR", "gif",  "GIF",
	"JFIF", "JVWR", "jpg",  "JPEG",
	"JFIF", "JVWR", "jpeg", "JPEG", 
	"MPEG", "mMPG", "mpg",  "MPEG", 
	"MPEG", "mMPG", "mpeg", "MPEG", 
	"MooV", "TVOD", "qt",   "QuickTime movie",
	"MooV", "TVOD", "mov",  "QuickTime movie",
	"pZIP", "pZIP", "zip",  "ZIP (UnZip 2.0.1 by Maika)",
	"Gzip", "Gzip", "z",    "gzip (MacGzip by SPDsoft)", 
	"Gzip", "Gzip", "gz",   "gzip (MacGzip by SPDsoft)", 
	"GL  ", "AnVw", "gl",   "GL (MacAnim Viewer 1.0.5 by Martin Fong)",
	"DL  ", "AnVw", "dl",   "DL (MacAnim Viewer 1.0.5 by Martin Fong)",
	"BMPp", "GKON", "bmp",  "BMP (GraphicConverter by Thorsten Lemke)";
}};
