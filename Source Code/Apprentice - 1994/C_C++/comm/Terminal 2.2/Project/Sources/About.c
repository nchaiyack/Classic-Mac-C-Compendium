/*
	Terminal 2.2
	"About.c"
*/

#ifdef THINK_C
#include "MacHeaders"
#endif
#ifdef applec
#pragma load ":(Objects):MacHeadersMPW"
#pragma segment About
#endif

#include "Text.h"
#include "Main.h"
#include "Utilities.h"

#define AboutOk		1
#define AboutText	2
#define AboutIcon	3
#define AboutHp		4
#define AboutVers	5
#define AboutLine1	6
#define AboutLine2	7

char Erny[] = 
	"\pWritten by:\015��Erny Tontlinger (CIS 73720,2200)\015"
	"��33, route d'Arlon\015��L-8410 Steinfort\015��Luxembourg";

/* ----- Get long version message from 'vers' resource ----------------- */

static void GetVersion(
	register short id,
	register Byte *message)
{
	register Handle h;
	register Byte *p;

	if (h = GetResource('vers', id)) {
		p = (Byte *)*h;
		p += 7 + p[6];
		memcpy(message, p, (long)(p[0] + 1));
		ReleaseResource(h);
	} else
		message[0] = 0;
}

/* ----- Draw user item in dialog box ---------------------------------- */

static pascal void DrawUser(	/* Called by Dialog Manager */
	register DialogPtr dialog,
	register short item)
{
	register Handle h;
	short type;
	Handle itemHdl;
	Rect box;

	GetDItem(dialog, item, &type, &itemHdl, &box);
	if (h = GetResource('ICN#', Application.icon)) {
		HLock(h);
		PlotIcon(&box, h);
		HUnlock(h);
	}
}

/* ----- About... dialog ----------------------------------------------- */

void About(short option)
{
	register Handle IDStrHandle;
	register Byte *s;
	register DialogPtr dialog;
	register long heap;
	long grow;
	short item;
	Handle itemHdl;
	Rect box;
	Byte num[256];

	heap = MaxMem(&grow);
	s = (Byte *)Erny;
	if (option & (optionKey | cmdKey | shiftKey | controlKey))
		IDStrHandle = 0;
	else
		if (IDStrHandle = GetResource(Application.signature,
				Application.version)) {
			HLock(IDStrHandle);
			s = (Byte *)*IDStrHandle;
		}
	CenterDialog('DLOG', DLOG_ABOUT);
	if (dialog = GetNewDialog(DLOG_ABOUT, 0, (WindowPtr)-1L)) {
		SetEText(dialog, AboutText, s);
		NumToString(heap, num);
		SetEText(dialog, AboutHp, num);
		GetDItem(dialog, AboutIcon, &item, &itemHdl, &box);
		SetDItem(dialog, AboutIcon, item, (Handle)DrawUser, &box);
		GetDItem(dialog, AboutLine1, &item, &itemHdl, &box);
		SetDItem(dialog, AboutLine1, item, (Handle)DrawUserLine, &box);
		GetDItem(dialog, AboutLine2, &item, &itemHdl, &box);
		SetDItem(dialog, AboutLine2, item, (Handle)DrawUserLine, &box);
		GetVersion(1, num);
		SetEText(dialog, AboutVers, num);
		ModalDialog(0, &item);
		DisposDialog(dialog);
	}
	if (IDStrHandle)
		ReleaseResource(IDStrHandle);
}
