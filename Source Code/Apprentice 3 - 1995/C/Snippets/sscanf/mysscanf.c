/************************************************************************
*                                                                       *
* sscanf.c                                                              *
*                                                                       *
* Source code for a freely distributable sscanf() function.             *
*                                                                       *
* ********************************************************************* *
*                                                                       *
* Copyright (C) 1992, by Brent Burton                                   *
*                                                                       *
* Permission is granted for free use of this code for any purpose it is *
* needed.  No strings attached.  However, out of courtesy, if you do    *
* use this code, I'd appreciate a quick mention.  Enjoy!                *
*                                                                       *
* New news:                                                             *
* If you come across bugs/undocumented features when you use this, I'd  *
* appreciate a note explaining the error so I can fix it.  This new     *
* version has gone through more testing, and I believe it returns the   *
* same values as Think C's.  Actually, for all the various cases I      *
* tested, it now works properly.                                        *
* I officially do have all types but generic character sets implemented.*
* In the last version I didn't implement character scan types.  They    *
* are now.                                                              *
*                                                                       *
* Stepan Riha rewrote the floating point conversions and now the sscanf *
* is *much* quicker than the original - instead of 3 times slower.      *
* I have Stepan's permission to redistribute his modifications.         *
*                                                                       *
* Brent Burton, brentb@tamsun.tamu.edu    12/10/92                      *
*                                                                       *
* ********************************************************************* *
*                                                                       *
* What is the benefit to using this routine over, say, the original     *
* routines?  Well, if you want your conversions to be faster, then      *
* use this routine; here are some numbers to grok:                      *
*                                                                       *
*  (all number comparisons were done by scanning 10000 items on         *
*   a Mac IIsi; all times given are in seconds.)                        *
*                                                                       *
*  result = sscanf( s, f, ...)                                          *
*                                                                       *
*   s             f             Think C time      Our time              *
*  ---------------------------------------------------------            *
*  0123456789     %s                 2.60             0.83              *
*  0123456789     %10s               2.30             0.88              *
*  40 A's         %s                 6.32             1.75              *
*  40 A's         %40s               5.65             1.97              *
*     1           %d                 1.72             0.75              *
*     1.1         %d.%d              2.80             1.48              *
*   12345         %d                 2.48             1.42              *
*   12345         %ld                2.53             1.45              *
*   12345         %lx                2.53             2.05              *
*  ffffffff       %lx                3.30             3.07 *            *
*    1.23         %f                 8.02             3.45              *
*    123          %f                 7.30             3.10              *
*    123e-2       %f                 8.67             4.10              *
*    123e2        %f                 7.20             4.10              *
*   1.234e56      %f                11.63             6.95              *
*   1.234e123     %f                16.02             7.95              *
*   1.234e123     %lf (double)      13.48             7.82              *
*   1.234e-123    %lf               18.12             7.80              *
*                                                                       *
*  * This is the worst case input string for hex numbers.               *
*    (FFFFFFFF can be slightly better than ffffffff.)                   *
*                                                                       *
* ********************************************************************* *
*                                                                       *
* Here are the type specifiers that this sscanf() recognizes:           *
*                                                                       *
* Format spec =  "%[*][width][size][type]"                              *
* The meaning is:                                                       *
*                                                                       *
*  * - process the next item as usual, but don't save.  No argument is  *
*      passed to sscanf() for this specifier.  Optional.                *
*                                                                       *
*  width - This series of digits specifies the absolute maximum number  *
*          of characters to be read for this item specifier.  Fewer     *
*          characters may be read, but no more.  Optional.              *
*                                                                       *
*  size - This value may be only one of:  l,L,h, or #.  This notes      *
*         the size of the data type to use.  "l" (lower case L) marks   *
*         a longsize (long) C type, "h" marks a halfsize (short) type,  *
*         and the "#" notation is explained below.  If no size spec is  *
*         given, the default C type is assumed.  For floating point     *
*         numbers, "l" is double, h is short double, and L long double. *
*                                                                       *
*  type - This is the actual data type to scan.  Valid types are:       *
*       d - decimal (signed or not).                                    *
*       u - unsigned decimal (any sign seen in input is ignored and     *
*           the value is stored as unsigned).                           *
*       o - octal.  Octal digits (0-7) are read and stored.             *
*       x - hexadecimal.  Hex digits (ex:  beef) are read and stored.   *
*           Note:  no preceding "0x" is assumed.                        *
*       c - Read a character, or read [width] characters.  The char-    *
*           acter(s) read are stored into the array pointed to by the   *
*           corresponding (char*) argument.  NOTE: you may use the      *
*           "%4c" to read in a file type, rsrc type, creator, etc.  You *
*           may also store it in a long:                                *
*              OSType type;                                             *
*              char s="APPLications";                                   *
*              sscanf(s, "%4c", &foo);                                  *
*       s - read characters and store in the string pointer passed to   *
*           sscanf().  Whitespace terminates a string input, unless a   *
*           width value was given.  Then sscanf() reads as many as      *
*           possible up to the width limit.                             *
*           For compatibility (for the most part) with Symantec's       *
*           implementation, I added the '#' specifier for reading       *
*           Pascal strings (actually, storing P strings).               *
*           Note:  Since the first byte (offset zero) of a pascal       *
*           string is the length, the string length is limited to 255.  *
*           ex: Read a pascal string like:                              *
*               int numWhat;                                            *
*               Str255 what;  // or char what[STR_SIZE] or ...          *
*               sscanf(" 49 dogs", "%d %#s", &numWhat, (char*)what);    *
*       f - floating point. "lf" is double type, "hf" is "short float". *
*           Other type spec variants (eEgG) are treated the same.       *
*       % - The format specifier sequence "%%" means match one % in the *
*           input string.                                               *
*                                                                       *
************************************************************************/


#include "sscanf.h"

static int c_value(char c, char spec);

/*********************************/
/* Macros sscanf() needs.        */
/* Note:  I changed the name     */
/* of the isxxxxx() macros       */
/* to Isxxxxx() to differentiate */
/* mine from the Std. C.         */
/*********************************/

#define Isdigit(c)   (( '0' <= (c)) && ( (c) <= '9'))  /* dec digit */
#define Isodigit(c)   (( '0' <= (c)) && ( (c) <= '7'))  /* octal digit */
#define Isxdigit(c)   ( (( '0' <= (c)) && ( (c) <= '9')) ||\
            (( 'a' <= (c)) && ( (c) <= 'f')) ||\
            (( 'A' <= (c)) && ( (c) <= 'F')) )  /* hex digit */
#define Isspace(a)   (( (a) == ' ') || ( (a) == '\t'))

#define skip_spc(a)   while( Isspace(*a) ) (a)++;

#define TRUE 1
#define FALSE 0
#define EOF -1

#define REALLY_LONG -1		/* used to read any number of chars */

/*******************************************************************/
/* If you do not want the floating point conversions compiled in,  */
/* then uncomment the following line.                              */
/*******************************************************************/

/* #define NO_FLOATING   /* add "/*" at beginning of line for fpt. */

#ifndef NO_FLOATING
static double pow10(int i);  /* returns 10 raised to the i power */
#endif

/****************************/
/* Variable arguments stuff */
/*******************************************************************/
/* This is a modified version of the varargs definition from the   */
/* Think C's "stdarg.h" header.  I changed the name of them all    */
/* and deleted their va_end() since it was totally useless.        */
/*                                                                 */
/* VA_next returns the next value passed as an argument.  Yes, all */
/* that typecasting is necessary to get around C's lvalue stuff.   */
/* I'll be safe and say these macros are Copyright (C) Symantec.   */
/*******************************************************************/

typedef void *VA_list;

#define VA_start(ap, arg1)    ap = &arg1 + 1
#define VA_next(ap, type)    *(* (type **) &ap)++


/********************************************************************/

#ifdef COMPARING  /* if comparing this to the original, rename this one */
int mysscanf(const char *str, const char *fmt, ...)
#else
int   sscanf(const char *str, const char *fmt, ...)
#endif
{
  register char   *s=(char*)str;  /* the pointers we manipulate */
  register char   *f=(char*)fmt;
  VA_list      ap;                /* the argument pointer */
  int nstored=0;                  /* number of items we've scanned */
  int width = REALLY_LONG;        /* "as many as needed" */
  int base = 10;                  /* default base for numbers */
  char longsize = FALSE,          /* flags for var sizes, */
       longdbl  = FALSE,          /* for long double type */
       halfsize = FALSE,
       ignored  = FALSE,          /* and if we save result or not */
       pasc     = FALSE,          /* "pasc" means a pascal string. */
       sign     = FALSE,
       c_spec   = FALSE;          /* was a "%c" type seen? */
  char item_spec;                 /* type of item to read in */
    
  
  VA_start(ap, fmt);              /* set ap to 1st arg after fmt */
  
  if (!*f) return 0;
  if (!*s) return EOF;

  /********************** start of all code **********************/

  while ( *f && *s)          /* while we have chars to scan */
  {
    skip_spc(s);             /* skip any whitespace we may see */
    skip_spc(f);
    
    /** if we run out of the input string, return EOF **/

    if ( *f && !*s) return (nstored ? nstored:EOF);
    
    if ( *f == '%')          /* beginning of format specifier */
    {
      char c;
      
      c = *++f;              /* read next char */
      
      if ( c == '*' )
      {
        ignored = TRUE;      /* scan but ignore this item */
        c = *++f;            /* read next char */
      }
      
      if ( Isdigit(c) )      /* was width specified? */
      {
        width = 0;
        while (Isdigit(c))
        {
          width = width*10 + (c - '0');
          c = *++f;          /* read next char */
        } /* while */
      } /* if */
      
      f--;                   /* backup; we increment it below */
      
nextchar:
      if ( !*f ) return nstored;  /* end of string or format */
      
      switch ( c = *++f ) {
        case '%':                /* we must match a percent */
            if (*s != '%')
              return ( !*s) ? EOF : nstored;    /* error - exit */
            s++;             /* we matched it, so continue */
            break;
  
        case 'l':            /* longsize */
            longsize = TRUE;
            longdbl = halfsize = pasc = FALSE;
            goto nextchar;

        case 'h':            /* halfsize */
            halfsize = TRUE;
            longdbl = longsize = pasc = FALSE;
            goto nextchar;
            
        case '#':            /* pascal string spec */
            pasc = TRUE;
            longdbl = halfsize = longsize = FALSE;
            goto nextchar;
  
        case 'd':            /* decimal */
            item_spec = 'd';
            base = 10;
            goto scan_int;
            
        case 'u':            /* unsigned int */
            item_spec = 'u';
            base = 10;
            goto scan_int;
  
        case 'o':            /* octal */
            item_spec = 'o';
            base = 8;
            goto scan_int;
  
        case 'x':            /* hex */
            item_spec = 'x';
            base = 16;
            goto scan_int;

        case 'c':  c_spec = TRUE;     /* save raw chars - not a string */
        case 's':                     /* C or Pascal string */
            {
              char *sp, *p_sp;        /* string ptr, pascal ptr */
              int count=0;
              
              p_sp = sp = VA_next( ap, char*);
              if (sp == (void*)0) return nstored; /* error if NULL */
              
              if (pasc && !c_spec) sp++; /* move past length byte */
              
              if (width == REALLY_LONG)  /* read what we need to */
                while ( *s && !Isspace(*s) )
                {
                  count++;
                  if ( ignored)
                    s++;              /* don't copy if ignored */
                  else
                    *sp++ = *s++;
                }
              else                    /* else read # of chars specified */
              {
                while ( *s && (count++ < width))
                  if (ignored)
                    s++;              /* don't copy if ignored */
                  else
                    *sp++ = *s++;
              } /* if else */
              if (pasc )              /* store as Pascal string */
                *p_sp = (char)count;  /* store length */
              else                    /* store as C string */
                if (!c_spec)          /*   if not a "%c" type.... */
                  *sp = '\000';       /*     terminate new string */
              f++;
              if (!ignored) nstored++;
              c_spec = pasc = FALSE;
            }
            break;

#ifndef NO_FLOATING  
/****************************************/
/*	Optimized floating point scaning    */
/*  Written by Stepan Riha 10/15/92     */
/****************************************/

        case 'L':            /* long double */
            longdbl = TRUE;
            longsize = halfsize = pasc = FALSE;
            goto nextchar;

        case 'e':
        case 'E':
        case 'f':            /* float */
        case 'g':
        case 'G':
         {
			register char	*p;
			register long double sum=0.0;
			register int	sign = 1, exponent = 0, expsign = 1;

		 	if(*s == '-'){			/* get the sign, if any */
				sign = -1;
				s++;
				}
			else if(*s == '+')
				s++;
			p = s;					/* get the integral part of number, if any */
			while(*s>='0' && *s<='9')
				sum = sum*10.0 + *s++ - '0';
			if(*s == '.'){			/* get the fractional part of the number, if any */
				double	factor = 1;
				s++;
				while(*s>='0' && *s<='9'){
					factor *= 10;
					sum += (*s++ -'0')/factor;
					}
				if(s==p+1)			/* there were no digits around the '.' */
					goto failure;
				}
			if(s==p)				/* not a valid number */
				goto failure;
			if(sign == -1)			/* adjust for sign */
				sum = -sum;
			if(*s=='e' || *s=='E'){	/* get the expornent, if any */
				s++;
				if(*s == '-'){		/* get the sign of the exponent, if any */
					expsign = -1;
					s++;
					}
				else if(*s == '+')
					s++;
				p = s;				/* get the number of the exponent */
				while(*s>='0' && *s<='9')
					exponent = exponent*10 + *s++ - '0';
				if(s==p)			/* not a valid number */
					goto failure;
				exponent *= expsign;			/* adjust exponent for sign */
				}
			if(exponent)					/* adjust for exponent */
				sum *= pow10(exponent);
		
       /*** Store the flt-pt number in the correct size ***/
            if (!ignored)
            {
              if (longdbl)
              {  long double *ld;
                ld = VA_next(ap, long double*);
                if (ld == (void*)0) return nstored;
                *ld = (long double)sum;
              }
              else if (longsize)
              {  double *dp;
                dp = VA_next(ap, double*);
                if (dp == (void*)0) return nstored;
                *dp = (double)sum;
              }
              else if (halfsize)
              {  short double *sd;
                sd = VA_next(ap, short double*);
                if (sd == (void*)0) return nstored;
                *sd = (short double)sum;
              }
              else  /* normal floating pt */
              {  float *ip;
                ip = VA_next(ap, float*);
                if (ip == (void*)0) return nstored;
                *ip = (float)sum;
              }
              nstored++;
            } /* if !ignored */
            f++;              /* get next format char */

          }  /* scan a floating pt num in */
          break;
failure:
		if(nstored)
			return nstored;
		else
			return ((*s) ? 0 : EOF);
		return nstored;	
		break;
#endif  /* NO_FLOATING */
            
        default:            /* unknown item_spec */
          return nstored;
            break;

      } /* switch */
      
      goto reset;           /* reset the size and other flags */
    }
    else    /* we didn't read a format spec, but instead a string to match */
    {
      /*
       * As long as the strings match, they aren't whitespace, and
       * we have chars to left to check (*f and *s are not '\000'),
       * keep checking.
       */
      while ( !(Isspace(*f) || Isspace(*s)) && *f && *s && *f!='%')
      {  if (*f == *s)
        {
          f++; s++;
        }
        else
          return nstored;  /* we found a difference - return */
      } /* while */
      if (!*f)		/* why did it terminate? */
        return nstored;
      if (!*s)
        return EOF;
      
      /* since we exited the comparison loop, we found a difference */
      goto reset;
    } /* else */

scan_int: /*
           * Once we get a 'd', 'u', 'x', or 'o' item spec, we
           * come here to actually scan the number and convert it
           * to the correctly sized data type (long, short or normal).
           */
    { /* scan_int block */
      int count=width;
      long sum=0;
      
      if ( *s == '-')
      {
        if ( item_spec == 'd')
          { sign=TRUE; s++;}
        else
          { sign=FALSE; s++;}  /* else ignore sign for all else */
      }
      else
        if ( *s == '+') { sign=FALSE; s++;}
      
      if ( !Isxdigit(*s))	/* if it's not *any* type of digit */
        return nstored;

      /*
       * Next read & convert the actual number.  This is
       * split up into two parts to optimize for speed.
       * Since for hex numbers we have to check extra
       * characters (a-f), it takes more time and it's done
       * differently.
       */
      if (item_spec != 'x')  /* if 'd','u', or 'o' */
      {
        if (count == REALLY_LONG)  /* read what we need to */
          while (Isdigit(*s) && !Isspace(*s))
            sum = sum*base + (*s++ -'0');
        else                       /* else only scan count characters */
          while ( Isdigit(*s) && !Isspace(*s)
              && count-- > 0)
            sum = sum*base + (*s++ -'0');
      }
      else  /* for 'x' */
      {
        if (count == REALLY_LONG)  /* read what we need to */
          while ( Isxdigit(*s) && !Isspace(*s) )
            sum = sum*base + c_value(*s++, item_spec);
        else                       /* else only scan count characters */
          while ( Isxdigit(*s) && !Isspace(*s) &&
              count-- > 0)
            sum = sum*base + c_value(*s++, item_spec);
      }

      if (sign) sum = -sum;  /* negate if sign exists */
      
      /*
       * Now the number is converted with sign and all; we
       * need to determine the data type to save it as and
       * then write it to that memory location.
       */
      if (!ignored)
      {
        if (longsize)
        {  long int *ip;
          ip = VA_next(ap, long int*);
          if (ip == (void*)0) return nstored;
          *ip = (long int) sum;
        }
        else if (halfsize)
        {  short int *ip;
          ip = VA_next(ap, short int*);
          if (ip == (void*)0) return nstored;
          *ip = (short int) sum;
        }
        else  /* normal ints */
        {  int *ip;
          ip = VA_next(ap, int*);
          if (ip == (void*)0) return nstored;
          *ip = (int) sum;
        }
        nstored++;
      } /* if !ignored */
      f++;              /* get next format char */
    } /* scan_int block */


reset:
    /*----------------------------------------
    ** At this point, we've either matched a string in str and fmt
    ** or we correctly scanned an item in from str.
    ** We now continue, looking for another item or chars to match.
    **  (i.e., continue with while loop )
    ** We also need to reset our flags...
    **----------------------------------------*/
     c_spec = ignored = halfsize = longdbl =
            longsize = pasc = sign = FALSE;
     base = 10;
     width = REALLY_LONG;
    
  } /* while() */
  
  return nstored;
} /* sscanf() */



/*
 * Given the item specifier, what is the number value of the
 * character in c?
 */
 
static int c_value( char c, char spec)
{

  if (spec == 'x')
  {
    switch (c) {    /* simple, but does the job -- 2.75 secs */
      case 'A':
      case 'a': return 10;
      case 'B':
      case 'b': return 11;
      case 'C':
      case 'c': return 12;
      case 'D':
      case 'd': return 13;
      case 'E':
      case 'e': return 14;
      case 'F':
      case 'f': return 15;
      default:  return c-'0';    /* for digits */
    } /* switch */
  }
  else
    return ( c-'0');  /* for 'd', 'u', and 'o' specs */
} /* c_value() */



#ifndef NO_FLOATING
/*
	pow10	returns the ith power of 10
	Written by Stepan Riha 10/15/92
*/

static double pow10(register int i)
{
	register double result = 1.0, factor;

	if (i == 0) return result;
	
	if (i <  0){
		factor = 0.1;
		i = -i;
		}
	else
		factor = 10.0;

	goto multiply;
	
	while(i){
		factor *= factor;
multiply:
		if(i&0x1L)
			result *= factor;
		i>>=1;
		}
  return result;
} /* pow10() */

#endif	/* NO_FLOATING */

