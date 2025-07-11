/*______________________________________________________________________

	rslv.h - Domain name resolver Interface.
_____________________________________________________________________*/


#ifndef __rslv__
#define __rslv__

/*_____________________________________________________________________

	Constants.
_____________________________________________________________________*/

#define rslvNameSyntaxErr		-24000		/* syntax error in domain name */
#define rslvNoServers			-24001		/* no domain name servers defined */
#define rslvNoResponse			-24002		/* no response from servers */
#define rslvCancel				-24003		/* canceled by user */
#define rslvNoSuchDomain		-24004		/* no such domain name */
#define rslvServError			-24005		/* server error (internal use) */
#define rslvNotOurQuery			-24006		/* not our query (internal use) */

/*_____________________________________________________________________

	Types.
_____________________________________________________________________*/

typedef Boolean (*rslv_CheckCancelPtr)(void);

/*_____________________________________________________________________

	Functions.
_____________________________________________________________________*/

extern unsigned char *rslv_DottedDecimal (unsigned long addr, 
	unsigned char *str);
extern OSErr rslv_Init (short refNum);
extern OSErr rslv_Resolve (Str255 name, 
	rslv_CheckCancelPtr checkCancel, unsigned long *addr);

#endif