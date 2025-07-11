// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// URealNntp.cp

#include "URealNntp.h"
#include "UProgress.h"
#include "UPrefsDatabase.h"
#include "UPassword.h"
#include "NetAsciiTools.h"
#include "Tools.h"

#include <ErrorGlobals.h>
#include <RsrcGlobals.h>

#include <Resources.h>
#include <ToolUtils.h>
#include <OSUtils.h>
#include <Script.h>

#ifndef __STDIO__
#include <stdio.h>
#endif

#pragma segment MyComm

#define qDebugConstruct qDebug & 0


PRealNntp::PRealNntp()
{
#if qDebugConstruct
	fprintf(stderr, "PRealNntp::PRealNntp() at $%lx called\n", long(this));
#endif
	fPostingAllowed = false;
	fCmdP = nil;
	fAltCmdP = nil;
	fAvoidListOfGroupDescriptions = false;
}

void PRealNntp::IRealNntp(long newsServerAddr)
{
#if qDebugConstruct
	fprintf(stderr, "void PRealNntp::IRealNntp() at $%lx called\n", long(this));
#endif
	INntp();
	INetAsciiProtocol(newsServerAddr, gPrefs->GetShortPrefs('SvPo'));
	FailInfo fi;
	if (fi.Try())
	{
		fCmdP = NewPermPtr(kCommandBufferSize);
		*fCmdP = 0;
		fAltCmdP = NewPermPtr(kCommandBufferSize);
		*fAltCmdP = 0;

		LogCommands("Opening real nntp\n");
		
	// grab the hello msg
		GetRespondLine();
		long errorNumber, respondCode;
		if (sscanf(fRespondLineP, "gethostbyaddr: error %ld %ld", &errorNumber, &respondCode) == 2)
		{
			// bogous respond line from server (mac without dotname)
			fRespondCode = short(respondCode);
		}
		if (fRespondCode == 502)  // access restriction or permission denied
			FailOSErr(errNotAllowedToReadNews);
		if (fRespondCode != 200 && fRespondCode != 201)
			DoFailNntpError(fCmdP);
		fPostingAllowed = (fRespondCode == 200);
		if (strstr(fRespondLineP, "ANU"))
			fAvoidListOfGroupDescriptions = true;
		if (gPrefs->GetBooleanPrefs('AlAu'))
			DoAuth();
		fi.Success();
	}
	else // fail
	{
		delete this;
		fi.ReSignal();
	}
}

PRealNntp::~PRealNntp()
{
#if qDebugConstruct
	fprintf(stderr, "PRealNntp::~PRealNntp() at $%lx called\n", long(this));
#endif
	if (fCmdP && fAltCmdP && fRespondLineP) // say goodbye
		SayGoodbye();
	fCmdP = DisposeIfPtr(fCmdP);
	fAltCmdP = DisposeIfPtr(fAltCmdP);
	LogCommands("Closed real nntp\n");
}

void PRealNntp::DoFailNntpError(char *cmd)
{
#if qDebug
	fprintf(stderr, "PRealNntp::DoFailNntpError()\n");
	fprintf(stderr, "-   command = %s\n", cmd);
	fprintf(stderr, "-   respond code = %hd\n", fRespondCode);
	fprintf(stderr, "-   respond line = '%s'\n", fRespondLineP);
#else 
	cmd = cmd; // pragma unused
#endif
	if (fRespondCode >= 190 && fRespondCode <= 199)
		FailOSErr(errDebugOutput);
	switch (fRespondCode)
	{
		case 400:												// service discontinued
			{
				CStr255 tooManyString;
				MyGetIndString(tooManyString, kTooManyUsersRespond);
				if (strstr(fRespondLineP, tooManyString))
					FailOSErr(errTooManyUsers);
			}
			FailOSErr(errServiceAborted);
		case 411:												// no such news group
			FailOSErr(errNntpBadGroup);
		case 412:												// no newsgroup has been selected
			FailOSErr(errUnexpectedNntpError);
		case 420:												// no current article has been selected
			FailOSErr(errUnexpectedNntpError);
		case 423:												// no such article number in this group
			FailOSErr(errNoSuchArticle);
		case 430:												// no such article found
			FailOSErr(errNoSuchArticle);
		case 440:												// posting not allowed
			FailOSErr(errNotAllowedToPost);
		case 441:												// posting failed
			FailOSErr(errPostingFailed);
		case 500:												// command not recognized
			FailOSErr(errBadNntpServer);
		case 501:												// command syntax error
			FailOSErr(errBadNntpServer);
		case 502:												// access restriction or permission denied
			FailOSErr(errNntpPermissionDenied);
		case 503:												// program fault - command not performed
			FailOSErr(errNntpServerFailed);
		default:
			FailOSErr(errUnknownNntpError);
	}
}
		
void PRealNntp::DoSingleCommand(char *cmd)
{
	LogCommands(cmd);
	LogCommands("\n");
	SendCommand(cmd);
	while (true)
	{
		GetRespondLine();
		if (fRespondCode == 100)
			continue;
		if (fRespondCode >= 190 && fRespondCode <= 199) // debug output
			continue;
		break;
	}
}

void PRealNntp::DoAuth()
{
	CStr255 username, password;
	FailInfo fi;
	if (fi.Try())
	{
		GetUserNameAndPassword(username, password);
		sprintf(fAltCmdP, "authinfo user %s", (char*)username);
		BlockSet(Ptr(&username), sizeof(username), 0);
		DoSingleCommand(fAltCmdP);
		BlockSet(fAltCmdP, kCommandBufferSize, 0);
		if (fRespondCode == 482) // 482 Authorization already completed
		{
			fi.Success();
			return;
		}
		if (fRespondCode == 502) // 502 Authentication error
		{
			ForgetCurrentPassword();
			FailOSErr(errBadPassword);
		}
		if (fRespondCode == 281) // 281 Authentication accepted
		{
			fi.Success();
			return;
		}
		if (fRespondCode != 381) // 381 PASS required
		{
			ForgetCurrentPassword();
			DoFailNntpError(fAltCmdP);
		}
		sprintf(fAltCmdP, "authinfo pass %s", (char*)password);
		BlockSet(Ptr(&password), sizeof(password), 0);
		DoSingleCommand(fAltCmdP);
		BlockSet(fAltCmdP, kCommandBufferSize, 0);
		if (fRespondCode == 502) // 502 Authentication error
		{
			ForgetCurrentPassword();
			FailOSErr(errBadPassword);
		}
		if (fRespondCode != 281) // 281 Authentication accepted
		{
			ForgetCurrentPassword();
			DoFailNntpError(fAltCmdP);
		}
		fi.Success();
	}
	else // fail
	{
		BlockSet(fAltCmdP, kCommandBufferSize, 0);
		BlockSet(Ptr(&username), sizeof(username), 0);
		BlockSet(Ptr(&password), sizeof(password), 0);
		fi.ReSignal();
	}
}
		
void PRealNntp::DoCmd()
{
	DoSingleCommand(fCmdP);
	if (fRespondCode == 480) // 480 Authentication required for command
	{
		DoAuth();
		DoSingleCommand(fCmdP); // reissue command
	}
}
		
void PRealNntp::DoSetGroup(const CStr255 &name)
{
	sprintf(fCmdP, "group %s", (char*) name);
	DoCmd();
	if (fRespondCode == 502)  // access restriction or permission denied
		FailOSErr(errNotAllowedToReadNews);
	if (fRespondCode != 211) // n f l s group selected
		DoFailNntpError(fCmdP);
	long i1, i2, first, last;
	if (sscanf(fRespondLineP, "%ld %ld %ld %ld", &i1, &i2, &first, &last) != 4)
		FailOSErr(errBadNntpRespons);
	fFirstArticleID = first;
	fLastArticleID = last;
}


Handle PRealNntp::GetListOfAllGroups()
{
	strcpy(fCmdP, "list");
	DoCmd();
	if (fRespondCode != 215) // list of group follows
		DoFailNntpError(fCmdP);
	return GetDotTerminatedText();
}


Handle PRealNntp::GetListOfNewGroups(unsigned long fromDate)
{
	if (fromDate == 0) // 1904
		return GetListOfAllGroups();
	DateTimeRec dt;
	Secs2Date(fromDate, dt);
	MachineLocation loc;
	ReadLocation(loc);
	long gmt = dt.hour + 24 - short(loc.gmtFlags.gmtDelta & 0xFFFF) / 3600;
	sprintf(fCmdP, "newgroups %02hd%02hd%02hd %02hd%02hd%02hd GMT",
		dt.year%100, dt.month, dt.day, gmt % 24, dt.minute, dt.second);
	DoCmd();
	if (fRespondCode == 432) // list of new newsgroups unavaible
		FailOSErr(errNoListOfNewGroups);
	if (fRespondCode != 231) // list of new newsgroups follows
		DoFailNntpError(fCmdP);
	return GetDotTerminatedText();
}


Handle PRealNntp::GetListOfGroupDesciptions()
{
	if (fAvoidListOfGroupDescriptions)
		return NewPermHandle(0);
	strcpy(fCmdP, "list newsgroups");
	DoCmd();
	if (fRespondCode == 500) // command not recognized
		return NewPermHandle(0);
	if (fRespondCode == 501) // command syntax error
		return NewPermHandle(0);
	if (fRespondCode == 503) // No list of newsgroup descriptions available
		return NewPermHandle(0);
	if (fRespondCode != 215) // list of group follows
		DoFailNntpError(fCmdP);
	return GetDotTerminatedText();
}


Handle PRealNntp::GetHeaderList(const char *headerName, long firstArticleID, long lastArticleID)
{
	sprintf(fCmdP, "xhdr %s %ld-%ld", headerName, firstArticleID, lastArticleID);
	DoCmd();
	if (fRespondCode != 221) // xxxx fields follow
		DoFailNntpError(fCmdP);
	long oldProgress = gCurProgress->GetWork();
	Handle h = nil;
	VOLATILE(h);
	FailInfo fi;
	if (fi.Try())
	{
		h = GetDotTerminatedText();
		gCurProgress->SetWorkDone(oldProgress + lastArticleID - firstArticleID + 1);
		fi.Success();
		return h;
	}
	else // fail
	{
		h = DisposeIfHandle(h);
		fi.ReSignal();
	}
}


Handle PRealNntp::GetArticle(long articleID)
{
	sprintf(fCmdP, "article %ld", articleID);
	DoCmd();
	if (fRespondCode != 220) // <a> Article retrieved; head and body follow.
		DoFailNntpError(fCmdP);
	return GetDotTerminatedText();
}

void PRealNntp::SayGoodbye()
{
	FailInfo fi;
	if (fi.Try())
	{
		sprintf(fCmdP, "quit");
		// DoCmd() cannot be used as not response is sent
		SendCommand(fCmdP); // so we just sends it
		fi.Success();
	}
	else // fail
	{
#if qDebug
		if (fi.error)
			fprintf(stderr, "Got error when sending quit nntp command: %ld\n", long(fi.error));
#endif
		// don't care about errors when saying goodbye
	}
}

Boolean PRealNntp::IsPostingAllowed()
{
	return fPostingAllowed;
}

void PRealNntp::PostArticle(Handle h, short ackStringID)
{
	if (!fPostingAllowed)
		FailOSErr(errNotAllowedToPost);
	gCurProgress->SetWorkToDo(kCandyStribes);
	sprintf(fCmdP, "post");
	DoCmd();
	if (fRespondCode == 502) // access restriction or permission denied
		FailOSErr(errNotAllowedToPost);
	if (fRespondCode != 340) // 340 send article to be posted. End with <CR-LF>.<CR-LF>
		DoFailNntpError(fCmdP);
	gCurProgress->SetWorkToDo(GetHandleSize(h));
	SendDotTerminatedText(h);
	gCurProgress->SetWorkToDo(kCandyStribes);
	gCurProgress->SetText(ackStringID);
	GetRespondLine();
	if (fRespondCode != 240) // 240 article posted ok
		DoFailNntpError(fCmdP);
}

void PRealNntp::ExamineNewsServer()
{
	Handle h = GetResource('STR#', kCheckServerGroupsStrings);
	FailNILResource(h);
	short noGroups = **( (short**) h);
	Boolean foundGroup = false;
	Boolean gotDeniedPermissionError = false;
	long articleID;
	for (short groupIndex = 1; groupIndex <= noGroups; groupIndex++)
	{
		CStr255 groupName;
		GetIndString(groupName, kCheckServerGroupsStrings, groupIndex);
		sprintf(fCmdP, "group %s", (char*) groupName);
		DoCmd();
		if (fRespondCode == 502) // access restriction or permission denied
		{
			LogCommands("Was not allowed to select the group, jumps to next group\n");
			gotDeniedPermissionError = true;
			continue; // it could be that single group
		}
		if (fRespondCode != 211) // n f l s group selected
		{
			LogCommands("Got non-211 respond code, jumps to next group\n");
			continue;
		}
		long i1, i2, firstArticleID, lastArticleID;
		if (sscanf(fRespondLineP, "%ld %ld %ld %ld", &i1, &i2, &firstArticleID, &lastArticleID) != 4) 
		{
			LogCommands("Problem with sscan of line\n");
			FailOSErr(errBadNntpRespons);
		}
		if (firstArticleID > lastArticleID)
		{
			LogCommands("firstArticleID > lastArticleID, group is empty, jumps to next group\n");
			continue; // empty group
		}
		foundGroup = true;
		articleID = firstArticleID;
		break;
	}
	if (!foundGroup)
		if (gotDeniedPermissionError)
			FailOSErr(errNotAllowedToReadNews);
		else
			FailOSErr(errNewsServerNoKnownGroups);

	sprintf(fCmdP, "xhdr Subject %ld-%ld", articleID, articleID);
	DoCmd();
	if (fRespondCode == 500) // command not recognized
		FailOSErr(errMissingXHDRCommand);
	if (fRespondCode != 221) // xxxx fields follow
		DoFailNntpError(fCmdP);
	h = GetDotTerminatedText();
	h = DisposeIfHandle(h);
}
