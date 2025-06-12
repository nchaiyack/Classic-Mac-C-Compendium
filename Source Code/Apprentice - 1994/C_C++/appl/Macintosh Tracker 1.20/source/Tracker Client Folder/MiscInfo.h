/* MiscInfo.h */


/********************************************************************************/
/* generic includes */

//#define DEBUG /* debugging macros */
//#define ALWAYSRESUME /* resume button availability */
//#define THINKC_DEBUGGER /* break points in PRERR */
//#define AUDIT /* audit file generation */
//#define MEMDEBUG /* handle/ptr allocation checking */

#define _VIATIMER_ /* use VIA when profiling (comment out for ticks) */
#pragma options(!class_names) /* make data segment smaller */
#include "Utilities.h"


/********************************************************************************/
/* file creator information */

#define CREATORCODE ('ºTcp')

#define NUMFILETYPES (1)
#define FILETYPE1 ('ºTcd')

#define FILETYPELIST {FILETYPE1}


/********************************************************************************/
/* memory consumption information */

#define NumMasters (50)

#define PRIMARYMEMCACHESIZE (16384)
#define SECONDARYMEMCACHESIZE (8192)
#define USETEMPMEM (False)
#define GRABTEMPMEM (False)


#define STACKSIZE (8192)


/********************************************************************************/
/* audit file creator */

#define AUDITCREATOR ('KAHL') /* THINK C creator */

/********************************************************************************/
/* profile information */

#define ProfileNumSymbols (1000)  /* max number of symbols (functions) */

#define ProfileNumLevels (100)  /* max number of nested function calls */

