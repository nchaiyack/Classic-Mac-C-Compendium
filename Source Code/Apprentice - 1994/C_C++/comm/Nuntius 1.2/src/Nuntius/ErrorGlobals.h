// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// ErrorGlobals.h

#define __ERRORGLOBALS__
//==================================================
//==================================================
//================================================== TNewsAppl (26000-26099)
#define phNeedSystem7												 26000
#define phPanicExitAlert										 26001
#define phFatalReportAlert			 						 26001
#define phOldPreFinalVersionWarning					 26005
#define phMySpaceIsLow											 26010

#define kTNewsApplMessageStrings						 26020

#define kMiscErrorStrings										 26021
#define kMiscRecoveryStrings								 26022
#define errIncompatibleFileFormat						-26090
#define errTooOldFileFormat									-26091
#define errTooNewFileFormat									-26092


#define messageOpenGroupFailedRez						 26020
#define messageOpenGroupFailed (messageOpenGroupFailedRez + messageLookup)

#define messageUpdateGroupFailedRez					 26025
#define messageUpdateGroupFailed (messageUpdateGroupFailedRez + messageLookup)

#define messageGetGroupListFailedRez				 26030
#define messageGetGroupListFailed (messageGetGroupListFailedRez + messageLookup)

#define messageUpdateGroupListFailedRez			 26031
#define messageUpdateGroupListFailed (messageUpdateGroupListFailedRez + messageLookup)

#define messageUseNewNewsServerRez					 26040
#define messageUseNewNewsServer		(messageUseNewNewsServerRez + messageLookup)

#define messageUpdateServerIPRez						 26041
#define messageUpdateServerIP		(messageUpdateServerIPRez + messageLookup)

//================================================== GroupTree/GroupList /26200-26299) 
#define phSubscribeVeryManyGroups						 26200

//================================================== PreferencesMgr (26300-26399)
#define kPrefsMessageStrings								 26300
#define kPrefsErrorStrings									 26301
#define phLocalPrefsNotAllowed							 26305

#define errNoSuchPrefs											-26300
#define errIncompatiblePrefsFileFormat			-26310
#define errScrambledPrefs										-26311

#define messageUsePrefsRez									 26310
#define messageUsePrefs (messageUsePrefsRez + messageLookup)

#define messageOpenPrefsFailedRez						 26315
#define messageOpenPrefsFailed (messageOpenPrefsFailedRez + messageLookup)
#define messageWritePrefsFailedRez					 26316
#define messageWritePrefsFailed (messageWritePrefsFailedRez + messageLookup)
#define messageReadPrefsFailedRez						 26317
#define messageReadPrefsFailed (messageReadPrefsFailedRez + messageLookup)

//================================================== PreferencesViews (26400-26499)
#define phNameError													 26400
#define kYourNameErrorStrings								 26410
#define kYourSnailErrorStrings							 26411
#define kOrganizErrorStrings								 26412

//================================================== TGroupDoc (27000-27099)
#define kGroupErrorStrings									 27000
#define errDatabaseScrambled								-27000

//================================================== TArticleView (27300-27399)
#define kArticleViewMessageStrings					 27300
#define kArticleViewErrorStrings						 27301

#define messageShowArticlesFailedRez				 27300
#define messageShowArticlesFailed (messageShowArticlesFailedRez + messageLookup)

#define messageExpandArticleRez							 27301
#define messageExpandArticle (messageExpandArticleRez + messageLookup)

#define messageSaveNotesRez									 27310
#define messageSaveNotes (messageSaveNotesRez + messageLookup)
#define messageOpenNotesFileRez							 27311
#define messageOpenNotesFile (messageOpenNotesFileRez + messageLookup)
#define errNoNoteFolder											-27312

//================================================== Binaries Commands (27400-27499)
#define phMissingSaveArticle								 27400
#define phMissingSaveArticleMany						 27401
#define phMissingAllSaveArticles						 27402

#define kSaveArticlesErrorStrings						 27400
#define messageCannotSaveFilesRez						 27400
#define messageCannotSaveFiles (messageCannotSaveFilesRez + messageLookup)

#define kTExtractBinariesErrorStrings				 27410
#define kTExtractBinariesMessageStrings			 27420

#define messageCannotExtractBinariesRez			 27450
#define messageCannotExtractBinaries (messageCannotExtractBinariesRez + messageLookup)
#define errBinariesMissingPart							-27450
#define errBinHexMultiSubjectError					-27451
#define errBinHexBadSubject									-27452
#define errNoBinaryFound										-27460
#define errBadBinaryFile										-27461
#define errNoExtractor											-27462
#define errBadBinHex												-27463

#define messageLaunchExtractorFailedRez			 27470
#define messageLaunchExtractorFailed (messageLaunchExtractorFailedRez + messageLookup)

//================================================== Post Commands (27500-27599)
#define phPossiblePostedAnyway							 27505
#define phMissingTranslateTable							 27506

#define kTPostErrorStrings									 27500
#define kTPostMessageStrings								 27510

#define errMissingYourName									-27500
#define errBadHeader												-27501
#define errCannotPostNonTextFiles						-27502
#define errArticleNotEdited									-27503
#define errNoEditor													-27505
#define errCancelNotYourArticle							-27510

#define messageEmptySubject									 27510 + messageAlert
#define messageEmptyGroups									 27511 + messageAlert
#define messageEmptyDistribution						 27512 + messageAlert
#define messageInvalidHeaderChar						 27530 + messageAlert

#define messageCouldNotPostRez							 27510
#define messageCouldNotPost (messageCouldNotPostRez + messageLookup)

#define messageCouldNotPostArticleRez				 27515
#define messageCouldNotPostArticle (messageCouldNotPostArticleRez + messageLookup)

#define messageSignatureProblemRez					 27520
#define messageSignatureProblem (messageSignatureProblemRez + messageLookup)

#define messageLaunchEditorFailedRez				 27530
#define messageLaunchEditorFailed (messageLaunchEditorFailedRez + messageLookup)

#define messageTrashPostedDocumentRez				 27550
#define messageTrashPostedDocument (messageTrashPostedDocumentRez + messageLookup)

//================================================== Mail Commands (27600-27699)
#define messageReplyWithLetterRez						 27600
#define messageReplyWithLetter (messageReplyWithLetterRez + messageLookup)

#define msgLaunchEudoraRez									 27601
#define msgLaunchEudora (msgLaunchEudoraRez + messageLookup)

#define messageCancelArticleRez							 27602
#define messageCancelArticle (messageCancelArticleRez + messageLookup)

#define errNoMailer													-27605
#define errNoMailerDoc											-27606
#define errCannotUseAUX											-27610
#define errLaunchEudoraTimeout							-27620
#define errBringEudoraToFrontTimeout				-27621
//================================================== MacTCP (28000-28099)
#define kFirstMacTCPError										-28000
#define kLastMacTCPError										-28099
#define kMacTCPMessageStrings								 28000
#define kMacTCPErrorStrings									 28001
#define kMacTCPDirectErrorStrings						 28002
#define messageCannotOpenMacTcpRez					 28010
#define messageCannotOpenMacTcp (messageCannotOpenMacTcpRez + messageLookup)

#define errOpenCommandTimeout								-28000
#define errMyMacHasNoDotName								-28001
//================================================== NNTP (28100-28199)
#define kFirstNntpError											-28100
#define kLastNntpError											-28199
#define kNntpErrorStrings										 28100
#define errUnknownNntpError									-28100
#define errBadNntpRespons										-28101
#define errUnexpectedNntpError							-28102
#define errBadNntpServer										-28103
#define errNntpServerFailed									-28104
#define errNoListOfNewGroups								-28105
#define errCouldNotFindServer								-28106

#define errDebugOutput											-28110
#define errServiceAborted										-28111
#define errTooManyUsers											-28112
#define errNntpBadGroup											-28113

#define errNntpPermissionDenied 						-28120
#define errNotAllowedToReadNews							-28121
#define errNotAllowedToPost									-28122
#define errPostingFailed										-28123
#define errBadPassword											-28124

#define errNoSuchArticle										-28130

#define errNewsServerNoKnownGroups					-28190
#define errMissingXHDRCommand								-28191

//================================================== Misc Network (28200-28299)
#define kNetworkRecoveryStrings							 28200
