/* Copyright (c) 1992, 1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Handling for the Macintosh implementation of image families. */

/* Note! that this file does not use the standard "conq" header, so can't assume
   the usual definitions. */
 
#ifdef THINK_C
#include <MacHeaders>
#else /* assume MPW */
#include <Types.h>
#include <Memory.h>
#include <Resources.h>
#include <Quickdraw.h>
#endif /* THINK_C */

#include "::src:config.h"
#include "::src:misc.h"
#include "::src:lisp.h"

#include "macimf.h"

#define pascalify(STR, BUF) \
  strcpy(((char *) BUF) + 1, STR);  \
  BUF[0] = strlen(STR);

extern int hasColorQD;

char *copy_string();

/* Clear the entire image family. */

init_image_family(imf)
ImageFamily *imf;
{
	int i, j;
	Image *img;

	imf->name = NULL;
	imf->ersatz = FALSE;
	imf->numsizes = 0;
	for (i = 0; i < MAXIMSIZES; ++i) {
		img = &(imf->images[i]);
		/* (should clear by just zeroing the block?) */
		img->w = img->h = 0;
		img->minw = img->minh = 0;
		img->maxw = img->maxh = 0;
		img->istile = 0;
		img->embedname = NULL;
		img->embedx = img->embedy = -1;
		img->embedw = img->embedh = 0;
		img->monosicn = img->masksicn = nil;
		img->monoicon = img->colricon = img->maskicon = nil;
		img->monopict = img->colrpict = img->maskpict = nil;
		img->groupmonopict = img->groupcolrpict = img->groupmaskpict = nil;
		img->groupmonox = img->groupmonoy = 0;
		img->groupcolrx = img->groupcolry = 0;
		img->groupmaskx = img->groupmasky = 0;
		img->patdefined = 0;
		for (j = 0; j < 8; ++j) img->pat[8] = '\0';
		img->colrpat = nil;
	}
}

Image *
get_img(imf, w, h)
ImageFamily *imf;
int w, h;
{
	int i;
	Image *img;
	
	for (i = 0; i < imf->numsizes; ++i) {
		img = &(imf->images[i]);
		if (w == img->w && h == img->h) return img;
	}
	if (imf->numsizes >= MAXIMSIZES) return NULL;
	img = &(imf->images[imf->numsizes]);
	++(imf->numsizes);
	img->w = w;  img->h = h;
	/* Default min/max limits to actual size. */
	img->minw = w;  img->minh = h;
	img->maxw = w;  img->maxh = h;
	return img;
}

/* This tries to fill in the given image family from various resources.  The order
   should be "best first", so as to improve search time. */

load_image_family(imf)
ImageFamily *imf;
{
    int w, h, i, rsize, startlineno, endlineno;
    char tmpstrbuf[100], embeddedname[100];
    Obj *imfspec;
    Handle handle, imfhandle, pathandle, sicnhandle, iconhandle, maskhandle;
    PicHandle pichandle;
    PixPatHandle ppathandle;
    CIconHandle cicnhandle;
    Image *img;
    Str255 namestr, maskstr, resname;
	short resid;
	ResType restype;

	/* Can't do anything without a name for the image family. */
	if (imf->name == NULL) return;
	pascalify(imf->name, namestr);
	/* The name of the mask is always formed by appending " mask". */
	sprintf(tmpstrbuf, "%s mask", imf->name);
	pascalify(tmpstrbuf, maskstr);
	if (imf->numsizes >= MAXIMSIZES) {
		/* (should maybe mention about discarding images, but unlikely to happen ever. */
		return;
	}
	embeddedname[0] = '\0';
	/* Look for and load the "image family" specification resource first. */
	if ((imfhandle = (Handle) GetNamedResource('XCif', namestr)) != nil) {
		imfspec = read_form_from_string(copy_string(*imfhandle), &startlineno, &endlineno);
		if (symbolp(imfspec)) {
			strcpy(embeddedname, c_string(imfspec));
		}
	}
	if ((pichandle = (PicHandle) GetNamedResource('PICT', namestr)) != nil) {
		if ((img = get_img(imf, 16, 16) /* should get real bounds */) != NULL) {
			img->minw = img->minh = 8;
			img->maxw = img->maxh = 9999;
			/* (should distinguish mono and color picts) */
			img->monopict = pichandle;
			/* Look for a mask too. */
			if ((pichandle = (PicHandle) GetNamedResource('PICT', maskstr)) != nil) {
				img->maskpict = pichandle;
			}
			img->istile = 0;
		}
	}
	/* (should also be able to pick up picts with rows and columns of images) */
	/* Pick up cicns, if we're doing color. */
	if (hasColorQD) {
		if ((cicnhandle = (CIconHandle) GetNamedResource('cicn', namestr)) != nil) {
			/* Need to get id so we can use special trap. */
			GetResInfo((Handle) cicnhandle, &resid, &restype, &resname);
			cicnhandle = GetCIcon(resid);
			if ((img = get_img(imf, 32, 32)) != NULL) {
				img->minw = img->minh = 8;
				img->maxw = img->maxh = 128;
				img->colricon = (Handle) cicnhandle;
				/* Mask is built in, don't need to load separately. */
			}
		}
	}
	/* Pick up ICONs. */
	if ((iconhandle = (Handle) GetNamedResource('ICON', namestr)) != nil) {
		if ((img = get_img(imf, 32, 32)) != NULL) {
			img->minw = img->minh = 8;
			img->maxw = img->maxh = 128;
			img->monoicon = iconhandle;
			/* Look for a mask too. */
			if ((iconhandle = (Handle) GetNamedResource('ICON', maskstr)) != nil) {
				img->maskicon = iconhandle;
			}
			img->istile = 0;
		}
	}
	/* Pick up SICNs. */
	if ((sicnhandle = (Handle) GetNamedResource('SICN', namestr)) != nil) {
		if ((img = get_img(imf, 16, 16)) != NULL) {
			img->minw = img->minh = 8;
			img->maxw = img->maxh = 64;
			/* Image itself is just the first 32 bytes, mask is second 32 if present. */
			img->monosicn = sicnhandle;
			rsize = SizeResource(sicnhandle);
			if (rsize >= 64) {
				maskhandle = (Handle) NewHandle(32);
				for (i = 0; i < 32; ++i) {
					(*maskhandle)[i] = (*sicnhandle)[i+32];
				}
				img->masksicn = maskhandle;
			} else {
				/* Mask could be separate resource, so look for it. */
				if ((iconhandle = GetNamedResource('SICN', maskstr)) != nil) {
					img->masksicn = sicnhandle;
				} else {
					/* no mask to be found */
				}
			}
			img->istile = 0;
		}
	}
	/* Pick up color patterns, if we're capable of doing color. */
	if (hasColorQD) {
		if ((ppathandle = (PixPatHandle) GetNamedResource('ppat', namestr)) != nil) {
			/* Need to get the id of the ppat so we can use special trap. */
			GetResInfo((Handle) ppathandle, &resid, &restype, &resname);
			ppathandle = GetPixPat(resid);
			if ((img = get_img(imf, 8, 8)) != NULL) {
				/* Indicate that we can use this pattern for any size area. */
				img->minw = img->minh = 1;
				img->maxw = img->maxh = 9999;
				img->colrpat = ppathandle;
				img->istile = 1;
			}
		}
	}
	/* Load a pattern, which can be used for any size area, but whose "natural" size
	   is always 8x8. */
	if ((pathandle = GetNamedResource('PAT ', namestr)) != nil) {
		if ((img = get_img(imf, 8, 8)) != NULL) {
			img->minw = img->minh = 1;
			img->maxw = img->maxh = 9999;
			for (i = 0; i < 8; ++i) {
				img->pat[i] = ((char *) *pathandle)[i];
			}
			img->patdefined = 1;
			img->istile = 1;
		}
	}
	if (strlen(embeddedname) > 0) {
		for (i = 0; i < imf->numsizes; ++i) {
			img = &(imf->images[i]);
			img->embedname = copy_string(embeddedname);
		}
	}
}

load_image_color(imc)
ImageColor *imc;
{
	Handle imchandle;
    Str255 namestr;

	if (imc->name == NULL) return;
	pascalify(imc->name, namestr);
	if ((imchandle = GetNamedResource('XCic', namestr)) != nil) {
		imc->r = ((short *) (*imchandle))[0];
		imc->g = ((short *) (*imchandle))[1];
		imc->b = ((short *) (*imchandle))[2];
	} else {
		/* Not found. */
		imc->r = imc->g = imc->b = 0;
	}
}

/* Try to find the best of multiple images for the given bounding box. */
/* Don't return anything that won't fit in min space. */

Image *
best_image(imf, w, h)
ImageFamily *imf;
int w, h;
{
	int i;
	Image *img, *best = NULL, *fallback = NULL;
	
	for (i = 0; i < imf->numsizes; ++i) {
		img = &(imf->images[i]);
		/* Exact matches need no further searching. */
		if (w == img->w && h == img->h && right_depth(img)) {
			return img;
		} else if (between(img->minw, w, img->maxw) && between(img->minh, h, img->maxh)) {
			/* Image is plausible - go for the largest one that will fit. */
			if (!best || (img->w > best->w && img->h > best->h)) {
				best = img;
			}
		} else if (w >= img->minw && h >= img->minh) {
			/* Image is not really big enough, but keep as a fallback. */
			if (!fallback || (img->w > fallback->w && img->h > fallback->h)) {
				fallback = img;
			}
		}
	}
	return (best ? best : fallback);
}

right_depth(img)
Image *img;
{
	return TRUE;
}
