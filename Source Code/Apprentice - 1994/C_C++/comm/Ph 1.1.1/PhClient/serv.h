/*______________________________________________________________________

	serv.h - Ph Server Interface.
_____________________________________________________________________*/

#ifndef __serv__
#define __serv__

#include "glob.h"

/*_____________________________________________________________________

	Defines.
_____________________________________________________________________*/

#define phInProgress		100			/* Ph server response codes */
#define phEcho				101
#define phMatchCount		102
#define phSuccess			200
#define phReadOnly		201
#define phMoreInfo		300
#define phEncrypt			301
#define phTempErr			400
#define phInternalErr	401
#define phLockTimeout	402
#define phFailReadOnly2	403
#define phUnavailable	475
#define phPermErr			500
#define phNoMatches		501
#define phManyMatches	502
#define phNoAuthInfo		503
#define phNoAuthSearch	504
#define phNoAuthField	505
#define phMustLogin		506
#define phNoSuchField	507
#define phFieldNotThere	508
#define phDupAlias		509
#define phNoAuthChange	510
#define phNoAuthAdd		511
#define phIllegalVal		512
#define phUnknownOpt		513
#define phUnknownCmd		514
#define phNoInxField		515
#define phNoAuthRequest	516
#define phFailReadOnly	517
#define phTooManyChange	518
#define phCPULimit		520
#define phAddOnly			521
#define phViewEncrypt	522
#define phClearAnswer	523
#define phBadHelp			524
#define phDatabaseOff	555
#define phCmdUnknown		598
#define phSyntax			599

/*_____________________________________________________________________

	Functions.
_____________________________________________________________________*/

extern OSErr serv_Login (Str255 server, Str255 loginUser, Str255 pswd,
	Str255 loginAlias, FieldInfo ***fields, short *numFields,
	short *sCode, Handle *proxyList, short *proxyCode);
extern OSErr serv_GetRecord (Str255 server, Str255 loginUser, Str255 pswd, 
	Str255 user, FieldInfo ***fields, short *numFields, short *sCode);
extern OSErr serv_PutRecord (Str255 server, Str255 loginAlias, Str255 pswd,
	Str255 alias, FieldInfo **fields, short numFields, short *sCode);
extern OSErr serv_ChangePassword (Str255 server, Str255 loginAlias, 
	Str255 pswd, Str255 alias, Str255 newPswd, short *sCode,
	Str255 servErrMsg);
extern OSErr serv_CreateRecord (Str255 server, Str255 loginAlias, 
	Str255 loginPswd, Str255 alias, Str255 name, Str255 type, Str255 pswd, 
	FieldInfo ***fields, short *numFields, short *sCode, short *whichField,
	Str255 servErrMsg);
extern OSErr serv_DeleteRecord (Str255 server, Str255 loginAlias,
	Str255 loginPswd, Str255 alias, short *sCode);
extern OSErr serv_DoQuery (Str255 server, Handle query,
	unsigned short queryLen, Str255 emailDomain, Handle reply);
extern OSErr serv_GetHelp (Str255 server, Str255 topic, Handle reply);
extern void serv_GetReadOnlyReason (Str255 reason);
extern OSErr serv_GetSiteList (Str255 server);
extern OSErr serv_GetHelpList (Str255 server);

#endif