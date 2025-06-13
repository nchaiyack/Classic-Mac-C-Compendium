/*
** File:		Constants.h
**
** Written by:	Bill Hayden
**				Nikol Software
**
** Copyright © 1995 Nikol Software
** All rights reserved.
*/


/*
** This file contains almost all the defined constants used by MacWT in one
** convenient location.
*/


/*****************************************************************************/


/*
** Menu Constants
*/

#define rMenuBar		128

#define	mApple			128
#define	kAbout			1

#define mFile			129
#define kNew			1
#define kShowFPS		3
#define kUseQuickdraw	4
#define kPause			5
#define kQuit			7

#define mEdit			130
#define kUndo			1
#define kCut			3
#define kCopy			4
#define kPaste			5
#define kClear			6


/*
** Application Start-up
*/

#define kMinHeap	64 * 1024		/* Needs at least 64k of heap space. */
#define kMinSpace	64 * 1024		/* Needs this much after calling PurgeSpace. */
#define	kMinMem		(640<<10)		/* Absolute minimum memory required */

#define kFrontTime		0L
#define kBackTime		3600L


enum {kTab = 9, kSpace = 20, kEsc = 27};	/* Hacky ASCII codes */

enum {rScreenSize = 32764,
	  rFatalErrorAlert = 32765,
	  rAboutAlert = 32766,
	  rBadStartAlert = 32767};

enum {rWindowID = 200};

enum {rUtilityStrs = 128,
	  rStrPause = 1, rStrContinue};

enum {gotNoEvent, gotOtherEvent, gotKeyEvent};


#define	kBottomBorder	20


