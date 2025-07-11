#pragma once
/*****
 *
 *	constants.h
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

/*	RESOURCE TYPES	*/

#define krtErrStr				'Estr'
#define krtVersion				'vers'


/*  CHARACTER CODES  */

/* ASCII */
#define kHomeKey				0x01
#define kEnterKey				0x03
#define kEndKey					0x04
#define kHelpKey				0x05
#define kDeleteKey				0x08
#define kTabKey					0x09
#define kPageUpKey				0x0B
#define kPageDownKey			0x0C
#define kReturnKey				0x0D
#define kEscapeKey				0x1B
#define kLeftArrow				0x1C
#define kRightArrow				0x1D
#define kUpArrow				0x1E
#define kDownArrow				0x1F
#define kPeriodKey				0x2E
#define kForwardDelKey			0x7F

#define kBulletChar				'�'

/* Special Characters and Strings */
#define CR						'\r'
#define LF						'\n'
#define	CRSTR					"\r"
#define	LFSTR					"\n"
#define CRLF					"\r\n"
#define CRCR 					"\r\r"


/* APPLE EVENTS */

#define kAEClassCGI			'WWW�'
#define kAEIDSearchDoc		'sdoc'


/* ERRORS */

#define kerrStartupDefault		1
#define kerrStartupMemory		2
#define kerrStartupGestalt		3
#define kerrStartupSysEnv		4
#define kerrStartupWNE			5
#define kerrStartupAppleEvent	6
#define kerrStartupMenu			7


/* MISCELANEOUS */

#define kVisualDelay			8

#define kSleepTicks				20	/* default setting */


/***** EOF *****/
