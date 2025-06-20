/*
	File:		FinderRegistry.h

	Contains:	Classes, properties and data types used by the Scriptable Finder

	Written by:	Francis Stanbach & Greg Anderson

	Copyright:	� 1991-4 by Apple Computer, Inc., all rights reserved.

	Change History (most recent first):


	To Do:
*/

#ifndef __FINDERREGISTRY__
#define __FINDERREGISTRY__
#define FinderRegistry_h

////////////////////////////////////////
// Gestalt Selector
////////////////////////////////////////

#define gestaltFinderSelector				'fndr'

#define gestaltFinderCallsAEProcess			2
#define gestaltOSLCompliantFinder			3

////////////////////////////////////////
// Finder Suite
////////////////////////////////////////

//
// The old Finder Event suite was 'FNDR'
// The new suite is 'fndr'
//
#define kAEFinderSuite			'fndr'


////////////////////////////////////////
// Finder Events
////////////////////////////////////////

#define kAECleanUp				'fclu'
#define kAEEject				'ejct'
#define kAEEmpty				'empt'
#define kAEErase				'fera'
#define kAEGestalt				'gstl'
#define kAEPutAway				'ptwy'
#define kAERebuildDesktopDB		'rddb'
#define kAERestart				'rest'
#define kAEShutDown				'shut'
#define kAESnooze				'snoz'				// Go to sleep
#define kAEUpdate				'fupd'

////////////////////////////////////////
// Classes
////////////////////////////////////////

#define cFinder					cApplication		// the finder object � null

#define cDisk					'cdis'
#define cFolder					'cfol'
#define cTrash					'ctrs'
#define cDesktop				'cdsk'
#define cContainer				'ctnr'				// Disks, folders, trashes, desktops, suitcases
#define cSharableContainer		'sctr'				// Disks and folders
#define cSuitcase				'stcs'
#define cFontSuitcase			'fsut'
#define cAccessorySuitcase		'dsut'

#define cAliasFile				'alia'

#define cApplicationFile		'appf'
#define cDocumentFile			'docf'
#define cControlPanel			'ccdv'
#define cDeskAccessoryFile		'dafi'
#define cSoundFile				'sndf'
#define cFontFile				'fntf'

#define cDemiWindow				'dwnd'				// Content space:  includes desktop window & other windows
#define cContainerWindow		'cwnd'
#define cInfoWindow				'iwnd'
#define cSharingWindow			'swnd'
#define cStatusWindow			'qwnd'				// Empty trash progress, 'copy' window

#define cPrivilege				'priv'
#define cGestalt				'cgst'
#define cProcess				'prcs'				// Information about a process running on this machine (like cApplication)
#define cAccessoryProcess		'pcda'
#define cApplicationProcess		'pcap'

#define cEntireContents			'ects'

#define cIconFamily				'ifam'

//
// User and groups:
//
#define cUser					'cuse'
#define cGroup					'sgrp'

////////////////////////////////////////
// Properties
////////////////////////////////////////

//
// Properties of cObject:
//
#define pComment				'comt'
#define pContainer				cContainer			// The container the object is stored in
#define pCreationDate			'crtd'
#define pDemiWindow				cDemiWindow
#define pDisk					cDisk				// the disk the object is stored on
#define pIconBitmap				'iimg'
#define pInformationWindow		cInfoWindow			// Get info window
#define pStatusWindow			cStatusWindow		// Status window (copy, empty trash, rebuild desktop database)
#define pIsLocked				'islk'
#define pIsSelected				'issl'
#define pKind					'kind'
#define pModificationDate		'modd'
#define pPhysicalSize			'phys'
#define pPosition				'posn'
#define pSize					pPointSize			// both are called "Size" in their respective terminologies, so they must have the same four-character code
#define pWindow					cWindow

//
// Properties of cFile:
//
#define pCreatorType			'fcrt'
#define pFileType				'fitp'
#define pVersion2				'ver2'

//
// Properties of cAliasFile:
//
#define pOriginalItem			'orig'				// Item pointed to by alias

//
// Properties of cApplicationFile:
//
#define pAppPartition			'appt'
#define pSuggestedAppPartition	'sprt'
#define pMinAppPartition		'mprt'

//
// Properties of cContainer:
//
#define pContainerWindow		cContainerWindow	// Window that contains children
#define pEntireContents			cEntireContents
#define pLabelIndex				'labi'
#define pExpanded				'pexp'				// Same as kAEExpanded
#define pExpandable				'pexa'
#define pCompletelyExpanded		'pexc'
#define pSharingWindow			cSharingWindow		// Sharing window
#define pView					'pvew'
#define pPreviousView			'svew'				// The last listwindow (non-icon) view

#define pSharing				'shar'
#define pSharingProtection		'spro'
#define pExported				'sexp'
#define pMounted				'smou'
#define pInheritedPrivileges	'iprv'				// "Same as enclosing" checkbox
#define pOwner					'sown'
#define pGroup					cGroup
#define pOwnerPrivileges		'ownr'
#define pGroupPrivileges		'gppr'
#define pGuestPrivileges		'gstp'

//
// Properties of cDisk:
//
#define pCapacity				'capa'
#define pEjectable				'isej'
#define pFreeSpace				'frsp'
#define pLocal					'isrv'
#define pIsStartup				'istd'

//
// Properties of cDesktop:
//
#define pTrash					'trsh'				// Can't use cTrash, and pTrash must == kTrashFolderType
#define pStartupDisk			'sdsk'

//
// Properties of cTrash:
//
#define pWarnOnEmpty			'warn'

//
// Properties of cFinder / cApplication:
//
#define pAppleMenuItemsFolder	'amnu'				// kAppleMenuFolderType
#define pControlPanelsFolder	'ctrl'				// kControlPanelFolderType
#define pDesktop				'desk'				// Can't use cDesktop, and pDesktop must == kDesktopFolderType
#define pExtensionsFolder		'extn'				// kExtensionFolderType
#define pFileShareOn			'fshr'
#define pFileShareStartingUp	'fsup'
#define pFontsFolder			'ffnt'

#define pLargestFreeBlock		'mfre'
#define pPreferencesFolder		'pref'				// kPreferencesFolderType
#define pAboutMacintosh			'abbx'				// Open this to get 'About this Macintosh' window
#define pShortcuts				'scut'				// Finder shortcuts in help menu
#define pShutdownFolder			'shdf'
#define pStartupItemsFolder		'strt'				// kStartupFolderType
#define pSystemFolder			'macs'				// kSystemFolderType
#define pTemporaryFolder		'temp'				// kTemporaryFolderType
#define pTimerItemsFolder		'timf'
#define pViewPreferences		'pvwp'				// Finder view preferences ("Views" control panel)

//
// View preferences properties
//
#define pViewFont				'vfnt'
#define pViewFontSize			'vfsz'
#define pGridIcons				'fgrd'
#define pListViewIconSize		'lvis'
#define pShowComment			'scom'
#define pShowDate				'sdat'
#define pShowDiskInfo			'sdin'
#define pShowFolderSize			'sfsz'
#define pShowKind				'sknd'
#define pShowLabel				'slbl'
#define pShowSize				'ssiz'
#define pShowVersion			'svrs'
#define pStaggerIcons			'fstg'

//
// Properties of cPrivilegs
//
#define pSeeFiles				'prvr'
#define pSeeFolders				'prvs'
#define pMakeChanges			'prvw'

//
// Properties of cProcess
//
#define pApplicationFile		cApplicationFile

#define pDeskAccessoryFile		cDeskAccessoryFile
#define pIsScriptable			'isab'
#define pLocalAndRemoteEvents	'revt'
#define pPartitionSpaceUsed		'pusd'

#define pFolder					cFolder
#define pObject					cObject
#define pSharableContainer		cSharableContainer
#define pSuitcase				cSuitcase
#define pFontSuitcase			cFontSuitcase
#define pAccessorySuitcase		cAccessorySuitcase


////////////////////////////////////////
// Enumerations defined by the Finder
////////////////////////////////////////

#define enumViewBy				'vwby'

#define pSmallIcon				'smic'

#define enumGestalt				'gsen'

#define	enumConflicts			'cflc'
#define enumExistingItems		'exsi'

////////////////////////////////////////
// Types defined by the Finder
////////////////////////////////////////

#define typeIconFamily			cIconFamily			// An AEList of typeIconAndMask, type8BitIcon, & c.
#define typeIconAndMask			'ICN#'
#define type8BitIcon			'icl8'
#define type4BitIcon			'icl4'
#define typeSmallIconAndMask	'ics#'
#define typeSmall8BitIcon		'ics8'
#define typeSmall4BitIcon		'ics4'

////////////////////////////////////////
// Keywords defined by the Finder
////////////////////////////////////////

#define keyIconAndMask			'ICN#'
#define key8BitIcon				'icl8'
#define key4BitIcon				'icl4'
#define keySmallIconAndMask		'ics#'
#define keySmall8BitIcon		'ics8'
#define keySmall4BitIcon		'ics4'

#define keyAEUsing				'usin'
#define keyAEReplacing			'alrp'

////////////////////////////////////////
// New prepositions used by the Finder
////////////////////////////////////////

#define keyASPrepositionHas		'has '
#define keyAll					'kyal'
#define keyOldFinderItems		'fsel'

////////////////////////////////////////
// New key forms used by the Finder
////////////////////////////////////////

#define formAlias				typeAlias
#define formCreator				pCreator


////////////////////////////////////////
// Finder error codes
////////////////////////////////////////

#define errFinderIsBusy					-15260
#define errFinderWindowNotOpen			-15261
#define errFinderCannotPutAway			-15262
#define errFinderWindowMustBeIconView	-15263		// RequireWindowInIconView
#define errFinderWindowMustBeListView	-15264		// RequireWindowInListView
#define errFinderCantMoveToDestination	-15265
#define errFinderCantMoveSource			-15266
#define errFinderCantOverwrite			-15267
#define errFinderIncestuousMove			-15268		
#define errFinderCantMoveToAncestor		-15269		
#define errFinderCantUseTrashedItems	-15270
#define errFinderItemAlreadyInDest		-15271		// Move from folder A to folder A
#define errFinderUnknownUser			-15272		// Includes unknown group
#define errFinderSharePointsCantInherit	-15273

#define errFinderBoundsWrong			-15278
#define errAEValueOutOfRange			-15279

#endif
