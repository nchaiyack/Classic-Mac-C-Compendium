/* MiscInfo.h for DirTree */

#ifndef Included_MiscInfo_h
#define Included_MiscInfo_h

/********************************************************************************/
/* control flags */

/* to enable one of the following debugging features, but a 1 in the definition. */
/* to disable, put a 0.  Do NOT use macros True and False since they may not be */
/* defined when this file is read. */
#define DEBUG (1)  /* debugging macros */
#define ALWAYSRESUME (1)  /* resume button availability */
#define DEBUGGER_PRESENT (1)  /* break points in PRERR */
#define AUDIT (0)  /* audit file generation */
#define AUDITFLUSHING (0)  /* flush AUDIT file after each write (very slow) */
#define MEMDEBUG (0)  /* pointer allocation checking */


/********************************************************************************/
/* application information */

#define VersionString "1.00"

#define ApplicationCreator ('\xba'*(1L<<24) + '\D'*(1L<<16) + 'T'*(1L<<8) + 'r')


/********************************************************************************/
/* audit file creator */

#define AUDITCREATOR ('KAHL') /* THINK C creator */


/********************************************************************************/

#endif
