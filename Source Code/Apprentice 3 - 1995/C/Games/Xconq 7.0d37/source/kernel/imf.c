/* Interpretation of generic GDL images for Xconq.
   Copyright (C) 1994, 1995 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* Note!  This file does not use the standard "conq.h" header, so can't assume
   all the usual definitions. */
 
#include "config.h"
#include "misc.h"
#include "lisp.h"
#include "imf.h"
ImageFile *get_image_file PROTO ((char *name));

#define MAXIMAGEFAMILIES 1000

#define MAXIMAGECOLORS 1000

static void write_pixmap PROTO ((FILE *fp, int w, int h, int aw, int ah,
				 int pixelsize, int rowbytes,
				 Obj *palette, int *rawpalette, int numcolors,
				 Obj *lispdata, char *rawdata));
static void write_bitmap PROTO ((FILE *fp, char *subtyp, int w, int h,
				 Obj *data, char *rawdata));
static void write_palette_contents PROTO ((FILE *fp, Obj *palette));

/* This is the array of known image families. */

ImageFamily **images;

/* This is the count of known image families. */

int numimages = 0;

ImageColor **colors;

int numcolors = 0;

ImageFile *image_files;

/* Create and return an image family. */

ImageFamily *
new_imf(name)
char *name;
{
    ImageFamily *imf;

    imf = (ImageFamily *) xmalloc(sizeof(ImageFamily));
    imf->name = name;
    return imf;
}

ImageFamily *
clone_imf(imf)
ImageFamily *imf;
{
    Image *img, *img2, *truenext;
    ImageFamily *imf2;

    imf2 = new_imf(imf->name);
    memcpy(imf2, imf, sizeof(ImageFamily));
    imf2->images = NULL;
    /* Clone the images. */
    for (img = imf->images; img != NULL; img = img->next) {
	img2 = get_img(imf2, img->w, img->h);
	truenext = img2->next;
	memcpy(img2, img, sizeof(Image));
	/* Clear the hook, we expect that the caller of this routine
	   will supply any new hook that might be necessary. */
	img2->hook = NULL;
	/* Restore the link. */
	img2->next = truenext;
	/* Note that pointers to raw image data and suchlike can be
	   left as-is, since they should be shared by image clones. */
    }
    return imf2;
}

/* Test that the given name is a valid image family name (all alphanumeric,
   hyphens anywhere but as first char). */

int
valid_imf_name(name)
char *name;
{
    char *tmp;

    for (tmp = name; *tmp; ++tmp) {
	if (!(isalnum(*tmp)
	      || (tmp != name && *tmp == '-')))
	  return FALSE;
    }
    return TRUE;
}

/* Given a name, find or create an image family with that name. */

ImageFamily *
get_imf(name)
char *name;
{
    ImageFamily *imf = NULL;
    
    if (name == NULL) {
	init_warning("can't get an unnamed imf");
	return NULL;
    }
    if (!valid_imf_name(name)) {
	init_warning("\"%s\" is not a valid imf name", name);
	return NULL;
    }
    if (images == NULL) {
	images =
	  (ImageFamily **) xmalloc(MAXIMAGEFAMILIES * sizeof(ImageFamily *));
    }
    imf = find_imf(name);
    if (imf == NULL) {
	if (numimages >= MAXIMAGEFAMILIES) {
	    return NULL;
	}
	imf = new_imf(name);
	if (imf != NULL) {
	    images[numimages++] = imf;
	}
    }
    return imf;
}

ImageFile *
get_image_file(name)
char *name;
{
    ImageFile *imfile;
    
    if (name == NULL)
      run_error("can't get an unnamed imfile");
    for (imfile = image_files; imfile != NULL; imfile = imfile->next) {
	if (strcmp(name, imfile->name) == 0)
	  return imfile;
    }
    imfile = (ImageFile *) xmalloc(sizeof(ImageFile));
    imfile->name = copy_string(name);
    imfile->next = image_files;
    image_files = imfile;
    return imfile;
}

void
load_image_families(fp, loadnow, callback)
FILE *fp;
int loadnow;
void (*callback) PROTO ((ImageFamily *imf, int loadnow));
{
    int done = FALSE, rslt, first = TRUE;
    char buf1[80], buf2[80];
    ImageFamily *imf;
    ImageFile *imfile;
    
    while (!done) {
	rslt = fscanf(fp, "%s %s\n", buf1, buf2);
	if (rslt != 2)
	  break;
	else if (strcmp(buf1, ".") == 0
		 && strcmp(buf2, ".") == 0)
	  done = TRUE;
	else if (first) {
	    if (strcmp(buf1, "ImageFamilyName") == 0
		&& strcmp(buf2, "FileName") == 0)
	      first = FALSE;
	    else {
		init_warning("File not a valid imf dir, will close and ignore");
		/* We've already given a warning message, so pretend we're done
		   so the format error message doesn't get displayed below. */
		done = TRUE;
		break;
	    }
	} else {
	    imf = get_imf(copy_string(buf1));
	    if (imf != NULL) {
		imfile = get_image_file(buf2);
		imf->location = imfile;
		if (loadnow && !imfile->loaded) {
		    load_imf_file(imfile->name, callback);
		    imfile->loaded = TRUE;
		} else {
		    if (callback != NULL)
		      (*callback)(imf, loadnow);
		}
	    }
	}
    }
    if (!done) {
	init_warning("Format error in imf dir near %s, will only use part",
		     (imf ? imf->name : "???"));
    }
}

/* Given a filename, open it and read/interpret all the image-related
   forms therein. */

int
load_imf_file(filename, callback)
char *filename;
void (*callback) PROTO ((ImageFamily *imf, int loadnow));
{
    int startlineno = 1, endlineno = 1;
    Obj *form;
    FILE *fp;

    fp = fopen(filename, "r");
    if (fp != NULL) {
	/* Read everything in the file. */
	while ((form = read_form(fp, &startlineno, &endlineno)) != lispeof) {
	    interp_imf_form(form, callback);
	}
	fclose(fp);
	return TRUE;
    }
    return FALSE;
}

/* Interpret a form, looking specifically for image-related forms. */

void
interp_imf_form(form, imf_callback)
Obj *form;
void (*imf_callback) PROTO ((ImageFamily *imf, int loadnow));
{
    Obj *head;
    ImageFamily *imf;
    ImageColor *imc;

    head = car(form);
    if (match_keyword(head, K_IMF)) {
	imf = interp_imf(form);
	if (imf_callback != NULL && imf != NULL)
	  (*imf_callback)(imf, TRUE);
    } else if (match_keyword(head, K_PALETTE)) {
	/* (should eventually be able to interpret shared palettes) */
    } else if (match_keyword(head, K_COLOR)) {
	interp_color(form);
    } else {
	/* Ignore any non-image forms, we might be reading from a 
	   normal game design. */
    }
}

/* Find the image family of the given name, if it exists. */

ImageFamily *
find_imf(name)
char *name;
{
    int i;

    for (i = 0; i < numimages; ++i) {
	if (strcmp(name, images[i]->name) == 0)
	  return images[i];
    }
    return NULL;
}

/* Get an image of the given size from the family, creating a new one
   if necessary. */

Image *
get_img(imf, w, h)
ImageFamily *imf;
int w, h;
{
    Image *img, *previmg = NULL;

    for (img = imf->images; img != NULL; img = img->next) {
	if (w == img->w && h == img->h)
	  return img;
	previmg = img;
    }
    /* Not found; create a new image and add it to the family. */
    img = (Image *) xmalloc(sizeof(Image));
    img->w = w;  img->h = h;
    /* Default min/max limits to actual size. */
    img->minw = img->maxw = w;  img->minh = img->maxh = h;
    img->embedx = img->embedy = -1;
    img->monodata = img->colrdata = img->maskdata = lispnil;
    img->palette = lispnil;
    /* Rely on zeroing of xmalloc blocks to avoid clearing other fields. */
    /* Link at front of list of images. */
    if (previmg != NULL)
      previmg->next = img;
    else
      imf->images = img;
    ++(imf->numsizes);
    return img;
}

Image *
find_img(imf, w, h)
ImageFamily *imf;
int w, h;
{
    Image *img;
	
    for (img = imf->images; img != NULL; img = img->next) {
	if (w == img->w && h == img->h)
	  return img;
    }
    return NULL;
}

ImageFamily *
interp_imf(form)
Obj *form;
{
    ImageFamily *imf;

    if (stringp(cadr(form))) {
	imf = get_imf(c_string(cadr(form)));
	if (imf != NULL) {
	    interp_imf_contents(imf, cddr(form));
	}
	return imf;
    } else {
	/* garbage form */
    }
    return NULL;
}

void
interp_imf_contents(imf, clauses)
ImageFamily *imf;
Obj *clauses;
{
    Obj *rest, *clause;

    for (rest = clauses; rest != lispnil; rest = cdr(rest)) {
	clause = car(rest);
	if (consp(clause)) {
	    interp_image(imf, car(clause), cdr(clause));
	} else {
	    /* garbage? */
	}
    }
}

void
interp_image(imf, size, parts)
ImageFamily *imf;
Obj *size, *parts;
{
    int w, h, imtype;
    Image *img;
    Obj *head, *rest, *typ, *prop, *proptype, *datalist;
    
    w = c_number(car(size));  h = c_number(cadr(size));
    img = get_img(imf, w, h);
    if (img == NULL)
      run_error("no image?");
    if (match_keyword(car(cddr(size)), K_TILE))
      img->istile = TRUE;
    img->actualw = w;  img->actualh = h;
    img->pixelsize = img->rowbytes = 0;
    img->palette = lispnil;
    for (rest = parts; rest != lispnil; rest = cdr(rest)) {
	head = car(rest);
	typ = car(head);
	imtype = K_OTHER_;
	if (match_keyword(typ, K_MONO)) {
	    imtype = K_MONO_;
	} else if (match_keyword(typ, K_MASK)) {
	    imtype = K_MASK_;
	} else if (match_keyword(typ, K_COLOR)) {
	    imtype = K_COLR_;
	} else if (match_keyword(typ, K_EMBED)) {
	    img->embedname = c_string(cadr(head));
	} else if (match_keyword(typ, K_EMBED_AT)) {
	    img->embedx = c_number(cadr(head));
	    img->embedy = c_number(caddr(head));
	} else {
	    run_warning("unknown image property");
	}
	if (imtype == K_OTHER_)
	  continue;
	datalist = cdr(head);
	/* Interpret random image subproperties. */
	while (consp(car(datalist))) {
	    prop = car(datalist);
	    proptype = car(prop);
	    if (match_keyword(proptype, K_ACTUAL)) {
		img->actualw = c_number(cadr(prop));
		img->actualh = c_number(caddr(prop));
	    } else if (match_keyword(proptype, K_PIXEL_SIZE)) {
		img->pixelsize = c_number(cadr(prop));
	    } else if (match_keyword(proptype, K_ROW_BYTES)) {
		img->rowbytes = c_number(cadr(prop));
	    } else if (match_keyword(proptype, K_PALETTE)) {
		img->palette = cdr(prop);
	    } else {
		/* (should warn?) */
	    }
	    datalist = cdr(datalist);
	}
	switch (imtype) {
	  case K_MONO_:
	    img->monodata = datalist;
	    break;
	  case K_COLR_:
	    img->colrdata = datalist;
	    break;
	  case K_MASK_:
	    img->maskdata = datalist;
	    break;
	  default:
	    break;
	}
    }
    /* Kind of a hack. */
    img->minw = img->w / 4;  img->minh = img->h / 4;
    img->maxw = img->w * 4;  img->maxh = img->h * 4;
}

void
interp_bytes(datalist, numbytes, destaddr, jump)
Obj *datalist;
int numbytes, jump;
char *destaddr;
{
    int i, j = 0;
    char *data = NULL;

    for (i = 0; i < numbytes; ++i) {
	if (data == NULL || data[j] == '\0') {
	    if (!stringp(car(datalist)))
	      return; /* (should warn somehow?) */
	    data = c_string(car(datalist));
	    j = 0;
	    datalist = cdr(datalist);
	}
	/* Just skip over slashes, which are for appearance only. */
	if (data[j] == '/')
	  ++j;
	destaddr[i] = hextoi(data[j]) * 16 + hextoi(data[j+1]);
	if (jump == 1 || (jump > 0 && i % jump == 0)) {
	    i += jump;
	    /* Be neat, put a zero in the location we're jumping over. */
	    /* (doesn't work for jump > 1, but that never happens anymore?) */
	    destaddr[i] = 0;
	}
	j += 2;
    }
}

void
interp_palette(form)
Obj *form;
{
}

ImageColor *
interp_color(form)
Obj *form;
{
    Obj *elts;
    ImageColor *imc;

    if (stringp(cadr(form))) {
	imc = get_imc(c_string(cadr(form)));
	elts = cddr(form);
	imc->r = c_number(car(elts));
	imc->g = c_number(car(cdr(elts)));
	imc->b = c_number(car(cddr(elts)));
	return imc;
    }
    return NULL;
}

ImageColor *
new_image_color(name)
char *name;
{
	ImageColor *imc;

	imc = (ImageColor *) xmalloc(sizeof(ImageColor));
	imc->name = name;
	return imc;
}

char *canonical_color_name PROTO ((char *str));

char *
canonical_color_name(str)
char *str;
{
    return str;
}

/* Given a name, find or create an image color with that name. */

ImageColor *
get_imc(name)
char *name;
{
    ImageColor *imc = NULL;

    if (name == NULL)
      return NULL;
    if (colors == NULL)
      colors =
	(ImageColor **) xmalloc(MAXIMAGECOLORS * sizeof(ImageColor *));
    if ((imc = find_imc(name)) == NULL) {
	if (numcolors >= MAXIMAGECOLORS)
	  return NULL;
	imc = new_image_color(canonical_color_name(name));
	if (imc != NULL) {
	    imc->r = imc->g = imc->b = 0;
	    colors[numcolors++] = imc;
	}
    }
    return imc;
}

static int colornamecmp PROTO ((char *str1, char *str2));

/* Find the image color of the given name, if it exists. */

ImageColor *
find_imc(name)
char *name;
{
    int i;

    for (i = 0; i < numcolors; ++i) {
	if (colornamecmp(name, colors[i]->name) == 0)
	  return colors[i];
    }
    return NULL;
}

/* X-style color names have several variants, but we only want one of them,
   so this matches the variants with each other. */

static int
colornamecmp(str1, str2)
char *str1, *str2;
{
    while (*str1 != '\0' && *str2 != '\0') {
	if (*str1 == *str2) {
	    ++str1;  ++str2;
	} else if (isalpha(*str1) && isalpha(*str2)
		   && tolower(*str1) == tolower(*str2)) {
	    ++str1;  ++str2;
	} else if (*str1 == 'a' && *str2 == 'e'
		   && *(str1+1) == 'y' && *(str2+1) == 'y') {
	    ++str1;  ++str2;
	} else if (*str1 == 'e' && *str2 == 'a'
		   && *(str1+1) == 'y' && *(str2+1) == 'y') {
	    ++str1;  ++str2;
#if 0
	} else if (*str1 == ' ') {
	    ++str1;
	} else if (*str2 == ' ') {
	    ++str2;
#endif
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
	    /* can never happen, but humor the compiler */
	    return 0;
	}
    }
}


/* Try to find the best of multiple images for the given bounding box. */
/* Don't return anything that won't fit in min space. */

Image *
best_image(imf, w, h)
ImageFamily *imf;
int w, h;
{
    Image *img, *best = NULL, *fallback = NULL;

    if (imf == NULL)
      return NULL;
    for (img = imf->images; img != NULL; img = img->next) {
	/* Exact matches need no further searching. */
	if (w == img->w && h == img->h && right_depth(img)) {
	    return img;
	} else if (between(img->minw, w, img->maxw)
		   && between(img->minh, h, img->maxh)) {
	    /* Image is plausible - go for the largest one that will fit. */
	    if (!best || (img->w > best->w && img->h > best->h))
	      best = img;
	} else if (w >= img->minw && h >= img->minh) {
	    /* Image is not really big enough, but keep as a fallback. */
	    /* Prefer the largest fallback possible. */
	    if (!fallback || (img->w > fallback->w && img->h > fallback->h))
	      fallback = img;
	} else if (img->istile) {
	    /* Image is not really appropriate, but keep as a fallback. */
	    /* Prefer the largest fallback possible. */
	    if (!fallback || (img->w > fallback->w && img->h > fallback->h))
	      fallback = img;
	}
    }
    return (best ? best : fallback);
}

int
right_depth(img)
Image *img;
{
    return TRUE;
}

/* The comparison function for the image list just does "strcmp" order
   and *requires* that all image families be named and named uniquely. */

static int
image_name_compare(im1, im2)
#ifdef THINK_C
const
#endif
void *im1, *im2;
{
    return strcmp((*((ImageFamily **) im1))->name,
		  (*((ImageFamily **) im2))->name);
}

void
sort_all_images()
{
    qsort(&(images[0]), numimages, sizeof(ImageFamily *), image_name_compare);
}

static int
color_name_compare(im1, im2)
#ifdef THINK_C
const
#endif
void *im1, *im2;
{
    return strcmp((*((ImageColor **) im1))->name,
		  (*((ImageColor **) im2))->name);
}

void
sort_all_colors()
{
    qsort(&(colors[0]), numcolors, sizeof(ImageColor *),  color_name_compare);
}

/* Write out the entire image family. */

void
write_imf(fp, imf)
FILE *fp;
ImageFamily *imf;
{
    int w, h;
    Image *img;
    
    if (imf == NULL)
      return;
    if (imf->name == NULL) {
	fprintf(fp, "; garbage image family?\n");
	return;
    }
    for (img = imf->images; img != NULL; img = img->next) {
	if (img->monodata != lispnil
	    || img->maskdata != lispnil
	    || img->colrdata != lispnil
	    || img->rawmonodata != NULL
	    || img->rawmaskdata != NULL
	    || img->rawcolrdata != NULL) {
	    fprintf(fp, "(imf \"%s\"", imf->name);
	    fprintf(fp, " (");
	    fprintf(fp, "(%d %d", img->w, img->h);
	    if (img->istile) fprintf(fp, " tile");
	    fprintf(fp, ")");
	    if (img->embedname) {
		fprintf(fp, " (embed \"%s\")", img->embedname);
	    }
	    /* (should write embedrect if defined, etc) */
	    if (img->colrdata != lispnil || img->rawcolrdata) {
		fprintf(fp, "\n  ");
		write_pixmap(fp, img->w, img->h, img->actualw, img->actualh,
			     img->pixelsize, img->rowbytes,
			     img->palette, img->rawpalette, img->numcolors,
			     img->colrdata, img->rawcolrdata);
	    }
	    if (img->monodata != lispnil || img->rawmonodata) {
		fprintf(fp, "\n  ");
		write_bitmap(fp, "mono", img->w, img->h,
			     img->monodata, img->rawmonodata);
	    }
	    if (img->maskdata != lispnil || img->rawmaskdata) {
		fprintf(fp, "\n  ");
		write_bitmap(fp, "mask", img->w, img->h,
			     img->maskdata, img->rawmaskdata);
	    }
	    fprintf(fp, "))\n");
	}
    }
}

static void
write_pixmap(fp, w, h, actualw, actualh, pixelsize, rowbytes,
	     palette, rawpalette, numcolors, lispdata, rawdata)
FILE *fp;
int w, h, actualw, actualh, pixelsize, rowbytes, numcolors;
Obj *palette, *lispdata;
int *rawpalette;
char *rawdata;
{
    int dolisp, i, j = 0, numbytes, byte, jump = 0;
    char *datastr = NULL;
    
    actualw = (actualw != 0 ? actualw : w);
    actualh = (actualh != 0 ? actualh : h);
    dolisp = (lispdata != lispnil);	
    numbytes = actualh * rowbytes;
    fprintf(fp, "(color");
    if (actualw != w || actualh != h)
      fprintf(fp, " (actual %d %d)", actualw, actualh);
    fprintf(fp, " (pixel-size %d)", pixelsize);
    fprintf(fp, " (row-bytes %d)", rowbytes);
    if (palette != lispnil) {
	write_palette_contents(fp, palette);
    } else if (rawpalette && numcolors) {
 	fprintf(fp, "    (palette");
 	for (i = 0; i < numcolors; i++) {
 	    fprintf(fp, "\n      (%d %d %d %d)",
 		    rawpalette[4*i],   rawpalette[4*i+1],
 		    rawpalette[4*i+2], rawpalette[4*i+3]);
 	}
 	fprintf(fp, ")");
    }
    fprintf(fp, "\n   \"");
    for (i = 0; i < numbytes; ++i) {
	if (i > 0 && i % 32 == 0)
	  fprintf(fp, "\"\n   \"");
	if (i > 0 && i % 32 != 0 && i % rowbytes == 0)
	  fprintf(fp, "/");
	if (dolisp) {
	    if (datastr == NULL || datastr[j] == '\0') {
		if (!stringp(car(lispdata)))
		  break;
		datastr = c_string(car(lispdata));
		j = 0;
		lispdata = cdr(lispdata);
	    }
	    if (datastr[j] == '/')
	      ++j;
	    byte = hextoi(datastr[j]) * 16 + hextoi(datastr[j+1]);
	    if (jump == 1 || (jump > 0 && i % jump == 0))
	      i += jump;
	    j += 2;
	} else {
	    byte = rawdata[i];
	}
	fprintf(fp, "%02x", (unsigned char) byte);
    }
    fprintf(fp, "\")");
}

static void
write_bitmap(fp, subtyp, w, h, lispdata, rawdata)
FILE *fp;
char *subtyp;
int w, h;
Obj *lispdata;
char *rawdata;
{
    int dolisp, i, j = 0, rowbytes, numbytes, byte, jump = 0;
    char *datastr = NULL;
	
    dolisp = (lispdata != lispnil);	
    rowbytes = (w + 7) / 8;
    numbytes =  h * rowbytes;
    fprintf(fp, "(%s", subtyp);
    if (w > 16 || h > 16)
      fprintf(fp, "\n  ");
    fprintf(fp, " \"");
    for (i = 0; i < numbytes; ++i) {
	if (i > 0 && i % 32 == 0)
	  fprintf(fp, "\"\n   \"");
	if (i > 0 && i % 32 != 0 && i % rowbytes == 0)
	  fprintf(fp, "/");
	if (dolisp) {
	    if (datastr == NULL || datastr[j] == '\0') {
		if (!stringp(car(lispdata)))
		  break;
		datastr = c_string(car(lispdata));
		j = 0;
		lispdata = cdr(lispdata);
	    }
	    if (datastr[j] == '/')
	      ++j;
	    byte = hextoi(datastr[j]) * 16 + hextoi(datastr[j+1]);
	    if (jump == 1 || (jump > 0 && i % jump == 0))
	      i += jump;
	    j += 2;
	} else {
	    byte = rawdata[i];
	}
	fprintf(fp, "%02x", (unsigned char) byte);
    }
    fprintf(fp, "\")");
}

static void
write_palette_contents(fp, palette)
FILE *fp;
Obj *palette;
{
    int len;
    Obj *color;

    if (palette != lispnil) {
    	len = length(palette);
	if (len > 2)
	  fprintf(fp, "\n  ");
	fprintf(fp, " (palette");
	for (; palette != lispnil; palette = cdr(palette)) {
	    color = car(palette);
	    if (len > 2)
	      fprintf(fp, "\n   ");
	    fprintf(fp, " (%d %d %d %d)",
		    c_number(car(color)),
		    c_number(car(cdr(color))),
		    c_number(car(cdr(cdr(color)))),
		    c_number(car(cdr(cdr(cdr(color))))));
	}
	fprintf(fp, ")");
    }
}

void
write_imc(fp, imc)
FILE *fp;
ImageColor *imc;
{
    if (imc == NULL)
      return;
    if (imc->name == NULL) {
	fprintf(fp, "; garbage color?\n");
	return;
    }
    fprintf(fp, "(color \"%s\" %d %d %d)\n",
	    imc->name, imc->r, imc->g, imc->b);
}
