/* Definitions for Xconq images.
   Copyright (C) 1992, 1993, 1994 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* An image family is like a finder icon family, but allows multiple kinds
   of images of an arbitrary set of sizes. */

typedef struct a_image {
    int w, h;
    int minw, minh;
    int maxw, maxh;
    int istile;
    char *embedname;
    int embedx, embedy;
    int embedw, embedh;
    Obj *monodata;
    Obj *colrdata;
    Obj *maskdata;
    int actualw, actualh;
    int pixelsize;
    int rowbytes;
    Obj *palette;
    char *rawmonodata;
    char *rawcolrdata;
    char *rawmaskdata;
    int *rawpalette;
    int numcolors;
    char *hook;
    struct a_image *next;
} Image;

typedef struct a_image_family {
    char *name;			/* Name of the family */
    int ersatz;			/* True if this image is a substitute */
    struct a_image_file *location;  /* File or whatever to look for data */
    int numsizes;		/* Number of images in the list */
    Image *images;
} ImageFamily;

/* (should add a palette structure) */

typedef struct a_image_color {
    char *name;			/* Name of the color */
    struct a_image_file *location;  /* File or whatever to look for data */
    int defined;
    short r, g, b;
} ImageColor;

typedef struct a_image_file {
    char *name;
    int loaded;
    struct a_image_file *next;
} ImageFile;

enum {
    K_MONO_,
    K_MASK_,
    K_COLR_,
    K_OTHER_
};

extern ImageFamily **images;

extern int numimages;

extern ImageColor **colors;

extern int numcolors;

extern ImageFile *image_files;

#define hextoi(c) (((c) >= '0' && (c) <= '9') ? ((c) - '0') : ((c) - 'a' + 10))

ImageFamily *new_imf PROTO ((char *name));
ImageFamily *clone_imf PROTO ((ImageFamily *imf));
ImageFamily *get_imf PROTO ((char *name));
ImageFamily *find_imf PROTO ((char *name));
Image *find_img PROTO ((ImageFamily *imf, int w, int h));
Image *get_img PROTO ((ImageFamily *imf, int w, int h));
int valid_imf_name PROTO ((char *name));

ImageColor *new_image_color PROTO ((char *name));
ImageColor *get_imc PROTO ((char *name));
ImageColor *find_imc PROTO ((char *name));

void load_image_families PROTO ((FILE *fp, int loadnow,
				void (*callback)(ImageFamily *, int)));
int load_imf_file PROTO ((char *filename,
			   void (*callback)(ImageFamily *, int)));
void interp_imf_form PROTO ((Obj *form,
			     void (*callback)(ImageFamily *, int)));

ImageFamily *interp_imf PROTO ((Obj *form));
void interp_imf_contents PROTO ((ImageFamily *imf, Obj *form));
void interp_image PROTO ((ImageFamily *imf, Obj *size, Obj *parts));
void interp_bytes PROTO ((Obj *datalist, int numbytes, char *destaddr,
			  int jump));
void interp_palette PROTO ((Obj *form));
ImageColor *interp_color PROTO ((Obj *form));

Image *best_image PROTO ((ImageFamily *imf, int w, int h));
int right_depth PROTO ((Image *img));

void sort_all_images PROTO ((void));
void sort_all_colors PROTO ((void));

void write_imf PROTO ((FILE *fp, ImageFamily *imf));
void write_imc PROTO ((FILE *fp, ImageColor *imc));
