#pragma once
/*****
 *
 *	WindowInt.h
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

#define kMinWindSize	48


/***  TYPE DECLARATIONS  ***/

typedef enum
{
	Window_UNKNOWN,				/* unrecognized type of window */
	
	Window_none,				/* not a window */
	Window_dlgModal,			/* modal dialog */
	Window_dlgMoveableModal,	/* moveable modal dialog */
	Window_about,				/* about box */
	Window_DA					/* desk accessory window */
} window_type;
/*
Window_UNKNOWN
Window_none
Window_dlgModal
Window_dlgMoveableModal
Window_about
Window_DA
*/
	
typedef struct
{
	window_type		wType;
	Boolean			scrollbars;
	Handle			dataHdl;
} windowInfo;

typedef windowInfo** windowInfoHdl;


/***  FUNCTION PROTOTYPES  ***/

	void			WindowMenuClose		( short );
	
	void			WindowActivate		( WindowPtr, Boolean, const EventRecord * );
	
	void 			WindowContentClick	( WindowPtr, const EventRecord *);
	void			WindowZoomBox		( WindowPtr, short );
	void 			WindowGrow			( WindowPtr, Point );
	
	Boolean			WindowClose			( WindowPtr, short, Boolean );
	Boolean			WindowCloseAll		( Boolean );
	
	window_type		WindowType			( WindowPtr );
	
	windowInfoHdl	WindowNewInfoHdl	( window_type, Boolean, Handle );


/***** EOF *****/
