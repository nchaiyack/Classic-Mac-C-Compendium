#pragma once
/*****
 *
 *	menuFunc.h
 *
 *	This is a support file for "Grant's CGI Framework".
 *	Please see the license agreement that accompanies the distribution package
 *	for licensing details.
 *
 *	Copyright �1995 by Grant Neufeld
 *	grant@acm.com
 *	http://arpp1.carleton.ca/grant/
 *
 *****/

/***  CONSTANT DECLARATIONS  ***/

#define kmMenuBarID			128		/* resource ID of the main application menubar */
#define kmDefaultMenuStr	299
#define kmTheWholeMenu		0
#define kmAfterOtherMenus	0
#define kmSubMenu			-1

#define	kmAppleMenuID		128
#define	kmiAbout			1

#define	kmFileMenuID		129
#define kmiClose			1
#define	kmiQuit				3

#define	kmEditMenuID		130
#define	kmiUndo				1
#define	kmiCut				3
#define	kmiCopy				4
#define	kmiPaste			5
#define	kmiClear			6
#define	kmiSelectAll		7


/***  GLOBAL DECLARATIONS  ***/

#ifdef __MainSegment__
	#ifdef _GLOBAL_
		#undef _GLOBAL_
	#endif
	#define	_GLOBAL_
#else
	#ifdef _GLOBAL_
		#undef _GLOBAL_
	#endif
	#define	_GLOBAL_	extern
#endif


_GLOBAL_	MenuHandle 	gmAppleMenu;
_GLOBAL_	MenuHandle	gmFileMenu;
_GLOBAL_	MenuHandle	gmEditMenu;

_GLOBAL_	short		gOldMBarHeight;


#ifdef _GLOBAL_
#undef _GLOBAL_
#endif


/***  FUNCTION PROTOTYPES  ***/

	void	adjustMenus	( void );
	void	doMenu		( long, short );	/* menuResult, modifiers */


/***** EOF *****/
