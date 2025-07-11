/*

Note: to avoid confusion between the two types of threads, I calling the
threads on the article level for discussions in the source.

 // basic MacApp ----------------------------------------------------------------------
class TNewsAppl								// subclass of TApplication
class TGroupDoc								// document for a group (main window is list of threads)
class TThread									// see "d e v e l o p" issue 6
class TGroupListDoc						// a ListOfSubscribedGroups
class TGroupTreeDoc						// for managing the tree. Has 0 to � windows, always open

// main views -------------------------------------------------------------------------
class TGroupListView					// show TGroupList
class TGroupTreeView					// show TGroupTree

class TDiscListView						// shows the list of threads

class TArticleListView				// for managing multiple articles, eg selection of multiple
class TArticleView						// shows one article
class TNavigatorView					// for showing the box
class TNavigatorArrowView			// for showing one arrow

// internal data structs --------------------------------------------------------------
class TGroupList							// list of subscribed groups
class TGroupTree							// hierarchical group list

class TArticle								// for managing an article, eg GetHeader, GetLine
class TArticleCache						// caches TArticle's

class TArticleStatus					// which articles have you read/seen
class TDiscList								// keeps the list of CDiscussion (see the headerfiles)
class CDiscussion							// management of one discussion
class THashTable							// for finding msg-id's
class THeaderList							// header database

// nntp stuff -------------------------------------------------------------------------
class TArticleTextCache				// caches article bodies to implement "virtual memory"
class TNntpCache							// caches nntp connections as they take time to establish
class TNntp										// base for TFakeNntp and TRealNntp
class TFakeNntp								// simulates an nntp connection at my home mac
class TRealNntp								// handles real-nntp commands and nntp-errors
class TNetAsciiProtocol				// for parsing respond codes, dot-terminated text
class TMacTcp									// does mactcp stuff

// Posting ----------------------------------------------------------------------------
class TCreateNewDiscussionCommand		// creates the dialog and finds initial headers
class TCreateFollowupCommand				// subclass of the above for making a follow-up

class TEditArticleToPostDialogView	// dialog with subject, groups & distribution
class TEditArticleToPostCommand 		// lauches the editor and gets it to open the article

class TPostArticleDialogView	// dialog with "Post" key
class TPostArticleCommand			// does the actual posting
class TPostArticleInfo				// global variable keeper for "post article" classes

// ProgressMeter ----------------------------------------------------------------------
class TProgress								// main progress class
class TProgressMeterView			// the thermometer view
class TProgressDialogView			// handles stop/cancel button
class TProgressCache					// caches TProgress's to speed up creating of threads

// tools ------------------------------------------------------------------------------
class TDynDynList							// for maintaining "an array where the elements can have
															   different size"
class TStatDynList						// subclass of TDynDynList, keeps list of free items
															   Deleted items are reused instaed of being removed
class TOffsetLengthList				// array of OffsetLength

class TObjectCache						// for caching objects that can be reused
// Preferences views ------------------------------------------------------------------
class TNewsPrefDialogView			// news server
class TBinariesPrefDialogView	// extract of binaries
class TEditorPrefDialogView 	// which editor and folder for articles
class TPrefRammeView					// for a frame like those around groups of radio-buttons


// misc views -------------------------------------------------------------------------
class TFileSelectView					// SFGetFile
class TFolderSelectView				// the ever missing SFGetFolder
class TApplicationSelectView	// for choosing prefered editor
class TViewListView						// list of views like list of text in TTextListView


// Preferences database ---------------------------------------------------------------
class TPrefsDoc								// preferences manager, always open and has no windows
class TResFile								// improved TFile for handling resources

// Improved scroller ------------------------------------------------------------------
class TTextScroller						// for better-page-down handling and cursor-block keys
class TSuperScrollBar					// for TTextScroller

// View/Target list windows -----------------------------------------------------------
class TIdler									// all taken from DemoDialogs
class TObjectListView
class TTargetChainView
class TViewHierarchyView

// Commands ---------------------------------------------------------------------------
class TArticleTextSelectCommand
class TCopyArticleTextCommand
class TExecuteCommandInNewThreadCommand
class TExpandGroupTracker
class TExtractBinariesCommand
class TGroupTracker
class TImprovedRowSelectCommand
class TMultiSuperNextKeyCommand
class TOpenArticleCommand
class TOpenGroupCommand
class TOpenPrefsCommand
class TSaveArticlesCommand
class TShowDiscJunkCommand
class TStickySelectCommand
class TSubscribeGroupTracker
class TUnsubscribeGroupTracker
class TUpdateGroupCommand
class TUpdatePrefsCommand

class TGroupTracker
class TGroupViewKeyCommand
class TGroupViewTabKeyCommand
class TGroupViewTypeNameCommand
class TListTabKeyCommand
class TListTypeName
class TTreeTabKeyCommand
class TTreeTypeName

// Iterators ---------------------------------------------------------------------------
class CFindSubGroupIterator		// for TGroupTree
class CRealGroupIterator			// for TGroupTree

class CDiscListArticleIterator 			// for TDiscListView
class CArticleViewIterator					// for TArticleListView
class CSelectedArticleViewIterator	// for TArticleListView

*/