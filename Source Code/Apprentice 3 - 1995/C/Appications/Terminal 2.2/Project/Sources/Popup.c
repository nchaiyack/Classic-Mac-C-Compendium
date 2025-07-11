/*
	Terminal 2.2
	"Popup.c"
*/

#ifdef THINK_C
#include "MacHeaders"
#endif
#ifdef applec
#pragma load ":(Objects):MacHeadersMPW"
#pragma segment Options
#endif

#include "Popup.h"

static POPUP *Popup;	/* Current popup structure array */

#ifdef NEWPOPUP
/* ----- Draw downward pointing triangle ------------------------------- */

/*		***********
		 *********
		  *******
		   *****
		    ***
		+    *
*/

#define SYMBOL_WIDTH 12

static void DrawTriangle(register short h, register short v)
{
	register short i;

	for (i = 0; i < 6; ++i)	{
		MoveTo(h + 5 - i, v - i);
		Line(2*i, 0);
	}
}
#endif

/* ----- Find popup structure ------------------------------------------ */

static POPUP *FindPopup(
		register short item)	/* Dialog item number */
{
	register POPUP *p = Popup;

	while (p->item) {
			if (p->item == item)
				return p;		/* Here it is */
			++p;
	}
	return 0;					/* Not found */
}

/* ----- Draw popup box ------------------------------------------------ */

static pascal void DrawPopUp(	/* Called as user item in dialog */
	register DialogPtr dialog,
	register short item)
{
	register Byte text[256];
	PenState savePen;
	Rect box;
	FontInfo info;

	GetPenState(&savePen);
	GetFontInfo(&info);

	/* Get user item box. Reference point is top left. */
	{
		short type;
		Handle hdl;

		GetDItem(dialog, item, &type, &hdl, &box);
	}

	/* Get and draw menu title. Get item text. */
	{
		register POPUP *p = FindPopup(item);
		register Byte *s;

		if (!p || !p->h)
			return;
		s = (**(p->h)).menuData;	/* Menu title */
		MoveTo(box.left - StringWidth(s) - 2, box.top + info.ascent);
		DrawString(s);
		GetItem(p->h, p->choice, text);
	}

	/* Adjust item text so it fits in the box. */
	{
		register short wid = (box.right - box.left) -
			(CharWidth(checkMark) + 2);
		register short newWid = StringWidth(text);

		if (newWid > wid) {
			wid -= CharWidth('�');
			do {
				newWid -= CharWidth(text[text[0]]);
				--text[0];
			} while (newWid > wid && text[0]);
			++text[0];
			text[text[0]] = '�';
		}
	}

	/* Draw item text. */
	MoveTo(box.left + CharWidth(checkMark) + 2, box.top + info.ascent);
	DrawString(text);

#ifdef NEWPOPUP
	/* Draw downward pointing arrow */
	DrawTriangle(box.right - SYMBOL_WIDTH - 2, box.top + info.ascent);
#endif

	/* Draw box with dropping shadow. */
	InsetRect(&box, -1, -1);
	PenSize(1, 1);
	FrameRect(&box);
	MoveTo(box.left + 2, box.bottom);
	LineTo(box.right, box.bottom);
	LineTo(box.right, box.top + 2);

	SetPenState(&savePen);
}

/* ----- Handle MouseDown ---------------------------------------------- */

Boolean PopupMousedown(				/* Called from dialog filter */
	register DialogPtr dialog,
	register EventRecord *event,
	register short *i)
{
	register short item;
	register POPUP *p;
	Point loc;
	short choice;
	long chosen;
	short type;
	Handle hdl;
	Rect box, title;
	Boolean result = FALSE;

	/* Find dialog item where mouse click is in */

	loc = event->where;
	GlobalToLocal(&loc);
	if ((item = FindDItem(dialog, loc) + 1) < 1)
		return result;
	p = FindPopup(item);
	if (!p || !p->h)
		return result;

	/* Coordinate calculations */

	GetDItem(dialog, item, &type, &hdl, &box);
	loc = topLeft(box);
	LocalToGlobal(&loc);
	title = box;
	title.right = box.left - 1;
	title.left = title.right - StringWidth((**(p->h)).menuData) - 2;
	box.top -= 1;
	box.left -= 1;
	box.bottom += 2;
	box.right += 2;

	/* Erase popup box and invert title */

	EraseRect(&box);
	InvertRect(&title);

	/* Pop the menu up */

	InsertMenu(p->h, hierMenu);
	SetItemMark(p->h, p->choice, checkMark);
	CalcMenuSize(p->h);		/* Menu Mgr bug */
	chosen = PopUpMenuSelect(p->h, loc.v, loc.h, p->choice);
	SetItemMark(p->h, p->choice, noMark);
	DeleteMenu(p->menu);

	/* Get choice from popup menu */

	if (chosen) {
		choice = chosen & 0xFFFF;
		if (choice != p->choice) {	/* New choice */
			p->choice = choice;
			*i = item;
			result = TRUE;
		}
	}

	/* Redraw popup box */

	InvertRect(&title);
	DrawPopUp(dialog, item);

	return result;
}

/* ----- Init popup menus in dialog ------------------------------------ */

void PopupInit(
	register DialogPtr dialog,
	POPUP *popups)
{
	register POPUP *p;
	register short w;
	FontInfo info;
	short type;
	Handle hdl;
	Rect box;

	Popup = p = popups;
	SetPort(dialog);
	GetFontInfo(&info);
	while (p->item) {
		if (p->h = GetMenu(p->menu)) {
			/* Correct user item box. */
			GetDItem(dialog, p->item, &type, &hdl, &box);
			CalcMenuSize(p->h);
			w = (**(p->h)).menuWidth;
#ifdef NEWPOPUP
			w += SYMBOL_WIDTH + 2;
#endif
			if (w < (box.right - box.left))
				box.right = box.left + w;
			box.bottom = box.top +
				info.ascent + info.descent + info.leading;
			SetDItem(dialog, p->item, type, (Handle)DrawPopUp, &box);
		}
		++p;
	}
}

/* ---- Get rid of popup menus ----------------------------------------- */

void PopupCleanup(void)
{
	register POPUP *p = Popup;

	while (p->item) {
		if (p->h)
			ReleaseResource((Handle)p->h);
		++p;
	}
}
