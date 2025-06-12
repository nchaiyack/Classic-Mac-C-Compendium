/*______________________________________________________________________

	mtcp.h - MacTCP Interface.
_____________________________________________________________________*/


#ifndef __mtcp__
#define __mtcp__

/*_____________________________________________________________________

	Defines.
_____________________________________________________________________*/

#define mtcpCancel		17000			/* Operation canceled by user */

/*_____________________________________________________________________

	Functions.
_____________________________________________________________________*/

extern void mtcp_BeginTransaction (void);
extern void mtcp_CancelTransaction (void);
extern OSErr mtcp_Initialize (void);
extern OSErr mtcp_Terminate (void);
extern OSErr mtcp_OpenConnection (char *host);
extern OSErr mtcp_CloseConnection (void);
extern OSErr mtcp_AbortConnection (void);
extern OSErr mtcp_PhCommand (Handle command, Handle response, 
	Boolean *truncated);

#endif