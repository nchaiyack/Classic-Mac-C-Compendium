#define kMBarID				128			/* resource id of MBAR rsrc */
#define kEntireMenuMask		0x00000001	/* mask for enabling an entire menu */

#define kHeaderMenu			200			/* popup menu for search dialog */
#define	kKillFieldMenu		201			/* Popup menu for header in Kill dialog */
#define kKillMatchMenu		202			/* Popup menu for match type in kill dialog */
#define kKillKeepMenu		208			/* Popup menu for keeping/killing matches */



/*	Apple menu. */

#define kAppleMenu				128

#define kAboutItem				1



/*	File menu. */

#define kFileMenu				129

#define kNewItem				1
#define kOpenItem				2
#define kCloseItem				3
#define kSaveItem				5
#define kSaveAsItem				6
#define kPageSetupItem			8
#define kPrintItem				9
#define kPrintSelItem			10
#define kGetFromHostItem		12
#define kSendToHostItem			13
#define kPrefsItem				15
#define kQuitItem				17

#define kNumFileMenuItems		17

#define	kNewMask				(1L << kNewItem)
#define kOpenMask				(1L << kOpenItem)
#define kCloseMask				(1L << kCloseItem)
#define kSaveMask				(1L << kSaveItem)
#define kSaveAsMask				(1L << kSaveAsItem)
#define kPageSetupMask			(1L << kPageSetupItem)
#define kPrintMask				(1L << kPrintItem)
#define kPrintSelMask			(1L << kPrintSelItem)
#define kGetFromHostMask		(1L << kGetFromHostItem)
#define kSendToHostMask			(1L << kSendToHostItem)
#define kPrefsMask              (1L << kPrefsItem)
#define kQuitMask				(1L << kQuitItem)

#define kNoneFileEnabled		(kEntireMenuMask | kNewMask | kOpenMask | \
	kPageSetupMask | kGetFromHostMask | kPrefsMask | kQuitMask)
	
#define kDAFileEnabled			(kEntireMenuMask | kNewMask | kOpenMask | \
	kPageSetupMask | kGetFromHostMask | kPrefsMask | kQuitMask)
	
#define kGroupFileEnabled		(kEntireMenuMask | kNewMask | kOpenMask | \
	kCloseMask | kPageSetupMask | kGetFromHostMask | kPrefsMask | kQuitMask)
	
#define kNewGroupFileEnabled	(kEntireMenuMask | kNewMask | kOpenMask | \
	kCloseMask | kPageSetupMask | kGetFromHostMask | kPrefsMask | kQuitMask)
		
#define kUserGroupFileEnabled	(kEntireMenuMask | kNewMask | kOpenMask | \
	kCloseMask | kSaveMask | kSaveAsMask | \
	kPageSetupMask | kGetFromHostMask | kSendToHostMask | \
	kPrefsMask | kQuitMask)
	
#define kSubjectFileEnabled		(kEntireMenuMask | kNewMask | kOpenMask | \
	kCloseMask | kPageSetupMask | kGetFromHostMask | kPrefsMask | kQuitMask)
	
#define kArticleFileEnabled		(kEntireMenuMask | kNewMask | kOpenMask | \
	kCloseMask | kSaveMask | kSaveAsMask | kPageSetupMask | kPrintMask | \
	kPrintSelMask | kGetFromHostMask | kPrefsMask | kQuitMask)
	
#define kMiscArticleFileEnabled	(kEntireMenuMask | kNewMask | kOpenMask | \
	kCloseMask | kSaveMask | kSaveAsMask | kPageSetupMask | kPrintMask | \
	kPrintSelMask | kGetFromHostMask | kPrefsMask | kQuitMask)
	
#define kMailMessageFileEnabled	(kEntireMenuMask | kNewMask | kOpenMask | \
	kCloseMask | kSaveMask | kSaveAsMask | kPageSetupMask | kPrintMask | \
	kPrintSelMask | kGetFromHostMask | kPrefsMask | kQuitMask)
	
#define kPostMessageFileEnabled	(kEntireMenuMask | kNewMask | kOpenMask | \
	kCloseMask | kSaveMask | kSaveAsMask | kPageSetupMask | kPrintMask | \
	kPrintSelMask | kGetFromHostMask | kPrefsMask | kQuitMask)
	
#define kStartupBadFileEnabled	(kEntireMenuMask | kPrefsMask | kQuitMask)



/*	Edit menu. */

#define kEditMenu				130

#define kUndoItem				1
#define kCutItem				3
#define kCopyItem				4
#define kPasteItem				5
#define kPasteQuoteItem			6
#define kClearItem				7
#define kSelectAllItem			8
#define kShowHideHeaderItem		10
#define kRotItem				11

#define kNumEditMenuItems		11

#define kUndoMask				(1L << kUndoItem)
#define kCutMask				(1L << kCutItem)
#define kCopyMask				(1L << kCopyItem)
#define kPasteMask				(1L << kPasteItem)
#define kPasteQuoteMask			(1L << kPasteQuoteItem)
#define kClearMask				(1L << kClearItem)
#define kSelectAllMask			(1L << kSelectAllItem)
#define kShowHideHeaderMask		(1L << kShowHideHeaderItem)
#define kRotMask				(1L << kRotItem)

#define kNoneEditEnabled		0
#define kEditOnlyCopyEnabled	(kEntireMenuMask | kCopyMask)

#define kDAEditEnabled			(kEntireMenuMask | kUndoMask | kCutMask | \
	kCopyMask | kPasteMask | kClearMask)
	
#define kGroupEditEnabled		(kEntireMenuMask | kSelectAllMask)
	
#define kNewGroupEditEnabled	(kEntireMenuMask | kSelectAllMask)
	
#define kUserGroupEditEnabled	(kEntireMenuMask | kSelectAllMask)
	
#define kSubjectEditEnabled		(kEntireMenuMask | kSelectAllMask)
	
#define kArticleEditEnabled		(kEntireMenuMask | kCopyMask | \
	kSelectAllMask | kShowHideHeaderMask | kRotMask)
	
#define kMiscArticleEditEnabled	(kEntireMenuMask | kCopyMask | \
	kSelectAllMask | kShowHideHeaderMask | kRotMask)
	
#define kMailMessageEditEnabled	(kEntireMenuMask | kCutMask | kCopyMask | \
	kPasteMask | kPasteQuoteMask | kClearMask | kSelectAllMask | \
	kShowHideHeaderMask)
	
#define kPostMessageEditEnabled	(kEntireMenuMask | kCutMask | kCopyMask | \
	kPasteMask | kPasteQuoteMask | kClearMask | kSelectAllMask | \
	kShowHideHeaderMask | kRotMask)
	
#define kStartupBadEditEnabled	0



/*	News menu. */

#define kNewsMenu				131

#define kNextArticleItem		1
#define kNextThreadItem			2
#define kNextGroupItem			3
#define kMarkReadItem			5
#define kMarkUnreadItem			6
#define kPostItem				8
#define kFollowUpItem			9
#define kReplyViaEmailItem		10
#define kSendItem				11
#define kCheckNewArticlesItem	13

#define kNumNewsMenuItems		13

#define kNextArticleMask		(1L << kNextArticleItem)
#define kNextThreadMask			(1L << kNextThreadItem)
#define kNextGroupMask			(1L << kNextGroupItem)
#define kMarkReadMask			(1L << kMarkReadItem)
#define kMarkUnreadMask			(1L << kMarkUnreadItem)
#define kPostMask				(1L << kPostItem)
#define kFollowUpMask			(1L << kFollowUpItem)
#define kReplyViaEmailMask		(1L << kReplyViaEmailItem)
#define kSendMask				(1L << kSendItem)
#define kCheckNewArticlesMask	(1L << kCheckNewArticlesItem)

#define kNoneNewsEnabled		(kEntireMenuMask | kPostMask)
	
#define kDANewsEnabled			(kEntireMenuMask | kPostMask)
	
#define kGroupNewsEnabled		(kEntireMenuMask | kNextArticleMask | \
	kNextThreadMask | kNextGroupMask | kPostMask)
	
#define kNewGroupNewsEnabled	(kEntireMenuMask | kNextArticleMask | \
	kNextThreadMask | kNextGroupMask | kPostMask)
	
#define kUserGroupNewsEnabled	(kEntireMenuMask | kNextArticleMask | \
	kNextThreadMask | kNextGroupMask | kMarkReadMask | kMarkUnreadMask | \
	kPostMask | kCheckNewArticlesMask)
	
#define kSubjectNewsEnabled		(kEntireMenuMask | kNextArticleMask | \
	kNextThreadMask | kNextGroupMask | kMarkReadMask | kMarkUnreadMask | \
	kPostMask)
	
#define kArticleNewsEnabled		(kEntireMenuMask | kNextArticleMask | \
	kNextThreadMask | kNextGroupMask | kMarkReadMask | kMarkUnreadMask | \
	kFollowUpMask | kReplyViaEmailMask | kPostMask)
	
#define kMiscArticleNewsEnabled	(kEntireMenuMask | kNextArticleMask | \
	kNextThreadMask | kNextGroupMask | \
	kFollowUpMask | kReplyViaEmailMask | kPostMask)
	
#define kMailMessageNewsEnabled (kEntireMenuMask | kPostMask | \
	kSendMask)
	
#define kPostMessageNewsEnabled	(kEntireMenuMask | kPostMask | \
	kSendMask)
	
#define kStartupBadNewsEnabled	0



/* Search menu */

#define	kSearchMenu				132

#define kFindItem				1
#define kFindAgainItem			2
#define kSearchItem				4
#define kGroupKillItem			6
#define kKillAuthorItem			7
#define kKillSubjectItem		8

#define kNumSearchMenuItems		8

#define kFindMask				(1L << kFindItem)
#define kFindAgainMask			(1L << kFindAgainItem)
#define kSearchMask				(1L << kSearchItem)
#define kGroupKillMask			(1L << kGroupKillItem)
#define kKillAuthorMask			(1L << kKillAuthorItem)
#define kKillSubjectMask		(1L << kKillSubjectItem)

#define kNoneSearchEnabled			0

#define kDASearchEnabled			0
	
#define kGroupSearchEnabled			(kEntireMenuMask | kFindMask | kFindAgainMask | \
									kSearchMask | kGroupKillMask)
	
#define kNewGroupSearchEnabled		(kEntireMenuMask | kFindMask | kFindAgainMask | \
									kSearchMask | kGroupKillMask)
	
#define kUserGroupSearchEnabled		(kEntireMenuMask | kFindMask | kFindAgainMask | \
									kSearchMask | kGroupKillMask)
	
#define kSubjectSearchEnabled		(kEntireMenuMask | kFindMask | kFindAgainMask | \
									kGroupKillMask | kKillSubjectMask)
	
#define kArticleSearchEnabled		(kEntireMenuMask | kFindMask | kFindAgainMask | \
									kGroupKillMask | kKillAuthorMask | kKillSubjectMask)
	
#define kMiscArticleSearchEnabled	(kEntireMenuMask | kFindMask | kFindAgainMask | \
									kGroupKillMask | kKillAuthorMask | kKillSubjectMask)
	
#define kMailMessageSearchEnabled	(kEntireMenuMask | kFindMask | kFindAgainMask)
	
#define kPostMessageSearchEnabled	(kEntireMenuMask | kFindMask | kFindAgainMask)
	
#define kStartupBadSearchEnabled	0



/*	Special menu. */

#define kSpecialMenu			133

#define kSubscribeItem			1
#define kUnsubscribeItem		2
#define kCheckNewGroupsItem		4
#define kCheckDeletedGroupsItem	5
#define kRebuildFullGroupsItem	6
#define kOpenSelRefItem			8
#define kOpenAllRefsItem		9
#define kGetServerInfoItem		11

#define kNumSpecialMenuItems	11

#define kSubscribeMask			(1L << kSubscribeItem)
#define kUnsubscribeMask		(1L << kUnsubscribeItem)
#define kCheckNewGroupsMask		(1L << kCheckNewGroupsItem)
#define kCheckDeletedGroupsMask	(1L << kCheckDeletedGroupsItem)
#define kRebuildFullGroupsMask	(1L << kRebuildFullGroupsItem)
#define kOpenSelRefMask			(1L << kOpenSelRefItem)
#define kOpenAllRefsMask		(1L << kOpenAllRefsItem)
#define kGetServerInfoMask		(1L << kGetServerInfoItem)

#define kNoneSpecialEnabled	(kEntireMenuMask | kCheckNewGroupsMask | \
	kCheckDeletedGroupsMask | kRebuildFullGroupsMask | kGetServerInfoMask)

#define kDASpecialEnabled		(kEntireMenuMask | kCheckNewGroupsMask | \
	kCheckDeletedGroupsMask | kRebuildFullGroupsMask | kGetServerInfoMask)
	
#define kGroupSpecialEnabled	(kEntireMenuMask | kSubscribeMask | \
    kCheckNewGroupsMask | kCheckDeletedGroupsMask | kRebuildFullGroupsMask | \
    kGetServerInfoMask)
	
#define kNewGroupSpecialEnabled	(kEntireMenuMask | kSubscribeMask | \
    kCheckNewGroupsMask | kCheckDeletedGroupsMask | kRebuildFullGroupsMask | \
    kGetServerInfoMask)
	
#define kUserGroupSpecialEnabled	(kEntireMenuMask | kUnsubscribeMask | \
    kCheckNewGroupsMask | kCheckDeletedGroupsMask | kRebuildFullGroupsMask | \
    kGetServerInfoMask)
	
#define kSubjectSpecialEnabled	(kEntireMenuMask | \
    kCheckNewGroupsMask | kCheckDeletedGroupsMask | kRebuildFullGroupsMask | \
    kGetServerInfoMask)
	
#define kArticleSpecialEnabled	(kEntireMenuMask | \
    kCheckNewGroupsMask | kCheckDeletedGroupsMask | kRebuildFullGroupsMask | \
    kOpenSelRefMask | kOpenAllRefsMask | kGetServerInfoMask)
	
#define kMiscArticleSpecialEnabled	(kEntireMenuMask | \
    kCheckNewGroupsMask | kCheckDeletedGroupsMask | kRebuildFullGroupsMask | \
    kOpenSelRefMask | kOpenAllRefsMask | kGetServerInfoMask)
	
#define kMailMessageSpecialEnabled (kEntireMenuMask | \
    kCheckNewGroupsMask | kCheckDeletedGroupsMask | kRebuildFullGroupsMask | \
    kGetServerInfoMask)
	
#define kPostMessageSpecialEnabled	(kEntireMenuMask | \
    kCheckNewGroupsMask | kCheckDeletedGroupsMask | kRebuildFullGroupsMask | \
    kGetServerInfoMask)
	
#define kStartupBadSpecialEnabled	0



/*	Windows menu. */

#define kWindMenu				134

#define kCycleItem				1
#define kZoomItem				2
#define kShowHideFullItem		4
#define kFirstWindOffset		6

#define kNumWindMenuItems		4

#define kCycleMask				(1L << kCycleItem)
#define kZoomMask				(1L << kZoomItem)
#define kShowHideFullMask		(1L << kShowHideFullItem)

#define kNoneWindEnabled		(kEntireMenuMask | kShowHideFullMask)

#define kDAWindEnabled			(kEntireMenuMask | kCycleMask | \
	kShowHideFullMask)
	
#define kGroupWindEnabled		(kEntireMenuMask | kCycleMask | \
	kZoomMask | kShowHideFullMask)
	
#define kNewGroupWindEnabled	(kEntireMenuMask | kCycleMask | \
	kZoomMask | kShowHideFullMask)
	
#define kUserGroupWindEnabled	(kEntireMenuMask | kCycleMask | \
	kZoomMask | kShowHideFullMask)
	
#define kSubjectWindEnabled		(kEntireMenuMask | kCycleMask | \
	kZoomMask | kShowHideFullMask)
	
#define kArticleWindEnabled		(kEntireMenuMask | kCycleMask | \
	kZoomMask | kShowHideFullMask)
	
#define kMiscArticleWindEnabled	(kEntireMenuMask | kCycleMask | \
	kZoomMask | kShowHideFullMask)
	
#define kMailMessageWindEnabled	(kEntireMenuMask | kCycleMask | \
	kZoomMask | kShowHideFullMask)
	
#define kPostMessageWindEnabled	(kEntireMenuMask | kCycleMask | \
	kZoomMask | kShowHideFullMask)
	
#define kStartupBadWindEnabled	0


void SetMenusTo (
	Boolean newAppleMenuEnabled,
	unsigned long newFileMenuState, 
	unsigned long newEditMenuState,
	unsigned long newNewsMenuState, 
	unsigned long newSearchMenuState,
	unsigned long newSpecialMenuState, 
	unsigned long newWindMenuState);
	
void AdjustMenus (void);

