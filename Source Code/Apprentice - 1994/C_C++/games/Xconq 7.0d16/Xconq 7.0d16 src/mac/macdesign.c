/* Copyright (c) 1992, 1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Mac designer handling. */

#include "conq.h"
#include "mac.h"

/* (should be declared in world.h) */

extern int mintemp, maxtemp, tempscanvary, tempscanvaryinlayer;
extern int minwindforce, maxwindforce, windscanvary, windscanvaryinlayer;

extern int anyclouds;

extern int anymaterialsinterrain;

#ifdef DESIGNERS

#define numtooltypes 12

extern MenuHandle featuremenu;

WindowPtr designwin = nil;

ControlHandle ttypepopup = nil;
ControlHandle utypepopup = nil;
ControlHandle mtypepopup = nil;
ControlHandle sidepopup = nil;
ControlHandle newfeaturebutton = nil;
ControlHandle featurerenamebutton = nil;
ControlHandle featurepopup = nil;

CursHandle paintcursors[numtooltypes];
CursHandle bordpaintor;
CursHandle connpaintor;
CursHandle coatpaintor;

/* This is the width and height of each design tool's entry. */

int dtoolw = 120;
int dtoolh = 36;

/* The type of designer tool currently in use. */

enum tooltype tooltype = notool;

/* All the state of the designer palette. */

short curbrushradius = 0;
short curttype = 0;
short curbgttype = 0;
short curdepth = 1;
short curutype = 0;
short cursidenumber = 0;
short curmtype = 0;
short curmamount = 0;
short curfid = 0;
Feature *curfeature = NULL;
char *curfeaturetypename;
char *curfeaturename;
short curelevation = 0;
short curtemperature = 0;
short curcloudtype = 0;
short curcloudbottom = 0;
short curcloudheight = 0;
short curwinddir = 0;
short curwindforce = 0;

/* These are globals used in drag painting. */

short painttype;
short paintpeop;
short paintfid;

short enabledtooltype[20];

enable_designing(forsure)
int forsure;
{
	extern int compromised;

	if (dside == NULL || dside->designer) return;
	if (!forsure && !compromised) {
		switch (CautionAlert(aConfirmDesign, nil)) {
			case aiConfirmDesignOK:
				break;
			case aiConfirmDesignCancel:
				return;
		}
	}
	/* Actually change designer status, this will call back to alter all displays. */
	become_designer(dside);
	/* Create and add the designer's palette. */
	if (designwin == nil) {
		create_design_window();
	}
	if (designwin != nil) {
		position_design_window();
		ShowWindow(designwin);
/*		SelectWindow(designwin); */
	}
	/* Recache visibility flags. */
	calc_vision();
}

disable_designing()
{
	if (dside == NULL || !dside->designer) return;
	/* Hide (but don't destroy) the designer's palette. */
	if (designwin != nil) {
		HideWindow(designwin);
	}
	/* Actually change designer status, this will call back to alter all displays. */
	become_nondesigner(dside);
	/* Recache visibility flags. */
	calc_vision();
}

/* Each type of design tool has a distinct cursor. */

init_design_cursors()
{
	paintcursors[terraintool] = GetCursor(cCell);
	bordpaintor = GetCursor(cBord);
	connpaintor = GetCursor(cConn);
	coatpaintor = GetCursor(cCoat);
	paintcursors[unittool] = GetCursor(cUnit);
	paintcursors[peopletool] = GetCursor(cPeople);
	paintcursors[materialtool] = GetCursor(cMaterial);
	paintcursors[featuretool] = GetCursor(cFeature);
	paintcursors[elevationtool] = GetCursor(cElevation);
	paintcursors[temperaturetool] = GetCursor(cTemperature);
	paintcursors[cloudstool] = GetCursor(cClouds);
	paintcursors[windstool] = GetCursor(cWinds);
}

/* Adjust the cursor to reflect the current designer tool. */

CursPtr
adjust_designer_cursor(mouse, region)
Point mouse;
RgnHandle region;
{
	if (tooltype == terraintool && !t_is_cell(curttype)) {
		switch (t_subtype(curttype)) {
			case bordersubtype:
				return *bordpaintor;
			case connectionsubtype:
				return *connpaintor;
			case coatingsubtype:
				return *coatpaintor;
			default:
				terrain_subtype_warning("cursor adjust", curttype);
				return &QD(arrow);
		}
	}
	return *(paintcursors[tooltype]);
}

/* Create the designer tool window. */

create_design_window()
{
	if (hasColorQD) {
		designwin = GetNewCWindow(wDesign, NULL, (WindowPtr) -1L);
	} else {
		designwin = GetNewWindow(wDesign, NULL, (WindowPtr) -1L);
	}
	add_window_menu_item("Design", designwin);  /* until this becomes a windoid */
	/* Make the current side be the one with the display. */
	cursidenumber = side_number(dside);
	build_terrain_type_menu();
	build_unit_type_menu();
	build_side_menu();
	build_material_type_menu();
	build_feature_menu();
	ttypepopup = GetNewControl(mTerrainTypes, designwin);
	utypepopup = GetNewControl(mUnitTypes, designwin);
	sidepopup = GetNewControl(mSides, designwin);
	mtypepopup = GetNewControl(mMaterialTypes, designwin);
	featurepopup = GetNewControl(mFeatures, designwin);
	newfeaturebutton = GetNewControl(cNewFeatureButton, designwin);
	featurerenamebutton = GetNewControl(cFeatureRenameButton, designwin);
	SizeWindow(designwin, 2 * dtoolw - 1, (numtooltypes / 2) * dtoolh - 1, 1);
	position_design_window();
	init_design_cursors();
	enabledtooltype[notool] = TRUE;
	enabledtooltype[terraintool] = TRUE;
	enabledtooltype[unittool] = TRUE;
	enabledtooltype[peopletool] = TRUE;
	enabledtooltype[featuretool] = TRUE;
	enabledtooltype[brushsizetool] = TRUE;
	enabledtooltype[materialtool] = anymaterialsinterrain;
	enabledtooltype[elevationtool] = !world_is_flat();
	enabledtooltype[temperaturetool] = tempscanvary;
	enabledtooltype[cloudstool] = anyclouds;
	enabledtooltype[windstool] = windscanvary;
	ShowWindow(designwin);
}

/* Try to put the palette alongside the frontmost window. */

position_design_window()
{
	Point pt;
	WindowPtr win;
	GrafPtr oldport;

	/* (should fix all this) */
	if ((win = FrontWindow()) != nil) {
		GetPort(&oldport);
		SetPort(win);
		SetPt(&pt, win->portRect.right + 3, win->portRect.top);
		LocalToGlobal(&pt);
		SetPort(oldport);
	} else {
		SetPt(&pt, 500, 50);
	}
	/* (should make sure is not off the edge of the screen) */
	if (pt.h + 2 * dtoolw > 640 /* is off screen entirely */) {
		SetPt(&pt, 640 - 2 * dtoolw - 2, 50);
	}
	MoveWindow(designwin, pt.h, pt.v, TRUE);
}

draw_design_window()
{
	int i;
	GrafPtr oldport;

	if (!active_display(dside) || designwin == nil) return;
	GetPort(&oldport);
	SetPort(designwin);
	/* Draw each tool's palette item. */
	for (i = 0; i < numtooltypes; ++i) {
		draw_design_window_tool(i);
	}
	DrawControls(designwin);
	SetPort(oldport);
}

draw_design_window_tool(tool)
enum tooltype tool;
{
	int enabled = TRUE, paintable = TRUE;
	char *toolname = NULL, *auxtoolname = NULL;
	Rect tmprect, imagerect;
	char imbuf[BUFSIZE];
	Str255 tmpstr;

	SetRect(&tmprect, 0, 0, dtoolw, dtoolh);
	OffsetRect(&tmprect, (tool / (numtooltypes/2)) * dtoolw, (tool % (numtooltypes/2)) * dtoolh);
	EraseRect(&tmprect);
	/* Confine the image to a square subrect on the left side of the window. */
	imagerect = tmprect;
	imagerect.right = imagerect.left + dtoolh;
	imbuf[0] = '\0';
	switch (tool) {
		case notool:
			toolname = "Normal";
			break;
		case terraintool:
			toolname = t_type_name(curttype);
			InsetRect(&imagerect, (dtoolh - hws[4]) / 2, (dtoolh - hhs[4]) / 2);
			/* Only do bg terrain type if painting cell terrain. */
			if (t_is_cell(curttype)) {
				auxtoolname = t_type_name(curbgttype);
				/* bg type is always cell subtype. */
				OffsetRect(&imagerect, 3, 3);
				draw_terrain_sample(imagerect, curbgttype);
				OffsetRect(&imagerect, -6, -6);
			}
			draw_terrain_sample(imagerect, curttype);
			break;
		case unittool:
			toolname = u_type_name(curutype);
			InsetRect(&imagerect, (dtoolh - 32) / 2, (dtoolh - 32) / 2);
		 	draw_unit_image(designwin, imagerect.left, imagerect.top, 32, 32,
		 					curutype, cursidenumber, 0);
			/* Gray out the unit if not allowed for the current side. */
			paintable = type_allowed_on_side(curutype, side_n(cursidenumber));
			break;
		case peopletool:
			shortest_side_title(side_n(cursidenumber), spbuf);
			toolname = spbuf;
			InsetRect(&imagerect, (dtoolh - 16) / 2, (dtoolh - 16) / 2);
			draw_side_emblem(designwin, imagerect.left, imagerect.top, 16, 16,
							 cursidenumber);
			break;
		case featuretool:
			if (enabledtooltype[featuretool]) {
				if (curfeature == NULL) {
					curfeature = (Feature *) find_feature(curfid);
				}
				if (curfeature != NULL) {
					toolname = curfeature->typename;
					auxtoolname = curfeature->name;
				}
				sprintf(imbuf, "#%d", curfid);
			} else {
				toolname = "Feature";
				enabled = FALSE;
			}
			break;
		case brushsizetool:
			OffsetRect(&imagerect, dtoolh/2 - curbrushradius, dtoolh/2 - curbrushradius);
			imagerect.right = imagerect.left + curbrushradius + 1;
			imagerect.bottom = imagerect.top + curbrushradius + 1;
			FillOval(&imagerect, QD(black));
			if (curbrushradius > 0) {
				sprintf(imbuf, "%d", curbrushradius);
			}
			toolname = "Brush";
			break;
		case materialtool:
			if (enabledtooltype[materialtool]) {
				toolname = m_type_name(curmtype);
				sprintf(imbuf, "%d", curmamount);
			} else {
				toolname = "Material";
				enabled = FALSE;
			}
			break;
		case elevationtool:
			if (enabledtooltype[elevationtool]) {
				sprintf(spbuf, "Elev %d", curelevation);
				toolname = spbuf;
			} else {
				toolname = "Elevation";
				enabled = FALSE;
			}
			break;
		case temperaturetool:
			if (enabledtooltype[temperaturetool]) {
				sprintf(spbuf, "Temp %d¡", curtemperature);
				toolname = spbuf;
			} else {
				toolname = "Temperature";
				enabled = FALSE;
			}
			break;
		case cloudstool:
			if (enabledtooltype[cloudstool]) {
				/* Black is not ideal here... */
				FillRect(&imagerect, QD(black));
				if (curcloudtype > 0) {
					InsetRect(&imagerect, (dtoolh - hws[4]) / 2, (dtoolh - hhs[4]) / 2);
					draw_clouds(imagerect.left, imagerect.top, 4, curcloudtype);
					sprintf(spbuf, "Cloudy (%d)", curcloudtype);
					toolname = spbuf;
				} else {
					toolname = "Clear";
				}
			} else {
				toolname = "Clouds";
				enabled = FALSE;
			}
			break;
		case windstool:
			if (enabledtooltype[windstool]) {
				InsetRect(&imagerect, (dtoolh - hws[4]) / 2, (dtoolh - hhs[4]) / 2);
				draw_winds(imagerect.left, imagerect.top, 4, curwinddir, curwindforce);
				if (curwindforce > 0) {
					sprintf(spbuf, "Winds %s, %d", dirnames[curwinddir], curwindforce);
					toolname = spbuf;
				} else {
					toolname = "Calm";
				}
			} else {
				toolname = "Winds";
				enabled = FALSE;
			}
			break;
#if 0
		case viewtool:
			break;
#endif
		default:
			/* ??? */
			break;
	}
	TextSize(12);
	/* Draw a (short) text string in the image area. */
	if (strlen(imbuf) > 0) {
		/* (should center) */
		MoveTo(tmprect.left + (dtoolh - StringWidth(tmpstr)) / 2, tmprect.bottom - 5);
		DrawText(imbuf, 0, strlen(imbuf));
	}
	if (toolname != NULL) {
		MoveTo(tmprect.left + dtoolh, tmprect.top + (auxtoolname != NULL ? dtoolh / 4 + 4
														   : dtoolh / 2 + 5));
		DrawText(toolname, 0, strlen(toolname));
	}
	if (auxtoolname != NULL) {
		MoveTo(tmprect.left + dtoolh, tmprect.top + (dtoolh * 3) / 4);
		DrawText(auxtoolname, 0, strlen(auxtoolname));
	}
	if (!paintable) {
		gray_out_rect(&tmprect);
	}
	if (!enabled) {
		gray_out_rect(&tmprect);
	}
	/* Draw gray dividing lines. */
	PenPat(QD(gray));
	MoveTo(tmprect.right - 1, tmprect.top);  Line(0, dtoolh);
	MoveTo(tmprect.left, tmprect.bottom - 1);  Line(dtoolw, 0);
	PenNormal();
	/* Highlight the currently selected tool with a heavy outline rect. */
	if (tool == tooltype) {
		tmprect.bottom -= 1;  tmprect.right -= 1;
		InvertRect(&tmprect);
		InsetRect(&tmprect, 3, 3);
		InvertRect(&tmprect);
		if (!enabledtooltype[tool]) {
			InsetRect(&tmprect, -3, -3);
			gray_out_rect(&tmprect);
		}
	}
}

/* Respond to a mouse down in the designer's window. */

/* This macro implements cycling of a variable through a set of consecutive
   values, with direction controlled by the shift key.  If the limit is 0,
   then the cycling part is not done. */

#define OPTION_CYCLE(var, lo, hi, mods)  \
  if ((hi) - (lo) > 0) {  \
    (var) = (((var) + ((mods) & shiftKey ? -1 : 1) - (lo) + ((hi) - (lo))) % ((hi) - (lo))) + (lo);  \
  } else {  \
    (var) = ((var) + ((mods) & shiftKey ? -1 : 1));  \
  }

do_mouse_down_design(mouse, mods)
Point mouse;
int mods;
{
	int oldtool, poppedtool, newutype, newbgttype, toolchoice;
	Feature *feature;
	Rect tmprect;
	ControlHandle control;
	long choice;
	extern int nextfid;

	tmprect.left = 0;  tmprect.right = dtoolw;
	oldtool = poppedtool = tooltype;
	toolchoice = (mouse.v / dtoolh) + (mouse.h > dtoolw ? (numtooltypes / 2) : 0);
	FindControl(mouse, designwin, &control);
	if (control == ttypepopup) {
		TrackControl(control, mouse, (void *) -1);
		choice = LoWord(GetCtlValue(control));
		if (choice > 0) curttype = choice - 1;
		poppedtool = terraintool;
	} else if (control == utypepopup) {
		mark_allowed_unit_types();
		TrackControl(control, mouse, (void *) -1);
		choice = LoWord(GetCtlValue(control));
		if (choice > 0) curutype = choice - 1;
		poppedtool = unittool;
	} else if (control == sidepopup) {
		mark_allowed_sides();
		TrackControl(control, mouse, (void *) -1);
		choice = LoWord(GetCtlValue(control));
		if (choice > 0) {
			cursidenumber = choice;
			if (cursidenumber > numsides) cursidenumber = 0;;
		}
		poppedtool = peopletool;
	} else if (control == mtypepopup) {
		TrackControl(control, mouse, (void *) -1);
		choice = LoWord(GetCtlValue(control));
		if (choice > 0) curmtype = choice - 1;
		poppedtool = materialtool;
	} else if (control == featurepopup) {
		TrackControl(control, mouse, (void *) -1);
		choice = LoWord(GetCtlValue(control));
		if (choice > 0) {
			curfid = choice - 1; /* not reliable */
			curfeature = find_feature(curfid);
		}
		poppedtool = featuretool;
	} else if (control == newfeaturebutton) {
		sprintf(spbuf, "%d", nextfid);
		if ((feature = create_feature("feature", copy_string(spbuf))) != NULL) {
			curfeature = feature;
			curfid = feature->id;
			feature_rename_dialog(feature);
			update_feature_menu(feature);
		}
		poppedtool = featuretool;
	} else if (control == featurerenamebutton) {
		feature_rename_dialog(curfeature);
		poppedtool = featuretool;
	} else if (enabledtooltype[toolchoice]) {
		/* Any other click selects the tool. */
		if (toolchoice != brushsizetool) tooltype = toolchoice;
		/* Now handle any shortcuts. */
		switch (toolchoice) {
			case notool:
				break;
			case terraintool:
				if (mods & optionKey) {
					/* Option-click and Option-Shift-click cycle through all
					   the "foreground" terrain types. */
					OPTION_CYCLE(curttype, 0, numttypes, mods);
				} else if ((mods & cmdKey) && t_is_cell(curttype)) {
					/* Cmd-click and Cmd-Shift-click cycle through all
					   the "background" terrain types. */
					newbgttype = curbgttype;
					do {
						OPTION_CYCLE(newbgttype, 0, numttypes, mods);
						if (newbgttype == curbgttype) break;
					} while (!t_is_cell(newbgttype));
					curbgttype = newbgttype;
				}
				break;
			case unittool:
				if (mods & optionKey) {
					/* Option-click and Option-Shift-click cycle through all
					   the types allowed for the current side. */
					newutype = curutype;
					do {
						OPTION_CYCLE(newutype, 0, numutypes, mods);
						if (newutype == curutype) break;
					} while (!type_allowed_on_side(newutype, side_n(cursidenumber)));
					curutype = newutype;
				}
				break;
			case peopletool:
				if (mods & optionKey) {
					/* Option-click and Option-Shift-click cycle around all the sides. */
					OPTION_CYCLE(cursidenumber, 0, numsides + 1, mods);
				}
				break;
			case featuretool:
				if (mods & optionKey && nextfid > 1) {
					/* Option-click and Option-Shift-click cycle around the features. */
					OPTION_CYCLE(curfid, 0, nextfid, mods);
					curfeature = find_feature(curfid);
				}
				break;
			case brushsizetool:
				if (mods & optionKey) {
					/* Option-click and Option-Shift-click cycle through brush sizes. */
					OPTION_CYCLE(curbrushradius, 0, 99, mods);
				}
				break;
			case materialtool:
				if (mods & optionKey) {
					/* Option-click and Option-Shift-click cycle around amounts. */
					OPTION_CYCLE(curmamount, 0, 99, mods);
				}
				break;
			case elevationtool:
				if (mods & optionKey) {
					/* Option-click and Option-Shift-click adjust the elevation. */
					OPTION_CYCLE(curelevation, minelev, maxelev + 1, mods);
				}
				break;
			case temperaturetool:
				if (mods & optionKey) {
					/* Option-click and Option-Shift-click adjust the temp. */
					OPTION_CYCLE(curtemperature, mintemp, maxtemp + 1, mods);
				}
				break;
			case cloudstool:
				if (mods & optionKey) {
					/* Option-click and Option-Shift-click adjust the cloud type. */
					OPTION_CYCLE(curcloudtype, 0, 4, mods);
				}
				break;
			case windstool:
				if (mods & optionKey) {
					/* Option-click and Option-Shift-click adjust the wind force. */
					OPTION_CYCLE(curwindforce, minwindforce, maxwindforce, mods);
				} else if (mods & cmdKey) {
					/* Cmd-click and Cmd-Shift-click adjust the direction. */
					OPTION_CYCLE(curwinddir, 0, NUMDIRS, mods);
				}
				break;
			default:
				break;
		}
	}
	/* Draw the old and new tools. */
	if (oldtool != tooltype) {
		draw_design_window_tool(oldtool);
	}
	if (poppedtool != tooltype) {
		draw_design_window_tool(poppedtool);
	}
	draw_design_window_tool(tooltype);
	if (toolchoice != tooltype) draw_design_window_tool(toolchoice);
	/* As a special case, redraw the unit tool if the side tool was touched. */
	if (tooltype == peopletool || poppedtool == peopletool) {
		draw_design_window_tool(unittool);
	}
	/* (should only draw controls in relevant tools) */
	DrawControls(designwin);
}

mark_allowed_unit_types()
{
	Side *side = side_n(cursidenumber);
	int u;

	for_all_unit_types(u) {
		EnableItem(utypemenu, u + 1);
		SetItemMark(utypemenu, u + 1,
					(type_allowed_on_side(u, side) ? diamondMark : noMark));
	}
}

mark_allowed_sides()
{
	Side *side;

	for_all_sides(side) {
		EnableItem(sidemenu, side_number(side));
		SetItemMark(sidemenu, side_number(side),
					(type_allowed_on_side(curutype, side) ? diamondMark : noMark));
	}
	EnableItem(sidemenu, numsides + 1);
	SetItemMark(sidemenu, numsides + 1,
				(type_allowed_on_side(curutype, NULL) ? diamondMark : noMark));
}

feature_rename_dialog(feature)
Feature *feature;
{
	short done = FALSE, ditem;
	char *newtypename, *newname;
	Str255 tmpstr;
	DialogPtr win;
	short itemtype;  Handle itemhandle;  Rect itemrect;

	if (feature == NULL) return;
	win = GetNewDialog(dFeatureRename, NULL, (DialogPtr) -1L);
	/* Seed the text items with the original names. */
	newtypename = feature->typename;
	if (newtypename == NULL) newtypename = "";
	GetDItem(win, diFeatureRenameType, &itemtype, &itemhandle, &itemrect);
	c2p(newtypename, tmpstr);
	SetIText(itemhandle, tmpstr);
	newname = feature->name;
	if (newname == NULL) newname = "";
	GetDItem(win, diFeatureRenameName, &itemtype, &itemhandle, &itemrect);
	c2p(newname, tmpstr);
	SetIText(itemhandle, tmpstr);
	ShowWindow(win);
	while (!done) {
		/* Deactivate the front window. */
		activate_window(FrontWindow(), FALSE);
		ModalDialog(NULL, &ditem);
		switch (ditem) {
			case diRenameOK:
				GetDItem(win, diFeatureRenameType, &itemtype, &itemhandle, &itemrect);
				set_feature_type_name(feature, get_string_from_item(itemhandle));
				GetDItem(win, diFeatureRenameName, &itemtype, &itemhandle, &itemrect);
				set_feature_name(feature, get_string_from_item(itemhandle));
				/* Fall into next case. */
			case diRenameCancel:
				done = TRUE;
				break;
		}
	}
	DisposDialog(win);
}

/* Handling of mousedowns in the map when designing. */

apply_designer_tool(map, h, v, mods)
Map *map;
int h, v, mods;
{
	int x, y, dir;
	int oldt, oldpeop, oldfid;
	Unit *unit;

	nearest_boundary(map, h, v, &x, &y, &dir);
	switch (tooltype) {
		case terraintool:
			/* Dispatch on terrain subtype. */
			switch (t_subtype(curttype)) {
				case cellsubtype:
					/* Choose to paint fg or bg type, depending on what's already
					   there. */
					oldt = terrain_at(x, y);
					painttype = (curttype == oldt ? curbgttype : curttype);
					paint_cell(dside, x, y, curbrushradius, painttype);
					paint_on_drag(map, h, v, mods);
					break;
				case bordersubtype:
					/* Toggle border on first mouse down. */
					paint_border(dside, x, y, dir, curttype, -1);
					/* Dragging then adds or removes, depending on toggle's result. */
					border_on_drag(map, h, v, mods, border_at(x, y, dir, curttype));
					break;
				case connectionsubtype:
					/* Toggle connection on first mouse down. */
					paint_connection(dside, x, y, dir, curttype, -1);
					/* Dragging then adds or removes, depending on toggle's result. */
					connect_on_drag(map, h, v, mods, connection_at(x, y, dir, curttype));
					break;
				case coatingsubtype:
					paint_coating(dside, x, y, curbrushradius, curttype, curdepth);
					paint_on_drag(map, h, v, mods);
					break;
				default:
					terrain_subtype_warning("apply tool", curttype);
					break;
			}
			return;
		case unittool:
			/* A last check, should never(?) fail. */
			if (!type_allowed_on_side(curutype, side_n(cursidenumber))) {
				SysBeep(20);
				return;
			}
			if ((unit = designer_create_unit(dside, curutype, cursidenumber, x, y)) != NULL) {
				/* Make the new unit automatically be the current selection. */
				unselect_all(map);
				select_unit_on_map(map, unit);
				draw_selections_at(map, unit->x, unit->y);
			} else {
				/* Beep if the creation failed for some reason. */
				SysBeep(20);
			}
			/* No use for drag painting here, unlike most other tools. */
			break;
		case peopletool:
			/* Paint people or clear, inverting from what is already here. */
			oldpeop = people_side_at(x, y);
			paintpeop = (cursidenumber == oldpeop ? NOBODY : cursidenumber);
			paint_people(dside, x, y, curbrushradius, paintpeop);
			paint_on_drag(map, h, v, mods);
			break;
		case featuretool:
			oldfid = raw_feature_at(x, y);
			paintfid = (curfid == oldfid ? 0 : curfid);
			paint_feature(dside, x, y, curbrushradius, paintfid);
			paint_on_drag(map, h, v, mods);
			break;
		case materialtool:
			paint_material(dside, x, y, curbrushradius, curmtype, curmamount);
			paint_on_drag(map, h, v, mods);
			break;
		case elevationtool:
			paint_elevation(dside, x, y, curbrushradius, curelevation);
			paint_on_drag(map, h, v, mods);
			break;
		case temperaturetool:
			paint_temperature(dside, x, y, curbrushradius, curtemperature);
			paint_on_drag(map, h, v, mods);
			break;
		case cloudstool:
			paint_clouds(dside, x, y, curbrushradius, curcloudtype, curcloudbottom, curcloudheight);
			paint_on_drag(map, h, v, mods);
			break;
		case windstool:
			paint_winds(dside, x, y, curbrushradius, curwinddir, curwindforce);
			paint_on_drag(map, h, v, mods);
			break;
		default:
			SysBeep(20);
	}
}

paint_on_drag(map, h0, v0, mods)
Map *map;
int h0, v0, mods;
{
	Point pt0, pt1, newmouse;
	int h1, v1, drawn = FALSE, x, y;
	Rect tmprect;

	SetPt(&pt0, h0, v0);
	SetPt(&pt1, h0, v0);
	SetRect(&tmprect, h0, v0, h0, v0);
	while (WaitMouseUp()) {
		GetMouse(&newmouse);
		if (!EqualPt(pt1, newmouse) /* && PtInRect(newmouse, &(map->window->portRect)) */) {
			pt1 = newmouse;
			nearest_cell(map, pt1.h, pt1.v, &x, &y);
			if (in_area(x, y)) {
				switch (tooltype) {
					case terraintool:
						/* Dispatch on terrain subtype. */
						switch (t_subtype(curttype)) {
							/* This sort of drag-paint only works for area fillers,
							   bords/conns use different algorithm. */
							case cellsubtype:
								paint_cell(dside, x, y, curbrushradius, painttype);
								break;
							case coatingsubtype:
								paint_coating(dside, x, y, curbrushradius, curttype, curdepth);
								break;
						}
						break;
					case peopletool:
						paint_people(dside, x, y, curbrushradius, paintpeop);
						break;
					case materialtool:
						paint_material(dside, x, y, curbrushradius, curmtype, curmamount);
						break;
					case featuretool:
						paint_feature(dside, x, y, curbrushradius, paintfid);
						break;
					case elevationtool:
						paint_elevation(dside, x, y, curbrushradius, curelevation);
						break;
					case temperaturetool:
						paint_temperature(dside, x, y, curbrushradius, curtemperature);
						break;
					case cloudstool:
						paint_clouds(dside, x, y, curbrushradius, curcloudtype, curcloudbottom, curcloudheight);
						break;
					case windstool:
						paint_winds(dside, x, y, curbrushradius, curwinddir, curwindforce);
						break;
				}
			}
		}
	}
}

border_on_drag(map, h0, v0, mods, paintmode)
Map *map;
int h0, v0, mods, paintmode;
{
	Point pt0, pt1, newmouse;
	int h1, v1, drawn = FALSE, x, y, dir;
	Rect tmprect;

	SetPt(&pt0, h0, v0);
	SetPt(&pt1, h0, v0);
	SetRect(&tmprect, h0, v0, h0, v0);
	while (WaitMouseUp()) {
		GetMouse(&newmouse);
		if (!EqualPt(pt1, newmouse) /* && PtInRect(newmouse, &(map->window->portRect)) */) {
			pt1 = newmouse;
			nearest_boundary(map, pt1.h, pt1.v, &x, &y, &dir);
			if (inside_area(x, y)) {
				paint_border(dside, x, y, dir, curttype, paintmode);
			}
		}
	}
}

connect_on_drag(map, h0, v0, mods, paintmode)
Map *map;
int h0, v0, mods, paintmode;
{
	Point pt0, pt1, newmouse;
	int h1, v1, drawn = FALSE, x, y, dir;
	Rect tmprect;

	SetPt(&pt0, h0, v0);
	SetPt(&pt1, h0, v0);
	SetRect(&tmprect, h0, v0, h0, v0);
	while (WaitMouseUp()) {
		GetMouse(&newmouse);
		if (!EqualPt(pt1, newmouse) /* && PtInRect(newmouse, &(map->window->portRect)) */) {
			pt1 = newmouse;
			nearest_boundary(map, pt1.h, pt1.v, &x, &y, &dir);
			if (inside_area(x, y)) {
				paint_connection(dside, x, y, dir, curttype, paintmode);
			}
		}
	}
}

#define set_flag_from_ditem(di,place)  \
  GetDItem(win, (di), &itemtype, &itemhandle, &itemrect);  \
  (place) = GetCtlValue((ControlHandle) itemhandle); 

#define put_number_into_ditem(di,num)  \
  GetDItem(win, (di), &itemtype, &itemhandle, &itemrect);  \
  NumToString((num), tmpstr);  \
  SetIText(itemhandle, tmpstr);

#define get_number_from_ditem(di,place)  \
  GetDItem(win, (di), &itemtype, &itemhandle, &itemrect);  \
  GetIText(itemhandle, tmpstr);  \
  StringToNum(tmpstr, &(place));

#define get_string_from_ditem(di,buf)  \
  GetDItem(win, (di), &itemtype, &itemhandle, &itemrect);  \
  GetIText(itemhandle, tmpstr);  \
  strncpy((buf), tmpstr+1, tmpstr[0]); /* this is basically p2c */  \
  (buf)[tmpstr[0]] = '\0';

/* This allows the designer to choose which parts of a game to write out. */

int defunitids; /* should be module slot */

designer_save_dialog()
{
	int done = FALSE;
	short ditem;
	char namebuf[BUFSIZE];
	Str255 tmpstr;
	Point pnt;
	WindowPtr win;
	Module *module;
    SFReply reply;
	short itemtype;  Handle itemhandle;  Rect itemrect;
	
	win = GetNewDialog(dDesignerSave, NULL, (DialogPtr) -1L);
	module = create_game_module(NULL);
	module->title = "Designer-saved data";
	if (module == NULL) { SysBeep(20); return; }
	init_module_reshape(module);
	/* Only rarely does the designer not want to compress all the area layers. */
	module->compresslayers = TRUE;
	GetDItem(win, diDesignerSaveCompress, &itemtype, &itemhandle, &itemrect);
	SetCtlValue((ControlHandle) itemhandle, module->compresslayers);
	while (!done) {
		/* Deactivate the front window. */
		activate_window(FrontWindow(), FALSE);
		ModalDialog(NULL, &ditem);
		switch (ditem) {
			case diDesignerSaveOK:
				get_string_from_ditem(diDesignerSaveName, namebuf);
				module->name = copy_string(namebuf);
				set_flag_from_ditem(diDesignerSaveTypes, module->deftypes);
				set_flag_from_ditem(diDesignerSaveTables, module->deftables);
				set_flag_from_ditem(diDesignerSaveGlobals, module->defglobals);
				set_flag_from_ditem(diDesignerSaveWorld, module->defworld);
				set_flag_from_ditem(diDesignerSaveAreas, module->defareas);
				set_flag_from_ditem(diDesignerSaveAreaTerrain, module->defareaterrain);
				set_flag_from_ditem(diDesignerSaveAreaMisc, module->defareamisc);
				set_flag_from_ditem(diDesignerSaveAreaWeather, module->defareaweather);
				set_flag_from_ditem(diDesignerSaveAreaMaterial, module->defareamaterial);
				set_flag_from_ditem(diDesignerSaveSides, module->defsides);
				set_flag_from_ditem(diDesignerSaveSideViews, module->defsideviews);
				set_flag_from_ditem(diDesignerSavePlayers, module->defplayers);
				set_flag_from_ditem(diDesignerSaveUnits, module->defunits);
				set_flag_from_ditem(diDesignerSaveUnitIds, defunitids);
				set_flag_from_ditem(diDesignerSaveUnitProps, module->defunitprops);
				set_flag_from_ditem(diDesignerSaveUnitMoves, module->defunitacts);
				set_flag_from_ditem(diDesignerSaveUnitPlans, module->defunitplans);
				set_flag_from_ditem(diDesignerSaveScoring, module->defscoring);
				set_flag_from_ditem(diDesignerSaveHistory, module->defhistory);
				set_flag_from_ditem(diDesignerSaveCompress, module->compresslayers);
				/* Collect the file and path to save to. */
				SetPt(&pnt, 100, 100);
				sprintf(spbuf, "%s.g", namebuf);
				c2p(spbuf, tmpstr);
				SFPutFile(pnt, "\p", tmpstr, (DialogPtr) nil, &reply);
				if (reply.good) {
					/* Make the location of the file be the current volume. */
					SetVol(reply.fName, reply.vRefNum);
					p2c(((char *) reply.fName), namebuf);
					module->filename = copy_string(namebuf);
					if (!write_game_module(module)) {
						run_warning("Couldn't write the module \"%s\"!", module->filename);
						/* Don't fall through, might be able to fix by changing save options. */
						break;
					}
				} else {
					break;
				}
				/* Fall through to next case. */
			case diDesignerSaveCancel:
				done = TRUE;
				break;
			case diDesignerSaveTypes:
			case diDesignerSaveTables:
			case diDesignerSaveGlobals:
			case diDesignerSaveWorld:
			case diDesignerSaveAreas:
			case diDesignerSaveAreaTerrain:
			case diDesignerSaveAreaMisc:
			case diDesignerSaveAreaWeather:
			case diDesignerSaveAreaMaterial:
			case diDesignerSaveSides:
			case diDesignerSaveSideNames:
			case diDesignerSaveSideProps:
			case diDesignerSaveSideViews:
			case diDesignerSavePlayers:
			case diDesignerSaveUnits:
			case diDesignerSaveUnitIds:
			case diDesignerSaveUnitProps:
			case diDesignerSaveUnitMoves:
			case diDesignerSaveUnitPlans:
			case diDesignerSaveScoring:
			case diDesignerSaveHistory:
			case diDesignerSaveCompress:
				/* Toggle check boxes. */
				GetDItem(win, ditem, &itemtype, &itemhandle, &itemrect);
				SetCtlValue((ControlHandle) itemhandle,
							!GetCtlValue((ControlHandle) itemhandle));
				break;
			case diDesignerSaveModule:
				/* (should bring up a dialog to set module properties) */
				break;
			case diDesignerSaveReshape:
				/* Bring up *another* modal dialog. */
				designer_reshape_dialog(module);
				break;
		}
	}
	DisposDialog(win);
}

/* A special dialog that allows for saving a world of different dimensions than is
   currently being used in the game. */

designer_reshape_dialog(module)
Module *module;
{
	int done = FALSE;
	short ditem;
	WindowPtr win;
	Str255 tmpstr;
	short itemtype;  Handle itemhandle;  Rect itemrect;
	
	win = GetNewDialog(dDesignerReshape, NULL, (DialogPtr) -1L);
	while (!done) {
		/* Deactivate the front window. */
		activate_window(FrontWindow(), FALSE);
  		put_number_into_ditem(diDesignerReshapeOrigWidth, area.width);
  		put_number_into_ditem(diDesignerReshapeOrigHeight, area.height);
  		put_number_into_ditem(diDesignerReshapeOrigWorld, world.circumference);
  		put_number_into_ditem(diDesignerReshapeOrigSubWidth, module->subareawidth);
  		put_number_into_ditem(diDesignerReshapeOrigSubHeight, module->subareaheight);
  		put_number_into_ditem(diDesignerReshapeOrigSubX, module->subareax);
  		put_number_into_ditem(diDesignerReshapeOrigSubY, module->subareay);
  		put_number_into_ditem(diDesignerReshapeOutputSubWidth, module->finalsubareawidth);
  		put_number_into_ditem(diDesignerReshapeOutputSubHeight, module->finalsubareaheight);
  		put_number_into_ditem(diDesignerReshapeOutputSubX, module->finalsubareax);
  		put_number_into_ditem(diDesignerReshapeOutputSubY, module->finalsubareay);
  		put_number_into_ditem(diDesignerReshapeOutputWidth, module->finalwidth);
  		put_number_into_ditem(diDesignerReshapeOutputHeight, module->finalheight);
  		put_number_into_ditem(diDesignerReshapeOutputWorld, module->finalcircumference);
/*  		put_number_into_ditem(diDesignerReshapeFillTerrain, module->fillterrain); */
		ModalDialog(NULL, &ditem);
		switch (ditem) {
			case diDesignerReshapeOK:
		  		get_number_from_ditem(diDesignerReshapeOrigSubWidth, module->subareawidth);
		  		get_number_from_ditem(diDesignerReshapeOrigSubHeight, module->subareaheight);
		  		get_number_from_ditem(diDesignerReshapeOrigSubX, module->subareax);
		  		get_number_from_ditem(diDesignerReshapeOrigSubY, module->subareay);
		  		get_number_from_ditem(diDesignerReshapeOutputSubWidth, module->finalsubareawidth);
		  		get_number_from_ditem(diDesignerReshapeOutputSubHeight, module->finalsubareaheight);
		  		get_number_from_ditem(diDesignerReshapeOutputSubX, module->finalsubareax);
		  		get_number_from_ditem(diDesignerReshapeOutputSubY, module->finalsubareay);
		  		get_number_from_ditem(diDesignerReshapeOutputWidth, module->finalwidth);
		  		get_number_from_ditem(diDesignerReshapeOutputHeight, module->finalheight);
		  		get_number_from_ditem(diDesignerReshapeOutputWorld, module->finalcircumference);
/*		  		get_number_from_ditem(diDesignerReshapeFillTerrain, module->fillterrain);  */
				/* Fall through to next case. */
			case diDesignerReshapeCancel:
				done = TRUE;
				break;
		}
	}
	DisposDialog(win);
}

#endif /* DESIGNERS */
