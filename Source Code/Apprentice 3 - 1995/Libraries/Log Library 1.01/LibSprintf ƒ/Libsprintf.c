/*
	libsprintf.c
	
	Stand alone sprintf clone (stand alone meaning that one doesn't need to use the ansi libraries to use this).
	
	Most of this is � Symantec, Inc. (I just had to do a little tinkering to get it to be stand alone...)
	
	06/26/94 - Updated for PPC.
*/

#ifdef _USE_ANSI_ // then use the ansi libraries...

#include <stdio.h>
#include <strings.h>

#include <fp.h>

#define libsprintf sprintf

#else // use our library...

#include <stdarg.h>
#include <fp.h>

#include <libsprintf.h>

#define BUFLEN			512
#define TRUE			1
#define FALSE			0

static struct format {
	unsigned 		leftJustify : 1;
	unsigned 		forceSign : 1;
	unsigned 		altForm : 1;
	unsigned 		zeroPad : 1;
	unsigned 		havePrecision : 1;
	unsigned 		hSize : 1;
	unsigned 		lSize : 1;
	unsigned 		LSize : 1;
	char			sign;
	char			exponent;
	int				fieldWidth;
	int				precision;
} default_format;

struct LibDecRec {
	decimal	dec;
	/* following fields aren't used by SANE */
	short			min;
	short			dot;
	short			max;
};

typedef struct LibDecRec LibDecRec;

void libf2d(int, int, struct LibDecRec *, long double);
static void post_conversion(LibDecRec *d);
char* libmemcpy(char* to,char* from,long size);

char libstrlen(char* str){
	char len=0;
	
	if (*str!=0){
		do {
			len++;
			str++;
		} while (*str!=0);
	}
	
	return len;
}

char* libstrncpy(char* to,char* from,short num);
char* libstrncpy(char* to,char* from,short num){
	
	BlockMoveData((Ptr)from,(Ptr)to,num);
	
	return to;
}

void * libmemchr(const void *s, int c,long n){
	register char* cp=(char*)s;
	register unsigned char ch=(unsigned char)c;
	register long sz=n;
	register long ct=0;
	
	while ((*cp!=ch)&&(*cp)&&(ct<sz)){
		cp++;
		ct++;
	}
	
	if ((ct==sz)&&(*cp!=ch))
		return (void*)0;
	
	return (void*)cp;
}

void libfixnl(char* buf){
	register char* cp=buf;
	
	if ((cp==(char*)0)||(*cp==0))
		return;
	
	do {
		if (*cp=='\n')
			*cp=0x06;
	} while (*cp!=0);
}

char* libmemcpy(char* to,char* from,long size){
	BlockMoveData((Ptr)from,(Ptr)to,size);
	
	return to;
}

int libsprintf(char *s, const char *fmt, ...){
	return(libvsprintf(s, fmt, __va(fmt)));
}

int libvsprintf(char *fp,const char *fmt, va_list arg){
	register int c, i, j, nwritten = 0;
	register unsigned long n;
	long double x;
	register char *s;
	char buf[BUFLEN], *digits, *t;
	struct format F;
	LibDecRec D;
	
	for (c = *fmt; c; c = *++fmt) {
		if (c != '%')
			goto copy1;
		F = default_format;
		
			/*  decode flags  */
			
		for (;;) {
			c = *++fmt;
			if (c == '-')
				F.leftJustify = TRUE;
			else if (c == '+')
				F.forceSign = TRUE;
			else if (c == ' ')
				F.sign = ' ';
			else if (c == '#')
				F.altForm = TRUE;
			else if (c == '0')
				F.zeroPad = TRUE;
			else
				break;
		}
		
			/*  decode field width  */
			
		if (c == '*') {
			if ((F.fieldWidth = va_arg(arg, int)) < 0) {
				F.leftJustify = TRUE;
				F.fieldWidth = -F.fieldWidth;
			}
			c = *++fmt;
		}
		else {
			for (; c >= '0' && c <= '9'; c = *++fmt)
				F.fieldWidth = (10 * F.fieldWidth) + (c - '0');
		}
		
			/*  decode precision  */
			
		if (c == '.') {
			if ((c = *++fmt) == '*') {
				F.precision = va_arg(arg, int);
				c = *++fmt;
			}
			else {
				for (; c >= '0' && c <= '9'; c = *++fmt)
					F.precision = (10 * F.precision) + (c - '0');
			}
			if (F.precision >= 0)
				F.havePrecision = TRUE;
		}
		
			/*  perform appropriate conversion  */
		
		s = &buf[BUFLEN];
		if (F.leftJustify)
			F.zeroPad = FALSE;
conv:	switch (c) {
				
				/*  'h' size modifier  */
				
			case 'h':
				F.hSize = TRUE;
				c = *++fmt;
				goto conv;
				
				/*  'l' size modifier  */
				
			case 'l':
				F.lSize = TRUE;
				c = *++fmt;
				goto conv;
						
				/*  'L' size modifier  */
				
			case 'L':
				F.LSize = TRUE;
				c = *++fmt;
				goto conv;
		
				/*  decimal (signed)  */
				
			case 'd':
			case 'i':
				if (F.lSize)
					n = va_arg(arg, long);
				else
					n = va_arg(arg, int);
				if (F.hSize)
					n = (short) n;
				if ((long) n < 0) {
					n = -n;
					F.sign = '-';
				}
				else if (F.forceSign)
					F.sign = '+';
				goto decimal;
				
				/*  decimal (unsigned)  */
				
			case 'u':
				if (F.lSize)
					n = va_arg(arg, unsigned long);
				else
					n = va_arg(arg, unsigned int);
				if (F.hSize)
					n = (unsigned short) n;
				F.sign = 0;
				goto decimal;
				
				/*  decimal (common code)  */

			decimal:
				if (!F.havePrecision) {
					if (F.zeroPad) {
						F.precision = F.fieldWidth;
						if (F.sign)
							--F.precision;
					}
					if (F.precision < 1)
						F.precision = 1;
				}
				for (i = 0; n; n /= 10, i++)
					*--s = n % 10 + '0';
				for (; i < F.precision; i++)
					*--s = '0';
				if (F.sign) {
					*--s = F.sign;
					i++;
				}
				break;
				
				/*  octal (unsigned)  */
				
			case 'o':
				if (F.lSize)
					n = va_arg(arg, unsigned long);
				else
					n = va_arg(arg, unsigned int);
				if (F.hSize)
					n = (unsigned short) n;
				if (!F.havePrecision) {
					if (F.zeroPad)
						F.precision = F.fieldWidth;
					if (F.precision < 1)
						F.precision = 1;
				}
				for (i = 0; n; n /= 8, i++)
					*--s = n % 8 + '0';
				if (F.altForm && i && *s != '0') {
					*--s = '0';
					i++;
				}
				for (; i < F.precision; i++)
					*--s = '0';
				break;
				
				/*  hexadecimal (unsigned)  */
				
			case 'p':
				F.havePrecision = F.lSize = TRUE;
				F.precision = 8;
				/* ... */
			case 'X':
				digits = "0123456789ABCDEF";
				goto hexadecimal;
			case 'x':
				digits = "0123456789abcdef";
				/* ... */
			hexadecimal:
				if (F.lSize)
					n = va_arg(arg, unsigned long);
				else
					n = va_arg(arg, unsigned int);
				if (F.hSize)
					n = (unsigned short) n;
				if (!F.havePrecision) {
					if (F.zeroPad) {
						F.precision = F.fieldWidth;
						if (F.altForm)
							F.precision -= 2;
					}
					if (F.precision < 1)
						F.precision = 1;
				}
				for (i = 0; n; n /= 16, i++)
					*--s = digits[n % 16];
				for (; i < F.precision; i++)
					*--s = '0';
				if (F.altForm) {
					*--s = c;
					*--s = '0';
					i += 2;
				}
				break;
				
#ifndef _NOFLOATING_
				/*  fixed-point  */

			case 'f':
				if (F.LSize)
					x = va_arg(arg, long double);
				else
					x = va_arg(arg, double);
				if (!F.havePrecision)
					F.precision = 6;
				libf2d(1, F.precision, (LibDecRec*)&D, x);
				D.dot = D.dec.exp + D.dec.sig.length;
				if ((D.min = D.dot) > 1)
					D.min = 1;
				D.max = D.dot + F.precision;
				if (D.max - D.min > 508)
					libmemcpy((char*)&D.dec.sig, "\6>>>>>>", 7);
				goto floating;

				/*  floating-point  */
				
			case 'e':
			case 'E':
				if (F.LSize)
					x = va_arg(arg, long double);
				else
					x = va_arg(arg, double);
				if (!F.havePrecision)
					F.precision = 6;
				F.exponent = c;
				libf2d(0, D.max = F.precision + 1,  (LibDecRec*)&D, x);
				D.min = D.dot = 1;
				D.dec.exp += D.dec.sig.length - 1;
				goto floating;
				
				/*  "general" notation  */
				
			case 'g':
			case 'G':
				if (F.LSize)
					x = va_arg(arg, long double);
				else
					x = va_arg(arg, double);
				if (!F.havePrecision)
					F.precision = 6;
				else if (F.precision == 0)
					F.precision = 1;
				F.exponent = c - 2;
				libf2d(0, D.max = F.precision, (LibDecRec*) &D, x);
				D.min = D.dot = 1;
				D.dec.exp += D.dec.sig.length - 1;
				if (D.dec.exp >= -4 && D.dec.exp < F.precision) {
					F.exponent = 0;
					if ((D.dot += D.dec.exp) < 1)
						D.min = D.dot;
				}
				if (!F.altForm && D.max > D.dec.sig.length) {
					if ((D.max = D.dec.sig.length) < D.dot)
						D.max = D.dot;
				}
				goto floating;
				
					/*  floating (common code)  */

			floating:
				if (D.dec.sig.text[0] > '9') {
					F.exponent = FALSE;
					D.dot = 0;
					D.min = 1;
					D.max = D.dec.sig.length;
				}
				i = 0;
				if (F.exponent) {
					n = D.dec.exp < 0 ? -D.dec.exp : D.dec.exp;
					for (; n; n /= 10, i++)
						*--s = n % 10 + '0';
					for (; i < 2; i++)
						*--s = '0';
					*--s = D.dec.exp < 0 ? '-' : '+';
					*--s = F.exponent;
					i += 2;
				}
				j = D.max;
				if (j == D.dot && !F.altForm)
					++D.dot;
				do {
					if (j == D.dot) {
						*--s = '.';
						i++;
					}
					*--s = j > 0 && j <= D.dec.sig.length ? D.dec.sig.text[j-1] : '0';
				} while (--j >= D.min);
				i += D.max - j;
				if (D.dec.sgn)
					F.sign = '-';
				else if (F.forceSign)
					F.sign = '+';
				if (F.zeroPad) {
					j = F.fieldWidth;
					if (F.sign)
						--j;
					for (; i < j; i++)
						*--s = '0';
				}
				if (F.sign) {
					*--s = F.sign;
					i++;
				}
				break;
#endif /* _NOFLOATING_ */

				/*  character  */
				
			case 'c':
				*--s = va_arg(arg, int);
				i = 1;
				break;
				
				/*  string  */
				
			case 's':
				s = va_arg(arg, char *);
				if (F.altForm) {
					i = (unsigned char) *s++;
					if (F.havePrecision && i > F.precision)
						i = F.precision;
				}
				else {
					if (!F.havePrecision)
						i = libstrlen(s);
					else if ((t = (char *)libmemchr(s, '\0', F.precision)) != NULL)
						i = t - s;
					else
						i = F.precision;
				}
				break;
				
				/*  store # bytes written so far  */
				
			case 'n':
				s = (char *)va_arg(arg, void *);
				if (F.hSize)
					* (short *) s = nwritten;
				else if (F.lSize)
					* (long *) s = nwritten;
				else
					* (int *) s = nwritten;
				continue;
			
				/*  oops - unknown conversion, abort  */
				
			default:
				if (c != '%')
					goto done;
			copy1:
				*fp=c;fp++;
				++nwritten;
				continue;
		}
			
			/*  pad on the left  */
			
		if (i < F.fieldWidth && !F.leftJustify) {
			do {
				*fp=' ';fp++;
				++nwritten;
			} while (i < --F.fieldWidth);
		}
		
			/*  write the converted result  */
		
		libstrncpy(fp,s,i);
		fp+=i;
		
		nwritten += i;
			
			/*  pad on the right  */
			
		for (; i < F.fieldWidth; i++) {
			*fp=' ';
			fp++;
			++nwritten;
		}
	}
	
		/*  all done!  */
		
done:
	return(nwritten);
}

#ifndef _NOFLOATING_

static void post_conversion(LibDecRec *d){
	int i;
	/*  strip trailing zeroes  */

	for (i = d->dec.sig.length; i > 1 && d->dec.sig.text[i-1] == '0'; --i)
		++d->dec.exp;
	d->dec.sig.length = i;
		
		/*  format 0, INF, NAN  */
		
	if (d->dec.sig.text[0] == '0') {
/*		d->dec.sgn = 0;			 add back to disallow printing "-0"  */
		d->dec.exp = 0;
	}
	else if (d->dec.sig.text[0] == 'I') {
		d->dec.sig.length = 3;
		d->dec.sig.text[1] = 'N';
		d->dec.sig.text[2] = 'F';
	}
	else if (d->dec.sig.text[0] == 'N') {
		d->dec.sig.length = 5;
		d->dec.sig.text[1] = 'A';
		d->dec.sig.text[2] = 'N';
	}
}

/* ---------- floating-point conversion ---------- */

#if powerc
static void libf2d( int fixed, int precision, LibDecRec *d, long double x){
	struct decform form;
	
	if (precision >= sizeof d->dec.sig)
		precision = sizeof d->dec.sig - 1;

	if (fixed)
		form.style = FIXEDDECIMAL;
	else
		form.style = FLOATDECIMAL;
	form.digits = precision;
	/*
		Until the long double data type is really implemented, this must be a call
		to num2dec and not num2decl even though the argument is declared as a long
		double 
	*/
	
	num2dec( &form, x, &(d->dec) );
	
		/*  handle fixed-point overflow  */
		
	if (d->dec.sig.text[0] == '?') {
		form.style = FLOATDECIMAL;
		form.digits = sizeof d->dec.sig - 1;
		num2dec( &form, x, &(d->dec));
	}
	post_conversion(d);
} 
#else

static void libf2d(int fixed, int precision, LibDecRec *d, long double x){
	struct { char style; short digits; } form;
	register short *p = (short *) &x;
	
		/*  point to SANE extended  */

	#if __option(mc68881) && _NATIVE_FP_
		p[1] = p[0];
	#endif
	#if __option(mc68881) || !_NATIVE_FP_
		p++;
	#endif
	
		/*  convert to decimal record  */
	
	if (precision >= sizeof d->dec.sig)
		precision = sizeof d->dec.sig - 1;
	form.style = fixed;
	form.digits = precision;
	fp68k(&form, p, d, FX2DEC);
	
		/*  handle fixed-point overflow  */
		
	if (d->dec.sig.text[0] == '?') {
		form.style = 0;
		form.digits = sizeof d->dec.sig - 1;
		fp68k(&form, p, d, FX2DEC);
	}
	
	post_conversion(d);	

}
#endif
#endif
#endif