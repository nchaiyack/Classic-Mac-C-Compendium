/* Copyright (c) 1992  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

#ifndef THINK_C
#include <Types.h>
#include <Quickdraw.h>
#endif

/* (should try to share basic stuff with X code) */

#define MAXIMSIZES 4

/* An image family is like a finder icon family, but allows multiple kinds of images -
   icon, pict, pattern, plus an arbitrary set of sizes. */

typedef struct a_image {
	/* These slots are all generic. */
	int w, h;
	int minw, minh;
	int maxw, maxh;
	int istile;
	char *embedname;
	int embedx, embedy;
	int embedw, embedh;
	/* The following slots are all Mac-specific. */
	Handle monosicn;
	Handle masksicn;
	Handle monoicon;
	Handle colricon;
	Handle maskicon;
	PicHandle monopict;
	PicHandle colrpict;
	PicHandle maskpict;
	PicHandle groupmonopict;
	PicHandle groupcolrpict;
	PicHandle groupmaskpict;
	int groupmonox, groupmonoy;
	int groupcolrx, groupcolry;
	int groupmaskx, groupmasky;
	int patdefined;
	char pat[8];
	PixPatHandle colrpat;
	/* Temp slots */
} Image;

typedef struct a_image_family {
	char *name;
	int ersatz;					/* true if this image was a made-up substitute */
	int numsizes;
	Image images[MAXIMSIZES];
} ImageFamily;

typedef struct a_image_color {
	char *name;
	short r, g, b;
} ImageColor;

Image *get_img();
Image *best_image();
