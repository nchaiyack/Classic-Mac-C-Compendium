/* MAC STDWIN -- MENUS. */

/* XXX Ought to find a way to allow the THINK C console menus
   to work when the console is active */

#include "macwin.h"
#ifdef MPW
#include <Fonts.h>
#include <Menus.h>
#include <ToolUtils.h>
#endif
#ifdef THINK_C_PRE_5_0
#include <MenuMgr.h>
#endif

#define MENUID(mp) ((*((MenuHandle)(mp)))->menuID)

bool _wmenuhilite;		/* Set if HiliteMenu(0) needed */

static bool deflocal= FALSE;	/* Default menu state */

static int firstlocal;		/* First local menu in menu bar */

/* Function prototypes */

STATIC void addtobar _ARGS((struct menubar *mbp, MENU *mp));
STATIC void delfrombar _ARGS((struct menubar *mbp, MENU *mp));

static void
addtobar(mbp, mp)
	struct menubar *mbp;
	MENU *mp;
{
	int i;
	
	for (i= 0; i < mbp->nmenus; ++i) {
		if (mp == mbp->menulist[i])
			return; /* Already attached */
	}
	L_APPEND(mbp->nmenus, mbp->menulist, MENU *, mp);
}

static void
delfrombar(mbp, mp)
	struct menubar *mbp;
	MENU *mp;
{
	int i;
	
	for (i= 0; i < mbp->nmenus; ++i) {
		if (mp == mbp->menulist[i]) {
			L_REMOVE(mbp->nmenus, mbp->menulist, MENU *, i);
			break;
		}
	}
}

MENU *
wmenucreate(id, title)
	int id;
	char *title;
{
	MENU *mp= (MENU *)NewMenu(id, PSTRING(title));
	
	if (!deflocal) {
		InsertMenu(mp, firstlocal);
		DrawMenuBar();
	}
	return mp;
}

void
wmenudelete(mp)
	MENU *mp;
{
	WindowPtr w;
	
	for (w= FrontWindow(); w != NULL;
		w= (WindowPtr)((WindowPeek)w)->nextWindow) {
		WINDOW *win= whichwin(w);
		if (win != NULL)
			delfrombar(&win->mbar, mp);
	}
	DeleteMenu(MENUID(mp));
	DrawMenuBar();
	DisposeMenu(mp);
}

int
wmenuadditem(mp, text, shortcut)
	MENU *mp;
	char *text;
	int shortcut;
{
	char buf[256];
	int item;
	
	if (text == NULL || *text == EOS)
		strcpy(buf, "(-");
	else
		strcpy(buf, "x");
	if (shortcut >= 0) {
		char *p= buf + strlen(buf);
		*p++= '/';
		*p++= shortcut;
		*p= EOS;
	}
	AppendMenu(mp, PSTRING(buf));
	item= CountMItems(mp);
	if (text != NULL && *text != EOS)
		SetItem(mp, item, PSTRING(text));
	return item-1;
}

void
wmenusetitem(mp, item, text)
	MENU *mp;
	int item;
	char *text;
{
	++item;
	if (text == NULL || *text == EOS)
		DisableItem(mp, item);
	else {
		EnableItem(mp, item);
		SetItem(mp, item, PSTRING(text));
	}
}

void
wmenuenable(mp, item, flag)
	MENU *mp;
	int item;
	int flag;
{
	++item;
	if (flag)
		EnableItem(mp, item);
	else
		DisableItem(mp, item);
}
void
wmenucheck(mp, item, flag)
	MENU *mp;
	int item;
	int flag;
{
	++item;
	CheckItem(mp, item, flag);
}

void
wmenuattach(win, mp)
	WINDOW *win;
	MENU *mp;
{
	addtobar(&win->mbar, mp);
	if (win == active) {
		InsertMenu(mp, 0);
		DrawMenuBar();
	}
}

void
wmenudetach(win, mp)
	WINDOW *win;
	MENU *mp;
{
	delfrombar(&win->mbar, mp);
	if (win == active) {
		DeleteMenu(MENUID(mp));
		DrawMenuBar();
	}
}

void
wmenusetdeflocal(local)
	bool local;
{
	deflocal= local;
}

/* Interface routines for the rest of the library. */

void
initmbar(mb)
	struct menubar *mb;
{
	L_INIT(mb->nmenus, mb->menulist);
}

void
killmbar(mb)
	struct menubar *mb;
{
	L_DEALLOC(mb->nmenus, mb->menulist);
}

void
addlocalmenus(win)
	WINDOW *win;
{
	int i;
	
	firstlocal= 0;
	for (i= 0; i < win->mbar.nmenus; ++i) {
		if (firstlocal == 0)
			firstlocal= MENUID(win->mbar.menulist[i]);
		InsertMenu(win->mbar.menulist[i], 0);
	}
	if (i > 0)
		DrawMenuBar();
}

void
rmlocalmenus(win)
	WINDOW *win;
{
	int i;
	
	firstlocal= 0;
	for (i= 0; i < win->mbar.nmenus; ++i)
		DeleteMenu(MENUID(win->mbar.menulist[i]));
	if (i > 0)
		DrawMenuBar();
}

void
_wdo_menu(ep, menu_item)
	EVENT *ep;
	long menu_item;
{
	int id= HiWord(menu_item);
	int item= LoWord(menu_item);
	
	if (id == 0)
		return;
	if (id == APPLE_MENU) {
		if (item == 1)
			do_about();
		else {
			char name[256];
			GetItem(GetMHandle(id), item, name);
			(void) OpenDeskAcc(name);
		}
		HiliteMenu(0);
	}
	else {
		ep->type= WE_MENU;
		ep->u.m.id= id;
		ep->u.m.item= item-1;
		_wmenuhilite= TRUE;
	}
}

/* Call this routine exactly once to initialize the Apple menu. */

char *about_item= "About STDWIN...";

void
setup_menus()
{
	static char applename[2]= {appleMark, EOS};
	MenuHandle m;
	
	m= (MenuHandle)wmenucreate(APPLE_MENU, applename);
	AppendMenu(m, PSTRING(about_item));
	AppendMenu(m, PSTRING("(-"));
	AddResMenu(m, 'DRVR');
}
