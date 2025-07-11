/*_____________________________________________________________________

  	serv.c - PH Server Interface.
_____________________________________________________________________*/

#pragma load "precompile"
#include "rez.h"
#include "mtcp.h"
#include "serv.h"
#include "utl.h"
#include "encrypt.h"
#include "glob.h"
#include "tran.h"
#include "query.h"
#include "help.h"

#pragma segment serv

/*_____________________________________________________________________

	Constants.
_____________________________________________________________________*/

#define initHandleSize	1000			/* initial command and response buffer size */
#define cmdHandleInc		100			/* command buffer size increment */
#define updateInterval	(24*60*60)	/* one day = help and site list update interval */

/*_____________________________________________________________________

	Global Variables.
_____________________________________________________________________*/

static Handle		Command;						/* handle to server command */
static Handle		Response;					/* handle to server response */
static Boolean		Truncated;					/* true if server response truncated */
static Str255		ReadOnlyReason="\p";		/* reason server is read-only */

/*_____________________________________________________________________

	OpenConnection - Open Connection to Server.
	
	Entry:	server = Ph server host domain name.
_____________________________________________________________________*/

static OSErr OpenConnection (Str255 server)

{
	Command = NewHandle(initHandleSize);
	if (!Command) return MemError();
	Response = NewHandle(initHandleSize);
	if (!Response) return MemError();
	return mtcp_OpenConnection(server);
}

/*_____________________________________________________________________

	CloseConnection - Close Connection to Server.
_____________________________________________________________________*/

static OSErr CloseConnection (void)

{
	if (Command) DisposHandle(Command);
	if (Response) DisposHandle(Response);
	return mtcp_CloseConnection();
}

/*_____________________________________________________________________

	AbortConnection - Abort Connection to Server.
_____________________________________________________________________*/

static OSErr AbortConnection (void)

{
	if (Command) DisposHandle(Command);
	if (Response) DisposHandle(Response);
	return mtcp_AbortConnection();
}

/*_____________________________________________________________________

	DoCommand - Issue One Command to Server.
	
	Entry:	cmd = command.
	
	Exit:		Response = response.
_____________________________________________________________________*/

static OSErr DoCommand (Str255 cmd)

{
	short				cmdLen;			/* command length */

	cmdLen = *cmd;
	memcpy(*Command, cmd+1, cmdLen);
	*(*Command + cmdLen) = '\r';
	return mtcp_PhCommand(Command, Response, &Truncated);
}

/*_____________________________________________________________________

	Skip - Skip to Next Response Field
				
	Entry:		*p = pointer into response.
	
	Exit:			function result = true if error (no next field).
					*p = pointer to first char of next field.
_____________________________________________________________________*/

static Boolean Skip (char **p)

{
	char *q;

	q = strpbrk(*p, ":\r");
	if (!q || *q == '\r') return true;
	q++;
	q += strspn(q, " \t");
	if (*q == '\r') return true;
	*p = q;
	return false;
}

/*_____________________________________________________________________

	GetServErrMsg - Get Server Error Message.
	
	Entry:	p = pointer to beginning of error message line in response.
	
	Exit:		servErrMsg = server error message.
_____________________________________________________________________*/

static void GetServErrMsg (char *p, Str255 servErrMsg)

{
	char			*q;			/* pointer into response line */
	short			len;			/* message length */
	short			sCode;		/* server error code */

	sCode = atoi(p);
	*servErrMsg = 0;
	while (true) {
		p = strchr(p, ':') + 1;
		q = strchr(p, '\r');
		len = q-p;
		if (*servErrMsg + len > 255) len = 255 - *servErrMsg;
		memcpy(servErrMsg+*servErrMsg+1, p, len);
		*servErrMsg += len;
		p = q+1;
		if (atoi(p) != sCode) break;
		if (*servErrMsg < 255) {
			(*servErrMsg)++;
			*(servErrMsg+*servErrMsg) = ' ';
		}
	}
}

/*_____________________________________________________________________

	GetFieldInfo - Get Ph Server Field Info.
				
	Exit:		function result = error code.
				fields = handle to allocated and initialized field info array.
				numFields = number of fields.
				sCode = server response code:
					phSuccess = success.
					other = unexpected.
				
fields
-200:6:alias:max 32 Indexed Lookup Public Default Change
-200:6:alias:Unique name for user, chosen by user.
-200:3:name:max 256 Indexed Lookup Public Default
-200:3:name:Full name.
...
-200:25:all:max 1
-200:25:all:reserved
...
-200:34:text:max 4095 Lookup Public Default
-200:34:text:Miscellaneous text
200:Ok.
_____________________________________________________________________*/

static OSErr GetFieldInfo (FieldInfo ***fields, short *numFields, short *sCode)

{
	Handle			h;						/* handle to field info array */
	OSErr				rCode;				/* result code */
	Str255			cmd;					/* server command */
	char				*p;					/* pointer into response */
	char				*q;					/* pointer into response */
	FieldInfo		*f;					/* pointer to field info record */
	short				len;					/* string length */
	short				code;					/* server response code */

	*numFields = 0;
	h = NewHandle(0);
	if (!h) return MemError();
	GetIndString(cmd, serverCmds, phFields);
	if (rCode = DoCommand(cmd)) return rCode;
	HLock(Response);
	p = *Response+1;
	while (true) {
		code = atoi(p);
		if (code != -phSuccess) break;
		HUnlock(h);
		SetHandleSize(h, GetHandleSize(h) + sizeof(FieldInfo));
		HLock(h);
		f = (FieldInfo*)(*h + *numFields*sizeof(FieldInfo));
		code = phSyntax;
		if (Skip(&p)) break;
		if (Skip(&p)) break;
		q = strpbrk(p, ":\r");
		if (!q || *q == '\r') break;
		len = q-p;
		if (len > maxFieldName) break;
		memcpy(f->name+1, p, len);
		*f->name = len;
		*(f->name+len+1) = 0;
		p = q+1;
		p = strpbrk(p, "0123456789\r");
		if (!p || *p == '\r') break;
		f->maxSize = atoi(p);
		if (f->maxSize <= 0) break;
		p = strpbrk(p, " \r");
		if (!p) break;
		if (*p == ' ') {
			p++;
			q = strchr(p, '\r');
			if (!q) break;
			len = q-p;
			if (len > 254) break;
			f->attributes = NewHandle(len+2);
			memcpy(*f->attributes+1, p, len);
			**f->attributes = len;
			*(*f->attributes+len+1) = 0;
			p = q+1;
		} else {
			p++;
			f->attributes = NewHandle(2);
			**f->attributes = *(*f->attributes+1) = 0;
		}
		code = atoi(p);
		if (code == phSuccess) {
			code = phSyntax;
			break;
		}
		if (code != -phSuccess) break;
		code = phSyntax;
		if (Skip(&p)) break;
		if (Skip(&p)) break;
		if (Skip(&p)) break;
		q = strchr(p, '\r');
		if (!q) break;
		len = q-p;
		if (len > 254) break;
		f->description = NewHandle(len+2);
		memcpy(*f->description+1, p, len);
		**f->description = len;
		*(*f->description+len+1) = 0;
		p = q+1;
		f->original = NewHandle(0);
		f->origSize = 0;
		(*numFields)++;
	}
	HUnlock(Response);
	HUnlock(h);
	*sCode = abs(code);
	if (code == phSuccess) {
		*fields = (FieldInfo**)h;
	} else {
		DisposHandle(h);
	}
	return noErr;
}

/*_____________________________________________________________________

	GetServerStatus - Get Server Status.
				
	Exit:		function result = error code.
				ReadOnlyReason = read-only reason.
				
status
200:Database ready.

status
100:The database is now read-only (for database update).
201:Database ready, read only (for database update).

status
201:Database ready, read only (for database update).
_____________________________________________________________________*/

OSErr GetServerStatus (void)

{
	OSErr				rCode;				/* result code */
	Str255			cmd;					/* server command */
	char				*p;					/* pointer into response */
	char				*q;					/* pointer into response */
	short				len;					/* string length */
	short				code;					/* server response code */
	
	GetIndString(cmd, serverCmds, phStatus);
	if (rCode = DoCommand(cmd)) return rCode;
	HLock(Response);
	p = *Response+1;
	code = atoi(p);
	*ReadOnlyReason = 0;
	while (true) {
		if (code == phSuccess) {
			break;
		} else if (code == phReadOnly) {
			p = strpbrk(p, "(\r");
			if (!p || *p == '\r') break;
			p++;
			q = strpbrk(p, ")\r");
			if (!q || *q == '\r') break;
			len = q-p;
			if (len > 255) len = 255;
			*ReadOnlyReason = len;
			memcpy(ReadOnlyReason+1, p, len);
			break;
		} else if (code == phInProgress) {
			p = strchr(p, '\r');
			if (!p) break;
			p++;
		}
	}
	HUnlock(Response);
	return noErr;
}

/*_____________________________________________________________________

	GetAlias - Get Alias for Username.
	
	Entry:	user = username.
				
	Exit:		function result = error code.
				alias = alias.
				sCode = server response code:
					phSuccess = success.
					phNoMatches = no matches to username.
					phManyMatches = more than one match to username.
					phFieldNotThere = record has no alias.
					other = unexpected.

query norstad return alias
102:There was 1 match to your request.
-200:1:   alias: j-norstad
200:Ok
.
query john return alias
102:There were 3 matches to your request.
-200:1:   alias: j-norstad
-200:2:   alias: J-Franks
-200:3:   alias: j-smith
200:Ok.

query susan return alias
501:No matches to your query.

query john return alias
502:Too many entries to print.

query noalias return alias
102:There was 1 match to your request.
-508:1:   alias: Not present in entry.
200:Ok.
_____________________________________________________________________*/

static OSErr GetAlias (Str255 user, Str255 alias, short *sCode)

{
	OSErr			rCode;				/* result code */
	Str255		tmpl;					/* server command template */
	Str255		cmd;					/* server command */
	char			*p;					/* pointer into response */
	char			*q;					/* pointer into response */
	short			nAlias;				/* number of aliases for this name */
	short			len;					/* string length */
	short			code;					/* server response code */
		
	GetIndString(tmpl, serverCmds, phQueryAlias);
	utl_PlugParams(tmpl, cmd, user, nil, nil, nil);
	if (rCode = DoCommand(cmd)) return rCode;
	nAlias = 0;
	HLock(Response);
	p = *Response+1;
	while (true) {
		code = atoi(p);
		if (code == -phSuccess) {
			nAlias++;
			if (nAlias > 1) break;
			code = phSyntax;
			if (Skip(&p)) break;
			if (Skip(&p)) break;
			if (Skip(&p)) break;
			q = strchr(p, '\r');
			if (!q) break;
			len = q-p;
			if (len > 255) break;
			memcpy(alias+1, p, len);
			*alias = len;
		} else if (code == phMatchCount) {
			q = strchr(p, '\r');
			code = phSyntax;
			if (!q) break;
		} else {
			break;
		}
		p = q+1;
	}
	HUnlock(Response);
	code = abs(code);
	if (code == phSuccess && nAlias != 1) {
		if (!nAlias) {
			*sCode = phNoMatches;
		} else if (nAlias > 1) {
			*sCode = phManyMatches;
		}
	} else {
		*sCode = code;
	}
	return noErr;
}

/*_____________________________________________________________________

	Login - Login to Server.
	
	Entry:	user = alias or usernmae.
				pswd = password.
				
	Exit:		function result = error code.
				alias = alias.
				sCode = server response code:
					phSuccess = successful login.
					phPermErr = login failed due to bad username or password.
					phManyMatches = more than one match to username.
					phFailReadOnly = database is read-only.
					phFieldNotThere = record has no alias.
					other = unexpected.
				
login "j-blow"
301:O;T`)4A&%,B;[4.\4C;14"YI";@20)B_D5^LH>QLI2
answer $%dkte&65k8(
200:j-blow:Hi how are you?

login "j-blow"
301:;4">_G0C&6[$[$_KA5`2_VR=#0K@XE,2X-OWS_9X44
answer #$%^&
500:Login failed

login j-norstad
301:A9E)*[=FYTAWCS5"?2NNHZMUB.EY]B,=ZQF$K"ICU)
clear x
517:login not allowed to read-only database.
status
201:Database ready, read only (for database update).
_____________________________________________________________________*/

static OSErr Login (Str255 user, Str255 pswd, Str255 alias, short *sCode)

{
	OSErr			rCode;				/* result code */
	Str255		tmpl;					/* server command template */
	Str255		cmd;					/* server command */
	Boolean		haveTriedAlias;	/* true after alias login attempt failed */
	char			*p;					/* pointer into response */
	char			*q;					/* pointer into response */
	short			encryptLen;			/* length of encrypted string */
	short			code;					/* server response code */
	
	utl_CopyPString(alias, user);
	haveTriedAlias = false;
	while (true) {
		GetIndString(tmpl, serverCmds, phLogin);
		utl_PlugParams(tmpl, cmd, alias, nil, nil, nil);
		if (rCode = DoCommand(cmd)) return rCode;
		code = atoi(*Response+1);
		if (code == phFailReadOnly || code == phFailReadOnly2) {
			code = phFailReadOnly;
			GetServerStatus();
			*sCode = code;
			return noErr;
		}
		if (code != phEncrypt) break;
		HLock(Response);
		p = *Response+1;
		code = phSyntax;
		if (Skip(&p)) break;
		q = strchr(p, '\r');
		if (!q) break;
		*q = 0;
		p2cstr(pswd);
		crypt_start(pswd);
		c2pstr(pswd);
		GetIndString(cmd, serverCmds, phAnswer);
		encryptLen = encryptit(cmd + *cmd + 1, p);
		HUnlock(Response);
		*cmd += encryptLen;
		if (rCode = DoCommand(cmd)) return rCode;
		code = atoi(*Response+1);
		if (code == phPermErr) {
			if (haveTriedAlias) break;
		} else if (code == phFailReadOnly || code == phFailReadOnly2) {
			code = phFailReadOnly;
			GetServerStatus();
			*sCode = code;
			return noErr;
		} else {
			break;
		}
		if (rCode = GetAlias(user, alias, &code)) return rCode;
		if (code != phSuccess) break;
		haveTriedAlias = true;
	}
	HUnlock(Response);
	if (code == phNoMatches) code = phPermErr;
	*sCode = abs(code);
	return noErr;
}

/*_____________________________________________________________________

	GetFieldData - Get Field Data from Server.
	
	Entry:	user = alias or name.
				fields = handle to field info array.
				numfields = number of fields.
				
	Exit:		function result = error code.
				field data set in original and origSize fields.
				sCode = server response code:
					phSuccess = success.
					phNoMatches = no matches to username.
					phManyMatches = more than one match to username.
					other = unexpected.
					
query alias="j-norstad" return all
102:There was 1 match to your request.
-200:1:             alias: j-norstad
-200:1:              name: John Norstad
-200:1:             email: jln@casbah.acns.nwu.edu
-200:1:             phone: (708)491-4077
-200:1:               fax: (708)491-3824
-200:1:           address: Academic Computing and Network Services
-200:1:                  : 2129 Sheridan Road
-200:1:                  : Evanston, IL 60208
-200:1:   office_location: Vogelback Room 125
-200:1:              type: Staff
-522:1:          password: Encrypted; cannot be viewed.
-200:1:          nickname: jln
-200:1:              hero: yes
-200:1:        department: ACNS, Networking & Communications
-200:1:             title: Network Analyst
-200:1:             hours: My hours vary widely.  I usually work days, but not always.
-200:1:                  : Your best bet is 10-4 M-F.
-200:1:             other: Mac networking, programming, and virus guru.
-200:1:                  : Author of the "Disinfectant" Mac anti-viral utility.
200:Ok.

query alias="bogus" return all
501:No matches to your query.
_____________________________________________________________________*/

static OSErr GetFieldData (Str255 user, FieldInfo **fields, short numFields, 
	short *sCode)

{
	OSErr			rCode;				/* result code */
	Str255		tmpl;					/* server command template */
	Str255		cmd;					/* server command */
	char			*p;					/* pointer into response */
	char			*q;					/* pointer into response */
	char			*r;					/* pointer into response */
	short			len;					/* string length */
	short			i;						/* loop index */
	FieldInfo	*f;					/* pointer to field info record */
	Handle		fText;				/* handle to field text */
	short			fSize;				/* field size */
	short			code;					/* server response code */
	static short tryQueries[] = {phAliasQuery,phIdQuery,phNameQuery};
											/* try these queries to find the user */
	short			trying;				/* the query we're currently trying */
	
	trying = 0;
	while (true) {
		GetIndString(tmpl, serverCmds, tryQueries[trying++]);
		utl_PlugParams(tmpl, cmd, user, nil, nil, nil);
		if (rCode = DoCommand(cmd)) return rCode;
		p = *Response+1;
		code = atoi(p);
		if (code == phNoMatches) {
			if (sizeof(tryQueries)/sizeof(short)==trying) break;
			continue;
		}
		HLock(Response);
		HLock((Handle)fields);
		while (true) {
			code = atoi(p);
			if (code == -phSuccess) {
				code = phSyntax;
				if (Skip(&p)) break;
				if (*p != '1') {
					code = phManyMatches;
					break;
				};
				if (Skip(&p)) break;
				q = strpbrk(p, ":\r");
				if (!q || *q == '\r') break;
				p += strspn(p, " \t");
				if (p == q) break;
				*q = 0;
				for (i = 0; i < numFields; i++) {
					f = &(*fields)[i];
					if (!strcmp(f->name+1, p)) break;
				}
				if (i == numFields) break;
				fText = f->original;
				SetHandleSize(fText, 0);
				fSize = 0;
				while (true) {
					p = q+1;
					if (*p == ' ') p++;
					q = strchr(p, '\r');
					if (!q) break;
					len = q-p;
					SetHandleSize(fText, fSize+len+1);
					memcpy(*fText+fSize, p, len);
					fSize += len;
					*(*fText+fSize) = '\n';
					fSize++;
					p = q+1;
					code = atoi(p);
					if (code != -phSuccess) break;
					code = phSyntax;
					if (Skip(&p)) break;
					if (Skip(&p)) break;
					code = -phSuccess;
					if (*p != ':') break;
					q = p;
				}
				if (code == phSyntax) break;
				r = *fText+fSize-1;
				while (r >= *fText && *r == '\n') r--;
				fSize = r - *fText + 1;
				SetHandleSize(fText, fSize);
				f->origSize = fSize;
			} else if (code == phMatchCount || code == -phViewEncrypt) {
				q = strchr(p, '\r');
				code = phSyntax;
				if (!q) break;
			} else {
				break;
			}
			p = q+1;
		}
		HUnlock(Response);
		HUnlock((Handle)fields);
		break;
	}
	*sCode = abs(code);
	return noErr;
}

/*_____________________________________________________________________

	Quit - Sent Quit Command to Server.
_____________________________________________________________________*/

static OSErr Quit (void)

{
	Str255		cmd;					/* server command */
	
	GetIndString(cmd, serverCmds, phQuit);
	return DoCommand(cmd);
}

/*_____________________________________________________________________

	DoChangeCommand - Send Change Command to Server.
	
	Entry:	tmpl = change command template.
				alias = alias of record to change.
				fName = name of field to change.
				val = handle to  new field value.
				len = length of new field value.
				
	Exit:		function result = error code.
				sCode = server response code:
					phSuccess = success.
					phNoAuthField = not authorized to change this field.
					phDupAlias = duplicate alias.
					phIllegalVal = illegal value.
					other = unexpected.
				servErrMsg = server error message if sCode != phSuccess.

change alias="j-blow" make other="test"
200:1 entry changed.

change alias="j-blow" make hero="yes"
-505:hero:you may not change this field.
500:1 entry found, none changed.

change alias="j-blow" make alias="c-nims"
-509:Alias c-nims conflicts with other users.
500:1 entry found, none changed.

change alias="j-blow" make alias=""
-512:Alias is too long or too short.
500:1 entry found, none changed.
_____________________________________________________________________*/

static OSErr DoChangeCommand (Str255 tmpl, Str255 alias, Str255 fName,
	Handle val, short len, short *sCode, Str255 servErrMsg)
	
{
	OSErr			rCode;				/* result code */
	Str255		cmd;					/* server command */
	long			cmdSize;				/* size of server command buffer */
	short			cmdLen;				/* length of server command */
	char			*p;					/* pointer into server command */
	char			*q;					/* pointer into field text */
	char			*qEnd;				/* pointer to end of field text */
	long			pOffset;				/* offset of p from beginning of command buffer */

	utl_PlugParams(tmpl, cmd, alias, fName, nil, nil);
	cmdSize = GetHandleSize(Command);
	cmdLen = *cmd;
	HLock(Command);
	HLock(val);
	p = *Command;
	memcpy(p, cmd+1, cmdLen);
	p += cmdLen;
	qEnd = *val + len;
	for (q = *val; q < qEnd; q++) {
		if (cmdLen + 2 > cmdSize) {
			cmdSize += cmdHandleInc;
			pOffset = p - *Command;
			HUnlock(Command);
			SetHandleSize(Command, cmdSize);
			HLock(Command);
			p = *Command + pOffset;
		}
		if (*q=='\n' || *q=='\t' || *q=='"' || *q=='\\') {
			*p++ = '\\';
			switch (*q) {
				case '\n': *p++ = 'n'; break;
				case '\t': *p++ = 't'; break;
				case '"': *p++ = '"'; break;
				case '\\': *p++ = '\\'; break;
			}
			cmdLen += 2;
		} else {
			*p++ = *q;
			cmdLen++;
		}
	}
	*p++ = '"';
	*p++ = '\r';
	HUnlock(Command);
	HUnlock(val);
	if (rCode = mtcp_PhCommand(Command, Response, &Truncated)) return rCode;
	*sCode = abs(atoi(*Response+1));
	if (*sCode != phSuccess) GetServErrMsg(*Response+1, servErrMsg);
	return noErr;
}

/*_____________________________________________________________________

	CreateNewFields - Create Name and Type Fields for New Record.
	
	Entry:	alias = alias of record to change.
				name = name field.
				type = type field.
				
	Exit:		function result = error code.
				sCode = server response code:
					phSuccess = success.
					phIllegalVal = illegal value.
					other = unexpected.
				whichField = illegal field number if sCode != phSuccess:
					1 = name.
					2 = type.
				servErrMsg = server error message if sCode != phSuccess.
_____________________________________________________________________*/

static OSErr CreateNewFields (Str255 alias, Str255 name, 
	Str255 type, short *sCode, short *whichField, Str255 servErrMsg)
	
{
	Str255			tmpl;				/* server command template */
	Handle			val;				/* handle to field value */
	short				len;				/* length of field value */
	OSErr				rCode;			/* result code */
	Str255			fName;			/* field name */
	
	GetIndString(tmpl, serverCmds, phMake);
	rCode = noErr;
	*whichField = 1;
	if (len = *name) {
		GetIndString(fName, fieldNames, nameFieldName);
		PtrToHand(name+1, &val, len);
		rCode = DoChangeCommand(tmpl, alias, fName, val, 
			len, sCode, servErrMsg);
		DisposHandle(val);
		if (rCode) return rCode;
		if (*sCode != phSuccess) return noErr;
	}
	*whichField = 2;
	if (len = *type) {
		GetIndString(fName, fieldNames, typeFieldName);
		PtrToHand(type+1, &val, len);
		rCode = DoChangeCommand(tmpl, alias, fName, val, 
			len, sCode, servErrMsg);
		DisposHandle(val);
	}
	return rCode;
}

/*_____________________________________________________________________

	GetProxyList - Get Proxy List.
	
	Entry:	alias = alias.
	
	Exit:		function result = error code.
				proxyList = handle to list of aliases for which the
					alias is a proxy, as a sequence of Pascal strings.
				sCode = server response code to proxy query:
					phSuccess = success.
					phManyMatches = too many matches.
					phNoAuthSearch = field does not have Lookup attribute.
					phNoInxField = field does not have Indexed attribute.
					other = unexpected.
				
query proxy=j-blow return alias
102:There were 2 matches to your request.
-200:1:   alias: j-doe
-200:2:   alias: j-smith
200:Ok.

query proxy=zzz return alias
501:No matches to your query.

query proxy=j-blow return alias
502:Too many entries to print.

query proxy="j-blow" return alias
-504:proxy:you may not use this field for lookup.
-515:no non-null key field in query.
500:Did not understand query.

query proxy="j-blow" return alias
-515:no non-null key field in query.
500:Did not understand query.
_____________________________________________________________________*/

static OSErr GetProxyList (Str255 alias, Handle *proxyList, short *sCode)

{
	Str255		tmpl;					/* query template */
	Str255		cmd;					/* query command */
	char			*p;					/* pointer into response */
	char			*q;					/* pointer into response */
	short			len;					/* length of alias */
	Handle		pList;				/* handle to proxy list */
	short			pSize;				/* size of proxy list */
	OSErr			rCode;				/* result code */
	short			code;					/* server result code */
	
	pList = NewHandle(0);
	pSize = 0;
	GetIndString(tmpl, serverCmds, phProxyQuery);
	utl_PlugParams(tmpl, cmd, alias, nil, nil, nil);
	if (rCode = DoCommand(cmd)) return rCode;
	HLock(Response);
	p = *Response+1;
	while (true) {
		code = atoi(p);
		if (code == phSuccess) break;
		if (code == -phSuccess) {
			code = phSyntax;
			if (Skip(&p)) break;
			if (Skip(&p)) break;
			if (Skip(&p)) break;
			q = strpbrk(p, ":\r");
			p += strspn(p, " \t");
			if (p == q) break;
			*q = 0;
			len = q-p;
			SetHandleSize(pList, pSize + len + 1);
			*(*pList + pSize) = len;
			memcpy(*pList+pSize+1, p, len);
			pSize += len+1;
		} else if (code == phMatchCount) {
			q = strchr(p, '\r');
			if (!q) break;
		} else {
			break;
		}
		p = q+1;
	}
	HUnlock(Response);
	*proxyList = pList;
	if (code == phNoMatches) code = phSuccess;
	*sCode = abs(code);
	return noErr;
}

/*_____________________________________________________________________

	PutFieldData - Put Changed Fields to Server.
	
	Entry:	alias = alias.
				fields = handle to field info array.
				numfields = number of fields.
				
	Exit:		function result = error code.
_____________________________________________________________________*/

static OSErr PutFieldData (Str255 alias, FieldInfo **fields, short numFields)

{
	OSErr			rCode;				/* result code */
	Str255		tmpl;					/* server command template */
	short			i;						/* loop index */
	FieldInfo	*f;					/* pointer to field info record */
	Handle		fText;				/* handle to field text */
	short			teLength;			/* length of field text */
	short			code;					/* server response code */
	Str255		realAlias;			/* the real alias */
	Str255		aliasFName;			/* name of alias field */
	short			nLines;				/* number of lines */
	short			*lineStarts;		/* pointer into lineStarts array */
	Handle		val;					/* handle to wrapped field data */
	char			*p;					/* pointer into unwrapped field data */
	char			*q;					/* pointer into wrapped field data */
	short			len;					/* length of text */
	short			j;						/* loop index */
	Str255		servErrMsg;			/* server error message */
	
	GetIndString(tmpl, serverCmds, phMake);
	GetIndString(aliasFName, fieldNames, aliasFieldName);
	rCode = noErr;
	HLock((Handle)fields);
	utl_CopyPString(realAlias, alias);
	for (i = 0; i < numFields; i++) {
		f = &(*fields)[i];
		if (!f->dirty) continue;
		fText = (**f->edit).hText;
		teLength = (**f->edit).teLength;
		nLines = (**f->edit).nLines;
		lineStarts = (**f->edit).lineStarts;
		val = NewHandle(teLength + nLines);
		if (nLines) {
			p = *fText;
			q = *val;
			for (j = 0; j < nLines; j++) {
				len = *(lineStarts+1) - *lineStarts;
				memcpy(q, p, len);
				p += len;
				q += len;
				lineStarts++;
				if (*(q-1) != '\n') *q++ = '\n';
			}
			q--;
			while (q >= *val && *q == '\n') q--;
			len = q - *val + 1;
		} else {
			len = 0;
		}
		rCode = DoChangeCommand(tmpl, realAlias, f->name, val, len, &code, servErrMsg);
		if (rCode) break;
		if (code == phSuccess) {
			f->putOK = true;
			if (EqualString(f->name, aliasFName, true, true)) {
				*realAlias = len;
				memcpy(realAlias+1, *val, len);
			}
		} else {
			f->servErrMsg = NewHandle(*servErrMsg + 1);
			utl_CopyPString(*(f->servErrMsg), servErrMsg);
		}
		DisposHandle(val);
		val = nil;
	}
	HUnlock((Handle)fields);
	return rCode;
}

/*_____________________________________________________________________

	ChangePassword - Change Ph Password.
	
	Entry:	alias = alias of record to change.
				newPswd = new password.
				
	Exit:		function result = error code.
				sCode = server response code:
					phSuccess = success.
					phIllegalVal = illegal value.
					other = unexpected.
				servErrMsg = server error message if sCode != phSuccess.

change alias="j-blow" make password="xxx"
200:1 entry changed.

change alias="j-blow" make password="xxx"
-512:Passwords must use only printable characters; sorry.
-512:If your password met this rule, reissue your login command, and try again.
500:1 entry found, none changed.
_____________________________________________________________________*/

static OSErr ChangePassword (Str255 alias, Str255 newPswd, short *sCode, 
	Str255 servErrMsg)
	
{
	OSErr				rCode;					/* result code */
	Str255			tmpl;						/* server command template */
	Str255			cmd;						/* server command */
	short				encryptLen;				/* length of encrypted string */

	GetIndString(tmpl, serverCmds, phChPswdCmd);
	utl_PlugParams(tmpl, cmd, alias, nil, nil, nil);
	p2cstr(newPswd);
	encryptLen = encryptit(cmd + *cmd + 1, newPswd);
	c2pstr(newPswd);
	*cmd += encryptLen;
	if (rCode = DoCommand(cmd)) return rCode;
	*sCode = abs(atoi(*Response+1));
	if (*sCode != phSuccess) GetServErrMsg(*Response+1, servErrMsg);
	return noErr;
}

/*_____________________________________________________________________

	CreateRecord - Create Ph Record.
	
	Entry:	alias = alias for new record.
				
	Exit:		function result = error code.
				sCode = server response code:
					phSuccess = success.
					phDupAlias = duplicate alias.
					phIllegalVal = illegal value.
					other = unexpected.
				servErrMsg = server error message if sCode != phSuccess.

add alias="w-xyz"
200:Ok.

add alias="c-nims"
-509:Alias c-nims in use or is too common a name.
599:Add command not understood.
_____________________________________________________________________*/

static OSErr CreateRecord (Str255 alias, short *sCode, Str255 servErrMsg)
	
{
	OSErr				rCode;					/* result code */
	Str255			tmpl;						/* server command template */
	Str255			cmd;						/* server command */

	GetIndString(tmpl, serverCmds, phNewRecordCmd);
	utl_PlugParams(tmpl, cmd, alias, nil, nil, nil);
	if (rCode = DoCommand(cmd)) return rCode;
	*sCode = abs(atoi(*Response+1));
	if (*sCode != phSuccess) GetServErrMsg(*Response+1, servErrMsg);
	return noErr;
}

/*_____________________________________________________________________

	DeleteRecord - Delete Ph Record.
	
	Entry:	alias = alias of record to be deleted.
				
	Exit:		function result = server response code.
				sCode = server response code:
					phSuccess = success.
					phNoMatches = no such alias.
					other = unexpected.
					
delete alias="w-xyz"
200:1 entries deleted.

delete alias="xyz"
501:No entries matched specifications.
_____________________________________________________________________*/

static OSErr DeleteRecord (Str255 alias, short *sCode)

{
	Str255			tmpl;				/* server command template */
	Str255			cmd;				/* server command */
	OSErr				rCode;			/* result code */
	
	GetIndString(tmpl, serverCmds, phDeleteCmd);
	utl_PlugParams(tmpl, cmd, alias, nil, nil, nil);
	if (rCode = DoCommand(cmd)) return rCode;
	*sCode = abs(atoi(*Response+1));
	return noErr;
}

/*_____________________________________________________________________

	AdjustBigFields - Adjust Big Fields.
	
	Entry:	reply = handle to query reply.
				replyLen = length of query reply.
				
	Exit:		fields adjusted.
	
	This function checks for lines in the reply which exceed 80 characters
	in length. Any fields which contain such lines are reformatted as
	follows:
	
					field_name:
	field text field text field text field text field text field text 
	field text field text field text field text field text field text 
	field text field text field text field text field text field text 
	field text field text field text field text field text field text 
	
	This function also truncates the reply to 32K if necessary.
_____________________________________________________________________*/

static void AdjustBigFields (Handle reply, long replyLen)

{
	char		*p;			/* pointer into reply */
	char		*pEnd;		/* pointer to end of reply */
	char		*q;			/* pointer into reply */
	char		*r;			/* pointer into reply */
	char		*z;			/* pointer into reply */
	Str255	msg;			/* truncation message */
	
	HLock(reply);
	p = *reply;
	pEnd = *reply + replyLen;
	z = *reply;
	while (p < pEnd) {
		q = strchr(p, '\n');
		if (q - p > 80) {
			r = p;
			while (true) {
				q = r + strspn(r, " ");
				if (*q != ':') break;
				r--;
				while (*--r != '\n' && r > *reply);
				r++;
				if (r <= *reply) break;
			}
			if (r > *reply) {
				/* r pts to first char of first line of field, q pts to first char
					of field name in first line. */
				q = strchr(q, ':') + 1;
				if (p < q && z != p) memmove(z, p, q-p);
				z += q-p;
				*z++ = '\n';
				while (true) {
					p = q+1;
					q = strchr(p, '\n') + 1;
					if (z != p) memmove(z, p, q-p);
					z += q-p;
					p = q;
					if (p >= pEnd) break;
					q = p + strspn(p, " ");
					if (*q != ':') break;
					q++;
				}
				continue;
			}
		}
		if (z != p) memmove(z, p, q-p+1);
		z += q-p+1;
		p = q+1;
	}
	while (*--z == '\n' && z > *reply);
	z++;
	replyLen = z - *reply;
	if (replyLen > 0x3fff || Truncated) {
		GetIndString(msg, stringsID, truncMsg);
		z = *reply + 0x3fff;
		while (*z != '\n' && z + *msg + 2 - *reply > 0x3fff) z--;
		memcpy(z+1, msg+1, *msg);
		z += *msg+1;
		*z++ = '\n';
		replyLen = z - *reply;
	}
	HUnlock(reply);
	SetHandleSize(reply, replyLen);
}

/*_____________________________________________________________________

	ExpandReply - Expand Reply Block.
	
	Entry:	reply = handle to locked reply block.
				*z = pointer into reply.
				len = number of bytes needed in reply.
				*replyLen = current size of reply block.
				
	Exit:		Relocatable block size increased if necessary to make
				room for len more bytes plus another 1000 bytes of slop.
_____________________________________________________________________*/

static void ExpandReply (Handle reply, char **z, long len, long *replyLen)

{
	long			offset;			/* offset into block of pointer */

	if (*z + len + 1000 < *reply + *replyLen) return;
	offset = *z - *reply;
	HUnlock(reply);
	*replyLen = offset + len + 1000;
	SetHandleSize(reply, *replyLen);
	HLock(reply);
	*z = *reply + offset;
}

/*_____________________________________________________________________

	SendQuery - Send a Query.
	
	Entry:	query = handle to query.
				queryLen = length of query.
				emailDomain = email domain name, empty string if none.
				reply = preallocated reply handle.
				
	Exit:		function result = error code.
				reply = handle to reply, size adjusted to exactly hold
					the reply.
				
				If server errors occur, the server error messages are
				placed in the reply. The match count message, if any,
				is also placed in the reply.
					
query john norstad
102:There was 1 match to your request.
-200:1:           alias: j-norstad
-200:1:            name: John Norstad
-200:1:           email: jln@casbah.acns.nwu.edu
-200:1:           phone: (708)491-4077
-200:1:         address: Academic Computing and Network Services
-200:1:                : 2129 Sheridan Road
-200:1:                : Evanston, IL 60208
-200:1: office_location: Vogelback Room 125
-200:1:      department: ACNS, Networking & Communications
-200:1:           title: Network Analyst
-200:1:           hours: My hours vary widely.  I usually work days, but not always.
-200:1:                : Your best bet is 10-4 M-F.
-200:1:           other: Mac networking, programming, and virus guru.
-200:1:                : Author of the "Disinfectant" Mac anti-viral utility.
200:Ok.

query john
102:There were 3 matches to your request.
-200:1:           alias: j-norstad
-200:1:            name: John Norstad
-200:1:           email: jln@casbah.acns.nwu.edu
-200:1:           phone: (708)491-4077
-200:1:         address: Academic Computing and Network Services
-200:1:                : 2129 Sheridan Road
-200:1:                : Evanston, IL 60208
-200:1: office_location: Vogelback Room 125
-200:1:      department: ACNS, Networking & Communications
-200:1:           title: Network Analyst
-200:1:           hours: My hours vary widely.  I usually work days, but not always.
-200:1:                : Your best bet is 10-4 M-F.
-200:1:           other: Mac networking, programming, and virus guru.
-200:1:                : Author of the "Disinfectant" Mac anti-viral utility.
-200:2:           alias: J-Franks
-200:2:            name: John Franks
-200:2:           email: john@math.nwu.edu
-200:2:           phone: (708) 491-5548
-200:2:         address: Department of Mathematics
-200:2:                : Northwestern University
-200:2:                : Evanston, IL 60208-2370
-200:2:      department: Mathematics
-200:2:           title: Professor of Mathematics
-200:3:           alias: j-smith
-200:3:            name: John Smith
200:Ok.

query mary
501:No matches to your query.

query xxx=yyy
-507:xxx:unknown field.
-515:no non-null key field in query.
500:Did not understand query.

query title=abc
-515:no non-null key field in query.
500:Did not understand query.

query john
502:Too many entries to print.
_____________________________________________________________________*/

static OSErr SendQuery (Handle query, unsigned short queryLen, 
	Str255 emailDomain, Handle reply)

{
	Str255			cmd;				/* "query " command prefix */
	Str255			aliasStr;		/* "alias" field name */
	Str255			emailStr;		/* "email" field name */
	Str255			alias;			/* last alias seen */
	Str255			msg;				/* error message */
	short				aliasIndex;		/* index of last alias seen, or 0 if none */
	long				cmdSize;			/* size of command buffer */
	OSErr				rCode;			/* error code */
	char				*p;				/* pointer into response */
	char				*q;				/* pointer into response */
	char				*r;				/* pointer into response */
	char				*z;				/* pointer into reply */
	Boolean			syntaxErr;		/* true if syntax error in response */
	short				lastIndex;		/* last match entry index seen */
	short				index;			/* current match entry index */
	short				code;				/* server response code */
	short				i;					/* loop index */
	short				len;				/* length of text */
	long				replyLen;		/* length of reply */

	GetIndString(cmd, serverCmds, phQuery);
	GetIndString(aliasStr, fieldNames, aliasFieldName);
	GetIndString(emailStr, fieldNames, emailFieldName);
	cmdSize = GetHandleSize(Command);
	if (*cmd + queryLen + 1 > cmdSize) SetHandleSize(Command, *cmd + queryLen + 1);
	memcpy(*Command, cmd+1, *cmd);
	memcpy(*Command + *cmd, *query, queryLen);
	*(*Command + *cmd + queryLen) = '\r';
	rCode = mtcp_PhCommand(Command, Response, &Truncated);
	if (rCode == mtcpCancel) {
		SetHandleSize(reply, 0);
		return rCode;
	}
	replyLen = GetHandleSize(Response) + 2000;
	SetHandleSize(reply, replyLen); 
	HLock(Response);
	HLock(reply);
	p = *Response+1;
	z = *reply;
	syntaxErr = false;
	lastIndex = 0;
	aliasIndex = 0;
	while (true) {
		ExpandReply(reply, &z, 1000, &replyLen);
		code = atoi(p);
		if (code == phSuccess) {
			break;
		} else if (code == -phSuccess) {
			if (syntaxErr = Skip(&p)) break;
			index = atoi(p);
			if (syntaxErr = !index) break;
			if (index != lastIndex) {
				lastIndex = index;
				for (i = 0; i < 79; i++) *z++ = '-';
				*z++ = '\n';
			}
			p = strpbrk(p, ":\r");
			if (syntaxErr = (!p || *p != ':')) break;
			p++;
			q = strpbrk(p, ":\r");
			if (syntaxErr = !q) break;
			if (*emailDomain && *q == ':') {
				if (!strncmp(aliasStr+1, q-*aliasStr, *aliasStr)) {
					q++;
					q += strspn(q, " \t");
					r = strpbrk(q, " \t\r");
					len = r-q;
					if (len > 255) len = 255;
					if (len) {
						aliasIndex = index;
						*alias = len;
						memcpy(alias+1, q, len);
					}
				} else if (index == aliasIndex &&
					!strncmp(emailStr+1, q-*emailStr, *emailStr)) {
					q++;
					len = q-p;
					ExpandReply(reply, &z, len, &replyLen);
					memcpy(z, p, len);
					z += len;
					*z++ = ' ';
					memcpy(z, alias+1, *alias);
					z += *alias;
					*z++ = '@';
					memcpy(z, emailDomain+1, *emailDomain);
					z += *emailDomain;
					*z++ = '\n';
					for (i = 1; i < len; i++) *z++ = ' ';
					*z++ = ':';
					*z++ = ' ';
					*z++ = '(';
					q += strspn(q, " \t");
					p = q;
					q = strchr(p, '\r');
					if (syntaxErr = !q) break;
					len = q-p;
					ExpandReply(reply, &z, len, &replyLen);
					memcpy(z, p, len);
					z += len;
					*z++ = ')';
					*z++ = '\n';
					p = q+1;
					continue;
				}
			}
			q = strchr(p, '\r');
			if (syntaxErr = !q) break;
			len = q-p;
			ExpandReply(reply, &z, len, &replyLen);
			memcpy(z, p, len);
			z += len;
			*z++ = '\n';
		} else if (code == phMatchCount) {
			if (syntaxErr = Skip(&p)) break;
			q = strchr(p, '\r');
			if (syntaxErr = !q) break;
			len = q-p;
			ExpandReply(reply, &z, len, &replyLen);
			memcpy(z, p, len);
			z += len;
			*z++ = '\n';
		} else {
			p = strpbrk(p, ":\r");
			if (syntaxErr = !p || *p != ':') break;
			p++;
			q = strchr(p, '\r');
			if (syntaxErr = !q) break;
			len = q-p;
			ExpandReply(reply, &z, len, &replyLen);
			memcpy(z, p, len);
			z += len;
			*z++ = '\n';
			if (code > 0) break;
		}
		p = q+1;
	}
	if (syntaxErr && (!Truncated || p)) {
		GetIndString(msg, servErrors, msgSyntaxError);
		memcpy(z, msg+1, *msg);
		z += *msg;
		*z++ = '\n';
	}
	replyLen = z - *reply;
	HUnlock(Response);
	HUnlock(reply);
	AdjustBigFields(reply, replyLen);
	return rCode;
}

/*_____________________________________________________________________

	GetHelp - Get Server Help Text.
	
	Entry:	topic = help topic.
				reply = preallocated reply handle.
				
	Exit:		function result = error code.
				reply = handle to reply, size adjusted to exactly hold
					the reply.
				
				If server errors occur, the server error messages are
				placed in the reply.
				
help macph 02.Server_Requirements
-200:1:02.Server_Requirements:
-200:1: Server Requirements
-200:1: ===================
-200:1:
-200:1: This section is for Ph server administrators. It describes in detail the
...
-200:1: Detailed mod to be presented here, along with instructions on how to convert
-200:1: an existing database to index the proxy field.
200:Ok.
_____________________________________________________________________*/

static OSErr GetHelp (Str255 topic, Handle reply)

{
	Str255			cmd;				/* server command */
	Str255			msg;				/* error message */
	OSErr				rCode;			/* error code */
	char				*p;				/* pointer into response */
	char				*q;				/* pointer into response */
	char				*r;				/* pointer into response */
	char				*s;				/* pointer into response */
	char				*z;				/* pointer into reply */
	Boolean			syntaxErr;		/* true if syntax error in response */
	short				code;				/* server response code */
	short				len;				/* length of text */
	short				replyLen;		/* length of reply */
	Boolean			strip;			/* true to strip leading blanks */

	GetIndString(cmd, serverCmds, HelpNative ? phHelpNative : phHelp);
	memcpy(cmd + *cmd + 1, topic+1, *topic);
	*cmd += *topic;
	if (rCode = DoCommand(cmd)) return rCode;
	SetHandleSize(reply, GetHandleSize(Response) + 2000);
	HLock(Response);
	HLock(reply);
	p = *Response+1;
	z = *reply;
	syntaxErr = strip = false;
	while (true) {
		code = atoi(p);
		if (code == phSuccess) {
			break;
		} else if (code == -phSuccess) {
			if (p == *Response+1) {
				q = p;
				if (syntaxErr = Skip(&q)) break;
				q = strpbrk(q, ":\r");
				if (syntaxErr = (!q || *q != ':')) break;
				q++;
				r = strchr(q, '\r');
				if (syntaxErr = !r) break;
				s = r-1;
				while (s > q && *s == ' ') s--;
				len = s-q;
				if (len == *topic && *s == ':' && !strncmp(topic+1, q, *topic)) {
					p = r+1;
					strip = true;
					continue;
				}
			}
			if (syntaxErr = Skip(&p)) break;
			p = strpbrk(p, ":\r");
			if (syntaxErr = (!p || *p != ':')) break;
			p++;
			if (strip && *p == ' ') p++;
			q = strchr(p, '\r');
			if (syntaxErr = !q) break;
			for (r = p; r < q; r++) {
				if ((*r == '_' && *(r+1) == 0x08) ||
					(*r == 0x08 && *(r+1) == '_')) {
					r++;
				} else if (*r >= 0x20 && *r <= 0x7e) {
					*z++ = *r;
				}
			}
			*z++ = '\n';
		} else {
			p = strpbrk(p, ":\r");
			if (syntaxErr = !p || *p != ':') break;
			p++;
			q = strchr(p, '\r');
			if (syntaxErr = !q) break;
			len = q-p;
			memcpy(z, p, len);
			z += len;
			*z++ = '\n';
			if (code > 0) break;
		}
		p = q+1;
	}
	if (syntaxErr) {
		GetIndString(msg, servErrors, msgSyntaxError);
		memcpy(z, msg+1, *msg);
		z += *msg;
		*z++ = '\n';
	}
	while (*--z == '\n' && z > *reply);
	z++;
	*z++ = '\n';
	*z++ = '\n';
	replyLen = z - *reply;
	HUnlock(Response);
	HUnlock(reply);
	SetHandleSize(reply, replyLen);
	return noErr;
}

/*_____________________________________________________________________

	GetHelpTopics - Get Help Topic List.
	
	Entry:	server = Ph server host domain name.
	
	Exit:		function result = error code.
				
help macph
-200:1: These ``macph'' help topics are available:
-200:1: 01.Introduction         02.Server_Requirements
-200:1: To view one of these topics, type ``help name-of-topic-you-want''.
-200:2: These ``native'' help topics are also available:
-200:2: 100             402             507             516             599
-200:2: 101             475             508             517             forwarding
-200:2: 102             500             509             518             policy
-200:2: 200             501             510             519             protocol
-200:2: 201             502             511             520             restaurant
-200:2: 300             503             512             521             site
-200:2: 301             504             513             522             update
-200:2: 400             505             514             523
-200:2: 401             506             515             598
-200:2: To view one of these topics, type ``help native name-of-topic-you-want''.
200:Ok.

help macph
/usr4/lookup.Help/macph not found
-200:1:These ``macph'' help topics are available:
-200:1:To view one of these topics, type ``help name-of-topic-you-want''.
-200:2:These ``native'' help topics are also available:
-200:2:100              475             509             519             fields
-200:2:101              500             510             520             general
-200:2:102              501             511             521             lastreg
-200:2:200              502             512             522             nohelp
-200:2:201              503             513             523             policy
-200:2:300              504             514             598             restrictions
-200:2:301              505             515             599             staff
-200:2:400              506             516             campus          student
-200:2:401              507             517             class           type
-200:2:402              508             518             errors          unit
-200:2:To view one of these topics, type ``help native name-of-topic-you-want''.
200:Ok.
_____________________________________________________________________*/

static OSErr GetHelpTopics (Str255 server)

{
	Str255			cmd;				/* server command */
	OSErr				rCode;			/* error code */
	char				*p;				/* pointer into response */
	char 				*q;				/* pointer into response */
	char				*r;				/* pointer into response */
	short				code;				/* server result code */
	short				item;				/* topic number */
	char				*topics[100];	/* array of pointers to topics */
	short				nTopics;			/* number of topics */
	Boolean			eol;				/* true if end of line */
	short				i;					/* bubble sort loop index */
	short				j;					/* bubble sort loop index */
	char				*temp;			/* bubble sort temp pointer */
	Boolean			allDigits;		/* true if topic name is all digits */
	Boolean			helpAvail;		/* true if help is available */
	short				len;				/* length of topic */
	short				size;				/* size of HelpTopics block */

	GetIndString(cmd, serverCmds, phHelp);
	if (rCode = DoCommand(cmd)) return rCode;
	helpAvail = false;
	HelpNative = false;
	nTopics = 0;
	HLock(Response);
	p = *Response+1;
	if (!rCode) {
		while (true) {
			code = atoi(p);
			if (!code) {
				q = strchr(p, '\r');
				if (!q) break;
				p = q+1;
				continue;
			}
			if (code != -phSuccess) break;
			if (Skip(&p)) break;
			code = atoi(p);
			if (code != 1 && !HelpNative) {
				if (helpAvail) break;
				HelpNative = true;
			}
			if (Skip(&p)) break;
			q = strpbrk(p, "'\r");
			if (!q) break;
			if (*q == '\'') {
				q = strchr(p, '\r');
				if (!q) break;
				p = q+1;
				continue;
			}
			while (true) {
				q = strpbrk(p, " \t\r");
				eol = *q == '\r';
				*q = 0;
				allDigits = true;
				for (r = p; *r; r++) {
					if (!isdigit(*r)) allDigits = false;
				}
				if (!allDigits) {
					helpAvail = true;
					if (nTopics < 100) topics[nTopics++] = p;
				}
				if (eol) break;
				p = q+1;
				p += strspn(p, " \t");
			}
			p = q+1;
		}
		DisposHandle(HelpTopics);
		HelpTopics = nil;
		if (helpAvail) {
			for (i = 0; i < nTopics-1; i++) {
				for (j = i+1; j < nTopics; j++) {
					if (strcmp(topics[i], topics[j]) > 0) {
						temp = topics[i];
						topics[i] = topics[j];
						topics[j] = temp;
					}
				}
			}
			HelpTopics = NewHandle(0);
			size = 0;
			for (item = 0; item < nTopics; item++) {
				c2pstr(topics[item]);
				len = *(topics[item]);
				SetHandleSize(HelpTopics, size + len + 1);
				HLock(HelpTopics);
				utl_CopyPString(*HelpTopics + size, topics[item]);
				HUnlock(HelpTopics);
				size += len + 1;
			}
		}
		utl_CopyPString(HelpServer, server);
		GetDateTime(&LastHelpUpdate);
		help_BuildHelpMenu();
	}
	HUnlock(Response);
	return rCode;
}

/*_____________________________________________________________________

	GetSiteInfo - Get Site Info.
	
	Exit:		function result = error code.
				SiteMenu = handle to site popup menu, or nil if error.
				NumSites = number of sites in popup menu.
				Servers = handle to list of server domain names.
				Domains = handle to list of email domain names.
				
query ns-servers type=serverlist return text
102:There was 1 match to your request.
-200:1:    text: site:Northwestern University
-200:1:        : server:ns.nwu.edu
-200:1:        : domain:nwu.edu
-200:1:        : site:University of Arizona
-200:1:        : server:zippy.telcom.arizona.edu
-200:1:        : site:University of Florida
-200:1:        : server:ns.eng.ufl.edu
-200:1:        : site:University of Illinois at Urbana-Champaign
-200:1:        : server:ns.uiuc.edu
-200:1:        : domain:uiuc.edu
-200:1:        : site:University of Laval, Quebec, Canada
-200:1:        : server:ns.gel.ulaval.ca
-200:1:        : site:University of Minnesota
-200:1:        : server:ns.acs.umn.edu
-200:1:        : site:University of Waterloo, Ontario, Canada
-200:1:        : server:ns.waterloo.edu
-200:1:        : site:University of Wisconsin at Madison
-200:1:        : server:wisc.edu
-200:1:        : domain:wisc.edu
200:Ok.

query ns-servers type=serverlist return text
501:No matches to your query.
_____________________________________________________________________*/

static OSErr GetSiteInfo (void)

{
	Str255			cmd;				/* server command */
	OSErr				rCode;			/* error code */
	Str255			site;				/* site name */
	Str255			server;			/* server domain name */
	Str255			domain;			/* email domain name */
	Str255			siteStr;			/* "site" constant string */
	Str255			serverStr;		/* "server" constant string */
	Str255			domainStr;		/* "domain" constant string */
	char				*p;				/* pointer into response */
	char				*q;				/* pointer into response */
	char				*r;				/* pointer into response */
	short				code;				/* server response code */
	short				len1;				/* length of tag */
	short				len2;				/* length of string */
	long				serversSize;	/* size of Servers list */
	long				domainsSize;	/* size of Domains list */

	GetIndString(cmd, serverCmds, phSiteQuery);
	if (rCode = DoCommand(cmd)) return rCode;
	if (SiteMenu) DisposeMenu(SiteMenu);
	SiteMenu = nil;
	GetIndString(siteStr, stringsID, siteTag);
	GetIndString(serverStr, stringsID, serverTag);
	GetIndString(domainStr, stringsID, domainTag);
	SiteMenu = NewMenu(popupID, "\p");
	NumSites = 0;
	Servers = NewHandle(0);
	serversSize = 0;
	Domains = NewHandle(0);
	domainsSize = 0;
	*site = *server = *domain = 0;
	HLock(Response);
	p = *Response+1;
	while (true) {
		code = atoi(p);
		if (abs(code) == phSuccess) {
			if (code == -phSuccess) {
				if (Skip(&p)) break;
				if (Skip(&p)) break;
				if (Skip(&p)) break;
				r = strpbrk(p, ":\r");
				if (!r || *r != ':') break;
				len1 = r-p;
				if (len1 > 255) len1 = 255;
				r++;
				q = strchr(r, '\r');
				if (!q) break;
				len2 = q-r;
				if (len2 > 255) len2 = 255;
			}
			if (code == phSuccess ||
				(len1 == *siteStr && !strncmp(p, siteStr+1, len1))) {
				if (*site) {
					NumSites++;
					AppendMenu(SiteMenu, "\p ");
					SetItem(SiteMenu, NumSites, site);
					SetHandleSize(Servers, serversSize + *server + 1);
					utl_CopyPString(*Servers + serversSize, server);
					serversSize += *server + 1;
					SetHandleSize(Domains, domainsSize + *domain + 1);
					utl_CopyPString(*Domains + domainsSize, domain);
					domainsSize += *domain + 1;
				}
				if (code == phSuccess) break;
				*site = len2;
				memcpy(site+1, r, len2);
				*server = *domain = 0;
			} else if (len1 == *serverStr && !strncmp(p, serverStr+1, len1)) {
				*server = len2;
				memcpy(server+1, r, len2);
			} else if (len1 == *domainStr && !strncmp(p, domainStr+1, len1)) {
				*domain = len2;
				memcpy(domain+1, r, len2);
			} else {
				break;
			}
		} else if (code == phMatchCount) {
			q = strchr(p, '\r');
			if (!q) break;
		} else {
			break;
		}
		p = q+1;
	}
	HUnlock(Response);
	if (NumSites) {
		CalcMenuSize(SiteMenu);
	} else {
		DisposeMenu(SiteMenu);
		SiteMenu = nil;
	}
	query_NewSiteList();
	GetDateTime(&LastSiteUpdate);
	return noErr;
}

/*_____________________________________________________________________

	CheckLists - Check If Time to Update Site and Help Lists.
	
	Entry:	server = Ph server host domain name.
	
	Exit:		function result = error code.
_____________________________________________________________________*/

OSErr CheckLists (Str255 server)

{
	unsigned long		now;			/* current time */
	OSErr					rCode;		/* error code */

	GetDateTime(&now);
	if (EqualString(server, HelpServer, true, true)) {
		if (now - LastHelpUpdate > updateInterval) {
			if (rCode = GetHelpTopics(server)) return rCode;
		}
	}
	if (EqualString(server, DefaultServer, true, true)) {
		if (now - LastSiteUpdate > updateInterval) {
			if (rCode = GetSiteInfo()) return rCode;
		}
	}
	return noErr;
}

/*_____________________________________________________________________

	serv_Login - Login
	
	Entry:	server = Ph server host domain name.
				loginUser = login alias or username.
				pswd = login password.
				
	Exit:		function result = error code.
				loginAlias = login user alias.
				fields = handle to initialized field info array.
				numfields = number of fields.
				sCode = server response code:
					phSuccess = success.
					phPermErr = login failed due to bad username or password.
					phManyMatches = more than one match to username.
					phFailReadOnly = database is read-only.
					phFieldNotThere = record has no alias.
					other = unexpected.
				proxyList = handle to list of aliases for which the login
					alias is a proxy, as a sequence of Pascal strings.
				proxyCode = server response code to proxy query:
					phSuccess = success.
					phManyMatches = too many matches.
					phNoAuthSearch = field does not have Lookup attribute.
					phNoInxField = field does not have Indexed attribute.
					other = unexpected.
_____________________________________________________________________*/

OSErr serv_Login (Str255 server, Str255 loginUser, Str255 pswd,
	Str255 loginAlias, FieldInfo ***fields, short *numFields,
	short *sCode, Handle *proxyList, short *proxyCode)
	
{
	OSErr				rCode;			/* result code */
	short				code;				/* server response code */

	tran_BeginTransaction(waitGetRecord, nil);
	rCode = OpenConnection(server);
	if (!rCode)
		rCode = CheckLists(server);
	if (!rCode) 
		rCode = GetFieldInfo(fields, numFields, &code);
	if (!rCode && code == phSuccess) 
		rCode = Login(loginUser, pswd, loginAlias, &code);
	if (!rCode && code == phSuccess) 
		rCode = GetFieldData(loginAlias, *fields, *numFields, &code);
	if (!rCode && code == phSuccess)
		rCode = GetProxyList(loginAlias, proxyList, proxyCode);
	*sCode = code;
	if (!rCode) rCode = Quit();
	if (rCode) {
		AbortConnection();
	} else {
		rCode = CloseConnection();
	}
	tran_EndTransaction();
	return rCode;
}

/*_____________________________________________________________________

	serv_GetRecord - Get Ph Record
	
	Entry:	server = Ph server host domain name.
				loginUser = login alias.
				pswd = login password.
				user = alias or username of record to get from server.
				
	Exit:		function result = error code.
				fields = handle to initialized field info array.
				numfields = number of fields.
				canPut = true if login alias has permission to make changes
					to the user alias record.
				sCode = server response code:
					phSuccess = success.
					phNoMatches = no matches to username.
					phManyMatches = more than one match to username.
					other = unexpected.
_____________________________________________________________________*/

OSErr serv_GetRecord (Str255 server, Str255 loginUser, Str255 pswd, Str255 user,
	FieldInfo ***fields, short *numFields, short *sCode)
	
{
	OSErr				rCode;			/* result code */
	short				code;				/* server response code */
	Str255			junk;				/* junk */

	tran_BeginTransaction(waitGetRecord, nil);
	rCode = OpenConnection(server);
	if (!rCode)
		rCode = CheckLists(server);
	if (!rCode) 
		rCode = GetFieldInfo(fields, numFields, &code);
	if (!rCode && code == phSuccess) 
		rCode = Login(loginUser, pswd, junk, &code);
	if (!rCode && code == phSuccess) 
		rCode = GetFieldData(user, *fields, *numFields, &code);
	*sCode = code;
	if (!rCode) rCode = Quit();
	if (rCode) {
		AbortConnection();
	} else {
		rCode = CloseConnection();
	}
	tran_EndTransaction();
	return rCode;
}

/*_____________________________________________________________________

	serv_PutRecord - Put Ph Record
	
	Entry:	server = Ph server host domain name.
				loginAlias = login alias.
				pswd = login password.
				alias = alias of record to update.
				fields = handle to field info array.
				numFields = number of fields.
				
	Exit:		function result = error code.
				sCode = server response code:
					phSuccess = success.
					phFailReadOnly = database is read-only.
					other = unexpected.
_____________________________________________________________________*/

OSErr serv_PutRecord (Str255 server, Str255 loginAlias, Str255 pswd,
	Str255 alias, FieldInfo **fields, short numFields, short *sCode)
	
{
	OSErr				rCode;			/* result code */
	Str255			junk;				/* junk */
	short				code;				/* server response code */

	tran_BeginTransaction(waitPutRecord, nil);
	rCode = OpenConnection(server);
	if (!rCode)
		rCode = CheckLists(server);
	if (!rCode) 
		rCode = Login(loginAlias, pswd, junk, &code);
	if (!rCode && code == phSuccess) 
		rCode = PutFieldData(alias, fields, numFields);
	*sCode = code;
	if (!rCode) rCode = Quit();
	if (rCode) {
		AbortConnection();
	} else {
		rCode = CloseConnection();
	}
	tran_EndTransaction();
	return rCode;
}

/*_____________________________________________________________________

	serv_ChangePassword - Change Ph Password.
	
	Entry:	server = Ph server host domain name.
				loginAlias = login alias.
				pswd = login password.
				alias = alias of record to change.
				newPswd = new password.
				
	Exit:		function result = error code.
				sCode = server response code:
					phSuccess = success.
					phFailReadOnly = database is read-only.
					phIllegalVal = illegal value.
					other = unexpected.
				servErrMsg = server error message if sCode != phSuccess,
					or empty string if none.
_____________________________________________________________________*/

OSErr serv_ChangePassword (Str255 server, Str255 loginAlias, Str255 pswd, 
	Str255 alias, Str255 newPswd, short *sCode, Str255 servErrMsg)

{
	OSErr				rCode;			/* result code */
	Str255			junk;				/* junk */
	short				code;				/* server response code */

	*servErrMsg = 0;
	tran_BeginTransaction(waitChangePswd, nil);
	rCode = OpenConnection(server);
	if (!rCode)
		rCode = CheckLists(server);
	if (!rCode) 
		rCode = Login(loginAlias, pswd, junk, &code);
	if (!rCode && code == phSuccess) 
		rCode = ChangePassword(alias, newPswd, &code, servErrMsg);
	*sCode = code;
	if (!rCode) rCode = Quit();
	if (rCode) {
		AbortConnection();
	} else {
		rCode = CloseConnection();
	}
	tran_EndTransaction();
	return rCode;
}

/*_____________________________________________________________________

	serv_CreateRecord - Create Ph Record.
	
	Entry:	server = Ph server host domain name.
				loginAlias = login alias.
				loginPswd = login password.
				alias = alias for new record.
				name = name for new record.
				type = type of new record.
				pswd = password for new record.
				
	Exit:		function result = server response code.
				fields = handle to initialized field info array.
				numfields = number of fields.
				sCode = server response code:
					phSuccess = successful.
					phFailReadOnly = database is read-only.
					phDupAlias = duplicate alias.
					phIllegalVal = illegal value.
					other = unexpected.
				whichField = illegal field number if sCode != phSuccess:
					-1 = no field.
					0 = alias.
					1 = name.
					2 = type.
					3 = password.
				servErrMsg = server error message text if sCode != phSuccess
					and whichField >= 0.
_____________________________________________________________________*/

OSErr serv_CreateRecord (Str255 server, Str255 loginAlias, 
	Str255 loginPswd, Str255 alias, Str255 name, Str255 type, Str255 pswd, 
	FieldInfo ***fields, short *numFields, short *sCode, short *whichField,
	Str255 servErrMsg)
	
{
	OSErr				rCode;			/* result code */
	Str255			junk;				/* junk */
	short				code;				/* server response code */
	
	tran_BeginTransaction(waitNewRecord, nil);
	*whichField = -1;
	rCode = OpenConnection(server);
	if (!rCode)
		rCode = CheckLists(server);
	if (!rCode) 
		rCode = Login(loginAlias, loginPswd, junk, &code);
	if (!rCode && code == phSuccess) {
		*whichField = 0;
		rCode = CreateRecord(alias, &code, servErrMsg);
	}
	if (!rCode && code == phSuccess)
		rCode = CreateNewFields(alias, name, type, &code, whichField, servErrMsg);
	if (!rCode && code == phSuccess && *pswd) {
		*whichField = 3;
		rCode = ChangePassword(alias, pswd, &code, servErrMsg);
	}
	if (!rCode && code == phSuccess) {
		*whichField = -1;
		rCode = GetFieldInfo(fields, numFields, &code);
	}
	if (!rCode && code == phSuccess) 
		rCode = GetFieldData(alias, *fields, *numFields, &code);
	*sCode = code;
	if (!rCode && code != phSuccess && *whichField > 0)
		DeleteRecord(alias, &code);
	if (!rCode) rCode = Quit();
	if (rCode) {
		AbortConnection();
	} else {
		rCode = CloseConnection();
	}
	tran_EndTransaction();
	return rCode;
}

/*_____________________________________________________________________

	serv_DeleteRecord - Delete Ph Record.
	
	Entry:	server = Ph server host domain name.
				loginAlias = login alias.
				loginPswd = login password.
				alias = alias of record to be deleted.
				
	Exit:		function result = error code.
				sCode = server response code:
					phSuccess = success.
					phNoMatches = no matches to alias.
					phFailReadOnly = database is read-only.
					other = unexpected.
_____________________________________________________________________*/

OSErr serv_DeleteRecord (Str255 server, Str255 loginAlias,
	Str255 loginPswd, Str255 alias, short *sCode)
	
{
	OSErr				rCode;			/* result code */
	Str255			junk;				/* junk */
	short				code;				/* server response code */
	
	tran_BeginTransaction(waitDelRecord, nil);
	rCode = OpenConnection(server);
	if (!rCode)
		rCode = CheckLists(server);
	if (!rCode) 
		rCode = Login(loginAlias, loginPswd, junk, &code);
	if (!rCode && code == phSuccess) rCode = DeleteRecord(alias, &code);
	*sCode = code;
	if (!rCode) rCode = Quit();
	if (rCode) {
		AbortConnection();
	} else {
		rCode = CloseConnection();
	}
	tran_EndTransaction();
	return rCode;
}

/*_____________________________________________________________________

	serv_DoQuery - Do a Server Query.
	
	Entry:	server = Ph server host domain name.
				query = handle to query.
				queryLen = length of query.
				emailDomain = email domain name, empty string if none.
				reply = preallocated reply handle.
				
	Exit:		function result = error code.
				reply = handle to reply, size adjusted to exactly hold
					the reply.
				
				If server errors occur, the server error messages are
				placed in the reply. The match count message, if any,
				is also placed in the reply.
_____________________________________________________________________*/

OSErr serv_DoQuery (Str255 server, Handle query,
	unsigned short queryLen, Str255 emailDomain, Handle reply)
	
{
	OSErr			rCode;			/* result code */
	
	tran_BeginTransaction(waitQuery, nil);
	rCode = OpenConnection(server);
	if (!rCode)
		rCode = CheckLists(server);
	if (!rCode)
		rCode = SendQuery(query, queryLen, emailDomain, reply);
	if (!rCode) rCode = Quit();
	if (rCode) {
		AbortConnection();
	} else {
		rCode = CloseConnection();
	}
	tran_EndTransaction();
	return rCode;
}

/*_____________________________________________________________________

	serv_GetHelp - Get Server Help Text.
	
	Entry:	server = Ph server host domain name.
				topic = help topic.
				reply = preallocated reply handle.
				
	Exit:		function result = error code.
				reply = handle to reply, size adjusted to exactly hold
					the reply.
				
				If server errors occur, the server error messages are
				placed in the reply.
_____________________________________________________________________*/

OSErr serv_GetHelp (Str255 server, Str255 topic, Handle reply)
	
{
	OSErr			rCode;			/* result code */
	
	tran_BeginTransaction(waitHelp, nil);
	rCode = OpenConnection(server);
	if (!rCode)
		rCode = CheckLists(server);
	if (!rCode)
		rCode = GetHelp(topic, reply);
	if (!rCode) rCode = Quit();
	if (rCode) {
		AbortConnection();
	} else {
		rCode = CloseConnection();
	}
	tran_EndTransaction();
	return rCode;
}

/*_____________________________________________________________________

	serv_GetReadOnlyReason - Get Read Only Reason
				
	Exit:		reason = read only reason.
_____________________________________________________________________*/

void serv_GetReadOnlyReason (Str255 reason)

{
	utl_CopyPString(reason, ReadOnlyReason);
}

/*_____________________________________________________________________

	serv_GetSiteList - Get Site List.
	
	Entry:	server = Ph server host domain name.
				
	Exit:		function result = error code.
_____________________________________________________________________*/

OSErr serv_GetSiteList (Str255 server)

{
	OSErr			rCode;			/* result code */
	
	tran_BeginTransaction(waitSiteList, server);
	rCode = OpenConnection(server);
	if (!rCode)
		rCode = GetSiteInfo();
	if (!rCode) rCode = Quit();
	if (rCode) {
		AbortConnection();
	} else {
		rCode = CloseConnection();
	}
	tran_EndTransaction();
	return rCode;
}

/*_____________________________________________________________________

	serv_GetHelpList - Get Help Topic List.
	
	Entry:	server = Ph server host domain name.
				
	Exit:		function result = error code.
_____________________________________________________________________*/

OSErr serv_GetHelpList (Str255 server)

{
	OSErr			rCode;			/* result code */
	
	tran_BeginTransaction(waitHelpList, server);
	rCode = OpenConnection(server);
	if (!rCode)
		rCode = GetHelpTopics(server);
	if (!rCode) rCode = Quit();
	if (rCode) {
		AbortConnection();
	} else {
		rCode = CloseConnection();
	}
	tran_EndTransaction();
	return rCode;
}
