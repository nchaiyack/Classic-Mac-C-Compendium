/*______________________________________________________________________

	scan.h - Scanning Module Interface.
	
	Copyright © 1988, 1989, 1990, Northwestern University.
_____________________________________________________________________*/

#ifndef __scan__
#define __scan__

#ifndef __glob__
#include "glob.h"
#endif

extern Boolean scan_CheckCancel(void);
extern Boolean scan_CheckFull (Handle report, short slop);
extern void scan_DoScan (ScanKind scanKind, ScanOp scanOp);
extern void scan_DoButton (ScanOp scanOp, short modifiers);


#endif

