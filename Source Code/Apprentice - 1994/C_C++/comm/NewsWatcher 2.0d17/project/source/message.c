/*----------------------------------------------------------------------------

	message.c

	This module handles the creation of message windows.
	
	Portions copyright © 1990, Apple Computer.
	Portions copyright © 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <Script.h>

#include "MacTCPCommonTypes.h"
#include "TCPPB.h"

#include "dlgutil.h"
#include "glob.h"
#include "header.h"
#include "message.h"
#include "open.h"
#include "resize.h"
#include "scroll.h"
#include "tcp.h"
#include "util.h"



#define kPostAlert			135			/* Post confirm dialog */
#define kPostDlg			136			/* Post dialog */
#define kRespDlg			137			/* Mail dialog */



static void CheckSubject (char *subject)
{
	if (*subject == 0)
		ErrorMessage("Please supply a Subject!");
}


static void CheckNewsgroups (char *newsgroups)
{
	if (*newsgroups == 0)
		ErrorMessage("You MUST supply Newsgroups!");
}


static OSErr MakeMsgId (char *MsgId)
{
	DateTimeRec	DateTime;
	CStr255	HostName;
	short	i;
	OSErr	err;

	*MsgId = 0;
	if(*gPrefs.address == 0)	return(-1);
	if((err = GetMyIPName(HostName)) != noErr)
	{
		if((err = GetMyIPAddrStr(HostName)) != noErr) return(err);
	}
	/*	Isolate the first part of the mail address	*/
	for(i=0; (gPrefs.address)[i] != 0 && (gPrefs.address)[i] != '@' && 
		(gPrefs.address)[i] != '%' && (gPrefs.address)[i] != '!'; i++);
	if(*HostName == 0)	return(-1);
	GetTime(&DateTime);
	sprintf(MsgId,"<%.*s-%.2i%.2i%.2i%.2i%.2i%.2i@%s>",
					i, gPrefs.address,
					DateTime.day,
					DateTime.month,
					DateTime.year%100,
					DateTime.hour,
					DateTime.minute,
					DateTime.second,
					HostName);
	return(noErr);
}


/*	InitHeaderDisplay intializes the header display for a message window
	(header shown or hidden)
*/

static void InitHeaderDisplay (WindowPtr wind)
{
	TWindow **info;
	
	info = (TWindow**)GetWRefCon(wind);
	(**info).headerShown = true;
	if (gPrefs.showHeaders) return;
	DoShowHideHeader(wind);
}

/*	Present "are you sure you want to post" alert. */

static Boolean AreYouSure (void)
{
	DialogPtr dlg;
	short item;

	if (!gPrefs.areYouSureAlert) return true;
	dlg = MyGetNewDialog(kPostAlert);
	MyModalDialog(DialogFilter, &item, true, true);
	MyDisposDialog(dlg);
	return (item == ok);
}


/*	IncludeQuote indents and includes the article to which the user is 
	responding in the new article template.
*/

static void IncludeQuote (WindowPtr parentWindow,
	WindowPtr newWindow, char refStr[4096])
{
	Handle text;
	TEHandle theTE;
	long teLength;
	short length;
	long offset;
	TWindow **info;
	CStr255 article, sender, mungeStr;
	char tmpStr[4096], *ptr;
	EWindowKind kind;
	
	info = (TWindow**) GetWRefCon(parentWindow);
	text = (**info).fullText;
	
	info = (TWindow**) GetWRefCon(newWindow);
	theTE = (**info).theTE;
	kind = (**info).kind;

	FindHeader(text,"Message-ID:",article, sizeof(article));
	if (kind == kPostMessage) {
		FindHeader(text,"From:",sender, sizeof(sender));
	} else {
		strcpy(sender, "you");
	}
	FindHeader(text,"References:",tmpStr, sizeof(tmpStr));

	ptr = tmpStr;
	length = strlen(article);
	while(*ptr && (strlen(ptr) + length > 4094)) {
		while(*ptr && *ptr != '<')	ptr++;
	}
	strcpy(refStr,ptr);
	if (*refStr != 0) strcat(refStr," ");
	strcat(refStr,article);
	
	if (MyHandToHand(&text) != noErr) return;
	if (MyMemErr() != noErr) return;
	
	offset = Munger(text,0,CRCR,2,nil,0) + 1;
	offset = Munger(text,0,nil,offset,"*",0);
	strcpy(mungeStr,CRSTR);
	strcat(mungeStr,"> ");
	while ( (offset = Munger(text,offset,CRSTR,1,mungeStr,3)) >= 0)
		;
	offset = Munger(text,0,nil,0,"In article ",11);
	offset = Munger(text,offset,nil,0,article,strlen(article));
	offset = Munger(text,offset,nil,0,", ",2);
	offset = Munger(text,offset,nil,0,sender,strlen(sender));
	offset = Munger(text,offset,nil,0," wrote:",7);
	offset = Munger(text,offset,nil,0,CRSTR,1);
	
	teLength = GetHandleSize(text);
	if (teLength > 25000) {
		MySetHandleSize(text,25000);
		teLength = 25000;
		ErrorMessage("Quoted text too long - truncated.");
	}
	HLock(text);
	TESetText(*text,teLength,theTE);
	TESetSelect(0,0,theTE);
	MyDisposHandle(text);
}


/*	AddEmptyLine adds an empty line after the headers	*/

static void AddEmptyLine (WindowPtr newWindow)
{
	TEHandle theTE;
	TWindow **info;
	
	info = (TWindow**)GetWRefCon(newWindow);
	theTE = (**info).theTE;
	
	TEInsert(CRSTR,1,theTE);
}

/*	AddHeader adds a header of name hName and contents hContents to the
	TextEdit field in window newWindow.
*/

static void AddHeader (char *hName, char *hContents, WindowPtr newWindow)
{
	TEHandle theTE;
	TWindow **info;
	
	info = (TWindow**)GetWRefCon(newWindow);
	theTE = (**info).theTE;
	
	if(*hContents == 0) return;
	
	TEInsert(hName,strlen(hName),theTE);
	TEInsert(hContents,strlen(hContents),theTE);
	TEInsert(CRSTR,1,theTE);
}


static void AddDateHeader (WindowPtr newWindow)
{
	static char *days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	static char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
							 "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

	DateTimeRec date;
	MachineLocation loc;
	long gmtDelta;
	char gmtSign;
	short gmtHours;
	short gmtMinutes;
	CStr255 dateHeader;
	
	ReadLocation(&loc);
	if (loc.latitude == 0 && loc.longitude == 0 && loc.gmtFlags.gmtDelta == 0) return;
	gmtDelta = loc.gmtFlags.gmtDelta & 0x00ffffff;
	if ((gmtDelta >> 23) & 1) gmtDelta |= 0xff000000;
	if (gmtDelta < 0) {
		gmtSign = '-';
		gmtDelta = -gmtDelta;
	} else {
		gmtSign = '+';
	}
	gmtHours = gmtDelta / 3600;
	gmtMinutes = (gmtDelta % 3600) / 60;
	GetTime(&date);
	sprintf(dateHeader, "%s, %.2d %s %.4d %.2d:%.2d:%.2d %c%.2d%.2d",
		days[date.dayOfWeek - 1],
		date.day,
		months[date.month - 1],
		date.year,
		date.hour,
		date.minute,
		date.second,
		gmtSign,
		gmtHours,
		gmtMinutes
	);
	AddHeader("Date: ", dateHeader, newWindow);
}


/*	AddSignature adds the signature, if any, to the
	TextEdit field in window newWindow.
*/

static void AddSignature (WindowPtr newWindow)
{
	TEHandle theTE;
	long tePos;
	TWindow **info;
	
	info = (TWindow**)GetWRefCon(newWindow);
	theTE = (**info).theTE;
	
	if (*gPrefs.signature != 0 && strlen(gPrefs.signature) != 0) {
		tePos = (**theTE).selStart;
		TESetSelect(0x7fff,0x7fff,theTE);
		TEInsert(CRSTR,1,theTE);
		TEInsert(CRSTR,1,theTE);
		if (gPrefs.addSigSeparatorLine) {
			TEInsert("-- ",3,theTE);
			TEInsert(CRSTR,1,theTE);
		}
		TEInsert(gPrefs.signature,strlen(gPrefs.signature),theTE);
		TEDeactivate(theTE);
		(**theTE).clikStuff = 255;
		TESetSelect(tePos,tePos,theTE);
		TEActivate(theTE);
	}
}


/*	FixGroupList takes a comma or space separated list of group names
	and replaces it by a comma separated list with no spaces. */

static void FixGroupList (char *str)
{
	char *p, *q;
	
	for (p = q = str; *p; p++) {
		if (*p == ' ') {
			if (q > str && *(q-1) != ',') *q++ = ','; 
		} else {
			*q++ = *p;
		}
	}
	*q = 0;
}


/*	MakeFollowUp creates a new article template for a follow-up back to
	a newsgroup.
*/

void MakeFollowUp (WindowPtr wind)
{
	DialogPtr dlg;
	short item;
	TWindow **info;
	WindowPtr newWind;
	GrafPtr savePort;
	Point firstOffset;
	CStr255 groupStr,distStr,subjectStr,followStr;
	CStr255 messageID,path,tmpStr;
	char fromStr[515];
	char refStr[4096];
	short i;
	Handle text;
	
	SetPt(&firstOffset,0,0);
	GetPort(&savePort);
	SetPort(wind);
	LocalToGlobal(&firstOffset);
	SetPort(savePort);

	if (!AreYouSure()) return;
		
	info = (TWindow**) GetWRefCon(wind);
	text = (**info).fullText;

	if(FindHeader(text,"Followup-To:", followStr, sizeof(followStr))) {
		if(strncasecmp(followStr,"poster",6) == 0) {
			MakeRespond(wind);
			return;
		}
		strcpy(groupStr,followStr);
	} else if (FindHeader(text,"Newsgroups:", followStr, sizeof(followStr))) {
		strcpy(groupStr,followStr);
		for(i=0; i<strlen(groupStr); i++)
			if(groupStr[i] == ',') groupStr[i] = 0;
	}

	if(FindHeader(text,"Subject:",tmpStr, sizeof(subjectStr)-4)) {
		strcpy(subjectStr,"Re: ");
		if (strncasecmp(tmpStr,"Re: ",4) == 0) {
			strcat(subjectStr, &tmpStr[4]);
		} else {
			strcat(subjectStr, tmpStr);
		}
	}

	if(!FindHeader(text,"Distribution:", distStr, sizeof(distStr))) {
		strcpy(distStr, "");		
	}

	dlg = MyGetNewDialog(kPostDlg);
	
	DlgSetCString(dlg, 3, groupStr);
	DlgSetCString(dlg, 4, followStr);
	DlgSetCString(dlg, 5, distStr);
	DlgSetCString(dlg, 6, subjectStr);

	SelIText(dlg,6,0,32000);
	
	do {
		DlgEnableItem(dlg, ok, *groupStr != 0 && *subjectStr != 0);
		MyModalDialog(DialogFilter, &item, true, true);
		DlgGetCString(dlg, 3, groupStr);
		DlgGetCString(dlg, 6, subjectStr);
	} while (item != ok && item != cancel);
	
	if (item == ok) {
		DlgGetCString(dlg, 4, followStr);
		DlgGetCString(dlg, 5, distStr);
	}
	
	MyDisposDialog(dlg);
	
	if (item == cancel) return;
	
	FixGroupList(groupStr);
	FixGroupList(followStr);
	
	strcpy(fromStr,gPrefs.address);
	strcat(fromStr," (");
	strcat(fromStr,gPrefs.fullName);
	strcat(fromStr,")");
	
	c2pstr(subjectStr);
	info = (TWindow**) GetWRefCon(newWind = 
		MakeNewWindow(kPostMessage,firstOffset,(StringPtr)subjectStr));
	p2cstr((StringPtr)subjectStr);

	(**info).changed = true;
	IncludeQuote(wind,newWind,refStr);

	if(GetMyIPName(path) == noErr && strlen(path) <= 250) {
		strcat(path,"!user");
		AddHeader("Path: ",path,newWind);
	} else {
		AddHeader("Path: ","NewsWatcher!user",newWind);
	}
	AddDateHeader(newWind);
	AddHeader("From: ",fromStr,newWind);
	AddHeader("Newsgroups: ",groupStr,newWind);
	AddHeader("Followup-To: ",followStr,newWind);
	AddHeader("Distribution: ",distStr,newWind);
	AddHeader("Subject: ",subjectStr,newWind);

	if(MakeMsgId(messageID) == noErr)
		AddHeader("Message-ID: ", messageID, newWind);
	AddHeader("References: ",refStr,newWind);
	AddHeader("Organization: ",gPrefs.organization,newWind);
	AddEmptyLine(newWind);

	CheckNewsgroups(groupStr);
	CheckSubject(subjectStr);

	AddSignature(newWind);
	InitHeaderDisplay(newWind);

	DoZoom(newWind,inZoomOut);
	AdjustScrollBar(newWind);

	ShowWindow(newWind);
}


/*	MakeRespond creates a new message template for an e-mail response to
	a news article.
*/

void MakeRespond (WindowPtr wind)
{
	DialogPtr dlg;
	short item;
	TWindow **info;
	WindowPtr newWind;
	Point firstOffset;
	GrafPtr savePort;
	CStr255 recipStr, subjectStr, tmpStr;
	char refStr[4096], fromStr[515];
	Handle text;

	SetPt(&firstOffset,0,0);
	GetPort(&savePort);
	SetPort(wind);
	LocalToGlobal(&firstOffset);
	SetPort(savePort);
	
	info = (TWindow**) GetWRefCon(wind);
	text = (**info).fullText;

	if(FindHeader(text,"Subject:",tmpStr,sizeof(subjectStr)-4)) {
		strcpy(subjectStr,"Re: ");
		if (strncasecmp(tmpStr,"Re: ",4) == 0) {
			strcat(subjectStr, &tmpStr[4]);
		} else {
			strcat(subjectStr, tmpStr);
		}
	}

	if (!FindHeader(text,"Reply-To:",recipStr, sizeof(recipStr)))
		FindHeader(text,"From:",recipStr, sizeof(recipStr));

	dlg = MyGetNewDialog(kRespDlg);
	
	DlgSetCString(dlg, 3, recipStr);
	DlgSetCString(dlg, 4, subjectStr);

	SelIText(dlg,4,32000,32000);

	do {
		DlgEnableItem(dlg, ok, *recipStr != 0 && *subjectStr != 0);
		MyModalDialog(DialogFilter,&item,true,true);
		DlgGetCString(dlg, 3, recipStr);
		DlgGetCString(dlg, 4, subjectStr);
	} while (item != ok && item != cancel);
	
	MyDisposDialog(dlg);

	if (item == cancel) return;

	strcpy(fromStr,gPrefs.address);
	strcat(fromStr," (");
	strcat(fromStr,gPrefs.fullName);
	strcat(fromStr,")");
		
	c2pstr(subjectStr);
	info = (TWindow**) GetWRefCon(newWind = 
		MakeNewWindow(kMailMessage,firstOffset,(StringPtr)subjectStr));
	p2cstr((StringPtr)subjectStr);
		
	(**info).changed = true;
	IncludeQuote(wind,newWind,refStr);

	AddDateHeader(newWind);
	AddHeader("From: ",fromStr,newWind);
	AddHeader("To: ",recipStr,newWind);
	if (gPrefs.replyCC) AddHeader("Cc: ",gPrefs.address,newWind);
	AddHeader("Subject: ",subjectStr,newWind);
	AddHeader("References: ",refStr,newWind);
	AddHeader("Organization: ",gPrefs.organization,newWind);
	AddEmptyLine(newWind);

	AddSignature(newWind);
	InitHeaderDisplay(newWind);

	DoZoom(newWind,inZoomOut);
	AdjustScrollBar(newWind);

	ShowWindow(newWind);
}


/*	MakePost creates a new article template for a new posting to a user-
	specified newsgroup.
*/

void MakePost (WindowPtr wind)
{
	DialogPtr dlg;
	short item;
	TWindow **info;
	WindowPtr newWind;
	Point firstOffset;
	CStr255 groupStr, distStr, subjectStr, followStr, messageID, path;
	char fromStr[515];
	ListHandle theList;
	TGroup **groupArray;
	short numSelected, cellData, cellDataLen;
	Cell theCell;
	Handle strings;
	
	if (!AreYouSure()) return;

	*groupStr = 0;
	if (wind != nil) {
		info = (TWindow**)GetWRefCon(wind);
		switch ((**info).kind) {
			case kFullGroup:
			case kNewGroup:
			case kUserGroup:
				theList = (**info).theList;
				groupArray = (**info).groupArray;
				strings = (**info).strings;
				SetPt(&theCell,0,0);
				numSelected = 0;
				while (LGetSelect(true,&theCell,theList)) {
					if (++numSelected > 1) break;
					cellDataLen = 2;
					LGetCell(&cellData, &cellDataLen, theCell, theList);
					strcpy(groupStr, *strings + (*groupArray)[cellData].nameOffset);
					theCell.v++;
				}
				if (numSelected != 1) *groupStr = 0;
				break;
			case kSubject:
				GetWTitle(wind, (StringPtr)groupStr);
				p2cstr((StringPtr)groupStr);
				break;
			case kArticle:
				if ((**info).parentWindow != nil) {
					GetWTitle((**info).parentWindow, (StringPtr)groupStr);
					p2cstr((StringPtr)groupStr);
				}
				break;
		}
	}
	
	dlg = MyGetNewDialog(kPostDlg);

	DlgSetCString(dlg, 3, groupStr);
	DlgSetCString(dlg, 4, groupStr);
	DlgSetCString(dlg, 5, "");

	if (*groupStr != 0) {
		SelIText(dlg,6,32000,32000);
	} else {
		SelIText(dlg,3,0,0);
	}

	do {
		DlgEnableItem(dlg, ok, *groupStr != 0 && *subjectStr != 0);
		MyModalDialog(DialogFilter,&item,true,true);
		DlgGetCString(dlg, 3, groupStr);
		DlgGetCString(dlg, 6, subjectStr);
	} while (item != ok && item != cancel);
	
	if (item == ok) {
		DlgGetCString(dlg, 4, followStr);
		DlgGetCString(dlg, 5, distStr);
	}

	MyDisposDialog(dlg);

	if (item == cancel) return;
	
	FixGroupList(groupStr);
	FixGroupList(followStr);
	
	strcpy(fromStr,gPrefs.address);
	strcat(fromStr," (");
	strcat(fromStr,gPrefs.fullName);
	strcat(fromStr,")");
		
	SetPt(&firstOffset,kOffLeft,kOffTop);
	
	c2pstr(subjectStr);
	info = (TWindow**) GetWRefCon(newWind = 
		MakeNewWindow(kPostMessage,firstOffset,(StringPtr)subjectStr));
	p2cstr((StringPtr)subjectStr);
		
	(**info).changed = true;
	
	if(GetMyIPName(path) == noErr && strlen(path) <= 250) {
		strcat(path,"!user");
		AddHeader("Path: ",path,newWind);
	} else {
		AddHeader("Path: ","NewsWatcher!user",newWind);
	}
	AddDateHeader(newWind);
	AddHeader("From: ",fromStr,newWind);
	AddHeader("Newsgroups: ",groupStr,newWind);
	AddHeader("Followup-To: ",followStr,newWind);
	AddHeader("Distribution: ",distStr,newWind);
	AddHeader("Subject: ",subjectStr,newWind);
	
	if(MakeMsgId(messageID) == noErr)
		AddHeader("Message-ID: ", messageID, newWind);
	AddHeader("Organization: ",gPrefs.organization,newWind);
	AddEmptyLine(newWind);

	CheckNewsgroups(groupStr);
	CheckSubject(subjectStr);
	
	AddSignature(newWind);
	InitHeaderDisplay(newWind);

	DoZoom(newWind,inZoomOut);
	AdjustScrollBar(newWind);

	ShowWindow(newWind);
}
