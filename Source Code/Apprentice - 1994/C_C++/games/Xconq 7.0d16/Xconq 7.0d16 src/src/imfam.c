/* Main program of ImFam. */

/* ImFam is a very basic app; its main functions are to translate and to preview
   Xconq images and image families. */

#ifdef THINK_C
#include <MacHeaders>
#else /* probably MPW */

#endif

/* (should fix these wired pathnames eventually) */

#include "::src:config.h"
#include "::src:misc.h"
#include "::src:lisp.h"

#include "::mac:macimf.h"

int interp_bytes(Obj *datalist, int numbytes, char **dataout);

/* The upper limit on number of image families. */

#define MAXIMAGES 1000

/* Definitions for the menus. */

#define mbMain 128

#define mApple 128

#define miAbout 1

#define mFile 129

#define miFileOpenImf 1
#define miFileOpenResources 2
/* 3 */
#define miFileSaveImf 4
#define miFileSaveResources 5
/* 6 */
#define miFileQuit 7

#define mEdit 130

#define miEditCut 1
#define miEditCopy 2
#define miEditPaste 3
#define miEditClear 4

#define mView 131
#define miView4x4 1
#define miView8x8 2
#define miView16x16 3
#define miView32x32 4
/* 5 */
#define miViewColor 6
#define miViewNames 7
#define miViewMask 8
/* 9 */
#define miViewAsUnit 10
#define miViewAsTerrain 11
#define miViewAsEmblem 12
#define miViewWithUnit 13
#define miViewWithTerrain 14
#define miViewWithEmblem 15
/* 16 */
#define miViewIcons 17
#define miViewTiles 18

#define wImages 128

#define aAbout 129

#define pascalify(STR,OUT) \
  strcpy(((char *) (OUT) + 1), STR);  (OUT)[0] = strlen(STR);

#define p2c(pstr,buf)  \
  strncpy(buf, ((char *) (pstr) + 1), pstr[0]);  buf[pstr[0]] = '\0';

#define hextoi(c) (((c) >= '0' && (c) <= '9') ? ((c) - '0') : ((c) - 'a' + 10))

int debugoutput = 1;

/* Variables tweaked by menu items. */

int showcolor = 1;
int shownames = 1;
int showmask = 1;

int varyunit = 1;
int varyterrain = 0;
int varyemblem = 0;

int withunit = 0;
int withterrain = 0;
int withemblem = 0;

int selecticons = 1;
int selecttiles = 1;

/* Pointers to the image families being held constant for display. */

ImageFamily *constterrain = NULL;

ImageFamily *constunit = NULL;

ImageFamily *constemblem = NULL;

int iw = 16, ih = 16;

int hasColorQD;

int useWNE;

char spbuf[1000];
char tmpbuf[1000];

Str255 tmpstr;

/* Resource ids. */

short nextimageid;
short nextpatid;
short nextsicnid;
short nexticonid;
short nextcolorid;
short nextimfid;

/* Pointer to storage for all the image families. */

ImageFamily **images;

int numimages = 0;

ImageFamily *selectedimf = NULL;

int selectedn = 0;

ImageColor **colors;

int numcolors = 0;

WindowPtr imagewin = NULL;

Rect dragrect = { 0, 0, 9999, 9999 };
Rect sizerect;

/* This variable leaves space for two lines of text at the top of the window. */

int toplineh = 32;

ControlHandle vscrollbar = nil;

Rect vscrollrect;

int sbarwid = 15;

int numvisrows = 0;

int firstvisrow = 0;

int winwidth, winheight;

int rows, cols;

int eltw, elth;

CursHandle watchcursor;

char *copy_string();
ImageFamily *get_imf();
ImageFamily *find_imf();
ImageColor *get_imc();
ImageColor *find_imc();

main()
{
	SysEnvRec se;
	Handle menubar;
	MenuHandle menu;
	WindowPtr win;
	Rect r;
	RgnHandle cursorrgn;
	Boolean gotevent;
	EventRecord	event;
	DialogPtr dialog;
	short itemhit;

	InitGraf(&thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(NULL);
	InitCursor();
	watchcursor = GetCursor(watchCursor);

	SysEnvirons(2, &se);
	hasColorQD = se.hasColorQD;
	/* Set up the menu bar.  No trickery needed. */
	menubar = GetNewMBar(mbMain);
	SetMenuBar(menubar);
	/* Add the DAs etc as usual. */
	if ((menu = GetMHandle(mApple)) != nil) {
		AddResMenu(menu, 'DRVR');
	}
	DrawMenuBar();

	init_lisp();
	numimages = 0;
	/* Allocate plenty of room to play in. */
	images = (ImageFamily **) xmalloc(MAXIMAGES * sizeof(ImageFamily *));
	colors = (ImageColor **) xmalloc(MAXIMAGES * sizeof(ImageColor *));

	/* Create the main window we're going to play in. */
	if (hasColorQD) {
		imagewin = GetNewCWindow(wImages, NULL, (WindowPtr) -1L);
	} else {
		imagewin = GetNewWindow(wImages, NULL, (WindowPtr) -1L);
	}
	vscrollrect = imagewin->portRect;
	vscrollrect.left = vscrollrect.right - sbarwid;  vscrollrect.top += toplineh - 1;
	vscrollrect.right += 1;  vscrollrect.bottom -= sbarwid - 1;
	vscrollbar =
		NewControl(imagewin, &vscrollrect, "\p", 1, 0, 0, 100, scrollBarProc, 0L);
	ShowWindow(imagewin);

/*	dragrect = screenBits.bounds; */
	dragrect.left += 30;
	dragrect.right -= 30;
	dragrect.bottom -= 30;
	sizerect.top = 50;
	sizerect.left = 50;
	sizerect.bottom = screenBits.bounds.bottom - screenBits.bounds.top;
	sizerect.right  = screenBits.bounds.right  - screenBits.bounds.left;

	useWNE = (NGetTrapAddress(0x60, ToolTrap) != NGetTrapAddress(0x9f, ToolTrap));
	/* Pass WNE an empty region the 1st time thru. */
	cursorrgn = NewRgn();
	while (1) {
		/* Use WaitNextEvent if it is available. */
		if (useWNE) {
			gotevent = WaitNextEvent(everyEvent, &event, 0L, cursorrgn);
		} else {
			SystemTask();
			gotevent = GetNextEvent(everyEvent, &event);
		}
		if (gotevent) {
			do_event(&event);
		}
	}
}

do_event(event)
EventRecord *event;
{
	short part, err;
	WindowPtr window;
	Boolean hit;
	char key;
	Point pnt;
	long winsize;
	GrafPtr oldport;

	switch (event->what) {
		case mouseDown:
			part = FindWindow(event->where, &window);
			switch (part) {
				case inMenuBar:
					adjust_menus();
					do_menu_command(MenuSelect(event->where));
					break;
				case inSysWindow:
					SystemClick(event, window);
					break;
				case inContent:
					if (window != FrontWindow()) {
						SelectWindow(window);
						/* We just want to discard the event, since clicks in a
						   windows are sometimes irreversible actions. */
						adjust_menus();
					} else {
						handle_mouse_click(event->where, event->modifiers);
					}
					break;
				case inDrag:
					DragWindow(window, event->where, &dragrect);
					break;
				case inGrow:
					winsize = GrowWindow(window, event->where, &sizerect);
					if (winsize != 0) {
						GetPort(&oldport);
						SetPort(window);
						EraseRect(&window->portRect);
						SizeWindow(window, LoWord(winsize), HiWord(winsize), 1);
						grow_scrollbar();
						InvalRect(&window->portRect);
						SetPort(oldport);
					}
					break;
				case inZoomIn:
				case inZoomOut:
					if (TrackBox(window, event->where, part)) {
						GetPort(&oldport);
						/* The window must be the current port. (ZoomWindow bug) */
						SetPort(window);
						EraseRect(&window->portRect);
						ZoomWindow(window, part, true);
						grow_scrollbar();
						InvalRect(&window->portRect);
						SetPort(oldport);
					}
					break;
				case inGoAway:
					/* Don't mess around, just shut down. */
					ExitToShell();
					break;
			}
			break;
		case mouseUp:
			part = FindWindow(event->where, &window);
			switch (part) {
				case inContent:
					if (0 /* up in diff window than down? */) {
					} else {
					}
					break;
			}
			break;
		case keyDown:
		case autoKey:
			key = event->message & charCodeMask;
			/* Check for menukey equivalents. */
			if (event->modifiers & cmdKey) {
				if (event->what == keyDown) {
					adjust_menus();
					do_menu_command(MenuKey(key));
				}
			} else {
				if (event->what == keyDown) {
					/* Random keypress, interpret it. */
				}
			}
			break;
		case activateEvt:
			break;
		case updateEvt:
			if (imagewin == ((WindowPtr) event->message)) {
				update_image_window();
			}
			break;
		case diskEvt:
			/*	Call DIBadMount in response to a diskEvt, so that the user can format
				 a floppy. (from DTS Sample) */
			if (HiWord(event->message) != noErr) {
				SetPt(&pnt, 50, 50);
				err = DIBadMount(pnt, event->message);
			}
			break;
#if 0
		case kOSEvent:
		/*	1.02 - must BitAND with 0x0FF to get only low byte */
			switch ((event->message >> 24) & 0x0FF) {		/* high byte of message */
				case kSuspendResumeMessage:		/* suspend/resume is also an activate/deactivate */
					gInBackground = (event->message & kResumeMask) == 0;
					DoActivate(FrontWindow(), !gInBackground);
					break;
			}
			break;
#endif
		default:
			break;
	}
}

grow_scrollbar()
{
	Rect tmprect = imagewin->portRect;

	MoveControl(vscrollbar, tmprect.right - sbarwid, toplineh - 1);
	SizeControl(vscrollbar, sbarwid + 1, tmprect.bottom - tmprect.top - toplineh - sbarwid + 1 + 1);
}

/* Decipher and do a menu command. */

do_menu_command(which)
long which;
{
	short menuid, menuitem;
	short itemHit;
	Str255 daname;
	short daRefNum;
	Boolean handledbyda;
	WindowPtr win;
	short ditem;
	int i;

	menuid = HiWord(which);
	menuitem = LoWord(which);
	switch (menuid) {
		case mApple:
			switch (menuitem) {
				case miAbout:
					Alert(aAbout, nil);
					break;
				default:
					GetItem(GetMHandle(mApple), menuitem, daname);
					daRefNum = OpenDeskAcc(daname);
			}
			break;
		case mFile:
			switch (menuitem) {
				case miFileOpenImf:
					open_imf_file();
					force_update();
					break;
				case miFileOpenResources:
					open_resource_file();
					force_update();
					break;
				case miFileSaveImf:
					save_imf_file();
					break;
				case miFileSaveResources:
					save_resource_file();
					break;
				case miFileQuit:
					ExitToShell();
					break;
			}
			break;
		case mEdit:
			/* handledbyda = SystemEdit(menuitem-1); */
			switch (menuitem)  {
				case miEditCut:
					break;
				case miEditCopy:
					break;
				case miEditPaste:
					break;
				case miEditClear:
					numimages = 0;
					force_update();
					break;
			}
			break;
		case mView:
			switch (menuitem) {
				case miView4x4:
					iw = ih = 4;
					break;
				case miView8x8:
					iw = ih = 8;
					break;
				case miView16x16:
					iw = ih = 16;
					break;
				case miView32x32:
					iw = ih = 32;
					break;
				case miViewColor:
					showcolor = !showcolor;
					break;
				case miViewNames:
					shownames = !shownames;
					break;
				case miViewMask:
					showmask = !showmask;
					break;
				case miViewAsUnit:
					varyunit = 1;
					varyterrain = 0;
					varyemblem = 0;
					break;
				case miViewAsTerrain:
					varyunit = 0;
					varyterrain = 1;
					varyemblem = 0;
					break;
				case miViewAsEmblem:
					varyunit = 0;
					varyterrain = 0;
					varyemblem = 1;
					break;
				case miViewWithUnit:
					withunit = !withunit;
					constunit = selectedimf;
					break;
				case miViewWithTerrain:
					withterrain = !withterrain;
					constterrain = selectedimf;
					break;
				case miViewWithEmblem:
					withemblem = !withemblem;
					constemblem = selectedimf;
					break;
				case miViewIcons:
					selecticons = !selecticons;
					break;
				case miViewTiles:
					selecttiles = !selecttiles;
					break;
			}
			force_update();
			break;
	}
	HiliteMenu(0);
}

adjust_menus()
{
	MenuHandle menu;

	if ((menu = GetMHandle(mView)) != nil) {
		CheckItem(menu, miView4x4, (iw == 4));
		CheckItem(menu, miView8x8, (iw == 8));
		CheckItem(menu, miView16x16, (iw == 16));
		CheckItem(menu, miView32x32, (iw == 32));
		if (hasColorQD) {
			EnableItem(menu, miViewColor);
			CheckItem(menu, miViewColor, showcolor);
		} else {
			DisableItem(menu, miViewColor);
		}
		CheckItem(menu, miViewNames, shownames);
		CheckItem(menu, miViewMask, showmask);
		CheckItem(menu, miViewAsUnit, varyunit);
		CheckItem(menu, miViewAsTerrain, varyterrain);
		CheckItem(menu, miViewAsEmblem, varyemblem);
		if (selectedimf != NULL && !varyunit) {
			EnableItem(menu, miViewWithUnit);
			CheckItem(menu, miViewWithUnit, withunit);
		} else {
			DisableItem(menu, miViewWithUnit);
		}
		if (selectedimf != NULL && !varyterrain) {
			EnableItem(menu, miViewWithTerrain);
			CheckItem(menu, miViewWithTerrain, withterrain);
		} else {
			DisableItem(menu, miViewWithTerrain);
		}
		if (selectedimf != NULL && !varyemblem) {
			EnableItem(menu, miViewWithEmblem);
			CheckItem(menu, miViewWithEmblem, withemblem);
		} else {
			DisableItem(menu, miViewWithEmblem);
		}
		CheckItem(menu, miViewIcons, selecticons);
		CheckItem(menu, miViewTiles, selecttiles);
	}
}

pascal void
scroll_proc(control, code)
ControlHandle control;
short code;
{
	int curvalue, jump;

	curvalue = GetCtlValue(control);
	switch (code) {
		case inPageDown:
			jump = (numvisrows > 2 ? numvisrows - 2 : 1);
			break;
		case inDownButton:
			jump = 1;
			break;
		case inPageUp:
			jump = - (numvisrows > 2 ? numvisrows - 2 : 1);
			break;
		case inUpButton:
			jump = -1;
			break;
		default:
			jump = 0;
			break;
	}
	curvalue += jump;
	SetCtlValue(control, curvalue);
}

handle_mouse_click(mouse, mods)
Point mouse;
int mods;
{
	int row, col, n;
	ControlHandle control;
	short part, value, oldvalue;

	SetPort(imagewin);
	GlobalToLocal(&mouse);
	part = FindControl(mouse, imagewin, &control);
	if (control == vscrollbar) {
		oldvalue = GetCtlValue(vscrollbar);
		switch (part) {
			case inThumb:
				part = TrackControl(control, mouse, NULL);
				break;
			default:
				part = TrackControl(control, mouse, &scroll_proc);
				break;
		}
		firstvisrow = GetCtlValue(vscrollbar);
		if (oldvalue != firstvisrow) {
			force_update();
		}
	} else if (mouse.v <= toplineh) {
		/* Clicking in the topline area de-selects always. */
		invert_selected_imf();
		selectedimf = NULL;
		draw_topline();
	} else {
		/* Figure out which image was clicked on. */
		col = mouse.h / eltw;
		row = (mouse.v - toplineh) / elth + firstvisrow;
		n = row * cols + col;
		if (n >= 0 && n < numimages) {
			if (images[n] != selectedimf) {
				invert_selected_imf();
				selectedimf = images[n];
				selectedn = n;
				invert_selected_imf();
			}
			if (mods & cmdKey) {
				withterrain = 1;
				constterrain = selectedimf;
				force_update();
			}
			if (mods & optionKey) {
				withemblem = 1;
				constemblem = selectedimf;
				force_update();
			}
		} else {
			invert_selected_imf();
			selectedimf = NULL;
		}
		draw_topline();
	}
}

force_update()
{
	GrafPtr oldport;

	GetPort(&oldport);
	SetPort(imagewin);
	EraseRect(&imagewin->portRect);
	InvalRect(&imagewin->portRect);
	SetPort(oldport);
}

/* The comparison function for the image list just does "strcmp" order and *requires*
   that all image families be named and named uniquely. */

image_name_compare(im1, im2)
ImageFamily **im1, **im2;
{
	return strcmp((*im1)->name, (*im2)->name);
}

color_name_compare(im1, im2)
ImageColor **im1, **im2;
{
	return strcmp((*im1)->name, (*im2)->name);
}

/* INPUT/OUTPUT */

/* Open and read/interpret the contents of an imf file. */

open_imf_file()
{
	char filename[BUFSIZE];
	int frefnum, startlineno = 0, endlineno = 0;
	FILE *fp;
	Point pnt;
#ifdef THINK_C
	SFTypeList typelist;
#else
	long typelist[4];
#endif
	SFReply reply;
	Obj *form;

	/* Gotta be somewhere... */
	SetPt(&pnt, 100, 100);
	/* Only read text files. */
	typelist[0] = 'TEXT';
	SFGetFile(pnt, "\p", NULL, 1, typelist, NULL, &reply);
	if (reply.good) {
		/* Make the location of the file be the current volume. */
		SetVol(reply.fName, reply.vRefNum);
		p2c(((char *) reply.fName), filename);
		SetCursor(*watchcursor);
		if ((fp = fopen(filename, "r")) != NULL) {
			/* Read everything in the file. */
			while ((form = read_form(fp, &startlineno, &endlineno)) != lispeof) {
				interp_imf_form(form);
			}
			fclose(fp);
		}
		/* Sort all into alphabetical order. */
		qsort(&(images[0]), numimages, sizeof(ImageFamily *), image_name_compare);
		qsort(&(colors[0]), numcolors, sizeof(ImageColor *),  color_name_compare);
		SetCursor(&arrow);
	}
}

interp_imf_form(form)
Obj *form;
{
	ImageFamily *imf;
	ImageColor *imc;
	Obj *rest, *clause, *size, *typ;

	if (equal(car(form), intern_symbol("imf"))) {
		if (stringp(cadr(form))) {
			imf = get_imf(c_string(cadr(form)));
			for (rest = cddr(form); rest != lispnil; rest = cdr(rest)) {
				clause = car(rest);
				if (consp(clause)) {
					interp_image(imf, car(clause), cdr(clause));
				} else {
					/* garbage? */
				}
			}
		} else {
			/* garbage form */
		}
	} else if (equal(car(form), intern_symbol("palette"))) {
		/* (should eventually be able to interpret shared palettes) */
	} else if (equal(car(form), intern_symbol("color"))) {
		if (stringp(cadr(form))) {
			imc = get_imc(c_string(cadr(form)));
			interp_color(imc, cddr(form));
		}
	} else {
		/* Ignore any non-image forms, we might be reading a normal game design. */
	}
}

enum {
	K_MONO,
	K_MASK,
	K_COLR
};

interp_image(imf, size, parts)
ImageFamily *imf;
Obj *size, *parts;
{
	int w, h, i, j, actualw, actualh, pixelsize, rowbytes = 0, numbytes, value, bitrowbytes;
	int imtype;
	char *data, *workarea;
	Image *img = NULL;
	Obj *rest, *typ, *prop, *datalist, *palette;
    PixMap pmap;
	Handle sicnhandle, iconhandle, datahandle;
    PixPatHandle ppathandle;
    CIconHandle cicnhandle = nil;
    PixMapHandle pmhandle;
    CTabHandle ctabhandle;

	w = c_number(car(size));  h = c_number(cadr(size));
	if ((img = get_img(imf, w, h)) != NULL) {
		if (equal(car(cddr(size)), intern_symbol("tile"))) img->istile = TRUE;
		for (rest = parts; rest != lispnil; rest = cdr(rest)) {
			actualw = w;  actualh = h;
			pixelsize = 8;
			palette = lispnil;
			typ = car(car(rest));
			if (equal(typ, intern_symbol("mono"))) {
				imtype = K_MONO;
			} else if (equal(typ, intern_symbol("mask"))) {
				imtype = K_MASK;
			} else if (equal(typ, intern_symbol("color"))) {
				imtype = K_COLR;
			} else {
				imtype = -1;
			}
			datalist = cdr(car(rest));
			while (consp(car(datalist))) {
				prop = car(datalist);
				if (equal(car(prop), intern_symbol("actual"))) {
					actualw = c_number(cadr(prop));  actualh = c_number(caddr(prop));
				} else if (equal(car(prop), intern_symbol("pixel-size"))) {
					pixelsize = c_number(cadr(prop));
					if (pixelsize < 1 || pixelsize > 8) pixelsize = 8;
				} else if (equal(car(prop), intern_symbol("row-bytes"))) {
					rowbytes = c_number(cadr(prop));
				} else if (equal(car(prop), intern_symbol("palette"))) {
					palette = cdr(prop);
				}
				datalist = cdr(datalist);
			}
			if (w == 8 && h == 8 && imtype == K_MONO) {
				/* Read exactly 8 bytes. */
#if 0 /* why doesn't this work? */
				datahandle = &(img->pat);
				interp_bytes(datalist, 8, datahandle);
#else
				data = NULL;
				for (i = 0; i < 8; ++i) {
					if (data == NULL || data[j] == '\0') {
						data = c_string(car(datalist));
						j = 0;
						datalist = cdr(datalist);
					}
					if (data[j] == '/') ++j;
					img->pat[i] = hextoi(data[j]) * 16 + hextoi(data[j+1]);
					j += 2;
				}
#endif
				img->patdefined = TRUE;
			} else if (w == 16 && h == 16
					   && (imtype == K_MONO || imtype == K_MASK)) {
				sicnhandle = NewHandle(32);
				memset(*sicnhandle, 0, 32);
				/* Read exactly 32 bytes. */
				interp_bytes(datalist, 32, sicnhandle);
				if (imtype == K_MONO) {
					img->monosicn = sicnhandle;
				} else if (imtype == K_MASK) {
					img->masksicn = sicnhandle;
				}
			} else if (imtype == K_MONO || imtype == K_MASK) {
				if (w == 32 && h == 32) {
					iconhandle = NewHandle(128);
					memset(*iconhandle, 0, 128);
					/* Read exactly 128 bytes. */
					interp_bytes(datalist, 128, iconhandle);
					if (imtype == K_MONO) {
						img->monoicon = iconhandle;
					} else if (imtype == K_MASK) {
						img->maskicon = iconhandle;
					}
				} else {
					if (rowbytes < 1)
					  rowbytes = (actualw+7/8)/*((actualw + 15) / 16) * 2*/;
					numbytes = actualh * rowbytes;
					workarea = (char *) xmalloc (numbytes * 2 + 1);
					interp_bytes(datalist, numbytes, &workarea);
				}
			} else if (imtype == K_COLR) {
				if (img->istile) {
					ppathandle = NewPixPat();
					pmhandle = (*ppathandle)->patMap;
					SetRect(&((*pmhandle)->bounds), 0, 0, actualw, actualh);
					if (rowbytes < 1) rowbytes = (actualw * pixelsize) / 8;
					(*pmhandle)->rowBytes = rowbytes;
					(*pmhandle)->pixelSize = pixelsize;
					if (length(palette) > 0) {
						ctabhandle = (CTabHandle) NewHandle(8 + length(palette) * 8);
						(*ctabhandle)->ctFlags = 0;
						(*ctabhandle)->ctSeed = GetCTSeed();
						(*ctabhandle)->ctSize = length(palette) - 1;
						interp_ctab(palette, ctabhandle);
						(*pmhandle)->pmTable = ctabhandle;
					}
					numbytes = actualh * rowbytes;
					datahandle = NewHandle(numbytes);
					(*ppathandle)->patData = datahandle;
					interp_bytes(datalist, numbytes, datahandle);
					(*ppathandle)->patXValid = -1;
					img->colrpat = ppathandle;
				} else if (actualw <= 64 && actualh <= 64) {
					bitrowbytes = ((actualw + 15) / 16) * 2;
					cicnhandle = (CIconHandle) NewHandle(sizeof(CIcon) + 2 * actualh * bitrowbytes);
					HLock((Handle) cicnhandle);
					(*cicnhandle)->iconPMap.baseAddr = 0;
					if (rowbytes < 1) rowbytes = ((actualw + 15) / 16) * pixelsize * 2;
					(*cicnhandle)->iconPMap.rowBytes = rowbytes | 0x8000;
					SetRect(&((*cicnhandle)->iconPMap.bounds), 0, 0, actualw, actualh);
					(*cicnhandle)->iconPMap.pmVersion = 0;
					(*cicnhandle)->iconPMap.packType = 0;
					(*cicnhandle)->iconPMap.packSize = 0;
					(*cicnhandle)->iconPMap.hRes = 0;
					(*cicnhandle)->iconPMap.vRes = 0;
					(*cicnhandle)->iconPMap.pixelType = 0;
					(*cicnhandle)->iconPMap.pixelSize = pixelsize;
					(*cicnhandle)->iconPMap.cmpCount = 1;
					(*cicnhandle)->iconPMap.cmpSize = pixelsize;
					(*cicnhandle)->iconPMap.planeBytes = 0;
					(*cicnhandle)->iconPMap.pmTable = 0;
					(*cicnhandle)->iconPMap.pmReserved = 0;
					if (length(palette) > 0) {
						ctabhandle = (CTabHandle) NewHandle(8 + length(palette) * 8);
						(*ctabhandle)->ctFlags = 0;
						(*ctabhandle)->ctSeed = GetCTSeed();
						(*ctabhandle)->ctSize = length(palette) - 1;
						interp_ctab(palette, ctabhandle);
						(*cicnhandle)->iconPMap.pmTable = ctabhandle;
					} else {
						(*cicnhandle)->iconPMap.pmTable = nil;
					}
					SetRect(&((*cicnhandle)->iconBMap.bounds), 0, 0, actualw, actualh);
					(*cicnhandle)->iconBMap.rowBytes = bitrowbytes;
					(*cicnhandle)->iconBMap.baseAddr = NULL;
					SetRect(&((*cicnhandle)->iconMask.bounds), 0, 0, actualw, actualh);
					(*cicnhandle)->iconMask.rowBytes = bitrowbytes;
					(*cicnhandle)->iconMask.baseAddr = NULL;
					numbytes = actualh * rowbytes;
					datahandle = NewHandle(numbytes);
					(*cicnhandle)->iconData = datahandle;
					interp_bytes(datalist, numbytes, datahandle);
					if (img->maskicon != nil) {
						memcpy((char *) (*cicnhandle)->iconMaskData, (*(img->maskicon)),
							   actualh * bitrowbytes);
					}
					img->colricon = (Handle) cicnhandle;
					HUnlock((Handle) cicnhandle);
				} else {
					/* should make a color pict? */
				}
			} else {
				/* garbage? */
			}
		}
#if 0
		if (img->colricon != nil
			&& (*((CIconHandle) img->colricon))->iconMask.baseAddr == NULL
			&& img->maskicon != nil) {
			workarea = (char *) xmalloc(128);
			memcpy(workarea, *(img->maskicon));
			(*((CIconHandle) img->colricon))->iconMask.baseAddr = workarea;
		}
#endif
	}
}

interp_bytes(datalist, numbytes, desthandle)
Obj *datalist;
int numbytes;
char **desthandle;
{
	int i, j = 0;
	char *data = NULL;

	for (i = 0; i < numbytes; ++i) {
		if (data == NULL || data[j] == '\0') {
			data = c_string(car(datalist));
			j = 0;
			datalist = cdr(datalist);
		}
		if (data[j] == '/') ++j;
		(*desthandle)[i] = hextoi(data[j]) * 16 + hextoi(data[j+1]);
		j += 2;
	}
}

interp_ctab(palette, ctabhandle)
Obj *palette;
CTabHandle ctabhandle;
{
	int c, ctsize;
	Obj *rest, *color;
	ColorSpec *ctdata;

	ctdata = (ColorSpec *) &((*ctabhandle)->ctTable);
	ctsize = (*ctabhandle)->ctSize;
	for (rest = palette; rest != lispnil; rest = cdr(rest)) {
		c = c_number(car(car(rest)));
		if (c < 0 || c > ctsize) return; /* should warn? */
		ctdata[c].value = c;
		color = cdr(car(rest));
		ctdata[c].rgb.red = c_number(car(color));
		ctdata[c].rgb.green = c_number(cadr(color));
		ctdata[c].rgb.blue = c_number(car(cddr(color)));
	}
}

interp_color(imc, elts)
ImageColor *imc;
Obj *elts;
{
	imc->r = c_number(car(elts));
	imc->g = c_number(car(cdr(elts)));
	imc->b = c_number(car(cddr(elts)));
}

open_resource_file()
{
	Point pnt;
	SFTypeList typelist;
	SFReply reply;
	int frefnum;
	Str255 filename;

	/* Gotta be somewhere... */
	SetPt(&pnt, 100, 100);
	/* Pick up any sort of file. */
	SFGetFile(pnt, "\p", NULL, -1, typelist, NULL, &reply);
	if (reply.good) {
		SetVol(reply.fName, reply.vRefNum);
		SetCursor(*watchcursor);
		if (OpenResFile(reply.fName) != -1) {
			/* Now that all the resources are available, go through all types of rsrcs
			   that might have useful images. */
			collect_all_resources('XCif');
			collect_all_resources('cicn');
			collect_all_resources('ICON');
			collect_all_resources('SICN');
			collect_all_resources('ppat');
			collect_all_resources('PAT ');
			collect_all_colors('XCic');
			/* Sort all into alphabetical order. */
			qsort(&(images[0]), numimages, sizeof(ImageFamily *), image_name_compare);
			qsort(&(colors[0]), numcolors, sizeof(ImageColor *),  color_name_compare);
		} else {
			/* should alert */
		}
		SetCursor(&arrow);
	}
}

/* Given a resource type, look for and load image families with names of that type. */

collect_all_resources(typ)
ResType typ;
{
    int i, n;
	char *imfname;
	Handle handle;
	short resid;  ResType restype;  Str255 resname;

	n = CountResources(typ);
	for (i = 0; i < n; ++i ) {
		imfname = NULL;
		handle = GetIndResource(typ, i + 1);
		GetResInfo(handle, &resid, &restype, resname);
		if (resname[0] > 0) {
			resname[resname[0]+1] = '\0';
			imfname = copy_string(resname+1);
		} else {
			/* (should warn about unnamed resources being ignored?) */
		}
		if (imfname != NULL) {
			get_imf(imfname);
		}
	}
}

collect_all_colors(typ)
ResType typ;
{
    int i, n;
	char *imcname;
	Handle handle;
	short resid;  ResType restype;  Str255 resname;

	n = CountResources(typ);
	for (i = 0; i < n; ++i ) {
		imcname = NULL;
		handle = GetIndResource(typ, i + 1);
		GetResInfo(handle, &resid, &restype, resname);
		if (resname[0] > 0) {
			resname[resname[0]+1] = '\0';
			imcname = copy_string(resname+1);
		} else {
			/* (should warn about unnamed resources being ignored?) */
		}
		if (imcname != NULL) {
			get_imc(imcname);
		}
	}
}

/* Given a name, find or create an image family with that name. */

ImageFamily *
get_imf(name)
char *name;
{
	int i;
	ImageFamily *imf = NULL;

	if (name == NULL) return NULL;
	/* Resource names ending in " mask" are parts of other images. */
	if (strcmp(name+strlen(name)-5, " mask") == 0) return NULL;
	if ((imf = find_imf(name)) == NULL) {
		if (numimages >= MAXIMAGES) {
			return NULL;
		}
		if ((imf = (ImageFamily *) xmalloc(sizeof(ImageFamily))) != NULL) {
			init_image_family(imf);
			imf->name = name;
			load_image_family(imf);
			images[numimages++] = imf;
			/* Do some visual feedback periodically. */
			if (numimages % 10 == 0) draw_topline();
		}
	}
	return imf;
}

/* Find the image family of the given name, if it exists. */

ImageFamily *
find_imf(name)
char *name;
{
	int i;

	for (i = 0; i < numimages; ++i) {
		if (strcmp(name, images[i]->name) == 0) return images[i];
	}
	return NULL;
}

/* Given a name, find or create an image color with that name. */

ImageColor *
get_imc(name)
char *name;
{
	int i;
	ImageColor *imc = NULL;

	if (name == NULL) return NULL;
	if ((imc = find_imc(name)) == NULL) {
		if (numcolors >= MAXIMAGES) return NULL;
		if ((imc = (ImageColor *) xmalloc(sizeof(ImageColor))) != NULL) {
			imc->name = name;
			imc->r = imc->g = imc->b = 0;
			load_image_color(imc);
			colors[numcolors++] = imc;
		}
	}
	return imc;
}

/* Find the image color of the given name, if it exists. */

ImageColor *
find_imc(name)
char *name;
{
	int i;

	for (i = 0; i < numcolors; ++i) {
		if (colornamecmp(name, colors[i]->name) == 0) return colors[i];
	}
	return NULL;
}

colornamecmp(str1, str2)
char *str1, *str2;
{
	while (*str1 != '\0' && *str2 != '\0') {
		if (*str1 == *str2) {
			++str1;  ++str2;
		} else if (isalpha(*str1) && isalpha(*str2) && tolower(*str1) == tolower(*str2)) {
			++str1;  ++str2;
		} else if (*str1 == 'a' && *str2 == 'e' && *(str1+1) == 'y' && *(str2+1) == 'y') {
			++str1;  ++str2;
		} else if (*str1 == 'e' && *str2 == 'a' && *(str1+1) == 'y' && *(str2+1) == 'y') {
			++str1;  ++str2;
		} else if (*str1 == ' ') {
			++str1;
		} else if (*str2 == ' ') {
			++str2;
		} else {
			return *str1 - *str2;
		}
	}
	if (*str1 == '\0') {
		if (*str2 == '\0') {
			return 0;
		} else {
			return 1;
		}
	} else {
		if (*str2 == '\0') {
			return -1;
		} else {
			/* can never happen */
		}
	}
}

/* Write all the images into an imf file. */

save_imf_file()
{
    int i, n;
    char filename[BUFSIZE];
    Point pnt;
    FILE *fp;
    SFReply reply;

	SetPt(&pnt, 100, 100);
	SFPutFile(pnt, "\p", "\plib.imf", nil, &reply);
	if (reply.good) {
		/* Make the location of the file be the current volume. */
		SetVol(reply.fName, reply.vRefNum);
		p2c(((char *) reply.fName), filename);
		SetCursor(*watchcursor);
		if ((fp = fopen(filename, "w")) != NULL) {
			/* Write out the imf forms of all the image families. */
			for (i = 0; i < numimages; ++i) {
				write_imf(fp, images[i]);
			}
			/* Write out the image colors. */
			for (i = 0; i < numcolors; ++i) {
				write_imc(fp, colors[i]);
			}
			fclose(fp);
		}
		SetCursor(&arrow);
	}
}

/* Write out the entire image family. */

write_imf(fp, imf)
FILE *fp;
ImageFamily *imf;
{
	int i;
	Image *img;

	if (imf != NULL && imf->name != NULL) {
		fprintf(fp, "(imf \"%s\"\n", imf->name);
		for (i = 0; i < imf->numsizes; ++i) {
			img = &(imf->images[i]);
			fprintf(fp, " ((%d %d", img->w, img->h);
			if (img->istile) fprintf(fp, " tile");
			fprintf(fp, ")\n");
			if (img->embedname) fprintf(fp, "  (embed \"%s\")\n", img->embedname);
			if (img->patdefined) write_pattern(fp, &(img->pat));
			if (hasColorQD && img->colrpat != nil) write_ppat(fp, img->colrpat);
			if (img->monosicn != nil) write_16x16(fp, "mono", *(img->monosicn));
			if (img->masksicn != nil) write_16x16(fp, "mask", *(img->masksicn));
			if (img->monoicon != nil) write_32x32(fp, "mono", *(img->monoicon));
			if (img->maskicon != nil) write_32x32(fp, "mask", *(img->maskicon));
			if (hasColorQD && img->colricon != nil) write_cicn(fp, img->colricon);
			fprintf(fp, "  )\n");
		}
		fprintf(fp, " )\n");
	} else {
		fprintf(fp, "; garbage?\n");
	}
}

write_pattern(fp, data)
FILE *fp;
char *data;
{
	int i;
	
	fprintf(fp, "  (mono \"");
	for (i = 0; i < 8; ++i) {
		if (debugoutput && i > 0) fprintf(fp, "/");
		fprintf(fp, "%02x", (unsigned char) data[i]);
	}
	fprintf(fp, "\")\n");
}

write_ppat(fp, pat)
FILE *fp;
PixPatHandle pat;
{
	int w, h, i, j, rowbytes, pixelsize, numbytes;
	Rect bounds;
	PixMapHandle pmhandle = (*pat)->patMap;
	CTabHandle ctabhandle = (*pmhandle)->pmTable;
	Handle patdata = (*pat)->patData;

	switch ((*pat)->patType) {
		case 0:
			fprintf(fp, "; type 0 ppat?\n");
			break;
		case 1:
			bounds = (*pmhandle)->bounds;
			w = bounds.right - bounds.left;  h = bounds.bottom - bounds.top;
			rowbytes = (*pmhandle)->rowBytes & 0x3fff;
			pixelsize = (*pmhandle)->pixelSize;
			fprintf(fp, "  (color (actual %d %d) (pixel-size %d) (row-bytes %d)\n",
					w, h, pixelsize, rowbytes);
			write_ctab(fp, ctabhandle);
			fprintf(fp, "   \"");
			numbytes = h * rowbytes;
			for (i = 0; i < numbytes; ++i) {
				if (i > 0 && i % 32 == 0) fprintf(fp, "\"\n   \"");
				if (debugoutput && i > 0 && i % 32 != 0 && i % rowbytes == 0) fprintf(fp, "/");
				fprintf(fp, "%02x", (unsigned char) (*patdata)[i]);
			}
			fprintf(fp, "\"\n   )\n");
			break;
		case 2:
			fprintf(fp, "; type 2 ppat?\n");
			break;
	}
}

write_16x16(fp, name, data)
FILE *fp;
char *name, *data;
{
	int i;
	
	fprintf(fp, "  (%s \"", name);
	for (i = 0; i < 32; ++i) {
		if (debugoutput && i > 0 && i % 2 == 0) fprintf(fp, "/");
		fprintf(fp, "%02x", (unsigned char) data[i]);
	}
	fprintf(fp, "\")\n");
}

write_32x32(fp, name, data)
FILE *fp;
char *name, *data;
{
	int i, j;
	
	fprintf(fp, "  (%4s\n", name);
	for (j = 0; j < 4; ++j) {
		fprintf(fp, "   \"");
		for (i = 0; i < 32; ++i) {
			if (debugoutput && i > 0 && i % 4 == 0) fprintf(fp, "/");
			fprintf(fp, "%02x", (unsigned char) data[32 * j + i]);
		}
		fprintf(fp, "\"\n");
	}
	fprintf(fp, "   )\n");
}

write_cicn(fp, cicnhandle)
FILE *fp;
CIconHandle cicnhandle;
{
	int w, h, i, j, rowbytes, pixelsize, pixel;
	Rect bounds;
	char *data, *baseaddr;
	PixMap pmap = (*cicnhandle)->iconPMap;
	CTabHandle ctabhandle = pmap.pmTable;
	Handle datahandle;

	HLock((Handle) cicnhandle);
	bounds = pmap.bounds;
	w = bounds.right - bounds.left;  h = bounds.bottom - bounds.top;
	rowbytes = pmap.rowBytes & 0x3fff;
	pixelsize = pmap.pixelSize;
	fprintf(fp, "  (color (actual %d %d) (pixel-size %d) (row-bytes %d)\n",
			w, h, pixelsize, rowbytes);
	write_ctab(fp, ctabhandle);
	datahandle = (*cicnhandle)->iconData;
	fprintf(fp, "   \"");
	for (i = 0; i < h * rowbytes; ++i) {
		if (i > 0 && i % 32 == 0) fprintf(fp, "\"\n   \"");
		if (debugoutput && i > 0 && i % 32 != 0 && i % rowbytes == 0) fprintf(fp, "/");
		fprintf(fp, "%02x", (unsigned char) (*datahandle)[i]);
	}
	fprintf(fp, "\"\n   )\n");
	rowbytes = (*cicnhandle)->iconBMap.rowBytes;
	if (rowbytes > 0) {
		baseaddr = ((char *) (*cicnhandle)->iconMaskData) + h * ((*cicnhandle)->iconMask.rowBytes);
		fprintf(fp, "  (mono (actual %d %d)\n", w, h);
		fprintf(fp, "   \"");
		for (i = 0; i < h * rowbytes; ++i) {
			if (i > 0 && i % 32 == 0) fprintf(fp, "\"\n   \"");
			if (debugoutput && i > 0 && i % 32 != 0 && i % rowbytes == 0) fprintf(fp, "/");
			fprintf(fp, "%02x", (unsigned char) baseaddr[i]);
		}
	}
	rowbytes = (*cicnhandle)->iconMask.rowBytes;
	if (rowbytes > 0) {
		baseaddr = (char *) (*cicnhandle)->iconMaskData;
		fprintf(fp, "\"\n   )\n");
		fprintf(fp, "  (mask (actual %d %d)\n", w, h);
		fprintf(fp, "   \"");
		for (i = 0; i < h * rowbytes; ++i) {
			if (i > 0 && i % 32 == 0) fprintf(fp, "\"\n   \"");
			if (debugoutput && i > 0 && i % 32 != 0 && i % rowbytes == 0) fprintf(fp, "/");
			fprintf(fp, "%02x", (unsigned char) baseaddr[i]);
		}
	}
	fprintf(fp, "\"\n   )\n");
	HUnlock((Handle) cicnhandle);
}

write_ctab(fp, ctabhandle)
FILE *fp;
CTabHandle ctabhandle;
{
	int c, ctsize;
	ColorSpec cspec;

	if ((ctsize = (*ctabhandle)->ctSize) >= 0) {	
		fprintf(fp, "   (palette\n");
		for (c = 0; c <= ctsize; ++c) {
			cspec = (*ctabhandle)->ctTable[c];
			fprintf(fp, "     (%d %u %u %u)\n",
					cspec.value, cspec.rgb.red, cspec.rgb.green, cspec.rgb.blue); 
		}
		fprintf(fp, "    )\n");
	}
}

write_imc(fp, imc)
FILE *fp;
ImageColor *imc;
{
	fprintf(fp, "(color \"%s\" %d %d %d)\n", imc->name, imc->r, imc->g, imc->b);
}

/* Write all the images, as resources, into a resource file. */

save_resource_file()
{
	int refnum, i;
	Handle handle;
	Str255 tmpstr;
	Point pnt;
    SFReply reply;

	SetPt(&pnt, 100, 100);
	SFPutFile(pnt, "\p", "\plib.imf Images", nil, &reply);
	if (reply.good) {
		/* Make the location of the file be the current volume. */
		SetVol(reply.fName, reply.vRefNum);
		SetCursor(*watchcursor);
		CreateResFile(reply.fName);
		if ((refnum = OpenResFile(reply.fName)) >= 0) {
			/* All resource ids should go from 1000 up. */
			nextpatid = 1000;
			nextsicnid = 1000;
			nexticonid = 1000;
			nextcolorid = 1000;
			nextimfid = 1000;
			/* Make resources for named colors. */
			for (i = 0; i < numcolors; ++i) {
				make_imc_resources(colors[i]);
			}
			/* Make resources for image family specifications. */
			for (i = 0; i < numimages; ++i) {
				make_imf_resources(images[i]);
			}
			CloseResFile(refnum);
		} else {
			/* alert about inability to open */
		}
		SetCursor(&arrow);
	}
}

make_imf_resources(imf)
ImageFamily *imf;
{
	int i, j;
	char buf[1000];
	Handle imfhandle, handle;
	Str255 tmpstr, tmpstrmask;
	Image *img;

	if (imf != NULL && imf->name != NULL) {
		pascalify(imf->name, tmpstr);
		sprintf(buf, "%s mask", imf->name);
		pascalify(buf, tmpstrmask);
		sprintf(buf, "(imf \"%s\"", imf->name);
		for (i = 0; i < imf->numsizes; ++i) {
			img = &(imf->images[i]);
			/* Compose the textual spec of the image that will become part of the
			   XCif resource. */
			sprintf(buf+strlen(buf), " ((%d %d", img->w, img->h);
			if (img->istile) strcat(buf, " tile");
			strcat(buf, ")");
			if (img->embedname)
			  sprintf(buf+strlen(buf), " (embed \"%s\")", img->embedname);
			strcat(buf, ")");
			/* Now make Mac-specific resources for image cases that we know about. */
			if (img->patdefined) {
				handle = NewHandle(8);
				for (j = 0; j < 8; ++j) {
					(*handle)[j] = img->pat[j];
				}
				AddResource(handle, 'PAT ', nextpatid++, tmpstr);
			}
			if (img->monosicn != nil) {
				if (img->masksicn != nil) {
					handle = NewHandle(64);
					for (j = 0; j < 32; ++j) {
						(*handle)[j] = (*(img->monosicn))[j];
					}
					for (j = 0; j < 32; ++j) {
						(*handle)[j+32] = (*(img->masksicn))[j];
					}
				} else {
					handle = img->monosicn;
				}
				AddResource(handle, 'SICN', nextsicnid++, tmpstr);
			}
			/* A weird case, but don't lose it. */
			if (img->masksicn != nil && img->monosicn == nil) {
				AddResource(img->masksicn, 'SICN', nextsicnid++, tmpstrmask);
			}
			if (img->monoicon != nil) {
				AddResource(img->monoicon, 'ICON', nexticonid++, tmpstr);
			}
			if (img->maskicon != nil) {
				AddResource(img->maskicon, 'ICON', nexticonid++, tmpstrmask);
			}
			if (hasColorQD && img->colrpat != nil) {
#if 0
				handle = NewHandle(sizeof(PixPat)+sizeof(PixMap)+300);
				memcpy(handle, (*(img->colrpat)), sizeof(PixPat));
					ppathandle = NewPixPat();
					pmhandle = (*ppathandle)->patMap;
					SetRect(&((*pmhandle)->bounds), 0, 0, actualw, actualh);
					if (rowbytes < 1) rowbytes = (actualw * pixelsize) / 8;
					(*pmhandle)->rowBytes = rowbytes;
					(*pmhandle)->pixelSize = pixelsize;
					if (length(palette) > 0) {
						ctabhandle = (CTabHandle) NewHandle(8 + length(palette) * 8);
						(*ctabhandle)->ctFlags = 0;
						(*ctabhandle)->ctSeed = GetCTSeed();
						(*ctabhandle)->ctSize = length(palette) - 1;
						interp_ctab(palette, ctabhandle);
						(*pmhandle)->pmTable = ctabhandle;
					}
					numbytes = actualh * rowbytes;
					datahandle = NewHandle(numbytes);
					(*ppathandle)->patData = datahandle;
					interp_bytes(datalist, numbytes, datahandle);
				
				AddResource(handle, 'ppat', nextpatid++, tmpstr);
#endif
			}
			if (hasColorQD && img->colricon != nil) {
			}
		}
		strcat(buf, ")");
		imfhandle = NewHandle(strlen(buf)+1);
		strcpy(*imfhandle, buf);
		AddResource(imfhandle, 'XCif', nextimfid++, tmpstr);
	}
}

make_imc_resources(imc)
ImageColor *imc;
{
	Handle handle;

	handle = NewHandle(6);
	((short *) (*handle))[0] = imc->r;
	((short *) (*handle))[1] = imc->g;
	((short *) (*handle))[2] = imc->b;
	pascalify(imc->name, tmpbuf);
	AddResource(handle, 'XCic', nextcolorid++, tmpbuf);
}

/* GRAPHICS */

/* Totally redraw the one window. */

update_image_window()
{
	int row, col, n;
	int namex, namey;
	Rect tmprect;
	GrafPtr oldport;

	BeginUpdate(imagewin);
	GetPort(&oldport);
	SetPort(imagewin);
	EraseRect(&(imagewin->portRect));
	draw_topline();
	eltw = (shownames ? 100 : iw + 4);  elth = ih + 4;
	/* (should be determined by choice of app font) */
	if (elth < 10) elth = 10;
	winwidth = imagewin->portRect.right - imagewin->portRect.left;
	winheight = imagewin->portRect.bottom - imagewin->portRect.top;
	if (withterrain && constterrain) {
		if (best_image(constterrain, iw, ih)->istile) {
			draw_as_terrain_image(0, toplineh,
								  winwidth - sbarwid, winheight - toplineh - sbarwid,
								  constterrain);
		} else {
			/* (should draw under each unit separately) */
		}
	}
	/* Compute how many columns we can fit, rounding down but always at least 1. */
	cols = winwidth / eltw;
	if (cols <= 0) cols = 1;
	/* We can get a little wider spacing by recalculating the element width. */
	eltw = (winwidth - 10) / cols;
	rows = numimages / cols + 1;
	numvisrows = (winheight - toplineh - 15) / elth;
	if (numvisrows > rows) numvisrows = rows;
	for (row = firstvisrow; row < (firstvisrow + numvisrows); ++row) {
		for (col = 0; col < cols; ++col) {
			n = row * cols + col;
			if (n >= numimages) break;
			draw_one_image(images[n], col, row);
		}
	}
	invert_selected_imf();
	set_scrollbar();
	DrawControls(imagewin);
	DrawGrowIcon(imagewin);
	SetPort(oldport);
	EndUpdate(imagewin);
}

draw_topline()
{
	Rect tmprect;
	GrafPtr oldport;

	GetPort(&oldport);
	SetPort(imagewin);
	tmprect = imagewin->portRect;
	tmprect.bottom = tmprect.top + toplineh;
	EraseRect(&tmprect);
	TextFont(monaco);
	TextSize(9);
	sprintf(spbuf, "%d images", numimages);
	if (withterrain && constterrain != NULL) {
		sprintf(spbuf+strlen(spbuf), " (on %s terrain)", constterrain->name);
	}
	if (withemblem && constemblem != NULL) {
		sprintf(spbuf+strlen(spbuf), " (with %s emblem)", constemblem->name);
	}
	if (numcolors > 0) {
		sprintf(spbuf+strlen(spbuf), ", %d colors", numcolors);
	}
	pascalify(spbuf, tmpbuf);
	MoveTo(4, 12);
	DrawString(tmpbuf);
	/* Draw a second line describing the selection. */
	if (selectedimf != NULL) {
		sprintf(spbuf, "[%s]", selectedimf->name);
		pascalify(spbuf, tmpbuf);
		MoveTo(4, 26);
		DrawString(tmpbuf);
		/* should describe the particular image in use */
	}
	/* Draw a dividing line. */
	MoveTo(0, toplineh - 2);
	Line(imagewin->portRect.right, 0);
	/* Restore the existing port. */
	SetPort(oldport);
}

/* Draw a single imf at a given row and column. */

draw_one_image(imf, col, row)
ImageFamily *imf;
int col, row;
{
	int namex, namey;
	Rect tmprect, maskrect;
	FontInfo fontinfo;

	tmprect.left = col * eltw;  tmprect.top = toplineh + (row - firstvisrow) * elth;
	tmprect.right = tmprect.left + iw + 4;  tmprect.bottom = tmprect.top + ih + 4;
	InsetRect(&tmprect, 2, 2);
	/* Maybe draw the background terrain. (should be in a hex shape sometimes?) */
	if (varyterrain) {
		draw_as_terrain_image(tmprect.left, tmprect.top, iw, ih, imf);
	} else if (withterrain && constterrain) {
		draw_as_terrain_image(tmprect.left-2, tmprect.top-2, iw+4, elth, constterrain);
	}
	if (varyunit) {
		draw_as_unit_image(imagewin, tmprect.left, tmprect.top, iw, ih, imf);
	} else if (withunit && constunit) {
		draw_as_unit_image(imagewin, tmprect.left, tmprect.top, iw, ih, constunit);
	}
	/* Maybe draw an emblem. */
	if (varyemblem) {
		draw_as_emblem_image(imagewin, tmprect.left + iw - 8, tmprect.top, 8, 8, imf);
	} else if (withemblem && constemblem) {
		draw_as_emblem_image(imagewin, tmprect.left + iw - 8, tmprect.top, 8, 8, constemblem);
	}
	/* Maybe draw the name of the image. */
	if (shownames) {
		/* If nonwhite background, add a white rect for the name. */
		namex = col * eltw + iw + 4;
		namey = toplineh + (row - firstvisrow) * elth + (elth / 2) + 5;
		if (withterrain && constterrain) {
			GetFontInfo(&fontinfo);
			maskrect.left = namex;
			maskrect.top = namey - fontinfo.ascent;
			maskrect.right = maskrect.left + TextWidth(imf->name, 0, strlen(imf->name));
			maskrect.bottom = namey + fontinfo.descent + 1;
			FillRect(&maskrect, white);
		}
		MoveTo(namex, namey);
		pascalify(imf->name, tmpbuf);
		DrawString(tmpbuf);
	}
}

/* Adjust the scrollbar to reflect the size at which the images are being rendered. */

set_scrollbar()
{
	SetCtlMax(vscrollbar, rows - numvisrows);
	HiliteControl(vscrollbar, (numvisrows < rows ? 0 : 255));
}

/* Indicate which image is currently selected. */

invert_selected_imf()
{
	int row, col;
	Rect tmprect;

	if (selectedimf != NULL) {
		col = selectedn % cols;
		row = selectedn / cols;
		/* Calculate the bounding box for the selected image. */
		tmprect.left = col * eltw;  tmprect.top = toplineh + (row - firstvisrow) * elth;
		tmprect.right = tmprect.left + eltw;  tmprect.bottom = tmprect.top + elth;
		if (tmprect.top < toplineh) return;
		/* This inverts a rectangle around the selected image. */
		InvertRect(&tmprect);
		InsetRect(&tmprect, 1, 1);
		InvertRect(&tmprect);
	}
}

draw_as_terrain_image(sx, sy, sw, sh, imf)
int sx, sy, sw, sh;
ImageFamily *imf;
{
	Rect rect;
	Image *timg;

	timg = best_image(imf, sw, sh);
	rect.left = sx;  rect.top = sy;
	rect.right = sx + sw;  rect.bottom = sy + sh;
	if (timg) {
		if (hasColorQD && showcolor && timg->colrpat != nil) {
			FillCRect(&rect, timg->colrpat);
		} else if (timg->patdefined) {
			FillRect(&rect, (Pattern *) &(timg->pat));
		} else {
			/* If no imagery, just leave blank, don't try to make a default image. */
		}
	}
}

/* This is similar (but not identical! beware!) to Xconq's main unit drawing routine,
   but it uses an arbitrary image family instead. */

draw_as_unit_image(win, sx, sy, sw, sh, imf)
WindowPtr win;
int sx, sy, sw, sh;
ImageFamily *imf;
{
	Rect srcrect, imagerect;
	RgnHandle tmprgn;
	BitMap bm, *winbits;
	Image *uimg;

	imagerect = win->portRect;
	uimg = best_image(imf, sw, sh);
	/* There should always be *some* image to display. */
	if (uimg) {
		imagerect.top += sy;  imagerect.left += sx;
		imagerect.bottom = imagerect.top + sh;  imagerect.right = imagerect.left + sw;
		winbits = &(((GrafPtr) win)->portBits);
		if (uimg->monopict != nil) {
			DrawPicture(uimg->monopict, &imagerect);
		} else if (hasColorQD && showcolor && uimg->colricon != nil) {
			PlotCIcon(&imagerect, uimg->colricon);
		} else if (uimg->monoicon != nil) {
			SetRect(&srcrect, 0, 0, 32, 32);
			bm.rowBytes = 4;
			bm.bounds = srcrect;
			if (uimg->maskicon != nil && showmask) {
				bm.baseAddr = *(uimg->maskicon);
				CopyBits(&bm, winbits, &srcrect, &imagerect, srcBic, nil);
			} else {
				/* Draw unit bbox as default mask. (maybe shrink a little??) */
				FillRect(&imagerect, white);
			}
			bm.baseAddr = *(uimg->monoicon);
			CopyBits(&bm, winbits, &srcrect, &imagerect, srcOr, nil);
		} else if (uimg->monosicn != nil) {
			SetRect(&srcrect, 0, 0, 16, 16);
			bm.rowBytes = 2;
			bm.bounds = srcrect;
			if (uimg->masksicn != nil && showmask) {
				bm.baseAddr = *(uimg->masksicn);
				CopyBits(&bm, winbits, &srcrect, &imagerect, srcBic, nil);
			} else {
				/* Draw unit bbox as default mask. (maybe shrink a little??) */
				FillRect(&imagerect, white);
			}
			bm.baseAddr = *(uimg->monosicn);
			CopyBits(&bm, winbits, &srcrect, &imagerect, srcOr, nil);
		} else if ((hasColorQD && showcolor && uimg->colrpat) || uimg->patdefined) {
			draw_as_terrain_image(sx, sy, sw, sh, imf);
		} else {
			/* should never be possible? */
		}
	}
}

/* Draw a given side's emblem. Uses the current GrafPort. */

draw_as_emblem_image(win, ex, ey, ew, eh, imf)
WindowPtr win;
int ex, ey, ew, eh;
ImageFamily *imf;
{
	Rect srcrect, imagerect = win->portRect;
	RgnHandle tmprgn;
	BitMap bm, *winbits;
	Image *eimg;

	eimg = best_image(imf, ew, eh);
	imagerect.top += ey;  imagerect.left += ex;
	imagerect.bottom = imagerect.top + eh;  imagerect.right = imagerect.left + ew;
	/* If an image is present, display it, otherwise just suppress. */
	if (eimg) {
		winbits = &(((GrafPtr) win)->portBits);
		if (eimg->monopict != nil) {
			DrawPicture(eimg->monopict, &imagerect);
		} else if (hasColorQD && showcolor && eimg->colricon != nil) {
			PlotCIcon(&imagerect, eimg->colricon);
		} else if (eimg->monoicon != nil) {
			SetRect(&srcrect, 0, 0, 32, 32);
			bm.rowBytes = 4;
			bm.bounds = srcrect;
			if (eimg->maskicon != nil && showmask) {
				bm.baseAddr = *(eimg->maskicon);
				CopyBits(&bm, winbits, &srcrect, &imagerect, srcBic, nil);
			} else {
				/* Draw unit bbox as default mask. (maybe shrink a little??) */
				FillRect(&imagerect, white);
			}
			bm.baseAddr = *(eimg->monoicon);
			CopyBits(&bm, winbits, &srcrect, &imagerect, srcOr, nil);
		} else if (eimg->monosicn != nil) {
			SetRect(&srcrect, 0, 0, 16, 16);
			bm.rowBytes = 2;
			bm.bounds = srcrect;
			if (eimg->masksicn != nil && showmask) {
				bm.baseAddr = *(eimg->masksicn);
				CopyBits(&bm, winbits, &srcrect, &imagerect, srcBic, nil);
			} else {
				/* Draw unit bbox as default mask. (maybe shrink a little??) */
				FillRect(&imagerect, white);
			}
			bm.baseAddr = *(eimg->monosicn);
			CopyBits(&bm, winbits, &srcrect, &imagerect, srcOr, nil);
		} else {
			/* should never be possible? */
		}
	}
}

/* Lisp reader support. */

FILE *errorfp = NULL;

announce_read_progress() {}

char *
copy_string(str)
char *str;
{
    int len = strlen(str);
    char *rslt;

    rslt = (char *) xmalloc(len + 1);
    strcpy(rslt, str);
    return rslt;
}

int
xmalloc(amt)
unsigned amt;
{
    int value = (int) malloc (amt);

    if (value == 0) {
		/* This is pretty serious, just have to get out quickly. */
		run_error("Memory exhausted!!");
		exit(1);  /* case run_error doesn't do it */
    }
    return value;
}

init_warning(str, a1, a2, a3)
char *str;
long a1, a2, a3;
{
	/* Cursor may be weird from loading, reset it. */
	SetCursor(&qd.arrow);
	sprintf(spbuf, str, a1, a2, a3);
	pascalify(spbuf, tmpbuf);
	ParamText(tmpbuf, "\p", "\p", "\p");
	switch (CautionAlert(128, nil)) {
		case 1:
			/* Just keep going, hope that warning was a false alarm. */
			/* (if option key on, should suppress future warnings) */
			break;
	}
}

/* An init error is not necessarily fatal, but have to start over. */

init_error(str, a1, a2, a3)
char *str;
long a1, a2, a3;
{
	/* Cursor may be weird from loading, reset it. */
	SetCursor(&qd.arrow);
	sprintf(spbuf, str, a1, a2, a3);
	pascalify(spbuf, tmpbuf);
	ParamText(tmpbuf, "\p", "\p", "\p");
	switch (StopAlert(128, nil)) {
		case 1:
			/* Just keep going, hope that warning was a false alarm. */
			/* (if option key on, should suppress future warnings) */
			break;
	}
}

/* An run error is fatal, but allow an emergency save, might be able to salvage. */

run_error(str, a1, a2, a3)
char *str;
long a1, a2, a3;
{
	sprintf(spbuf, str, a1, a2, a3);
	pascalify(spbuf, tmpbuf);
	ParamText(tmpbuf, "\p", "\p", "\p");
	/* (add the save button and handler code here) */
	switch (StopAlert(128, nil)) {
		case 1:
			break;
		case 2:
			break;
		case 3:
			break;
	}
	ExitToShell();
}

/* Runtime warnings are for when it's important to bug the players,
   but doesn't necessarily mean imminent danger of a crash. */

run_warning(str, a1, a2, a3)
char *str;
long a1, a2, a3;
{
	/* Cursor may be weird from loading, reset it. */
	SetCursor(&qd.arrow);
	sprintf(spbuf, str, a1, a2, a3);
	pascalify(spbuf, tmpbuf);
	ParamText(tmpbuf, "\p", "\p", "\p");
	switch (CautionAlert(128, nil)) {
		case 1:
			/* Just keep going, hope that warning was a false alarm. */
			/* (if option key on, should suppress future warnings) */
			break;
	}
}

/* Fake definitions of unneeded routines called by lisp.c. */

keyword_code() { return 0; }
keyword_value() { return 0; }
init_predefined_symbols() {}
lazy_bind() {}


