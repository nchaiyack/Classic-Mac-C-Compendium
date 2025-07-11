/*
 *	File:		CWASTEEditTask.h
 *	System:		Mark's Class Library
 *	Evironment:	MacOS 7.0/THINK C 7.0/TCL 2.0
 *	Author:		Mark Alldritt
 *
 *
 *		Copyright � 1994    All Rights Reserved
 *		Mark Alldritt
 *		1571 Deep Cove Road
 *		N. Vancouver, B.C. CANADA
 *
 *	
 *	Notes:
 *
 *	none.
 *
 *
 *	Edit History:
 *
 *	V1-0.0	Mark Alldritt	10-May-1994
 *	- Initial version of module.
 *  V1.0.1  Modifications by Dan Crevier to TCL 2.0
 *  V1.0.2  Added code by Mark Alldritt for undo/redo
 *
 */

#define _H_CWASTEEditTask
#include "CTextEditTask.h"

class CWASTEText;

class CWASTEEditTask : public CTextEditTask
{
public:

				 	CWASTEEditTask(CWASTEText *aTextPane, long anEditCmd, short firstTaskIndex);
				 	~CWASTEEditTask();
/*	virtual void 	Undo(void);   */
/*	virtual void 	Redo(void); */
	
	// hack 'cause I needed it -- DWC
	tTextRange		*GetInserted(void) { return &inserted; };

protected:

	StScrpHandle	origStyleScrap;		/* original styles on clipboard */
	StScrpHandle	deletedStyles;		/* styles of original text	*/
	StScrpHandle	insertedStyles;		/* styles of inserted text	*/
	
	virtual void 	DoBackspace(void);
	virtual void 	DoFwdDelete(void);

	virtual void 	SaveRange(tRangeSelector whichRange);
	virtual void 	RestoreRange(tRangeSelector whichRange, Boolean killData);
	virtual void 	StoreToClip(tClipSelector whichClip);
	
	virtual Boolean CheckNewStyle(ScrpSTElement *scrapEl, 
								  StScrpHandle styleH, Boolean atStart);

};