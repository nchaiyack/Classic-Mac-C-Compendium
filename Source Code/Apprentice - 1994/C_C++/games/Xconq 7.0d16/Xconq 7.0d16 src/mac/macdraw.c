/* Copyright (c) 1992, 1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Picture drawing for the Mac interface. */

#include "conq.h"
#include "mac.h"

#include <Sound.h>

#define move_cell_region(sx, sy, power)  \
  OffsetRgn(cellrgns[power], sx - lastcellrgnx[power], sy - lastcellrgny[power]);  \
  lastcellrgnx[power] = sx;  lastcellrgny[power] = sy;  \
  OffsetRgn(gridcellrgns[power], sx - lastgridcellrgnx[power], sy - lastgridcellrgny[power]);  \
  lastgridcellrgnx[power] = sx;  lastgridcellrgny[power] = sy;

extern BitMap bordbitmaps[];
extern BitMap connbitmaps[];

PolyHandle polygons[NUMPOWERS];
int lastpolyx[NUMPOWERS], lastpolyy[NUMPOWERS];

RgnHandle cellrgns[NUMPOWERS];
int lastcellrgnx[NUMPOWERS], lastcellrgny[NUMPOWERS];
RgnHandle gridcellrgns[NUMPOWERS];
int lastgridcellrgnx[NUMPOWERS], lastgridcellrgny[NUMPOWERS];

Image **bestterrainimages = NULL;

int numwindsicns = 0;

Handle windsicnhandle[5];

/* Draw an image of the given type of unit in the given rectangle, possibly adding
   an emblem if requested. */

void
draw_unit_image(win, sx, sy, sw, sh, u, e, mod)
WindowPtr win;
int sx, sy, sw, sh, u, e, mod;
{
	int ex, ey, ew, eh;
	Rect srcrect, imagerect;
	RgnHandle tmprgn;
	BitMap bm, *winbits;
	Image *uimg;

	imagerect = win->portRect;
	imagerect.top += sy;  imagerect.left += sx;
	imagerect.bottom = imagerect.top + sh;  imagerect.right = imagerect.left + sw;
	/* Filter out very small images. */
	if (sw <= 1) return;
	if (sw <= 4) {
		/* (should draw in a distinctive color if one is available) */
		FillRect(&imagerect, QD(black));
		return;
	}
	uimg = best_image(&(uimages[u]), sw, sh);
	/* There should always be *some* image to display. */
	if (uimg) {
		winbits = &(((GrafPtr) win)->portBits);
		if (uimg->monopict != nil) {
			DrawPicture(uimg->monopict, &imagerect);
		} else if (uimg->colricon != nil
				   && (minscreendepth > 1
				   	   || (uimg->monoicon == nil && uimg->monosicn == nil))) {
			PlotCIcon(&imagerect, (CIconHandle) uimg->colricon);
		} else if (uimg->monoicon != nil) {
			SetRect(&srcrect, 0, 0, 32, 32);
			bm.rowBytes = 4;
			bm.bounds = srcrect;
			if (uimg->maskicon != nil) {
				bm.baseAddr = *(uimg->maskicon);
				CopyBits(&bm, winbits, &srcrect, &imagerect, srcBic, nil);
			} else {
				/* Draw unit bbox as default mask. (maybe shrink a little??) */
				FillRect(&imagerect, QD(white));
			}
			bm.baseAddr = *(uimg->monoicon);
			CopyBits(&bm, winbits, &srcrect, &imagerect, srcOr, nil);
		} else if (uimg->monosicn != nil) {
			SetRect(&srcrect, 0, 0, 16, 16);
			if (sw > 64) {
				InsetRect(&imagerect, sw / 4, sh / 4);
			}
			bm.rowBytes = 2;
			bm.bounds = srcrect;
			if (uimg->masksicn != nil) {
				bm.baseAddr = *(uimg->masksicn);
				CopyBits(&bm, winbits, &srcrect, &imagerect, srcBic, nil);
			} else {
				/* Draw unit bbox as default mask. (maybe shrink a little??) */
				FillRect(&imagerect, QD(white));
			}
			bm.baseAddr = *(uimg->monosicn);
			CopyBits(&bm, winbits, &srcrect, &imagerect, srcOr, nil);
		} else {
			run_warning("Image with no content?");
		}
	} else {
		FillRect(&imagerect, QD(black));
	}
	if (mod != 0) {
		gray_out_rect(&imagerect);
	}
	/* Now draw a side emblem if asked for. */
	if (between(0, e, numsides)) {
		if (uimg &&
		    uimg->embedname &&
		    side_n(e) &&
		    side_n(e)->emblemname &&
		    strcmp(uimg->embedname, side_n(e)->emblemname) == 0) {
		    /* Correct emblem is part of the unit's image, don't need to draw. */
		} else {
		    /* Get the size of the emblem, either from the image or by computing
		       a reasonable default. */
			if (uimg && uimg->embedw > 0 && uimg->embedh > 0) {
				ew = uimg->embedw;  eh = uimg->embedh;
			} else {
				ew = min(sw, max(8, sw / 4));  eh = min(sh, max(8, sh / 4));
			}
			/* Position the emblem, either explicitly, or default to UR corner
			   (note that we need the emblem's width to do this) */
			if (uimg && uimg->embedx >= 0 && uimg->embedy >= 0) {
				ex = uimg->embedx;  ey = uimg->embedy;
			} else {
				ex = sw - ew;  ey = 0;
			}
			/* Do the drawing proper. */
			draw_side_emblem(win, sx + ex, sy + ey, ew, eh, e);
		}
	}
}

/* Draw a given side id's emblem. Uses the current GrafPort. */

void
draw_side_emblem(win, ex, ey, ew, eh, e)
WindowPtr win;
int ex, ey, ew, eh, e;
{
	int actualw, actualh;
	Rect srcrect, imagerect = win->portRect;
	RgnHandle tmprgn;
	BitMap bm, *winbits;
	Image *eimg;

	/* Filter out very small images. */
	if (ew <= 1) return;
	if (ew <= 2) {
		/* (should draw in a distinctive color if one is available) */
		/* FillRect(&imagerect, QD(black)); */
		return;
	}
	eimg = best_image(&(eimages[e]), ew, eh);
	imagerect.top += ey;  imagerect.left += ex;
	actualw = eimg->w;  actualh = eimg->h;
	if (ew >= actualw * 2 && eh >= actualh * 2) { actualw *= 2;  actualh *= 2; }
	if (ew >= actualw * 2 && eh >= actualh * 2) { actualw *= 2;  actualh *= 2; }
	if (actualw < ew) ew = actualw;
	if (actualh < eh) eh = actualh;
	imagerect.bottom = imagerect.top + eh;  imagerect.right = imagerect.left + ew;
	/* If an image is available, display it, otherwise do nothing. */
	if (eimg) {
		winbits = &(((GrafPtr) win)->portBits);
		if (eimg->monopict != nil) {
			DrawPicture(eimg->monopict, &imagerect);
		} else if (eimg->colricon != nil
				   && (minscreendepth > 1
				   	   || (eimg->monoicon == nil && eimg->monosicn == nil))) {
			PlotCIcon(&imagerect, (CIconHandle) eimg->colricon);
		} else if (eimg->monoicon != nil) {
			SetRect(&srcrect, 0, 0, 32, 32);
			bm.rowBytes = 4;
			bm.bounds = srcrect;
			if (eimg->maskicon != nil) {
				bm.baseAddr = *(eimg->maskicon);
				CopyBits(&bm, winbits, &srcrect, &imagerect, srcBic, nil);
			} else {
				/* Use emblem bbox as default mask. */
				FillRect(&imagerect, QD(white));
			}
			bm.baseAddr = *(eimg->monoicon);
			CopyBits(&bm, winbits, &srcrect, &imagerect, srcOr, nil);
		} else if (eimg->monosicn != nil) {
			SetRect(&srcrect, 0, 0, 16, 16);
			bm.rowBytes = 2;
			bm.bounds = srcrect;
			if (eimg->masksicn != nil) {
				bm.baseAddr = *(eimg->masksicn);
				CopyBits(&bm, winbits, &srcrect, &imagerect, srcBic, nil);
			} else {
				/* Use emblem bbox as default mask. */
				FillRect(&imagerect, QD(white));
			}
			bm.baseAddr = *(eimg->monosicn);
			CopyBits(&bm, winbits, &srcrect, &imagerect, srcOr, nil);
		} else {
			run_warning("Image with no content?");
		}
	}
}

draw_hex_block(sx, sy, n, power, t, t2)
int sx, sy, n, power, t, t2;
{
	Rect rect;
	Image *timg;
	RGBColor hexcolor, oldcolor;				

	rect.left = sx;  rect.top = sy;
	rect.right = rect.left + n * hws[power];  rect.bottom = rect.top + hcs[power];
	if (bestterrainimages == NULL) {
		calc_best_terrain_images();
	}
	timg = bestterrainimages[power * numttypes + t];
	if (timg) {
		if (hasColorQD) {
			if (timg->colrpat != nil
			    && (minscreendepth > 1 || !timg->patdefined)) {
				FillCRect(&rect, timg->colrpat);
			} else if (tcolors[t] != NULL && minscreendepth > 1) {
				hexcolor.red   = (tcolors[t]->r) << 8;
				hexcolor.green = (tcolors[t]->g) << 8;
				hexcolor.blue  = (tcolors[t]->b) << 8;
				RGBForeColor(&hexcolor);
				PaintRect(&rect);
				/* Restore the previous color. */
				oldcolor.red = oldcolor.green = oldcolor.blue = 0;
				RGBForeColor(&oldcolor);
			} else if (timg->patdefined) {
#ifdef THINK_C
				FillRect(&rect, (Pattern *) &(timg->pat));
#endif
			}
		} else {
#ifdef THINK_C
			FillRect(&rect, (Pattern *) &(timg->pat));
#endif
		}
		if (t2 == -1) {
			PenPat(QD(gray));
/*			PenMode(patBic);  */
			PenMode(notPatOr);
			PaintRect(&rect);
			PenNormal();
		}
	}
}

calc_best_terrain_images()
{
	int p, t;

	bestterrainimages = (Image **) xmalloc(NUMPOWERS * numttypes * sizeof(Image *));
	for_all_terrain_types(t) {
		for (p = 0; p < NUMPOWERS; ++p) {
			bestterrainimages[p * numttypes + t] = 
				best_image(&(timages[t]), hws[p], hcs[p]);
		}
	}
}

/* Do the grody work of drawing very large polygons accurately.  Do this by calculating
   the hex polygon only once (when first needed), then offsetting each time to draw. */

draw_hex_polygon(sx, sy, power, dogrid, t, t2)
int sx, sy, power, dogrid, t, t2;
{
	int hw = hws[power], hh = hhs[power], delt = (hhs[power] - hcs[power]);
	int nw = 0, se = 0;
	PolyHandle poly;
	Image *timg;
	RGBColor hexcolor, oldcolor;

	poly = polygons[power];
	if (poly == nil) {
		poly = OpenPoly();
		if (hw != 20) {		
			MoveTo(hw / 2, nw);
			LineTo(hw - se, delt);
			LineTo(hw - se, hh - delt);
			LineTo(hw / 2, hh - se);
			LineTo(0 + nw, hh - delt);
			LineTo(0 + nw, delt);
			LineTo(hw / 2, nw);
		} else {
			MoveTo(10, 0);
			LineTo(20 - se, 5);
			LineTo(20 - se, 17);
			LineTo(10, 22 - se);
			LineTo(0, 17);
			LineTo(0, 5);
			LineTo(10, 0);
		}
		ClosePoly();
		polygons[power] = poly;
		lastpolyx[power] = lastpolyy[power] = 0;
	}
	OffsetPoly(poly, sx - lastpolyx[power], sy - lastpolyy[power]);
	lastpolyx[power] = sx;  lastpolyy[power] = sy;
	if (bestterrainimages == NULL) {
		calc_best_terrain_images();
	}
	timg = bestterrainimages[power * numttypes + t];
	if (timg) {
		if (hasColorQD) {
			if (timg->colrpat != nil
			    && (minscreendepth > 1 || !timg->patdefined)) {
				FillCPoly(poly, timg->colrpat);
			} else if (tcolors[t] != NULL && maxscreendepth > 1) {
				hexcolor.red   = (tcolors[t]->r) << 8;
				hexcolor.green = (tcolors[t]->g) << 8;
				hexcolor.blue  = (tcolors[t]->b) << 8;
				RGBForeColor(&hexcolor);
				PaintPoly(poly);
				/* Restore the previous color. */
				oldcolor.red = oldcolor.green = oldcolor.blue = 0;
				RGBForeColor(&oldcolor);
			} else if (timg->patdefined) {
				/* Fall back on the b/w pattern. */
#ifdef THINK_C
				FillPoly(poly, (Pattern *) &(timg->pat));
#endif
			}
		} else {
#ifdef THINK_C
			FillPoly(poly, (Pattern *) &(timg->pat));
#endif
		}
		/* Darken the cell. */
		if (t2 == -1) {
			PenPat(QD(gray));
/*			PenMode(patBic);  */
			PenMode(notPatOr);
			PaintPoly(poly);
			PenNormal();
		}
	}
}

draw_hex_region(sx, sy, power, dogrid, t, t2)
int sx, sy, power, dogrid, t, t2;
{
	Image *timg;
	RGBColor hexcolor, oldcolor;
	RgnHandle rgn;

	if (bestterrainimages == NULL) {
		calc_best_terrain_images();
	}
	timg = bestterrainimages[power * numttypes + t];
	if (timg) {
		move_cell_region(sx, sy, power);
		rgn = (dogrid ? gridcellrgns[power] : cellrgns[power]);
		if (hasColorQD) {
			if (timg->colrpat != nil
			    && (minscreendepth > 1 || !timg->patdefined)) {
				FillCRgn(rgn, timg->colrpat);
			} else if (tcolors[t] != NULL && maxscreendepth > 1) {
				hexcolor.red   = (tcolors[t]->r) << 8;
				hexcolor.green = (tcolors[t]->g) << 8;
				hexcolor.blue  = (tcolors[t]->b) << 8;
				RGBForeColor(&hexcolor);
				PaintRgn(rgn);
				/* Restore the previous color. */
				oldcolor.red = oldcolor.green = oldcolor.blue = 0;
				RGBForeColor(&oldcolor);
			} else if (timg->patdefined) {
				/* Fall back on the b/w pattern. */
#ifdef THINK_C
				FillRgn(rgn, (Pattern *) &(timg->pat));
#endif
			}
		} else {
#ifdef THINK_C
			FillRgn(rgn, (Pattern *) &(timg->pat));
#endif
		}
		/* Darken the cell. */
		if (t2 == -1) {
			PenPat(QD(gray));
/*			PenMode(patBic);  */
			PenMode(notPatOr);
			PaintRgn(rgn);
			PenNormal();
		}
	}
}

/* Draw a set of borders for the given hex. */

draw_border_line_multiple(win, sx, sy, bitmask, power, t)
WindowPtr win;
int sx, sy, bitmask, power, t;
{
	int wid = bwid[power], wid2, dir;
	Image *timg;

	if (wid == 0) return;
	wid2 = wid / 2;
	if (0 /* power == 4*/) {
		Rect srcrect, destrect;
		BitMap *winbits;

		winbits = &(((GrafPtr) win)->portBits);
		SetRect(&srcrect, 0, 0, 32, 32);
		SetRect(&destrect, sx, sy, sx+32, sy+32);
		CopyBits(&(bordbitmaps[4]), winbits, &srcrect, &destrect, srcOr, nil);
		return;
	}
	PenSize(wid, wid);
	/* Decide on the line color/pattern to use. */
	timg = best_image(&(timages[t]), wid, wid);
	for_all_directions(dir) {
		if (bitmask & (1 << dir)) {
			if (timg) {
				if (hasColorQD) {
					if (timg->colrpat && (maxscreendepth > 1 || !timg->patdefined)) {
						PenPixPat(timg->colrpat);
					} else {
#ifdef THINK_C
						PenPat((Pattern *) &(timg->pat));
#endif
					}
				} else {
#ifdef THINK_C
					PenPat((Pattern *) &(timg->pat));
#endif
				}
			} else {
				PenPat(QD(dkGray));
			}
			/* Actually draw the line. */
			MoveTo(sx + bsx[power][dir] - wid2, sy + bsy[power][dir] - wid2);
			LineTo(sx + bsx[power][dir+1] - wid2, sy + bsy[power][dir+1] - wid2);
		}
	}
	PenNormal();
}

/* Draw a set of connections for the given ttype and given hex. */

draw_connection_line_multiple(win, sx, sy, bitmask, power, t)
WindowPtr win;
int sx, sy, bitmask, power, t;
{
	int dir, wid = cwid[power];
	Image *timg;

	if (wid == 0) return;
	if (0 /*power == 4*/) {
		Rect srcrect, destrect;
		BitMap *winbits;

		winbits = &(((GrafPtr) win)->portBits);
		SetRect(&srcrect, 0, 0, 32, 32);
		SetRect(&destrect, sx, sy, sx+32, sy+32);
		CopyBits(&(connbitmaps[4]), winbits, &srcrect, &destrect, srcOr, nil);
		return;
	}
	PenSize(wid, wid);
	timg = best_image(&(timages[t]), wid, wid);
	for_all_directions(dir) {
		if (bitmask & (1 << dir)) {
			if (timg) {
				if (hasColorQD) {
					if (timg->colrpat && (maxscreendepth > 1 || !timg->patdefined)) {
						PenPixPat(timg->colrpat);
					} else {
#ifdef THINK_C
						PenPat((Pattern *) &(timg->pat));
#endif
					}
				} else {
#ifdef THINK_C
					PenPat((Pattern *) &(timg->pat));
#endif
				}
			} else {
				PenPat(QD(gray));
			}
			MoveTo(sx + hws[power] / 2 - wid / 2, sy + hhs[power] / 2 - wid / 2);
			Line(lsx[power][dir], lsy[power][dir]);
		}
	}
	PenNormal();
}

/* This draws a type of terrain in a way that indicates its subtype. */

draw_terrain_sample(tmprect, t)
Rect tmprect;
int t;
{
	int dir;

	switch (t_subtype(t)) {
		case cellsubtype:
			draw_hex_polygon(tmprect.left, tmprect.top, 4, FALSE, t, 0);
			break;
		case bordersubtype:
			draw_border_line_multiple(designwin, tmprect.left, tmprect.top, -1, 4, t);
			break;
		case connectionsubtype:
			draw_connection_line_multiple(designwin, tmprect.left, tmprect.top, -1, 4, t);
			break;
		case coatingsubtype:
			draw_hex_polygon(tmprect.left, tmprect.top, 4, FALSE, t, 0);
			/* Make it a 50% pattern. (should make more obvious somehow?) */
			gray_out_rect(&tmprect);
			break;
		default:
			terrain_subtype_warning("draw sample", t);
			break;
	}
}

/* Draw a set of country border at the given position. */

draw_country_borders(win, sx, sy, bitmask, power, shade)
WindowPtr win;
int sx, sy, bitmask, power, shade;
{
	int wid = bwid2[power], wid2, dir;

	if (wid == 0) return;
	PenSize(wid, wid);
	if (shade == 0) PenPat(QD(black));
	if (shade == 2) PenPat(QD(gray));
	wid2 = wid / 2;
	for_all_directions(dir) {
		if (bitmask & (1 << dir)) {
			MoveTo(sx + bsx[power][dir] - wid2, sy + bsy[power][dir] - wid2);
			LineTo(sx + bsx[power][dir+1] - wid2, sy + bsy[power][dir+1] - wid2);
		}
	}
	PenNormal();
}

/* Draw a set of theater borders at the given position. */

draw_theater_borders(win, sx, sy, bitmask, power)
WindowPtr win;
int sx, sy, bitmask, power;
{
	int wid, wid2, dir;
	Rect tmprect;

	if (bwid[power] > 0) {
		wid = 2;
		PenSize(wid, wid);
		PenMode(notPatCopy);
		wid2 = wid / 2;
		for_all_directions(dir) {
			if (bitmask & (1 << dir)) {
				MoveTo(sx + bsx[power][dir] - wid2, sy + bsy[power][dir] - wid2);
				LineTo(sx + bsx[power][dir+1] - wid2, sy + bsy[power][dir+1] - wid2);
			}
		}
		PenNormal();
	} else {
		SetRect(&tmprect, sx, sy, sx + hws[power], sy + hhs[power]);
		FillRect(&tmprect, QD(white));
	}
}

/* Indicate the elevation of the given location, textually for now. */

draw_elevation(sx, sy, power, elev)
int sx, sy, power, elev;
{
	sx += hws[power] / 2;  sy += hhs[power] / 2;
	if (elev != 0) {
		sprintf(spbuf, "%d", elev);
		draw_legend_text(sx, sy, hhs[power] / 2, spbuf, 0);
	}
	/* (Also draw contour lines eventually) */
}

/* Don't draw the temperature in every hex, only do ones with even coords or
   ones where the temperature in any adjacent cell is different. */

draw_temperature_here(x, y)
int x, y;
{
	int dir, x1, y1, temphere = temperature_at(x, y);

	if (dside->designer) return TRUE;
	if (!g_see_all() && cover(dside, x, y) == 0) return FALSE;
	for_all_directions(dir) {
		point_in_dir(x, y, dir, &x1, &y1);
		if (temphere != temperature_at(x1, y1)) {
			return TRUE;
		}
	}
	return (x % 2 == 0 && y % 2 == 0);
}

/* Indicate the temperature of the given location, textually for now. */

draw_temperature(sx, sy, power, temp)
int sx, sy, power, temp;
{
	sx += hws[power] / 2;  sy += hhs[power] / 2;
	if (1 /* temp != 0 */) {
		sprintf(spbuf, "%d�", temp);  /* (should do char more portably) */
		draw_legend_text(sx, sy, hhs[power] / 2, spbuf, 0);
	}
	/* (Also draw isotherms eventually) */
}

/* Don't draw the winds in every hex, only do ones with odd coords or
   ones where the wind in any adjacent cell is different. */

draw_winds_here(x, y)
int x, y;
{
	int dir, x1, y1, windhere = raw_wind_at(x, y);

	if (dside->designer) return TRUE;
	if (!g_see_all() && cover(dside, x, y) == 0) return FALSE;
	for_all_directions(dir) {
		if (interior_point_in_dir(x, y, dir, &x1, &y1)) {
			if (windhere != raw_wind_at(x1, y1)) {
				return TRUE;
			}
		}
	}
	return (x % 2 == 1 && y % 2 == 1);
}

draw_clouds_here(x, y)
int x, y;
{
	int dir, x1, y1, cloudhere = raw_cloud_at(x, y);

	if (dside->designer) return TRUE;
	if (!g_see_all() && cover(dside, x, y) == 0) return FALSE;
	return TRUE;
}


/* Indicate the winds at the given location. */

draw_winds(sx, sy, power, wdir, wforce)
int sx, sy, power, wdir, wforce;
{
	GrafPtr curport;

	if (wforce > 0) {
		sx += (hws[power] - 16) / 2;  sy += (hhs[power] - 16) / 2;
		if (wforce > 4) wforce = 4;
		if (numwindsicns == 0) {
			int i;

			for (i = 1; i <= 4; ++i) {
				windsicnhandle[i] = GetResource('SICN', sicnWinds1 + i - 1);
			}
			numwindsicns = 4;
		}
		GetPort(&curport);
		plot_sicn(curport, sx, sy, windsicnhandle[wforce], wdir, FALSE);
	} else {
		sx += hws[power] / 2;  sy += hhs[power] / 2;
		sprintf(spbuf, "Calm");
		draw_legend_text(sx, sy, hhs[power] / 2, spbuf, 0);
	}
}

draw_clouds(sx, sy, power, cloudtype)
int sx, sy, power, cloudtype;
{
	Rect tmprect;

	if (cloudtype == 0) return;
	sx += hws[power] / 2;  sy += hhs[power] / 2;
	SetRect(&tmprect, sx - hws[power] / 2, sy - hhs[power] / 2, sx + hws[power] / 2, sy + hhs[power] / 2);
	/* Should use pat# 130 patterns for this instead. */
	PenPat(cloudtype == 3 ? QD(dkGray) : (cloudtype == 2 ? QD(gray) : QD(ltGray)));
	PenMode(patBic);
	PaintOval(&tmprect);
	PenNormal();
}

/* Draw the number of units observing a cell (for debugging). */

draw_coverage(sx, sy, power, cov)
int sx, sy, power, cov;
{
	sx += 2;  sy += hhs[power] - hcs[power] - 10;
	sprintf(spbuf, ":%d:", cov);
	draw_legend_text(sx, sy, hhs[power] / 2, spbuf, -1);
}

/* Draw a unit's name or number. */

draw_unit_name(unit, sx, sy, sw, sh)
Unit *unit;
int sx, sy, sw, sh;
{
	char legend[BUFSIZE];

	name_or_number(unit, legend);
	if (strlen(legend) > 0) {
		draw_legend_text(sx + sw + 1, sy + sh/2, sh, legend, -1);
	}
}

draw_legend_text(sx, sy, sh, legend, just)
int sx, sy, sh, just;
char *legend;
{
	int strwid, strleft;
	Rect maskrect;
	FontInfo fontinfo;
	Str255 tmpstr;
	
	/* Scale text sizes to fit in smaller cells if necessary. */
	TextSize(min(max(5, sh), 10));
	strwid = TextWidth(legend, 0, strlen(legend));
	if (just < 0) {
		strleft = sx;
	} else if (just > 0) {
		strleft = sx - strwid;
	} else {
		strleft = sx - strwid / 2;
	}
	MoveTo(strleft, sy);
	if (0) {
		/* Make it readable against a noisy background. */
/*		TextFace(bold|outline); */
		TextMode(srcBic);
	} else {
		/* This makes a big white box for name, less attractive but easier to read. */
		GetFontInfo(&fontinfo);
		maskrect.top = sy - fontinfo.ascent - 1;  /* this seems to be too much? */
		maskrect.top += 3;
		maskrect.left = strleft - 1;
		maskrect.bottom = sy + fontinfo.descent;
		maskrect.right = maskrect.left + strwid + 1;
		FillRect(&maskrect, QD(white));
	}
	DrawText(legend, 0, strlen(legend));
}

draw_blast_image(win, sx, sy, sw, sh, blasttype)
WindowPtr win;
int sx, sy, sw, sh, blasttype;
{
	long startcount;
	Rect tmprect;
	Handle sound;
	extern int numsoundplays;
	extern int playsounds;

	SetRect(&tmprect, sx, sy, sx + sw, sy + sh);
	if (playsounds && numsoundplays < 1) {
		if (0 /* certain games */) {
		    sound = GetNamedResource('snd ', "\plaser");
		} else {
			sound = GetNamedResource('snd ', "\pcrunch");
		}
	    if (sound != nil) {
	        SndPlay(nil, sound, false);
	        ReleaseResource(sound);
	    }
	    ++numsoundplays;
	}
    if (1) {
		if (sw >= 16) {
			/* Instead should save image under here, then draw blast */
			InvertRect(&tmprect);
		} else {
			InvertRect(&tmprect);
		}
		startcount = TickCount();
		if (playsounds) {
			if (blasttype > 1) {
		    	sound = GetNamedResource('snd ', "\pboom");
			} else if (blasttype > 0) {
		    	sound = GetNamedResource('snd ', "\pboom");
		    } else {
		    	sound = nil;
		    }
		    if (sound != nil) {
		        SndPlay(nil, sound, false);
		        ReleaseResource(sound);
		    }
		} else {
		    /* Delay for part of a second (should relinquish cpu tho) */
			while ((TickCount() - startcount) < (blasttype > 1 ? 27 : (blasttype > 0 ? 9 : 3)));
		}
		if (sw >= 16) {
			/* Instead should restore image under here */
			InvertRect(&tmprect);
		} else {
			InvertRect(&tmprect);
	    }
	}
}

picture_width(pichandle)
PicHandle pichandle;
{
	return ((*pichandle)->picFrame.right - (*pichandle)->picFrame.left);
}

picture_height(pichandle)
PicHandle pichandle;
{
	return ((*pichandle)->picFrame.bottom - (*pichandle)->picFrame.top);
}

/* Generic sicn drawer. */

plot_sicn(win, sx, sy, sicnhandle, n, erase)
WindowPtr win;
int sx, sy, n, erase;
Handle sicnhandle;
{
	Rect srcrect, imagerect;
	RgnHandle tmprgn;
	BitMap bm, *winbits;

	if (sicnhandle == nil) return;
	imagerect = win->portRect;
	imagerect.left += sx;  imagerect.top += sy;
	imagerect.right = imagerect.left + 16;  imagerect.bottom = imagerect.top + 16;
	winbits = &(((GrafPtr) win)->portBits);
	SetRect(&srcrect, 0, 0, 16, 16);
	bm.rowBytes = 2;
	bm.bounds = srcrect;
	if (erase) EraseRect(&imagerect);
	bm.baseAddr = *(sicnhandle) + 32 * n;
	CopyBits(&bm, winbits, &srcrect, &imagerect, srcOr, nil);
}

/* Given a rectangle, make half of its pixels white. */

gray_out_rect(rectptr)
Rect *rectptr;
{
	PenPat(QD(gray));
	PenMode(patBic);
	PaintRect(rectptr);
	PenNormal();
}
