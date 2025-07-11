//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the optimized assembly-language versions of the low-level
//| graphics routines used by HyperCuber.  See HyperCuber Graphics.c for the c
//| versions of these algorithms
//|________________________________________________________________________________


#include <asm.h>
#include <FixMath.h>

#if 0

Fract intensity_table[16] = {	0x40000000, 0x3EF8D2E5, 0x3BEFE0CE, 0x370C77DE,
								0x30965F52, 0x29086BDF, 0x214BD33D, 0x1A03A323,
								0x1371971C, 0x0DBFD2E9, 0x09086BDF, 0x055831F0,
								0x02AF5C0E, 0x0100BE01, 0x002EA30B, 0x00000000   };

#endif
				//  Gamma-corrected table
Fract intensity_table[16] = {	0x40000000, 0x3F7BE113, 0x3DEF698D, 0x3B5B190E,
								0x37C38384, 0x333EDBB1, 0x2E298B52, 0x28CDA516,
								0x2346A93B, 0x1DA9FF4C, 0x180B37DF, 0x127E9B84,
								0x0D1BD877, 0x0802F777, 0x036A209F, 0x00000000   };


void DrawAntialiasedLineO1asm(long x1, long y1, long x2, long y2, RGBColor *color);
void DrawAntialiasedLineO1asm(long x1, long y1, long x2, long y2, RGBColor *color)
{

	long 			dx, dy;
	register long	d;
	register long	line_width;
	register long	two_dx_invDenom;
	long			invDenom;
	register long	count;
	long 			incrE, incrNE, two_v_dx;
	RGBColor 		old_color;
	RGBColor 		new_color;
	PixMapHandle 	pixmap;
	register unsigned char	*pix;

	unsigned char	color_table[16];
	RGBColor		table_color;
	long			i, debug;
	Fract			fract_intensity;
	Fixed			intensity;
	
	pixmap = ((CGrafPtr) thePort)->portPixMap;		//  Find address of starting pixel
	line_width = (*pixmap)->rowBytes & 0x7FFF;
	pix = (unsigned char *) (*pixmap)->baseAddr -
							(*pixmap)->bounds.top*line_width -
							(*pixmap)->bounds.left +
							line_width*y1 + x1;

asm {

//==== create table of frequently used colors (used when drawing on black)

	move.l	d3, -(SP)					; save d3
	
	move.l	#15, count

color_loop:

	lea		table_color, a1
	
	lea		intensity_table, a0			; look up the intensity in the table
	move.l	count, d0
	move.l	d0, debug
	move.l	0(a0, d0.w*4), d3
	move.l	d3, debug

	move.l	#15, d1						; intensity in d3
	asr.l	d1, d3

	move.l	d3, debug

	move.l	color, a0					; multiply red by intensity
	moveq	#0, d0						
	move.w	0(a0), d0
	muls.l	d3, d0
	moveq	#15, d2
	asr.l	d2, d0
	move.w	d0, 0(a1)

	moveq	#0, d0						; multiply green by intensity
	move.w	2(a0), d0
	muls.l	d3, d0
	moveq	#15, d2
	asr.l	d2, d0
	move.w	d0, 2(a1)

	moveq	#0, d0						; multiply blue by intensity
	move.w	4(a0), d0
	muls.l	d3, d0
	moveq	#15, d2
	asr.l	d2, d0
	move.w	d0, 4(a1)

	move.l	a1, -(SP)					; find index for this color
	_Color2Index
	
	lea		color_table, a0				; save this index in the table
	move.b	d0, 0(a0, count)
	
	subq	#1, count
	bpl		@color_loop
	
	move.l	(SP)+, d3					; restore d3

//==== set up to draw line	
	
	move.l	x2, dx				; dx = x2 - x1
	move.l	x1, d0
	sub.l	d0, dx
	
	move.l	y2, dy				; dy = y2 - y1
	move.l	y1, d0
	sub.l	d0, dy
	
	move.l	dy, d				; d = 2*dy - dx
	add.l	d, d
	sub.l	dx, d
	
	move.l	dy, d0				; incrE = 2*dy
	add.l	d0, d0
	move.l	d0, incrE
	
	move.l	dy, d0				; incrNE = 2*(dy-dx)
	sub.l	dx, d0
	add.l	d0, d0
	move.l	d0, incrNE

	move.l	#0, two_v_dx;		; two_v_dx = 0
	
	; the next section computes 1/(2*sqrt(dx*dx + dy*dy) using integer arithmetic
	
	move.l	dx, d0				; compute dx*dx + dy*dy
	muls.l	d0, d0
	move.l	dy, d1
	muls.l	d1, d1
	add.l	d1, d0
	
	move.l	#12, d1
	asl.l	d1, d0				; take the square root
	move.l	d0, -(SP)
	_FracSqrt

	move.l	(SP), d0			; find the denominator
	asr.l	#4, d0
	
	move.l	#0x10000, -(SP)		; do the division
	move.l	d0, -(SP)
	_FracDiv

	move.l	(SP), invDenom
	
	move.l	invDenom, two_dx_invDenom	; two_dx_invDenom = 2*dx*invDenom
	muls.l	dx, two_dx_invDenom
	add.l	two_dx_invDenom, two_dx_invDenom

	move.l	dx, count
	
	bra		@move_done
	
xloop:

	sub.l	#1, count			; loop while count > 0
	bmi		@done

	tst.l	d
	bge		@move_northeast

move_east:

	move.l	d, d0				; two_v_dx = d + dx
	add.l	dx, d0
	move.l	d0, two_v_dx
	
	add.l	incrE, d			; d = d + incrE
		
	addq	#1, pix				; x = x + 1
	
	bra		@move_done
	
move_northeast:

	move.l	d, d0				; two_v_dx = d - dx
	sub.l	dx, d0
	move.l	d0, two_v_dx
	
	add.l	incrNE, d			; d = d + incrE
	
	add.l	line_width, pix		; y = y + 1
	addq	#1, pix				; x = x + 1

move_done:


//========== Draw first pixel
	
	moveq	#0, d0						; push pixel value
	move.b	(pix), d0
	move.l	d0, -(SP)
	pea		old_color					; push address of old_color
	_Index2Color						; get previous rgb pixel value

	move.l	two_v_dx, d0				; distance = two_v_dx * invDenom
	muls.l	invDenom, d0

	bpl		@positive1					; get absolute value of distance

	neg.l	d0

positive1:

	moveq.l	#14, d1						; index = (((abs_distance / 0x4000) * 10) + 0x8000) >> 16
	asr.l	d1, d0
	muls.l	#10, d0
	add.l	#0x8000, d0
	moveq.l	#16, d1
	asr.l	d1, d0
	
	lea		old_color, a0				; check if the old color was 0, 0, 0
	tst.l	(a0)
	bne		@notzero1
	tst.w	4(a0)
	bne		@notzero1
	
	lea		color_table, a0				; if all 0's, we can get the pixel value from the table
	move.b	0(a0, d0), d0
	bra		@drawpixel1

@notzero1
	
	lea.l	intensity_table, a0			; look up the intensity in the table
	move.l	0(a0, d0*4), d0

	move.l	#15, d1
	asr.l	d1, d0
	move.l	d0, intensity

	move.l	color, a0
	lea		new_color, a1
	lea		old_color, a2
	moveq	#0, d1						; Find intensities of color components
	move.w	0(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	0(a2), d0
	bcc		@red_ok1
	move.w	#0xFFFF, d0
red_ok1:
	move.w	d0, 0(a1)
	
	move.l	intensity, d0
	move.w	2(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	2(a2), d0
	bcc		@green_ok1
	move.w	#0xFFFF, d0
green_ok1:
	move.w	d0, 2(a1)
	
	move.l	intensity, d0
	moveq	#0, d1
	move.w	4(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	4(a2), d0
	bcc		@blue_ok1
	move.w	#0xFFFF, d0
blue_ok1:
	move.w	d0, 4(a1)

	move.l	a1, -(SP)			; find index for new_color
	_Color2Index
	
drawpixel1:

	move.b	d0, (pix)


//======== Draw the second pixel

	add.l	line_width, pix				; pix += line_width;

	moveq	#0, d0						; push pixel value
	move.b	(pix), d0
	move.l	d0, -(SP)
	pea		old_color					; push address of old_color
	_Index2Color						; get previous rgb pixel value

	move.l	two_v_dx, d1				; distance = two_dx_invDenom - two_v_dx * invDenom
	muls.l	invDenom, d1
	move.l	two_dx_invDenom, d0
	sub.l	d1, d0

	bpl		@positive2					; get absolute value of distance

	neg.l	d0

positive2:

	moveq.l	#14, d1						; index = (((abs_distance / 0x4000) * 10) + 0x8000) >> 16
	asr.l	d1, d0
	muls.l	#10, d0
	add.l	#0x8000, d0
	moveq.l	#16, d1
	asr.l	d1, d0
	
	lea		old_color, a0				; check if the old color was 0, 0, 0
	tst.l	(a0)
	bne		@notzero2
	tst.w	4(a0)
	bne		@notzero2
	
	lea		color_table, a0				; if all 0's, we can get the pixel value from the table
	move.b	0(a0, d0.w*1), d0
	bra		@drawpixel2

notzero2:
	lea.l	intensity_table, a0			; look up the intensity in the table
	move.l	0(a0, d0.w*4), d0

	move.l	#15, d1
	asr.l	d1, d0
	move.l	d0, intensity

	move.l	color, a0
	lea		new_color, a1
	lea		old_color, a2
	moveq	#0, d1						; Find intensities of color components
	move.w	0(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	0(a2), d0
	bcc		@red_ok2
	move.w	#0xFFFF, d0
red_ok2:
	move.w	d0, 0(a1)
	
	move.l	intensity, d0
	move.w	2(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	2(a2), d0
	bcc		@green_ok2
	move.w	#0xFFFF, d0
green_ok2:
	move.w	d0, 2(a1)
	
	move.l	intensity, d0
	moveq	#0, d1
	move.w	4(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	4(a2), d0
	bcc		@blue_ok2
	move.w	#0xFFFF, d0
blue_ok2:
	move.w	d0, 4(a1)

	move.l	a1, -(SP)			; find index for new_color
	_Color2Index
	
drawpixel2:
	move.b	d0, (pix)

	sub.l	line_width, pix				; pix -= line_width;


//======== Draw the third pixel

	sub.l	line_width, pix				; pix -= line_width;

	moveq	#0, d0						; push pixel value
	move.b	(pix), d0
	move.l	d0, -(SP)
	pea		old_color					; push address of old_color
	_Index2Color						; get previous rgb pixel value

	move.l	two_v_dx, d0				; distance = two_dx_invDenom + two_v_dx * invDenom
	muls.l	invDenom, d0
	add.l	two_dx_invDenom, d0

	bpl		@positive3					; get absolute value of distance

	neg.l	d0

positive3:

	moveq.l	#14, d1						; index = (((abs_distance / 0x4000) * 10) + 0x8000) >> 16
	asr.l	d1, d0
	muls.l	#10, d0
	add.l	#0x8000, d0
	moveq.l	#16, d1
	asr.l	d1, d0
	
	lea		old_color, a0				; check if the old color was 0, 0, 0
	tst.l	(a0)
	bne		@notzero3
	tst.w	4(a0)
	bne		@notzero3
	
	lea		color_table, a0				; if all 0's, we can get the pixel value from the table
	move.b	0(a0, d0.w*1), d0
	bra		@drawpixel3

@notzero3
	lea.l	intensity_table, a0			; look up the intensity in the table
	move.l	0(a0, d0.w*4), d0

	move.l	#15, d1
	asr.l	d1, d0
	move.l	d0, intensity

	move.l	color, a0
	lea		new_color, a1
	lea		old_color, a2
	moveq	#0, d1						; Find intensities of color components
	move.w	0(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	0(a2), d0
	bcc		@red_ok3
	move.w	#0xFFFF, d0
red_ok3:
	move.w	d0, 0(a1)
	
	move.l	intensity, d0
	move.w	2(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	2(a2), d0
	bcc		@green_ok3
	move.w	#0xFFFF, d0
green_ok3:
	move.w	d0, 2(a1)
	
	move.l	intensity, d0
	moveq	#0, d1
	move.w	4(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	4(a2), d0
	bcc		@blue_ok3
	move.w	#0xFFFF, d0
blue_ok3:
	move.w	d0, 4(a1)

	move.l	a1, -(SP)			; find index for new_color
	_Color2Index
	
drawpixel3:
	move.b	d0, (pix)

	add.l	line_width, pix				; pix += line_width;

	bra		@xloop;

	}


done:;

}	//==== DrawAntialiasedLineO1asm() ====\\



void DrawAntialiasedLineO2asm(long x1, long y1, long x2, long y2, RGBColor *color);
void DrawAntialiasedLineO2asm(long x1, long y1, long x2, long y2, RGBColor *color)
{

	long 			dx, dy;
	register long	d;
	register long	line_width;
	register long	two_dx_invDenom;
	long			invDenom;
	register long	count;
	long 			incrE, incrNE, two_v_dx;
	RGBColor 		old_color;
	RGBColor 		new_color;
	PixMapHandle 	pixmap;
	register unsigned char	*pix;

	unsigned char	color_table[16];
	RGBColor		table_color;
	long			i, debug;
	Fract			fract_intensity;
	Fixed			intensity;
	
	pixmap = ((CGrafPtr) thePort)->portPixMap;		//  Find address of starting pixel
	line_width = (*pixmap)->rowBytes & 0x7FFF;
	pix = (unsigned char *) (*pixmap)->baseAddr -
							(*pixmap)->bounds.top*line_width -
							(*pixmap)->bounds.left +
							line_width*x1 + y1;

asm {

//==== create table of frequently used colors (used when drawing on black)

	move.l	d3, -(SP)					; save d3
	
	move.l	#15, count

color_loop:

	lea		table_color, a1
	
	lea		intensity_table, a0			; look up the intensity in the table
	move.l	count, d0
	move.l	d0, debug
	move.l	0(a0, d0.w*4), d3
	move.l	d3, debug

	move.l	#15, d1						; intensity in d3
	asr.l	d1, d3

	move.l	d3, debug

	move.l	color, a0					; multiply red by intensity
	moveq	#0, d0						
	move.w	0(a0), d0
	muls.l	d3, d0
	moveq	#15, d2
	asr.l	d2, d0
	move.w	d0, 0(a1)

	moveq	#0, d0						; multiply green by intensity
	move.w	2(a0), d0
	muls.l	d3, d0
	moveq	#15, d2
	asr.l	d2, d0
	move.w	d0, 2(a1)

	moveq	#0, d0						; multiply blue by intensity
	move.w	4(a0), d0
	muls.l	d3, d0
	moveq	#15, d2
	asr.l	d2, d0
	move.w	d0, 4(a1)

	move.l	a1, -(SP)					; find index for this color
	_Color2Index
	
	lea		color_table, a0				; save this index in the table
	move.b	d0, 0(a0, count)
	
	subq	#1, count
	bpl		@color_loop
	
	move.l	(SP)+, d3					; restore d3

//==== set up to draw line	
	
	move.l	x2, dx				; dx = x2 - x1
	move.l	x1, d0
	sub.l	d0, dx
	
	move.l	y2, dy				; dy = y2 - y1
	move.l	y1, d0
	sub.l	d0, dy
	
	move.l	dy, d				; d = 2*dy - dx
	add.l	d, d
	sub.l	dx, d
	
	move.l	dy, d0				; incrE = 2*dy
	add.l	d0, d0
	move.l	d0, incrE
	
	move.l	dy, d0				; incrNE = 2*(dy-dx)
	sub.l	dx, d0
	add.l	d0, d0
	move.l	d0, incrNE

	move.l	#0, two_v_dx;		; two_v_dx = 0
	
	; the next section computes 1/(2*sqrt(dx*dx + dy*dy) using integer arithmetic
	
	move.l	dx, d0				; compute dx*dx + dy*dy
	muls.l	d0, d0
	move.l	dy, d1
	muls.l	d1, d1
	add.l	d1, d0
	
	move.l	#12, d1
	asl.l	d1, d0				; take the square root
	move.l	d0, -(SP)
	_FracSqrt

	move.l	(SP), d0			; find the denominator
	asr.l	#4, d0
	
	move.l	#0x10000, -(SP)		; do the division
	move.l	d0, -(SP)
	_FracDiv

	move.l	(SP), invDenom
	
	move.l	invDenom, two_dx_invDenom	; two_dx_invDenom = 2*dx*invDenom
	muls.l	dx, two_dx_invDenom
	add.l	two_dx_invDenom, two_dx_invDenom

	move.l	dx, count
	
	bra		@move_done
	
xloop:

	sub.l	#1, count			; loop while count > 0
	bmi		@done

	tst.l	d
	bge		@move_northeast

move_north:

	move.l	d, d0				; two_v_dx = d + dx
	add.l	dx, d0
	move.l	d0, two_v_dx
	
	add.l	incrE, d			; d = d + incrE
		
	add.l	line_width, pix		; y = y + 1
	
	bra		@move_done
	
move_northeast:

	move.l	d, d0				; two_v_dx = d - dx
	sub.l	dx, d0
	move.l	d0, two_v_dx
	
	add.l	incrNE, d			; d = d + incrE
	
	add.l	line_width, pix		; y = y + 1
	addq	#1, pix				; x = x + 1

move_done:


//========== Draw first pixel
	
	moveq	#0, d0						; push pixel value
	move.b	(pix), d0
	move.l	d0, -(SP)
	pea		old_color					; push address of old_color
	_Index2Color						; get previous rgb pixel value

	move.l	two_v_dx, d0				; distance = two_v_dx * invDenom
	muls.l	invDenom, d0

	bpl		@positive1					; get absolute value of distance

	neg.l	d0

positive1:

	moveq.l	#14, d1						; index = (((abs_distance / 0x4000) * 10) + 0x8000) >> 16
	asr.l	d1, d0
	muls.l	#10, d0
	add.l	#0x8000, d0
	moveq.l	#16, d1
	asr.l	d1, d0
	
	lea		old_color, a0				; check if the old color was 0, 0, 0
	tst.l	(a0)
	bne		@notzero1
	tst.w	4(a0)
	bne		@notzero1
	
	lea		color_table, a0				; if all 0's, we can get the pixel value from the table
	move.b	0(a0, d0), d0
	bra		@drawpixel1

@notzero1
	
	lea.l	intensity_table, a0			; look up the intensity in the table
	move.l	0(a0, d0*4), d0

	move.l	#15, d1
	asr.l	d1, d0
	move.l	d0, intensity

	move.l	color, a0
	lea		new_color, a1
	lea		old_color, a2
	moveq	#0, d1						; Find intensities of color components
	move.w	0(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	0(a2), d0
	bcc		@red_ok1
	move.w	#0xFFFF, d0
red_ok1:
	move.w	d0, 0(a1)
	
	move.l	intensity, d0
	move.w	2(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	2(a2), d0
	bcc		@green_ok1
	move.w	#0xFFFF, d0
green_ok1:
	move.w	d0, 2(a1)
	
	move.l	intensity, d0
	moveq	#0, d1
	move.w	4(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	4(a2), d0
	bcc		@blue_ok1
	move.w	#0xFFFF, d0
blue_ok1:
	move.w	d0, 4(a1)

	move.l	a1, -(SP)			; find index for new_color
	_Color2Index
	
drawpixel1:

	move.b	d0, (pix)


//======== Draw the second pixel

	addq	#1, pix						; pix += 1;

	moveq	#0, d0						; push pixel value
	move.b	(pix), d0
	move.l	d0, -(SP)
	pea		old_color					; push address of old_color
	_Index2Color						; get previous rgb pixel value

	move.l	two_v_dx, d1				; distance = two_dx_invDenom - two_v_dx * invDenom
	muls.l	invDenom, d1
	move.l	two_dx_invDenom, d0
	sub.l	d1, d0

	bpl		@positive2					; get absolute value of distance

	neg.l	d0

positive2:

	moveq.l	#14, d1						; index = (((abs_distance / 0x4000) * 10) + 0x8000) >> 16
	asr.l	d1, d0
	muls.l	#10, d0
	add.l	#0x8000, d0
	moveq.l	#16, d1
	asr.l	d1, d0
	
	lea		old_color, a0				; check if the old color was 0, 0, 0
	tst.l	(a0)
	bne		@notzero2
	tst.w	4(a0)
	bne		@notzero2
	
	lea		color_table, a0				; if all 0's, we can get the pixel value from the table
	move.b	0(a0, d0.w*1), d0
	bra		@drawpixel2

notzero2:
	lea.l	intensity_table, a0			; look up the intensity in the table
	move.l	0(a0, d0.w*4), d0

	move.l	#15, d1
	asr.l	d1, d0
	move.l	d0, intensity

	move.l	color, a0
	lea		new_color, a1
	lea		old_color, a2
	moveq	#0, d1						; Find intensities of color components
	move.w	0(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	0(a2), d0
	bcc		@red_ok2
	move.w	#0xFFFF, d0
red_ok2:
	move.w	d0, 0(a1)
	
	move.l	intensity, d0
	move.w	2(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	2(a2), d0
	bcc		@green_ok2
	move.w	#0xFFFF, d0
green_ok2:
	move.w	d0, 2(a1)
	
	move.l	intensity, d0
	moveq	#0, d1
	move.w	4(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	4(a2), d0
	bcc		@blue_ok2
	move.w	#0xFFFF, d0
blue_ok2:
	move.w	d0, 4(a1)

	move.l	a1, -(SP)			; find index for new_color
	_Color2Index
	
drawpixel2:
	move.b	d0, (pix)

	subq	#1, pix						; pix -= 1;


//======== Draw the third pixel

	subq	#1, pix						; pix -= 1;

	moveq	#0, d0						; push pixel value
	move.b	(pix), d0
	move.l	d0, -(SP)
	pea		old_color					; push address of old_color
	_Index2Color						; get previous rgb pixel value

	move.l	two_v_dx, d0				; distance = two_dx_invDenom + two_v_dx * invDenom
	muls.l	invDenom, d0
	add.l	two_dx_invDenom, d0

	bpl		@positive3					; get absolute value of distance

	neg.l	d0

positive3:

	moveq.l	#14, d1						; index = (((abs_distance / 0x4000) * 10) + 0x8000) >> 16
	asr.l	d1, d0
	muls.l	#10, d0
	add.l	#0x8000, d0
	moveq.l	#16, d1
	asr.l	d1, d0
	
	lea		old_color, a0				; check if the old color was 0, 0, 0
	tst.l	(a0)
	bne		@notzero3
	tst.w	4(a0)
	bne		@notzero3
	
	lea		color_table, a0				; if all 0's, we can get the pixel value from the table
	move.b	0(a0, d0.w*1), d0
	bra		@drawpixel3

@notzero3
	lea.l	intensity_table, a0			; look up the intensity in the table
	move.l	0(a0, d0.w*4), d0

	move.l	#15, d1
	asr.l	d1, d0
	move.l	d0, intensity

	move.l	color, a0
	lea		new_color, a1
	lea		old_color, a2
	moveq	#0, d1						; Find intensities of color components
	move.w	0(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	0(a2), d0
	bcc		@red_ok3
	move.w	#0xFFFF, d0
red_ok3:
	move.w	d0, 0(a1)
	
	move.l	intensity, d0
	move.w	2(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	2(a2), d0
	bcc		@green_ok3
	move.w	#0xFFFF, d0
green_ok3:
	move.w	d0, 2(a1)
	
	move.l	intensity, d0
	moveq	#0, d1
	move.w	4(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	4(a2), d0
	bcc		@blue_ok3
	move.w	#0xFFFF, d0
blue_ok3:
	move.w	d0, 4(a1)

	move.l	a1, -(SP)			; find index for new_color
	_Color2Index
	
drawpixel3:
	move.b	d0, (pix)

	addq	#1, pix				; pix += 1

	bra		@xloop;

	}


done:;

}	//==== DrawAntialiasedLineO2asm() ====\\



void DrawAntialiasedLineO3asm(long x1, long y1, long x2, long y2, RGBColor *color);
void DrawAntialiasedLineO3asm(long x1, long y1, long x2, long y2, RGBColor *color)
{

	long 			dx, dy;
	register long	d;
	register long	line_width;
	register long	two_dx_invDenom;
	long			invDenom;
	register long	count;
	long 			incrE, incrNE, two_v_dx;
	RGBColor 		old_color;
	RGBColor 		new_color;
	PixMapHandle 	pixmap;
	register unsigned char	*pix;

	unsigned char	color_table[16];
	RGBColor		table_color;
	long			i, debug;
	Fract			fract_intensity;
	Fixed			intensity;
	
	pixmap = ((CGrafPtr) thePort)->portPixMap;		//  Find address of starting pixel
	line_width = (*pixmap)->rowBytes & 0x7FFF;
	pix = (unsigned char *) (*pixmap)->baseAddr -
							(*pixmap)->bounds.top*line_width -
							(*pixmap)->bounds.left -
							line_width*x1 + y1;

asm {

//==== create table of frequently used colors (used when drawing on black)

	move.l	d3, -(SP)					; save d3
	
	move.l	#15, count

color_loop:

	lea		table_color, a1
	
	lea		intensity_table, a0			; look up the intensity in the table
	move.l	count, d0
	move.l	d0, debug
	move.l	0(a0, d0.w*4), d3
	move.l	d3, debug

	move.l	#15, d1						; intensity in d3
	asr.l	d1, d3

	move.l	d3, debug

	move.l	color, a0					; multiply red by intensity
	moveq	#0, d0						
	move.w	0(a0), d0
	muls.l	d3, d0
	moveq	#15, d2
	asr.l	d2, d0
	move.w	d0, 0(a1)

	moveq	#0, d0						; multiply green by intensity
	move.w	2(a0), d0
	muls.l	d3, d0
	moveq	#15, d2
	asr.l	d2, d0
	move.w	d0, 2(a1)

	moveq	#0, d0						; multiply blue by intensity
	move.w	4(a0), d0
	muls.l	d3, d0
	moveq	#15, d2
	asr.l	d2, d0
	move.w	d0, 4(a1)

	move.l	a1, -(SP)					; find index for this color
	_Color2Index
	
	lea		color_table, a0				; save this index in the table
	move.b	d0, 0(a0, count)
	
	subq	#1, count
	bpl		@color_loop
	
	move.l	(SP)+, d3					; restore d3

//==== set up to draw line	
	
	move.l	x2, dx				; dx = x2 - x1
	move.l	x1, d0
	sub.l	d0, dx
	
	move.l	y2, dy				; dy = y2 - y1
	move.l	y1, d0
	sub.l	d0, dy
	
	move.l	dy, d				; d = 2*dy - dx
	add.l	d, d
	sub.l	dx, d
	
	move.l	dy, d0				; incrE = 2*dy
	add.l	d0, d0
	move.l	d0, incrE
	
	move.l	dy, d0				; incrNE = 2*(dy-dx)
	sub.l	dx, d0
	add.l	d0, d0
	move.l	d0, incrNE

	move.l	#0, two_v_dx;		; two_v_dx = 0
	
	; the next section computes 1/(2*sqrt(dx*dx + dy*dy) using integer arithmetic
	
	move.l	dx, d0				; compute dx*dx + dy*dy
	muls.l	d0, d0
	move.l	dy, d1
	muls.l	d1, d1
	add.l	d1, d0
	
	move.l	#12, d1
	asl.l	d1, d0				; take the square root
	move.l	d0, -(SP)
	_FracSqrt

	move.l	(SP), d0			; find the denominator
	asr.l	#4, d0
	
	move.l	#0x10000, -(SP)		; do the division
	move.l	d0, -(SP)
	_FracDiv

	move.l	(SP), invDenom
	
	move.l	invDenom, two_dx_invDenom	; two_dx_invDenom = 2*dx*invDenom
	muls.l	dx, two_dx_invDenom
	add.l	two_dx_invDenom, two_dx_invDenom

	move.l	dx, count
	
	bra		@move_done
	
xloop:

	sub.l	#1, count			; loop while count > 0
	bmi		@done

	tst.l	d
	bge		@move_northwest

move_west:

	move.l	d, d0				; two_v_dx = d + dx
	add.l	dx, d0
	move.l	d0, two_v_dx
	
	add.l	incrE, d			; d = d + incrE
		
	sub.l	line_width, pix		; y = y + 1
	
	bra		@move_done
	
move_northwest:

	move.l	d, d0				; two_v_dx = d - dx
	sub.l	dx, d0
	move.l	d0, two_v_dx
	
	add.l	incrNE, d			; d = d + incrE
	
	sub.l	line_width, pix		; y = y + 1
	addq	#1, pix				; x = x - 1

move_done:


//========== Draw first pixel
	
	moveq	#0, d0						; push pixel value
	move.b	(pix), d0
	move.l	d0, -(SP)
	pea		old_color					; push address of old_color
	_Index2Color						; get previous rgb pixel value

	move.l	two_v_dx, d0				; distance = two_v_dx * invDenom
	muls.l	invDenom, d0

	bpl		@positive1					; get absolute value of distance

	neg.l	d0

positive1:

	moveq.l	#14, d1						; index = (((abs_distance / 0x4000) * 10) + 0x8000) >> 16
	asr.l	d1, d0
	muls.l	#10, d0
	add.l	#0x8000, d0
	moveq.l	#16, d1
	asr.l	d1, d0
	
	lea		old_color, a0				; check if the old color was 0, 0, 0
	tst.l	(a0)
	bne		@notzero1
	tst.w	4(a0)
	bne		@notzero1
	
	lea		color_table, a0				; if all 0's, we can get the pixel value from the table
	move.b	0(a0, d0), d0
	bra		@drawpixel1

@notzero1
	
	lea.l	intensity_table, a0			; look up the intensity in the table
	move.l	0(a0, d0*4), d0

	move.l	#15, d1
	asr.l	d1, d0
	move.l	d0, intensity

	move.l	color, a0
	lea		new_color, a1
	lea		old_color, a2
	moveq	#0, d1						; Find intensities of color components
	move.w	0(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	0(a2), d0
	bcc		@red_ok1
	move.w	#0xFFFF, d0
red_ok1:
	move.w	d0, 0(a1)
	
	move.l	intensity, d0
	move.w	2(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	2(a2), d0
	bcc		@green_ok1
	move.w	#0xFFFF, d0
green_ok1:
	move.w	d0, 2(a1)
	
	move.l	intensity, d0
	moveq	#0, d1
	move.w	4(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	4(a2), d0
	bcc		@blue_ok1
	move.w	#0xFFFF, d0
blue_ok1:
	move.w	d0, 4(a1)

	move.l	a1, -(SP)			; find index for new_color
	_Color2Index
	
drawpixel1:

	move.b	d0, (pix)


//======== Draw the second pixel

	addq	#1, pix						; pix += 1;

	moveq	#0, d0						; push pixel value
	move.b	(pix), d0
	move.l	d0, -(SP)
	pea		old_color					; push address of old_color
	_Index2Color						; get previous rgb pixel value

	move.l	two_v_dx, d1				; distance = two_dx_invDenom - two_v_dx * invDenom
	muls.l	invDenom, d1
	move.l	two_dx_invDenom, d0
	sub.l	d1, d0

	bpl		@positive2					; get absolute value of distance

	neg.l	d0

positive2:

	moveq.l	#14, d1						; index = (((abs_distance / 0x4000) * 10) + 0x8000) >> 16
	asr.l	d1, d0
	muls.l	#10, d0
	add.l	#0x8000, d0
	moveq.l	#16, d1
	asr.l	d1, d0
	
	lea		old_color, a0				; check if the old color was 0, 0, 0
	tst.l	(a0)
	bne		@notzero2
	tst.w	4(a0)
	bne		@notzero2
	
	lea		color_table, a0				; if all 0's, we can get the pixel value from the table
	move.b	0(a0, d0.w*1), d0
	bra		@drawpixel2

notzero2:
	lea.l	intensity_table, a0			; look up the intensity in the table
	move.l	0(a0, d0.w*4), d0

	move.l	#15, d1
	asr.l	d1, d0
	move.l	d0, intensity

	move.l	color, a0
	lea		new_color, a1
	lea		old_color, a2
	moveq	#0, d1						; Find intensities of color components
	move.w	0(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	0(a2), d0
	bcc		@red_ok2
	move.w	#0xFFFF, d0
red_ok2:
	move.w	d0, 0(a1)
	
	move.l	intensity, d0
	move.w	2(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	2(a2), d0
	bcc		@green_ok2
	move.w	#0xFFFF, d0
green_ok2:
	move.w	d0, 2(a1)
	
	move.l	intensity, d0
	moveq	#0, d1
	move.w	4(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	4(a2), d0
	bcc		@blue_ok2
	move.w	#0xFFFF, d0
blue_ok2:
	move.w	d0, 4(a1)

	move.l	a1, -(SP)			; find index for new_color
	_Color2Index
	
drawpixel2:
	move.b	d0, (pix)

	subq	#1, pix						; pix -= 1;


//======== Draw the third pixel

	subq	#1, pix						; pix -= 1;

	moveq	#0, d0						; push pixel value
	move.b	(pix), d0
	move.l	d0, -(SP)
	pea		old_color					; push address of old_color
	_Index2Color						; get previous rgb pixel value

	move.l	two_v_dx, d0				; distance = two_dx_invDenom + two_v_dx * invDenom
	muls.l	invDenom, d0
	add.l	two_dx_invDenom, d0

	bpl		@positive3					; get absolute value of distance

	neg.l	d0

positive3:

	moveq.l	#14, d1						; index = (((abs_distance / 0x4000) * 10) + 0x8000) >> 16
	asr.l	d1, d0
	muls.l	#10, d0
	add.l	#0x8000, d0
	moveq.l	#16, d1
	asr.l	d1, d0
	
	lea		old_color, a0				; check if the old color was 0, 0, 0
	tst.l	(a0)
	bne		@notzero3
	tst.w	4(a0)
	bne		@notzero3
	
	lea		color_table, a0				; if all 0's, we can get the pixel value from the table
	move.b	0(a0, d0.w*1), d0
	bra		@drawpixel3

@notzero3
	lea.l	intensity_table, a0			; look up the intensity in the table
	move.l	0(a0, d0.w*4), d0

	move.l	#15, d1
	asr.l	d1, d0
	move.l	d0, intensity

	move.l	color, a0
	lea		new_color, a1
	lea		old_color, a2
	moveq	#0, d1						; Find intensities of color components
	move.w	0(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	0(a2), d0
	bcc		@red_ok3
	move.w	#0xFFFF, d0
red_ok3:
	move.w	d0, 0(a1)
	
	move.l	intensity, d0
	move.w	2(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	2(a2), d0
	bcc		@green_ok3
	move.w	#0xFFFF, d0
green_ok3:
	move.w	d0, 2(a1)
	
	move.l	intensity, d0
	moveq	#0, d1
	move.w	4(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	4(a2), d0
	bcc		@blue_ok3
	move.w	#0xFFFF, d0
blue_ok3:
	move.w	d0, 4(a1)

	move.l	a1, -(SP)			; find index for new_color
	_Color2Index
	
drawpixel3:
	move.b	d0, (pix)

	addq	#1, pix				; pix += 1

	bra		@xloop;

	}


done:;

}	//==== DrawAntialiasedLineO3asm() ====\\



void DrawAntialiasedLineO4asm(long x1, long y1, long x2, long y2, RGBColor *color);
void DrawAntialiasedLineO4asm(long x1, long y1, long x2, long y2, RGBColor *color)
{

	long 			dx, dy;
	register long	d;
	register long	line_width;
	register long	two_dx_invDenom;
	long			invDenom;
	register long	count;
	long 			incrE, incrNE, two_v_dx;
	RGBColor 		old_color;
	RGBColor 		new_color;
	PixMapHandle 	pixmap;
	register unsigned char	*pix;

	unsigned char	color_table[16];
	RGBColor		table_color;
	long			i, debug;
	Fract			fract_intensity;
	Fixed			intensity;
	
	pixmap = ((CGrafPtr) thePort)->portPixMap;		//  Find address of starting pixel
	line_width = (*pixmap)->rowBytes & 0x7FFF;
	pix = (unsigned char *) (*pixmap)->baseAddr -
							(*pixmap)->bounds.top*line_width -
							(*pixmap)->bounds.left -
							line_width*y1 + x1;

asm {

//==== create table of frequently used colors (used when drawing on black	

	move.l	d3, -(SP)					; save d3
	
	move.l	#15, count

color_loop:

	lea		table_color, a1
	
	lea		intensity_table, a0			; look up the intensity in the table
	move.l	count, d0
	move.l	d0, debug
	move.l	0(a0, d0.w*4), d3
	move.l	d3, debug

	move.l	#15, d1						; intensity in d3
	asr.l	d1, d3

	move.l	d3, debug

	move.l	color, a0					; multiply red by intensity
	moveq	#0, d0						
	move.w	0(a0), d0
	muls.l	d3, d0
	moveq	#15, d2
	asr.l	d2, d0
	move.w	d0, 0(a1)

	moveq	#0, d0						; multiply green by intensity
	move.w	2(a0), d0
	muls.l	d3, d0
	moveq	#15, d2
	asr.l	d2, d0
	move.w	d0, 2(a1)

	moveq	#0, d0						; multiply blue by intensity
	move.w	4(a0), d0
	muls.l	d3, d0
	moveq	#15, d2
	asr.l	d2, d0
	move.w	d0, 4(a1)

	move.l	a1, -(SP)					; find index for this color
	_Color2Index
	
	lea		color_table, a0				; save this index in the table
	move.b	d0, 0(a0, count)
	
	subq	#1, count
	bpl		@color_loop
	
	move.l	(SP)+, d3					; restore d3

//==== set up to draw line	
	
	move.l	x2, dx				; dx = x2 - x1
	move.l	x1, d0
	sub.l	d0, dx
	
	move.l	y2, dy				; dy = y2 - y1
	move.l	y1, d0
	sub.l	d0, dy
	
	move.l	dy, d				; d = 2*dy - dx
	add.l	d, d
	sub.l	dx, d
	
	move.l	dy, d0				; incrE = 2*dy
	add.l	d0, d0
	move.l	d0, incrE
	
	move.l	dy, d0				; incrNE = 2*(dy-dx)
	sub.l	dx, d0
	add.l	d0, d0
	move.l	d0, incrNE

	move.l	#0, two_v_dx;		; two_v_dx = 0
	
	; the next section computes 1/(2*sqrt(dx*dx + dy*dy) using integer arithmetic
	
	move.l	dx, d0				; compute dx*dx + dy*dy
	muls.l	d0, d0
	move.l	dy, d1
	muls.l	d1, d1
	add.l	d1, d0
	
	move.l	#12, d1
	asl.l	d1, d0				; take the square root
	move.l	d0, -(SP)
	_FracSqrt

	move.l	(SP), d0			; find the denominator
	asr.l	#4, d0
	
	move.l	#0x10000, -(SP)		; do the division
	move.l	d0, -(SP)
	_FracDiv

	move.l	(SP), invDenom
	
	move.l	invDenom, two_dx_invDenom	; two_dx_invDenom = 2*dx*invDenom
	muls.l	dx, two_dx_invDenom
	add.l	two_dx_invDenom, two_dx_invDenom

	move.l	dx, count
	
	bra		@move_done
	
xloop:

	sub.l	#1, count			; loop while count > 0
	bmi		@done

	tst.l	d
	bge		@move_northeast

move_east:

	move.l	d, d0				; two_v_dx = d + dx
	add.l	dx, d0
	move.l	d0, two_v_dx
	
	add.l	incrE, d			; d = d + incrE
		
	addq.l	#1, pix				; x = x + 1
	
	bra		@move_done
	
move_northeast:

	move.l	d, d0				; two_v_dx = d - dx
	sub.l	dx, d0
	move.l	d0, two_v_dx
	
	add.l	incrNE, d			; d = d + incrE
	
	sub.l	line_width, pix		; y = y + 1
	addq.l	#1, pix				; x = x + 1
	
move_done:


//========== Draw first pixel
	
	moveq	#0, d0						; push pixel value
	move.b	(pix), d0
	move.l	d0, -(SP)
	pea		old_color					; push address of old_color
	_Index2Color						; get previous rgb pixel value

	move.l	two_v_dx, d0				; distance = two_v_dx * invDenom
	muls.l	invDenom, d0

	bpl		@positive1					; get absolute value of distance

	neg.l	d0

positive1:

	moveq.l	#14, d1						; index = (((abs_distance / 0x4000) * 10) + 0x8000) >> 16
	asr.l	d1, d0
	muls.l	#10, d0
	add.l	#0x8000, d0
	moveq.l	#16, d1
	asr.l	d1, d0
	
	lea		old_color, a0				; check if the old color was 0, 0, 0
	tst.l	(a0)
	bne		@notzero1
	tst.w	4(a0)
	bne		@notzero1
	
	lea		color_table, a0				; if all 0's, we can get the pixel value from the table
	move.b	0(a0, d0), d0
	bra		@drawpixel1

@notzero1
	
	lea.l	intensity_table, a0			; look up the intensity in the table
	move.l	0(a0, d0*4), d0

	move.l	#15, d1
	asr.l	d1, d0
	move.l	d0, intensity

	move.l	color, a0
	lea		new_color, a1
	lea		old_color, a2
	moveq	#0, d1						; Find intensities of color components
	move.w	0(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	0(a2), d0
	bcc		@red_ok1
	move.w	#0xFFFF, d0
red_ok1:
	move.w	d0, 0(a1)
	
	move.l	intensity, d0
	move.w	2(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	2(a2), d0
	bcc		@green_ok1
	move.w	#0xFFFF, d0
green_ok1:
	move.w	d0, 2(a1)
	
	move.l	intensity, d0
	moveq	#0, d1
	move.w	4(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	4(a2), d0
	bcc		@blue_ok1
	move.w	#0xFFFF, d0
blue_ok1:
	move.w	d0, 4(a1)

	move.l	a1, -(SP)			; find index for new_color
	_Color2Index
	
drawpixel1:

	move.b	d0, (pix)


//======== Draw the second pixel

	sub.l	line_width, pix				; pix -= line_width;

	moveq	#0, d0						; push pixel value
	move.b	(pix), d0
	move.l	d0, -(SP)
	pea		old_color					; push address of old_color
	_Index2Color						; get previous rgb pixel value

	move.l	two_v_dx, d1				; distance = two_dx_invDenom - two_v_dx * invDenom
	muls.l	invDenom, d1
	move.l	two_dx_invDenom, d0
	sub.l	d1, d0

	bpl		@positive2					; get absolute value of distance

	neg.l	d0

positive2:

	moveq.l	#14, d1						; index = (((abs_distance / 0x4000) * 10) + 0x8000) >> 16
	asr.l	d1, d0
	muls.l	#10, d0
	add.l	#0x8000, d0
	moveq.l	#16, d1
	asr.l	d1, d0
	
	lea		old_color, a0				; check if the old color was 0, 0, 0
	tst.l	(a0)
	bne		@notzero2
	tst.w	4(a0)
	bne		@notzero2
	
	lea		color_table, a0				; if all 0's, we can get the pixel value from the table
	move.b	0(a0, d0.w*1), d0
	bra		@drawpixel2

notzero2:
	lea.l	intensity_table, a0			; look up the intensity in the table
	move.l	0(a0, d0.w*4), d0

	move.l	#15, d1
	asr.l	d1, d0
	move.l	d0, intensity

	move.l	color, a0
	lea		new_color, a1
	lea		old_color, a2
	moveq	#0, d1						; Find intensities of color components
	move.w	0(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	0(a2), d0
	bcc		@red_ok2
	move.w	#0xFFFF, d0
red_ok2:
	move.w	d0, 0(a1)
	
	move.l	intensity, d0
	move.w	2(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	2(a2), d0
	bcc		@green_ok2
	move.w	#0xFFFF, d0
green_ok2:
	move.w	d0, 2(a1)
	
	move.l	intensity, d0
	moveq	#0, d1
	move.w	4(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	4(a2), d0
	bcc		@blue_ok2
	move.w	#0xFFFF, d0
blue_ok2:
	move.w	d0, 4(a1)

	move.l	a1, -(SP)			; find index for new_color
	_Color2Index
	
drawpixel2:
	move.b	d0, (pix)

	add.l	line_width, pix				; pix += line_width


//======== Draw the third pixel

	add.l	line_width, pix				; pix += line_width

	moveq	#0, d0						; push pixel value
	move.b	(pix), d0
	move.l	d0, -(SP)
	pea		old_color					; push address of old_color
	_Index2Color						; get previous rgb pixel value

	move.l	two_v_dx, d0				; distance = two_dx_invDenom + two_v_dx * invDenom
	muls.l	invDenom, d0
	add.l	two_dx_invDenom, d0

	bpl		@positive3					; get absolute value of distance

	neg.l	d0

positive3:

	moveq.l	#14, d1						; index = (((abs_distance / 0x4000) * 10) + 0x8000) >> 16
	asr.l	d1, d0
	muls.l	#10, d0
	add.l	#0x8000, d0
	moveq.l	#16, d1
	asr.l	d1, d0
	
	lea		old_color, a0				; check if the old color was 0, 0, 0
	tst.l	(a0)
	bne		@notzero3
	tst.w	4(a0)
	bne		@notzero3
	
	lea		color_table, a0				; if all 0's, we can get the pixel value from the table
	move.b	0(a0, d0.w*1), d0
	bra		@drawpixel3

@notzero3
	lea.l	intensity_table, a0			; look up the intensity in the table
	move.l	0(a0, d0.w*4), d0

	move.l	#15, d1
	asr.l	d1, d0
	move.l	d0, intensity

	move.l	color, a0
	lea		new_color, a1
	lea		old_color, a2
	moveq	#0, d1						; Find intensities of color components
	move.w	0(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	0(a2), d0
	bcc		@red_ok3
	move.w	#0xFFFF, d0
red_ok3:
	move.w	d0, 0(a1)
	
	move.l	intensity, d0
	move.w	2(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	2(a2), d0
	bcc		@green_ok3
	move.w	#0xFFFF, d0
green_ok3:
	move.w	d0, 2(a1)
	
	move.l	intensity, d0
	moveq	#0, d1
	move.w	4(a0), d1
	muls.l	d1, d0
	moveq	#15, d2
	asr.l	d2, d0
	add.w	4(a2), d0
	bcc		@blue_ok3
	move.w	#0xFFFF, d0
blue_ok3:
	move.w	d0, 4(a1)

	move.l	a1, -(SP)			; find index for new_color
	_Color2Index
	
drawpixel3:
	move.b	d0, (pix)

	sub.l	line_width, pix				; pix -= line_width

	bra		@xloop;

	}


done:;

}	//==== DrawAntialiasedLineO4asm() ====\\