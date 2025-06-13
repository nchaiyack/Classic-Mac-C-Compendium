// Copyright © 1992-1995 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// RsrcGlobals.h

#ifdef  __RSRCGLOBALS__
#error "Already included RsrcGlobals.h"
#endif

#define __RSRCGLOBALS__

/******************************************************** Resource types and such */
#define kSignature							'nNNn'
#define kPrefsFileType						'PREF'
#define kGroupDBFileType					'Pægl'
#define kArticleStatusFileType				'Fjæs'
#define kGroupListDocFileType				'ÅSYN'
#define kGroupTreeDocFileType				'GTræ'
#define kScrapType							'TEXT' /* Clipboard type */

#define cNewGroupTreeDoc					13
#define cOpenGroupTreeDoc					23

#define cNewGroupListDoc					14
#define cOpenGroupListDoc					24

#define cOpenGroupDoc						25

/******************************************************** Resources */
//------------------------------------------- Globals
#define cGenericCommand						4100
#define kViewTextWindow						4110
#define kFAQTextID							4110
#define kPointerHandCursor					4123
//------------------------------------------- NewsAppl
#define cUpdatePrefs						5000
#define cExecuteCommandInNewThreadCommand	5010
#define cDoPeriodicActionCommand			5011
#define cToggleFlushVolumes					5020
#define phAboutBoxDlog						5020
#define kNewServerStrings					5040
#define kNewServerObtainingAddress			5041
#define kNewServerBasicTest					5042
#define kStartupStrings						5060
#define kStartupTitle						5061
#define kStartupUpperText					5062
#define kStartupOpensPrefsFile				5063
#define kStartupPostPonedInitMgrs			5064
#define kStartupReadsGroupTree				5065

#define cForgetPassword						5100

#define kFatalErrorStrings					5100
#define kFEInitialMessage					5101
#define kFEHeaderBodySeparator				5102
#define kFEPostMessage						5103
#define kFEVersion1							5104
#define kFEVersion2							5105
#define kFEUSLangName						5106
#define kFEMacModel							5107
#define kFESystemVersion					5108
#define kFEErrorMessage						5109
#define kFECompileDateTimeNumber			5110
#define kFECompileDateString				5111

//------------------------------------------- GroupDoc
#define cDatabaseUpdated					6100
#define cChangedOption						6110
#define cSaveArticleStatus					6120
#define cSaveGroupDB						6121
#define cDebugDumpGroupDatabase				6130
#define kOpenGroupStrings					6200
#define kOpenGroupReadsFile					6201
#define kOpenGroupChecksForNew				6202
#define kOpenGroupDeletesOld				6203
#define kOpenGroupUpdatesDatabase			6204
#define kOpenGroupRebuildDatabase			6205
#define kOpenGroupSavesDatabase				6206

#define phOpenManyGroups					6200
#define kOpenManyGroupsLimit				3
// ------------------------------------------ GroupListDoc
#define kGroupListView						7000
#define kGroupListViewID					'GrLs'
#define kGroupListTextStyle					7000
#define cGroupListChange					7000
#define kOverwriteExistingGroupsAlert	 	7010
#define kGroupListDocStrings				7020
#define kGroupListUntitledName				7021
#define kMoveGroupsWordIndex				7022
#define kCopyGroupsWordIndex				7023
#define cCheckForNewArticles				7030
#define kGroupListNAStrings					7040
#define kGroupListNAUnknown					7041
#define kGroupListNANoNew					7042
#define kGroupListNAHasNew					7043

//------------------------------------------- GroupTree
#define kGroupTreeView						11000
#define kGroupTreeViewID					'GrTL'
#define kGroupTreeTextStyle					11000
#define kGroupTreeStrings					11020
#define kGroupTreeRootName					11021
	
#define cOpenListOfAllGroups				11030
#define cUpdateGroupTree					11040
#define cRebuildGroupTree					11041

#define kNonExpandedDiamondSicn				11000
#define kHalfExpandedDiamondSicn			11010
#define kExpandedDiamondSicn				11020
#define kDocumentSicn						11030
#define kFolderSicn							11040

#define cExpandGroup						11100
#define cTreeKeyCommand						11110

#define kUpdGroupTreeStrings				11200
#define kUpdGTFetchesGroupList				11201
#define kUpdGTBuildsTree					11202
#define kUpdGTUpdatesTree					11203
#define kUpdGTSavesTree						11204
#define kUpdGTProgressTitle					11205
#define kUpdGTNewGroupsDocName				11206
#define kUpdGTFetchesGroupDescriptions		11207
#define kUpdGTProcessGroupsDescriptions		11208
#define kUpdGTFetchesListOfNewGroups		11209
//------------------------------------------- DiscList
#define kDiscListView						12000
#define kDiscListViewID						'DiLs'
#define kDiscListViewTextStyle				12000
#define kDiscListViewHelp					12000
#define kDiscSeparatorPattern				12000
#define kDiscListStatusStrings				12020
#define kDiscListStatusNew					12021
#define kDiscListStatusSeen					12022
#define kDiscListStatusRead					12023
#define kDiscListStatusExpiredSubject		12024
#define kDiscListStatusExpiredAuthor		12025

#define cShowAllDiscussions					12100
#define cShowOnlyTodayDiscussions			12101
#define cShowDiscsWithUnreadArticles		12102
#define cShowDiscsWithNewArticles			12103

#define cPrefShowAllDiscussions				12110
#define cPrefShowOnlyTodayDiscussions		12111
#define cPrefShowDiscsWithUnreadArticles	12112
#define cPrefShowDiscsWithNewArticles		12113

#define cMarkThreadAsNew					12200
#define cMarkThreadAsSeen					12201
#define cMarkThreadAsRead					12202

//------------------------------------------- ArticleView
#define kArticleView						13000
#define kArticleListViewID					'ArLi'
#define kArticleViewTextStyle				13000
#define kArticleViewHelp					13000
#define kNavigatorView						'Navi'
#define kNavigatorLeftArrowView				'NavL'
#define kNavigatorSpaceArrowView			'NavS'
#define kNavigatorRightArrowView			'NavR'
#define kNavigatorLeftArrow					13100
#define kNavigatorSpaceArrow				13110
#define kNavigatorRightArrow				13120

#define kJapaneseFontTextStyle				13010

#define cOpenArticle						13200
#define cShowDiscJunkCommand				13210
#define cShowHideHeadersCommand				13220
#define cUseROT13							13221
#define cCopyAsQuote						13222
#define cUpdateArticleListCommand			13230
//
#define cShowAllArticles					13231
#define cShowOnlyFirstArticle				13232
#define cShowOnlyUnreadArticles				13233
#define cShowOnlyNewArticles				13234
#define cShowNoneArticles					13235
//
#define cPrefShowAllArticles				13241
#define cPrefShowOnlyFirstArticle			13242
#define cPrefShowOnlyUnreadArticles			13243
#define cPrefShowOnlyNewArticles			13244
#define cPrefShowNoneArticles				13245
//
#define cNextDiscussion						13251
#define cPreviousDiscussion					13252
#define cMultiSuperNextKey					13253
#define cNextArticle						13254
#define cPreviousArticle					13255
//
#define cArticleStatusChange				13300

#define kArticleStatusStrings				13300
#define kAllArticlesRead					13301
#define kWaitForArticleBody					13302
#define kArticleIsMissing					13303
#define kArticleNotDisplayed				13304
#define kArticleOtherError					13305

#define kArticleStrings						13320
#define kUnknownSubject						13321
#define kPreSubject							13322
#define kPostSubject						13323
#define kPreFrom							13324
#define kPostFrom							13325
#define kPreDate							13326
#define kPostDate							13327
#define kArticleClipboardSeparator			13328
#define kArticleReal2EmailSeparator			13329
#define kArticleQuoteLine					13330
#define kArticleQuoteYouLine				13331
#define kArticleCompactTemplate				13332
#define kNoteQuoteTemplate					13333
#define kUnknownArticleText					13334
#define kQuoteLineTemplate					13335
#define kArticlePageNumTemplate				13336

#define cStoreNotes							13800
#define cCancelArticle						13801
#define kEnglishShortWeekdayStrings			13801
#define kEnglishShortMonthStrings			13802

//------------------------------------------- Posting
#define phCancelArticle						14000
#define cPostNewDiscussion					14010
#define cPostFollowUpArticle				14011
#define cEditPostArticle					14020
#define cPostArticle						14030

#define kPostArticleStrings					14000
#define kPostArticleExaminesArticle			14001
#define kPostArticleSendingText				14002
#define kPostArticleACKWait					14003
#define kCancelArticleSendingRequest		14004
#define kCancelArticleProgressTitle			14005
#define kDefaultTranslateTableNameIn		14006
#define kDefaultTranslateTableNameOut		14007

#define kPostArticleWeekDayNames			14021
#define kPostArticleMonthNames				14022
#define kCancelArticleHeaderStrings			14030

#define kEditArticleToPostView				14100
#define kEditArticleToPostViewStrings		14100
#define kEditArticleToPostTextStyle			14100

#define kPostArticleView					14120
#define kPostArticleViewStrings				14120
#define kPostArticleTextStyle				14120

//------------------------------------------- Mail
#define cMailToAuthors						14200
#define kMailErrorStrings					14210
#define kMailMessageStrings					14211

#define kMailStrings						14200
#define kMailPreSubject						14201
#define kMailPostSubject					14202
#define kMailScriptName						14203
#define kMailHandlerName					14204
#define kURLScriptName						14205
#define kURLHandlerName						14206

#define cMailLetter							13900
#define phNoMailYet							13900

//------------------------------------------- Preferences
#define kPreferencesStrings					15000
#define kPrefsFileName						15001
#define kTempPrefsName						15002
#define kPublicFilesFolderName				15003
#define kPrivateFilesFolderName				15004
#define kUntitledFileName					15005
#define kGroupTreeFileName					15006
#define kObsoloteFolderName					15007
#define kErrorFileName						15008


#define kGetDirectoryDLOG					15020
#define kAskFileFolderStrings				15020
#define kAskFFDefaultApplSignature			15021
#define kAskFFDefaultApplName				15022
#define kAskFolderPrompt					15023
#define kFolderSelectButtonID				'FSbu'
#define kFolderSelectPathID					'FSpa'
#define kGetDirButton						10
#define kGetDirMessage						11
#define kGetDesktopButton					12
#define kFolderPathTextStyle				15020

//---------- Preferences views
#define cOpenYourNamePrefs					15140
#define kYourNamePrefsView					15140
#define kYourNamePrefsViewStrings			15140
#define kYourNamePromptTextStyle			15140
#define kYourNameTextStyle					15141

#define cOpenNewsServerPrefs				15180
#define kNewsServerPrefsView				15180
#define kNewsServerPrefsViewStrings			15181
#define kNewsServerTextStyle				15180
#define kNewsServerCommentTextStyle			15181
#define kNewsServerStrings					15180
#define kNewsServerPreName					15181
#define kNewsServerPostName					15182
#define kNewsServerPreIP					15183
#define kNewsServerPostIP					15184

#define cOpenBinariesPrefs					15200
#define kBinariesPrefsView					15200
#define kBinariesPrefsViewStrings			15200
#define mAskFileNameHit						15201
#define mLaunchExtractor					15202

#define cOpenEditorPrefs					15240
#define kEditorPrefsView					15240
#define kEditorPrefsViewStrings				15240
#define mLaunchEudoraWithDocument			15240

#define cOpenMailerPrefs					15280
#define kMailerHackPrefsView				15280
#define kMailerPrefsView					15281
#define kMailerPrefsViewStrings				15280

#define cOpenMiscPrefs						15300
#define kMiscPrefsView						15300
#define kMiscPrefsViewStrings				15300


#define cExpirePrefs						15320
#define kExpirePrefsView					15320

//----------- Ask something preferences
#define kAskNewsServerView					15800
#define kAskNewsServerStrings				15800
#define kNewsServerNameViewID				'Name'

#define kAskPasswordView					15840
#define kUsernameView						'UNam'
#define kPasswordView						'Pass'
#define kAskPasswordStrings					15840
#define kUserNameTextStyle					15840

//------------------------------------------- Progress-
#define kProgressOneLineView				16000
#define kProgressTwoLineView				16010
#define kModalProgressOneLineView			16020
#define kModalProgressTwoLineView			16030

#define kProgressViewStrings				16000
#define kFirstProgressCandyPictID			16000
#define kFirstProgressCandyBWPictID			16010

//------------------------------------------- Save Articles / Binaries-
#define cExtractBinaries					16100
#define kSaveArticlesStrings				16100
#define kSaveArticlesPrompt					16101
#define kSaveBinariesPrompt					16102
#define kSaveProgressTitle					16103
#define kExtractProgressTitle				16104
#define kSubjectPartHeader					16105
#define kSaveArticleSeparator				16106

#define kSuffixMappings						16110
//------------------------------------------- NNTP/MacTcp-
#define cFlushUnusedNntpConnections			16200
#define cAllCaches							16201

#define kNntpStrings						16200
#define kNntpOpensConnection				16201
#define kTooManyUsersRespond				16202

#define phUnableToForgetAllPasswords		16210
#define kCheckServerGroupsStrings			16220
//------------------------------------------- 
//------------------------------------------- Misc
#define kMiscStrings						17000
#define kPasswordDot						17001
#define kEmptyGroupListMenuText				17002
#define kPreSelectFolderText				17003
#define kPostSelectFolderText				17004
#define kLongGroupNameFolderSuffix			17005
#define kCloseIdleConnectionsMenuItemTmpl	17006
#define k68KProcessorString					17007
#define kPPCProcessorString					17008
#define kNuntiusHomeSiteURL					17009
#define kNuntiusWebPage						17010
#define kAboutBoxVersionTemplate			17011

#define kAboutBoxVersionTS					17000

#define kValidURLSchemeStrings				17010
//------------------------------------------- 

//------------------------------------------- Debugging
#define cShowViewInspector					29000
#define kViewInspector						29000
#define cShowTargetInspector				29010
#define kTargetInspector					29010
#define cMacsBug							29100
#define cSysBug								29101
#define cDumpPrefs							29102
#define cDebugDumpThreads					29103
#define cMakeFatalErrorDescription			29104
#define cDebugDumpArticleTextCache			29105
#define cTest1								29111
#define cTest2								29112
#define cTest3								29113
#define cTest4								29114
#define cMenuForcedFatalError				29120

