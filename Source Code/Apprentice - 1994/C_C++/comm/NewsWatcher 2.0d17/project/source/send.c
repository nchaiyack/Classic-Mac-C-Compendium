/*----------------------------------------------------------------------------

	send.c

	This module handles sending postings and mail messages.
	
	Portions copyright � 1990, Apple Computer.
	Portions copyright � 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <string.h>

#include "glob.h"
#include "dlgutil.h"
#include "nntp.h"
#include "save.h"
#include "send.h"
#include "smtp.h"
#include "util.h"



#define kAskSendAlert		134			/* Send confirm dialog */



/*	CheckForSend asks the user if he/she would like to send the message
	that they composed before the window has been closed.
*/

Boolean CheckForSend (WindowPtr wind)
{
	TWindow **info;
	Handle Hdl;
	DialogPtr dlg;
	short item;

	info = (TWindow**)GetWRefCon(wind);
	Hdl = GetResource('DITL',kAskSendAlert);
	
	HNoPurge(Hdl);
	if((**info).kind == kPostMessage)
	{
		strncpy(&((*Hdl)[16]),"Post",4);
		ParamText("\pPost","\p","\p","\p");
	}
	else
	{
		strncpy(&((*Hdl)[16]),"Mail",4);
		ParamText("\pMail","\p","\p","\p");
	}
	dlg = MyGetNewDialog(kAskSendAlert);
	SetItemKeyEquivalent(dlg, 3, 'D');
	MyModalDialog(DialogFilter,&item,true,true);
	MyDisposDialog(dlg);
	
	switch (item) {
		case 1: /* send */
			return DoSendMsg(wind);
			break;
		case 2: /* cancel */
			return false;
		case 3: /* discard */
			return true;
	}
}


/*	WordWrap wraps paragraphs to lines of at most 75 characters long in all
	outgoing messages. Only body lines which do not start with '>' are wrapped.
*/

static void WordWrap (Handle h)
{
	long offset, len;
	char *p, *pEnd, *q, *lastSpace;
	
	offset = Munger(h,0,CRCR,2,nil,0);
	if (offset < 0) return;
	len = GetHandleSize(h);
	p = *h + offset + 2;
	pEnd = *h + len;
	while (p < pEnd) {
		if (*p == '>') {
			while (p < pEnd && *p != CR) p++;
			p++;
		} else {
			q = p;
			lastSpace = 0;
			while (true) {
				while (q < pEnd && *q != ' ' && *q != CR) q++;
				if (q - p >= 76 && lastSpace != 0) {
					*lastSpace = CR;
					p = lastSpace + 1;
					lastSpace = 0;
				}
				if (q >= pEnd || *q == CR) break;
				lastSpace = q;
				q++;
			}
			p = q+1;
		}
	}
}


/*	DoSendMsg either posts a message or sends the message through
	electronic mail (SMTP) after converting the appropriate header
	fields, etc�
*/

Boolean DoSendMsg (WindowPtr wind)
{
	TWindow **info;
	Handle bufa=nil,bufb=nil;
	unsigned long lena,lenb;
	Ptr p,q,pEnd;
	Boolean result;
	OSErr err;
	Boolean mustDispose;

	info = (TWindow**)GetWRefCon(wind);
	if ((**info).kind == kPostMessage) {
		StatusWindow("Posting message.");
	} else {
		StatusWindow("Mailing message.");
	}
	
	GetFullMessageText(wind, &bufa, &mustDispose);
	if (!mustDispose) {
		if ((err = MyHandToHand(&bufa)) != noErr) {
			bufa = nil;
			goto exit;
		}
	}
	lena = GetHandleSize(bufa);
	
	WordWrap(bufa);
	
	bufb = MyNewHandle(2*lena);
	if ((err = MyMemErr()) != noErr) goto exit;
	
	/* Convert all CR to CRLF. Also convert all '.' at bol to '..'
	   Also strip all trailing CR's. */
	
	p = *bufa;
	pEnd = p + lena;
	while (pEnd > p && *(--pEnd) == CR);
	pEnd++;
	q = *bufb;
	while (p < pEnd) {
		if (*p == '.') *q++ = '.';
		while (p < pEnd && *p != CR) *q++ = *p++;
		if (p < pEnd) {
			*q++ = CR;
			*q++ = LF;
			p++;
		}
	}
	lenb = q - *bufb;
	MyDisposHandle(bufa);
	bufa = nil;

	/* Send message. */
	
	HLock(bufb);
	switch ((**info).kind) {
		case kMailMessage:
			result = SendSMTP(*bufb,lenb);
			break;
		case kPostMessage:
			result = PostArticle(*bufb,lenb);
			break;
	}
	MyDisposHandle(bufb);
	if (result) (**info).changed = false;
	return result;
	
exit:

	UnexpectedErrorMessage(err);
	MyDisposHandle(bufa);
	MyDisposHandle(bufb);
	return false;
}