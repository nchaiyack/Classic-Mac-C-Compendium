/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�

	CTermPaneDef.h
	
	Commtoolbox terminal emulation class: local definitions
		
	Ithran Einhorn 1994
	
같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같� */
 
 
#include <CommResources.h>						/* Apple includes */ 

#include <CBartender.h>							/* TCL includes */
#include <CClipboard.h>
#include "CPtrArray.h"
#include <CError.h>
#include <Commands.h>
#include <Constants.h>
#include <CWindow.h>
#include <Global.h>
#include <LongQD.h>
#include <TBUtilities.h>
#include <TCLUtilities.h>
#include "CBitMap.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "stringx.h"

#include "CTermPane.h"							/* Other includes */

/* Constants & Macros */

#define TERM_STR_RES_ID		2200	/* Emulation messages resource ID */

#define NO_TOOL_STR_INDEX	1		/* No terminal tool */
#define BAD_TOOL_STR_INDEX	2		/* Bad terminal tool */
#define NO_REC_STR_INDEX	3		/* Terminal record allocation error */
#define CHOOSE_STR_INDEX	4		/* Tool setup error */

#define H_CHOOSE_POS		10		/* Setup dialog position */
#define V_CHOOSE_POS		40

#define SPACE				0x20	/* Space char */
#define TAB					0x09	/* Tab char */

#define CHAR_DELAY_TICKS	15

#define RESET_ALRT_ID		2200

#define CACHE_LINE_OFFSET	0		/* amount of vertical offset to apply to cache line */
#define TERMINAL_OFFSET		0		/* amount of vertical offset to apply to terminal */

#define kNumCachedLines		128		/* number of lines to cache */

#define RgnRect(rgn,rct)	((*rct)=(*rgn)->rgnBBox)
#define RectHeight(r)		((r)->bottom-(r)->top)
#define RectWidth(r)		((r)->right-(r)->left)

	/* mnemonic constants for Boolean parameters */
											
enum {
	kEraseText = TRUE,
	kDontEraseText = FALSE,
	kUseSelection = TRUE,
	kDontUseSelection = FALSE
};

/* Application globals */

extern CBartender	*gBartender;
extern CClipboard	*gClipboard;
extern CError		*gError;
extern EventRecord  gLastMouseUp;
extern short		gClicks;			/* Click counter					*/
