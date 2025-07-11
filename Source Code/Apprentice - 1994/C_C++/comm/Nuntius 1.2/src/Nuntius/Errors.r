// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// Errors.r

#ifndef __ERRORGLOBALS__
#include "ErrorGlobals.h"
#endif

#ifndef __MACTCPCOMMONTYPES__
/* MacTCP return Codes in the range -23000 through -23049 */
#	define ipBadLapErr						-23000			/* bad network configuration */
#	define ipBadCnfgErr						-23001			/* bad IP configuration error */
#	define ipNoCnfgErr						-23002			/* missing IP or LAP configuration error */
#	define ipLoadErr							-23003			/* error in MacTCP load */
#	define ipBadAddr							-23004			/* error in getting address */
#	define connectionClosing			-23005			/* connection is closing */
#	define invalidLength					-23006
#	define connectionExists				-23007			/* request conflicts with existing connection */
#	define connectionDoesntExist	-23008			/* connection does not exist */
#	define insufficientResources	-23009			/* insufficient resources to perform request */
#	define invalidStreamPtr				-23010
#	define streamAlreadyOpen			-23011
#	define connectionTerminated		-23012
#	define invalidBufPtr					-23013
#	define invalidRDS							-23014
#	define invalidWDS							-23014
#	define openFailed							-23015
#	define commandTimeout					-23016
#	define duplicateSocket				-23017
# define nameSyntaxErr		 			-23041		
# define cacheFault							-23042
# define noResultProc						-23043
# define noNameServer						-23044
# define authNameErr						-23045
# define noAnsErr								-23046
# define dnrErr									-23047
# define outOfMemory						-23048
#endif

// OPERATION table
resource 'errs' (1130, "operation table", purgeable) {{
	whichList, 0, kTNewsApplMessageStrings;
	messageOpenGroupFailedRez,				messageOpenGroupFailedRez, 1;
	messageGetGroupListFailedRez,			messageGetGroupListFailedRez, 2;
	messageUpdateGroupListFailedRez,	messageUpdateGroupListFailedRez, 3;
	messageUseNewNewsServerRez,				messageUseNewNewsServerRez, 4;
	messageUpdateGroupFailedRez,			messageUpdateGroupFailedRez, 5;
	messageUpdateServerIPRez,					messageUpdateServerIPRez, 6;

	whichList, 0, kPrefsMessageStrings;
	messageUsePrefsRez,								messageUsePrefsRez, 1;
	messageOpenPrefsFailedRez,				messageOpenPrefsFailedRez, 2;
	messageReadPrefsFailedRez,				messageReadPrefsFailedRez, 3;
	messageWritePrefsFailedRez,				messageWritePrefsFailedRez, 4;
	
	whichList, 0, kArticleViewMessageStrings;
	messageShowArticlesFailedRez,			messageShowArticlesFailedRez, 1;
	messageLaunchExtractorFailedRez,	messageLaunchExtractorFailedRez, 2;
	messageExpandArticleRez,					messageExpandArticleRez, 3;
	messageSaveNotesRez,							messageSaveNotesRez, 4;
	messageOpenNotesFileRez,					messageOpenNotesFileRez, 5;
	
	whichList, 0, kTExtractBinariesMessageStrings;
	messageCannotExtractBinariesRez,	messageCannotExtractBinariesRez, 1;
	
	whichList, 0, kSaveArticlesErrorStrings;
	messageCannotSaveFilesRez, 				messageCannotSaveFilesRez, 1;

	whichList, 0, kTPostMessageStrings;
	messageCouldNotPostArticleRez, 		messageCouldNotPostArticleRez, 1;
	messageLaunchEditorFailedRez,			messageLaunchEditorFailedRez, 2;
	messageCouldNotPostRez,						messageCouldNotPostRez, 3;
	messageSignatureProblemRez,				messageSignatureProblemRez, 4;
	messageTrashPostedDocumentRez,		messageTrashPostedDocumentRez, 5;

	whichList, 0, kMailMessageStrings;
	messageReplyWithLetterRez,				messageReplyWithLetterRez, 1;
	msgLaunchEudoraRez,								msgLaunchEudoraRez, 2;
	messageCancelArticleRez,					messageCancelArticleRez, 3;

	whichList, 0, kMacTCPMessageStrings;
	messageCannotOpenMacTcpRez,				messageCannotOpenMacTcpRez, 1;	
}};

// REASON table
resource 'errs' (1128, "reason table", purgeable) {{
	whichList, 0, kMacTCPErrorStrings;
	ipBadLapErr,									ipBadLapErr, 1;
	ipBadCnfgErr,									ipBadCnfgErr, 2;
	ipNoCnfgErr,									ipNoCnfgErr, 3;
	ipLoadErr,										ipLoadErr, 4;
	ipBadAddr,										ipBadAddr, 5;
	connectionClosing,						connectionClosing, 6;
	invalidLength,								invalidLength, 7;
	connectionExists,							connectionExists, 8;
	connectionDoesntExist,				connectionDoesntExist, 9;
	insufficientResources,				insufficientResources, 10;
	invalidStreamPtr,							invalidStreamPtr, 11;
	streamAlreadyOpen,						streamAlreadyOpen, 12;
	connectionTerminated,					connectionTerminated, 13;
	invalidBufPtr,								invalidBufPtr, 14;
	invalidRDS,										invalidRDS, 15;
	openFailed,										openFailed, 16;
	commandTimeout,								commandTimeout, 17;
	duplicateSocket,							duplicateSocket, 18;
	nameSyntaxErr,								nameSyntaxErr, 19;
	cacheFault,										cacheFault, 20;
	noResultProc,									noResultProc, 21;
	noNameServer,									noNameServer, 22;
	authNameErr,									authNameErr, 23;
	noAnsErr,											noAnsErr, 24;
	dnrErr,												dnrErr, 25;
	outOfMemory,									outOfMemory, 26;
	errOpenCommandTimeout,				errOpenCommandTimeout, 27;
	errMyMacHasNoDotName,					errMyMacHasNoDotName, 28;
	
	
	whichList, 0, kNntpErrorStrings;
	errBadNntpServer,							errBadNntpServer, 1;
	errUnknownNntpError,					errUnknownNntpError, 2;
	errUnexpectedNntpError, 			errUnexpectedNntpError, 3;
	errBadNntpRespons,						errBadNntpRespons, 4;	
	errDebugOutput,								errDebugOutput, 5;
	errServiceAborted,						errServiceAborted, 6;
	errTooManyUsers,							errTooManyUsers, 7;
	errNntpServerFailed,					errNntpServerFailed, 8;
	errNntpBadGroup,							errNntpBadGroup, 9;
	errNntpPermissionDenied,			errNntpPermissionDenied, 10;
	errNotAllowedToReadNews,			errNotAllowedToReadNews, 11;
	errNotAllowedToPost,					errNotAllowedToPost, 12;
	errPostingFailed,							errPostingFailed, 13;
	errNoSuchArticle,							errNoSuchArticle, 14;
	errNewsServerNoKnownGroups,		errNewsServerNoKnownGroups, 15;
	errMissingXHDRCommand, 				errMissingXHDRCommand, 16;
	errBadPassword,								errBadPassword, 17;
	errNoListOfNewGroups,					errNoListOfNewGroups, 18;
	errCouldNotFindServer,				errCouldNotFindServer, 19;

	whichList, 0, kTPostErrorStrings;
	errMissingYourName,						errMissingYourName, 1;
	errBadHeader,									errBadHeader, 2;
	errCannotPostNonTextFiles,		errCannotPostNonTextFiles, 3;
	errNoEditor,									errNoEditor, 4;
	errArticleNotEdited,					errArticleNotEdited, 5;
	errCancelNotYourArticle,			errCancelNotYourArticle, 6;
	
	whichList, 0, kArticleViewErrorStrings;
	errNoNoteFolder,							errNoNoteFolder, 1;

	whichList, 0, kTExtractBinariesErrorStrings;
	errBinariesMissingPart,				errBinariesMissingPart, 1;
	errBinHexMultiSubjectError,		errBinHexMultiSubjectError, 2;
	errBinHexBadSubject,					errBinHexBadSubject, 3;
	errBadBinaryFile,							errBadBinaryFile, 4;
	errNoBinaryFound,							errNoBinaryFound, 5;
	errNoExtractor,								errNoExtractor, 6;
	errBadBinHex,									errBadBinHex, 7;

	whichList, 0, kGroupErrorStrings;
	errDatabaseScrambled,					errDatabaseScrambled, 1;

	whichList, 0, kPrefsErrorStrings;
	errNoSuchPrefs,								errNoSuchPrefs, 1;
	errIncompatiblePrefsFileFormat, errIncompatiblePrefsFileFormat, 2;
	errScrambledPrefs,						errScrambledPrefs, 3;

	whichList, 0, kMailErrorStrings;
	errCannotUseAUX,							errCannotUseAUX, 1;
	errLaunchEudoraTimeout,				errLaunchEudoraTimeout, 2;
	errBringEudoraToFrontTimeout,	errBringEudoraToFrontTimeout, 3;
	errNoMailer,									errNoMailer, 4;
	errNoMailerDoc,								errNoMailerDoc, 5;

	whichList, 0, kMiscErrorStrings;
	-43,													-43, 1; // fnfErr
	errIncompatibleFileFormat,		errIncompatibleFileFormat, 2;
	errTooOldFileFormat,					errTooOldFileFormat, 3;
	errTooNewFileFormat,					errTooNewFileFormat, 4;
	-35,													-35, 5; // no such volume
}};

resource 'errs' (1129, "recovery", purgeable) {{
// Groups
	whichList, 0, kMiscRecoveryStrings;
	errDatabaseScrambled,					errDatabaseScrambled, 3;

// NNTP/MacTCP
	whichList, 0, kMiscRecoveryStrings;
	errNoSuchArticle,							errNoSuchArticle, 1;
	errDebugOutput,								errDebugOutput, 2;

	whichList, 0, kNetworkRecoveryStrings;
	commandTimeout,								commandTimeout, 2;
	kLastNntpError,								kFirstNntpError, 1;
	kLastMacTCPError,							kFirstMacTCPError, 1;
}};

//===================================== MISC ==============================
resource 'STR#' (kMiscErrorStrings, "kMiscErrorStrings", purgeable) {{
	"it was not found";
	"it cannot be opened by this version of Nuntius"; // generic incompatible fileformat
	"it was created by a too old version of Nuntius";
	"it's created by a too new version of Nuntius. Please upgrade your version of Nuntius to the lastest one.";
	"the specified volume doesn't exist";
}};

resource 'STR#' (kMiscRecoveryStrings, "kMiscRecoveryStrings", purgeable) {{
	""; // no_recovery recovery string
	"Please write a letter about this problem to the author of Nuntius: speck@dat.ruc.dk";
	"Quit Nuntius, put the folder \"Nuntius public files\" in the trash, and "
	/**/ "restart Nuntius. If this doesn't help, call your system administrator or write to "
	/**/ "the author of Nuntius: speck@dat.ruc.dk";
}};
//==================================== NEWSAPPL ==========================
resource 'STR#' (kTNewsApplMessageStrings, "TNewsApplMessageStrings", purgeable) {{
	"open the group �^3�";
	"get the list of all groups from the news server";
	"update the list of all groups";
	"use the server"; // news server
	"open and update the group �^3�";
	"update the news servers IP number (call your system administrator)";
}};

resource 'STR#' (kPrefsMessageStrings, "kPrefsMessageStrings", purgeable) {{
	"use the stored preferences";
	"open or create the preferences document";
	"read the preferences document";
	"update the preferences document";
}};
resource 'STR#' (kPrefsErrorStrings, "kPrefsErrorStrings", purgeable) {{
	"a preference did not exist (bug in Nuntius, please report to me: speck@dat.ruc.dk)";
	"they were stored in an incompatible format.";
	"the preferences seem to be scrambled";
}};

resource 'STR#' (kGroupErrorStrings, "kGroupErrorStrings", purgeable) {{
	"the group database is scrambled";
}};

resource 'STR#' (kArticleViewMessageStrings, "kArticleViewMessageStrings", purgeable) {{
	"show the articles";
	"launch the extractor";
	"expand the article";
	"save the selection";
	"create/open the note document";
}};
resource 'STR#' (kArticleViewErrorStrings, "kArticleViewErrorStrings", purgeable) {{
	"you have not selected a folder for note documents. Choose \"Misc�\" from \"Prefs\" menu";
}};

//============================ NETWORK =========================================
resource 'STR#' (kNntpErrorStrings, "NntpErrorStrings", purgeable) {{
	"the news server did not understand a command";
	"an unknown response was received from the news server";
	"an unexpected error message was received from the news server";
	"an invalid response was received from the news server";
	"some debug-output from the news server was not ignored (bug in Nuntius)";
	"the news server discontinued the news service";
	"the news server denies to support more concurrent requests";
	"the news server could not perform a command";
	"that group is not available at your news server";
	"you were denied permission to do that";
	"you are not allowed to read news from this news server";
	"you do not have permission to post articles";
	"the news server failed to post it";
	"that article is not available from the news server";
	"the news server doesn't seem useable.  It is missing basic groups.";
	"the news server has not the standard 'XHDR' extension.";
	"your password is incorrect";
	"the list of new groups is unavailable from the news server";
	"the server does not exist (could not get its dot address)";
}};

resource 'STR#' (kMacTCPMessageStrings, "kMacTCPMessageStrings", purgeable) {{
	"open MacTCP";
}};
resource 'STR#' (kMacTCPErrorStrings, "kMacTCPErrorStrings", purgeable) {{
	"of the MacTCP error: bad network configuration";
	"of the MacTCP error: bad IP configuration error";
	"of the MacTCP error: missing IP or LAP configuration error";
	"of error in MacTCP load";
	"of error in getting address";
	"the connection is closing";
	"of the MacTCP error: invalid length";
	"of the MacTCP error: request conflicts with existing connection";
	"the connection does not exist";
	"of the MacTCP error: insufficient resources to preform request";
	"of the MacTCP error: invalidStreamPtr";
	"of the MacTCP error: streamAlreadyOpen";
	"the connection is terminated";
	"of the MacTCP error: invalidBufPtr";
	"of the MacTCP error: invalid RDS/WDS"; // 15
	"this machine is not a news server (could not open an NNTP connection to it)";
	"the connection to the news server timed out";
	"of the MacTCP error: duplicate socket";
	"the name of the server is not valid (syntax error)";
	"cachefault: this error should never be reported as it is not an error";
	"of the MacTCP error: noResultProc";
	"MacTCP could not find a nameserver";
	"its dot-name is not available";
	"the *name* server did not respond (try typing its dot number instead)";
	"of a generic Domain Name Resolver error";
	"MacTCP did not have enough memory";
	"the server does not respond";
	"this Macintosh has no dot-name";
}};

resource 'STR#' (kNetworkRecoveryStrings, "kNetworkRecoveryStrings", purgeable) {{
	"Contact your system administrator.";
	"Try increasing the timeout value in \"News server prefs\" from the \"Prefs\" menu.";
}};

//=============================== SAVE ARTICLES/BINARIES =======================
resource 'STR#' (kSaveArticlesErrorStrings, "SaveArticlesStrings", purgeable) {{
	"save the articles";
}};

resource 'STR#' (kTExtractBinariesMessageStrings, "TExtractBinariesMessageStrings", purgeable) {{
	"extract the binaries";
}};
resource 'STR#' (kTExtractBinariesErrorStrings, "TExtractBinariesErrorStrings", purgeable) {{
	"some of the parts are missing";
	"the part specification in the subject lines cannot be parsed adequately";
	"one of the subject lines contains garbage in the part specification";
	"one of the files contains an error in the control lines";
	"no binaries were found in the articles";
	"you have not selected an extractor. Choose \"Binaries\" from the \"Prefs\" menu";
	"the encoding of the binary is scrambled";
}};

//================================== P O S T I N G  ===============================
resource 'STR#' (kTPostMessageStrings, "kTPostMessageStrings", purgeable) {{
	"post the article";
	"launch the editor";
	"post an article";
	"get the signature";
	"Trash the disk copy of the posted article";
}};
resource 'STR#' (kTPostErrorStrings, "kTPostErrorStrings", purgeable) {{
	"you have not specified your name. Choose \"Your Name�\" from the \"Prefs\" menu.";
	"bad header"; // alert is used for this error
	"the article is not saved as \"plain text\"";
	"you have not selected an editor. Choose \"Editing articles\" from \"Prefs\" menu";
	"you have not edited the article";
	"it is not posted by you";
}};


//===================================   M A I L   =========================
resource 'STR#' (kMailMessageStrings, "kMailMessageStrings", purgeable) {{
	"reply by mail";
	"launch Eudora";
	"cancel the article";
}};
resource 'STR#' (kMailErrorStrings, "kMailErrorStrings", purgeable) {{
	"this hack cannot be used with A/UX";
	"<I don't know>. Launch Eudora yourself";
	"<I don't know>. Something unknown went wrong";
	"you have not selected a mailer. Choose \"Mail\" from \"Prefs\" menu";
	"you have not selected a document to launch the mailer with. Choose \"Mail\" from \"Prefs\" menu";
}};


//===================================== PREFERENCES =========================

resource 'STR#' (kYourNameErrorStrings, "kYourNameErrorStrings", purgeable) {{
	"Please specify your full name.";
	"Can't do it with a '@', keep it for your e-mail address";
	"Can't do it without a '@'"; // not used!
	"You may not use '<' or '>' here!";
	"The number of )s must match the number of (s.";
}};

resource 'STR#' (kYourSnailErrorStrings, "kYourSnailErrorStrings", purgeable) {{
	"You must have an e-mail address. Please specify it.";
	"Can't do it with more than one '@' in an e-mail address";
	"Can't do it without a '@' in an e-mail address";
	"You may not use '<' or '>' here, I take care of those!";
	"The number of )s must match the number of (s.\n\nPS. Nesting is allowed.";
}};

resource 'STR#' (kOrganizErrorStrings, "kOrganizErrorStrings", purgeable) {{
	"Please specify the name of your organization or affiliation.";
	"Can't do it with a '@'"; // not used
	"Can't do it without a '@'"; // not used!
	"You may not use '<' or '>' here!"; // not used
	"The number of )s must match the number of (s.\n(Nesting is allowed)";
}};

