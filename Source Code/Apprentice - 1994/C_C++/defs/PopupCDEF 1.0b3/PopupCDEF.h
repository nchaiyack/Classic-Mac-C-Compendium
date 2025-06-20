/* See the file Distribution for distribution terms.
	(c) Copyright 1994 Ari Halberstadt */

/* This file contains definitions for using my popup CDEF.
	94/03/15 aih - split out of PopupLib.h */

#pragma once

/* Version of this library. Useful for compatability between older versions
	of the CDEF and using a newer version of the popup library to manipulate
	the CDEF popups from within a program. The program must first check the
	popup's version before calling any popup library routines. */
#define kPopupVersion		(1)

/* Part code returned when the mouse is in a popup menu. */
#define kPopupPartCode		(1)

/*	The control variation code popupTypeIn is used by the popup CDEF.
	With this variation code, only the popup's down arrow is drawn,
	the popup's title and current selection are not drawn. Type-in
	popup menus are described in IM-VI, p2-37. */
#define kPopupTypeIn			(2)
#define popupTypeIn			(2)

/* The procID for the popup CDEF. You can use this value in calls
	to NewControl. */
#define kPopupProcID			(129 * 16)

/* This is the same structure as the popupPrivateData record described
	in IM-VI, p3-19. By making this the first element in the popup
	structure, the CDEF is made more compatible with Apple's CDEF. */
typedef struct {
	MenuHandle	mHandle;			/* handle to the popup's menu */
	short			mID;				/* id of popup's menu */
} PopupPrivateType, *PopupPrivatePtr, **PopupPrivateHandle;
