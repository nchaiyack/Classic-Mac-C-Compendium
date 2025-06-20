/**

	pstring.c
	Copyright (c) 1989-1992, joe holt

 **/


/**-----------------------------------------------------------------------------
 **
 ** Headers
 **
 **/

#ifndef __ctypes__
#include "ctypes.h"
#endif

#ifndef __pstring__
#include "pstring.h"
#endif


/**-----------------------------------------------------------------------------
 **
 ** Private Variables
 **
 **/

static unsigned char upper_case_Mac_encoding[] = {

0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
0x60,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x7B,0x7C,0x7D,0x7E,0x7F,

0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0xCB,0x89,0x80,0xCC,0x81,0x82,0x83,0x8F,
0x90,0x91,0x92,0x93,0x94,0x95,0x84,0x97,0x98,0x99,0x85,0xCD,0x9C,0x9D,0x9E,0x86,
0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xAE,0xAF,
0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCE,
0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,
0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF

};


/*******************************************************************************
 **
 **	Public Functions
 **
 **/

/*******************************************************************************
 ***
 *** unsigned char *pstrcopy( unsigned char *s1, unsigned char *s2 );
 ***
 *** Copy Pascal string <s1> to <s2>.
 ***
 *** Returns the address one byte past the end of new string <s2>.
 ***
 *** History:
 ***   jhh 25 jul 89 -- template project; joe holt
 ***
 ***/

unsigned char *pstrcopy( unsigned char *s1, unsigned char *s2 )
{
	s2[0] = 0;
	return pstrappend( s1, s2 );
}


/*******************************************************************************
 ***
 *** unsigned char *pstrappend( unsigned char *s1, unsigned char *s2 );
 ***
 *** Append Pascal string <s1> to <s2>.
 ***
 *** Returns the address one byte past the end of new string <s2>.
 ***
 *** History:
 ***   jhh 25 jul 89 -- template project; joe holt
 ***   jhh  9 dec 89 -- check for destination string overflow
 ***
 ***/

unsigned char *pstrappend( unsigned char *s1, unsigned char *s2 )
{
	asm {
			movea.l	s1, A0
			movea.l	s2, A1
			moveq	#0, D0
			moveq	#0, D1
			move.b	(A0)+, D0
			move.b	(A1), D1
			add.b	D0, (A1)+			; update length byte of s2
			bcc.s	@1					; no overflow

			addq.b	#1, -(A1)			; len(s1) + len(s2) > 255...
			sub.b	(A1), D0			; so append only as much as will fit
			move.b	#255, (A1)+
@1:			adda.w	D1, A1				; begin adding s1 to old end of s2
			bra.s	@3

@2:			move.b	(A0)+, (A1)+
@3:			dbra	D0, @2

			move.l	A1, D0				; return new end of s2
	}
}


/*******************************************************************************
 ***
 *** unsigned char *pchappend( unsigned char ch, unsigned char *s2 );
 ***
 *** Append character <ch> to Pascal string <s2>.
 ***
 *** Returns the address one byte past the end of new string <s2>.
 ***
 *** History:
 ***   jhh  9 dec 89 -- template project; joe holt
 ***
 ***/

unsigned char *pchappend( unsigned char ch, unsigned char *s2 )
{
	s2[pstrlen(s2)+1 ] = ch;
	++s2[0];
	return &s2[pstrlen(s2)+1];
}


/*******************************************************************************
 ***
 *** unsigned char *pchcopy( unsigned char ch, unsigned char *s2 );
 ***
 *** Copy character <ch> to Pascal string <s2>.
 ***
 *** Returns the address one byte past the end of new string <s2>.
 ***
 *** History:
 ***   jhh  9 dec 89 -- template project; joe holt
 ***
 ***/

unsigned char *pchcopy( unsigned char ch, unsigned char *s2 )
{
	s2[0] = 1;
	s2[1] = ch;
	return &s2[2];
}


/*******************************************************************************
 ***
 *** unsigned char *pnumcopy( int32 n, unsigned char *s2 );
 ***
 *** Copy (convert) number <n> to Pascal string <s2>.
 ***
 *** Returns the address one byte past the end of new string <s2>.
 ***
 *** History:
 ***   jhh    sep 89 -- template project; joe holt
 ***
 ***/

unsigned char *pnumcopy( int32 n, unsigned char *s2 )
{
	s2[0] = 0;
	return pnumappend( n, s2 );
}


/*******************************************************************************
 ***
 *** unsigned char *pnumappend( int32 n, unsigned char *s2 );
 ***
 *** Append (convert and append) number <n> to Pascal string <s2>.
 ***
 *** Returns the address one byte past the end of new string <s2>.
 ***
 *** History:
 ***   jhh    sep 89 -- template project; joe holt
 ***
 ***/

unsigned char *pnumappend( int32 n, unsigned char *s2 )
{
	unsigned char snum[20];

	asm {
			movem.l	D3/A2, -(A7)
			lea		snum, A0
			movea.l	A0, A1
			clr.b	(A0)+
			lea		@div, A2			; beginning of divisor table
			moveq	#0, D3				; haven't seen non-zero digit yet
			move.l	n, D0
			bpl.s	@1					; number is positive, so is fine

			move.b	#'-', D2			; negative, so add '-' and fix number
			neg.l	D0
			bra.s	@5

@1:			move.l	(A2)+, D1
			beq.s	@6					; end of divisor table, so we're done

			moveq	#0, D2
			bra.s	@3

@2:			addq.w	#1, D2				; subtract the base 10 divisor...
@3:			sub.l	D1, D0				; ...until we underflow
			bcc.s	@2

			add.l	D1, D0
			tst.w	D2					; is this digit a zero?
			bne.s	@4					; no, add into string

			cmpi.w	#1, D1				; is it the "ones" digit?
			beq.s	@4					; yes, add whether zero or not

			tst.w	D3					; otherwise ignore leading zeros
			beq.s	@1

@4:			add.b	#'0', D2			; convert to ASCII
			moveq	#1, D3
@5:			move.b	D2, (A0)+			; and stuff into string
			addq.b	#1, (A1)			; update Pascal string length
			bra.s	@1

@div:		dc.l	1000000000, 100000000, 10000000, 1000000, 100000
			dc.l	10000, 1000, 100, 10, 1, 0

@6:			movem.l	(A7)+, D3/A2
	}

	return pstrappend( snum, s2 );
}


/*******************************************************************************
 ***
 *** unsigned char *phexcopy( uns32 n, int16 d, unsigned char *s2 );
 ***
 *** Copy (convert) number <n> to Pascal string <s2>, hexadecimal.
 ***
 *** Returns the address one byte past the end of new string <s2>.
 ***
 *** History:
 ***   jhh 29 oct 90 -- swatch; joe holt
 ***
 ***/

unsigned char *phexcopy( uns32 n, int16 d, unsigned char *s2 )
{
	s2[0] = 0;
	return phexappend( n, d, s2 );
}


/*******************************************************************************
 ***
 *** unsigned char *phexappend(uns32 n, int16 d, unsigned char *s2);
 ***
 *** Append (convert) number <n> to Pascal string <s2>, hexadecimal.
 ***
 *** Returns the address one byte past the end of new string <s2>.
 ***
 *** History:
 ***   jhh 29 oct 90 -- swatch; joe holt
 ***
 ***/

unsigned char *phexappend( uns32 n, int16 d, unsigned char *s2 )
{
	asm {
			movea.l	s2, A0				; new length = length + d
			movea.l	A0, A1
			moveq	#0, D0
			move.b	(A0)+, D0
			add.w	D0, A0				; point to end of string
			add.w	d, D0
			move.b	D0, (A1)

;**
;
; Convert number to hexadecimal ASCII representation.  First, preshift number
; so that msdigit is in msbit.  For example, if <d> is 4, then convert the
; long xxxx1234 to 12340000.  From this point we roll the digits off the top
; and around into the lsbyte and convert.
;
; 23400001  -->  '1'    "1"
; 34000012  -->  '2'    "12"
; 40000123  -->  '3'    "123"
; 00001234  -->  '4'    "1234"
;
; **

			move.l	n, D2				; preshift
			move.w	d, D0
			moveq	#8, D1
			sub.w	D0, D1
			beq.s	@4

			add.w	D1, D1
			add.w	D1, D1
			lsl.l	D1, D2
			bra.s	@4

@1:			rol.l	#4, D2				; get next digit
			move.b	D2, D1
			andi.b	#0x0F, D1
			cmpi.b	#10, D1
			blt.s	@2

			addi.b	#'A'-10, D1			; convert 10-15 to 'A'-'F'
			bra.s	@3

@2:			addi.b	#'0', D1			; or convert 0-9 to '0'-'9'
@3:			move.b	D1, (A0)+
@4:			dbra	D0, @1

			move.l	A0, D0
	}
}


/*******************************************************************************
 ***
 *** Boolean pstrequal( unsigned char *s1, unsigned char *s2 )
 ***
 *** Compare Pascal string <s1> to <s2>.  Returns true if the strings are equal.
 *** The compare is case-insensitive but diacritical-sensitive.
 ***
 *** History:
 ***   jhh 24 jul 89 -- template project; joe holt
 ***   jhh    nov 89 -- added case insensitivity
 ***   jhh 14 dec 89 -- changed to use Macintosh case translations; see function
 ***					chtoupper()
 ***
 ***/

Boolean pstrequal( unsigned char *s1, unsigned char *s2 )
{
	asm {
			move.l	A2, -(A7)
			lea		upper_case_Mac_encoding, A2
			moveq	#0, D0
			moveq	#0, D1
			move.l	s1, A0
			move.l	s2, A1

			move.b	(A0)+, D1
			cmp.b	(A1)+, D1
			bra.s	@2
			
@1:			move.b	(A0)+, D0
			move.b	(A2, D0.w), D2

			move.b	(A1)+, D0
			cmp.b	(A2, D0.w), D2
@2:			dbne	D1, @1

			seq.b	D0
			and.b	#1, D0
			move.l	(A7)+, A2
	}
}


/*******************************************************************************
 ***
 *** unsigned char chtoupper( unsigned char ch )
 ***
 *** Convert character <ch> to upper case.  Uses case translation table which is
 *** specific to Macintosh in the upper 128-character codes.  See the table in
 *** _Inside AppleTalk_, Appendix D.
 ***
 *** History:
 ***   jhh 14 dec 89 -- template project; joe holt
 ***
 ***/

unsigned char chtoupper( unsigned char ch )
{
	asm {
			moveq	#0, D0
			move.b	ch, D0
			lea		upper_case_Mac_encoding, A0
			move.b	(A0, D0.w), D0
	}
}


/*******************************************************************************
 ***
 *** void blockzero( unsigned char *p1, int32 len );
 ***
 *** A very simple block clear function.
 ***
 *** History:
 ***   jhh 24 jul 89 -- template project; joe holt
 ***   jhh 10 dec 89 -- added odd-byte logic, now that the data structures are
 ***					packed tight.
 ***   jhh 27 dec 89 -- added zero-length test; improved odd-byte logic.
 ***
 ***/

void blockzero( unsigned char *p1, int32 len )
{
	asm {
			move.l	len, D0
			beq.s	@5

			move.l	p1, D1
			btst.l	#0, D1				; is the destination address odd?
			movea.l	D1, A0
			beq.s	@0					; no, begin clearing away

			clr.b	(A0)+				; yes, clear this one odd byte
			subq.l	#1, D0
			bmi.s	@5					; if that's it, then we're done

@0:			moveq	#0, D1				; set up to clear by longs
			bra.s	@2

@1:			move.l	D1, (A0)+
			move.l	D1, (A0)+
@2:			subq.l	#8, D0
			bpl.s	@1

			addq.l	#8, D0
			bra.s	@4

@3:			clr.b	(A0)+				; clear the rest by bytes
@4:			subq.l	#1, D0
			bpl.s	@3

@5:
	}
}
