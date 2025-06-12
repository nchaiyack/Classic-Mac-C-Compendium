/*
*	popup.c
*	Code for doing popup menus in our dialog boxes
*	Credit for this goes to Apple Sample Code and a conglomeration of other code I've seen.
*
*****************************************************************
*	NCSA Telnet for the Macintosh								*
*																*
*	National Center for Supercomputing Applications				*
*	Software Development Group									*
*	152 Computing Applications Building							*
*	605 E. Springfield Ave.										*
*	Champaign, IL  61820										*
*																*
*	Copyright (c) 1986-1992,									*
*	Board of Trustees of the University of Illinois				*
*****************************************************************
*  Revisions:
*  8/92		Telnet 2.6:	Initial version.  Jim Browne
*/

#ifdef MPW
#pragma segment Configure
#endif

#include "TelnetHeader.h"
#include "popup.h"
#include "popup.proto.h"

static popup *FindPopup(short item);
static pascal void DrawPopUp(DialogPtr dptr, short item);
PROTO_UPP(DrawPopUp, UserItem);

popup *Popup;	/* Current popup structure array */

static popup *FindPopup( short item)
{
	popup *p = Popup;

	while (p->item) {
			if (p->item == item)
				return p;
			++p;
	}
	return 0;					/* Not found */
}

SIMPLE_UPP(DrawPopUp, UserItem);
static pascal void DrawPopUp(DialogPtr dptr, short item)
{
	char text[256];
	char* s;
	PenState savePen;
	Rect box;
	FontInfo info;
	short type, width, newwidth;
	Handle hdl;
	popup *p;

	GetPenState(&savePen);
	GetFontInfo(&info);

	GetDItem(dptr, item, &type, &hdl, &box);

	/* Get and draw menu title. Get item text. */
	p = FindPopup(item);
	if (!p || !p->h) return;
	s = (char *)(**(p->h)).menuData;	/* Menu title */
	MoveTo(box.left - StringWidth((StringPtr)s) - 2, box.top + info.ascent);
	DrawString((StringPtr)s);
	GetItem(p->h, p->choice, (StringPtr)text);

	width = (box.right - box.left) - (CharWidth(checkMark) + 2);
	newwidth = StringWidth((StringPtr)text);

	if (newwidth > width) {
		width -= CharWidth('�');
		do {
			newwidth -= CharWidth(text[text[0]]);
			--text[0];
			} while (newwidth > width && text[0]);
		++text[0];
		text[text[0]] = '�';
		}
	
	MoveTo(box.left + CharWidth(checkMark) + 2, box.top + info.ascent);
	DrawString((StringPtr)text);

	InsetRect(&box, -1, -1);					/* A la Apple Sample Code */
	PenSize(1, 1);
	FrameRect(&box);
	MoveTo(box.left + 2, box.bottom);
	LineTo(box.right, box.bottom);
	LineTo(box.right, box.top + 2);

	SetPenState(&savePen);
}

Boolean PopupMousedown(	DialogPtr dptr, EventRecord *event, short *i)
{
	short item, choice, type;
	popup *p;
	Point clikloc;
	long chosen;
	Handle hdl;
	Rect box, title;
	Boolean result = FALSE;

	clikloc = event->where;
	GlobalToLocal(&clikloc);
	if ((item = FindDItem(dptr, clikloc) + 1) < 1)
		return result;
	p = FindPopup(item);
	if (!p || !p->h)
		return result;

	GetDItem(dptr, item, &type, &hdl, &box);
	clikloc = topLeft(box);
	LocalToGlobal(&clikloc);
	title = box;
	title.right = box.left - 1;
	title.left = title.right - StringWidth((**(p->h)).menuData) - 2;
	box.top -= 1;
	box.left -= 1;
	box.bottom += 2;
	box.right += 2;

	EraseRect(&box);
	InvertRect(&title);

	InsertMenu(p->h, hierMenu);
	SetItemMark(p->h, p->choice, checkMark);
	CalcMenuSize(p->h);
	chosen = PopUpMenuSelect(p->h, clikloc.v, clikloc.h, p->choice);
	SetItemMark(p->h, p->choice, noMark);
	DeleteMenu((**(p->h)).menuID);

	if (chosen) {
		choice = chosen & 0xFFFF;			/* Apple sez ignore high word */
		if (choice != p->choice) {
			p->choice = choice;
			*i = item;
			result = TRUE;
		}
	}

	InvertRect(&title);
	DrawPopUp(dptr, item);

	return result;
}

void PopupInit( DialogPtr dptr, popup *popups)
{
	popup *p;
	short w;
	FontInfo info;
	short type;
	Handle hdl;
	Rect box;

	Popup = p = popups;
	SetPort(dptr);
	GetFontInfo(&info);
	while (p->item) {
		if (p->h) {
			/* Correct user item box. */
			GetDItem(dptr, p->item, &type, &hdl, &box);
			CalcMenuSize(p->h);
			w = (**(p->h)).menuWidth;
			w += 14;
			if (w < (box.right - box.left))
				box.right = box.left + w;
			box.bottom = box.top + info.ascent + info.descent + info.leading;
			SetDItem(dptr, p->item, type, (Handle)DrawPopUpUPP, &box);
		}
		++p;
	}
}

void PopupCleanup(void)
{
	popup *p = Popup;

	while (p->item) {
		if (p->h)
			DisposeMenu(/*(Handle)*/p->h);	// Since we're using NewMenu's, we need
											// to do a DisposeMenu, rather than a 
											// Releaseresource
		++p;
	}
}


