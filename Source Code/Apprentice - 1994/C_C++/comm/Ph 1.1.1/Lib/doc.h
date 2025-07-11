/*______________________________________________________________________

	doc.h - Report Module Interface.
	
	Copyright � 1988-1991 Northwestern University.
	
	The definitions in this module are used by the rep.c, rpp.c, ldf.c, 
	hlp.c, and cvrt.c modules.  They are private to these modules.  
	This header file should not be included or used by any other modules.
_____________________________________________________________________*/

#ifndef __doc__
#define __doc__

/* For type 1 reports the userHandle field of the list record is used to
	store a handle to auxiliary information used to optimize the 
	performance of the list definition function.  The following typedef 
	describes this information. */
	
typedef struct auxInfo {
	short			cachedPictID;	/* resource id of cached picture, 
											or 0 if none */
	BitMap		cachedBitMap;	/* cached picture bitmap */
	Handle		auxArray[1];	/* array of handles to STR# resources -
											expanded by rep_Init to whatever size
											is required, and terminated with a zero
											entry */
} auxInfo;

/* Escape codes used in STR# lines. */

#define	docStyle		0
#define	docJust		1
#define	docSize		2
#define	docOnly		3
#define	docPict		4
#define	docPage		5
#define	docKeep		6
#define	docEndKeep	7
#define	docITcon		8

/* End-of-paragraph marker. */

#define	docEop		31

/* Justification constants used in STR# escape sequences. */

#define	docLeft		0
#define	docCenter	1
#define	docRight		2

/* Masks for STR# "only" escape sequences. */

#define	docScreen	1
#define	docPrint		2
#define	docSave		4

#endif