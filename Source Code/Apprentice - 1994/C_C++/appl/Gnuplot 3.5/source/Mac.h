/* GNUPLOT -- MacGraphics.trm */
/*
 * Copyright (C) 1990   
 *
 * Permission to use, copy, and distribute this software and its
 * documentation for any purpose with or without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and 
 * that both that copyright notice and this permission notice appear 
 * in supporting documentation.
 *
 * Permission to modify the software is granted, but not the right to
 * distribute the modified code.  Modifications are to be distributed 
 * as patches to released version.
 *  
 * This software  is provided "as is" without express or implied warranty.
 * 
 * This file is included by ../term.c.
 *
 * This terminal driver supports:
 *  Unix plot(5) graphics language
 *
 * AUTHORS
 *  Colin Kelley, Thomas Williams, Russell Lang
 * 
 * send your comments or suggestions to (pixar!info-gnuplot@sun.com).
 * 
 */

/*
Unixplot library writes to stdout.  A fix was put in place by
..!arizona!naucse!jdc to let set term and set output redirect
stdout.  All other terminals write to outfile.
*/

/* console window font width :6, font height 11
 window size  283 x 488 */

#define MAC_XMAX 448
#define MAC_YMAX 271
#define MAC_XOFFSET (468-MAC_XMAX)

#define MAC_XLAST (MAC_XMAX - 1)
#define MAC_YLAST (MAC_YMAX - 1)

#define MAC_VCHAR 11	 /* monaco 9 point plain style */
#define MAC_HCHAR 6	 
#define MAC_VTIC (MAC_YMAX/80)
#define MAC_HTIC (MAC_XMAX/80)
#define MAC_WINDOW_TITLE "\pMac Graphic Window"

// prototype of exported functions
/* Not needed, I think P. Laval
extern MAC_init();
extern MAC_graphics();
extern MAC_text();
extern MAC_linetype(int linetype);
extern MAC_move(unsigned int x,unsigned int y);
extern MAC_vector(unsigned int x,unsigned int y);
extern MAC_put_text(unsigned int x,unsigned int y,char *str);
extern MAC_reset();
*/
//
