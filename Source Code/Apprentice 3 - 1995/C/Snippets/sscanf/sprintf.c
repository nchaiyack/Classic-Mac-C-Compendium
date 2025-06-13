/*
**
** DOES NOT DO FLOATING POINT!!!! (%f and %g and %e).
**
** Written by Tim Endres, time@bomb.ice.com
** With permission to distribute.
**
** -brent
*/

#define BUFSIZE 256

typedef struct
  {
   char buffer[BUFSIZE];
   int count;
  } bufrec;

int sputc();   

sprintf(str, fmt, args)
char *str;
char   *fmt;
char   *args;
  {
   bufrec br;
  
    br.buffer[0] = '\0';
    br.count = 1;
   
    _doprnt(fmt, &args, sputc, &br);
    
    strncpy(str, &br.buffer[1], br.count - 1);
    str[br.count - 1] = '\0';
    return (br.count - 1);
  }

sputc(br, ch)
bufrec *br;
char ch;
   {
   br->buffer[(br->count)++] = ch;
 }

#define MAX_DIGITS     16

#define MAX_INTEGER       2147483647
#define INTEGER_HIBIT 0x80000000

#define isdigit(x)      ((x) - '0' >= 0 && (x) - '0' <= 9)
#define tonumber(x)     ((x) - '0')
#define todigit(x)      ((x) + '0')
                                       

#define max(a,b) ((a) > (b)? (a): (b))
#define min(a,b) ((a) < (b)? (a): (b))

int     _doprnt(format, args, func, farg)
char    *format;
char    *args;
int     (*func)();
int     *farg;
    {
        char *cp;
        char *bp, *p;
        int  width, prec;
        char fcode;

        int   lzero;
        short length;
        short fplus;
        short fminus;  
        short fblank;  
        short fsharp;
        short fzero;

        char *prefix;
                    
        char buf[MAX_DIGITS];
        long val;                
                                    
        char *tab;                     
        int n, hradix, lowbit;

     char PlusStr[4];
        char MinusStr[4];
       char SpaceStr[4];
       char EmptyStr[4];
       char UpperHex[20];
      char LowerHex[20];
      char UHexPrefix[4];
     char LHexPrefix[4];
     
        long *longptr;

      PlusStr[0] = '+'; PlusStr[1] = '\0';
        MinusStr[0] = '-'; MinusStr[1] = '\0';
      SpaceStr[0] = ' '; SpaceStr[1] = '\0';
      EmptyStr[0] = '\0';
     UHexPrefix[0] = '0'; UHexPrefix[1] = 'X'; UHexPrefix[2] = '\0';
     LHexPrefix[0] = '0'; LHexPrefix[1] = 'x'; LHexPrefix[2] = '\0';
     longptr = (long *)&UpperHex[0];
     *longptr++ = '0123';
        *longptr++ = '4567';
        *longptr++ = '89AB';
        *longptr++ = 'CDEF';
        longptr = (long *)&LowerHex[0];
     *longptr++ = '0123';
        *longptr++ = '4567';
        *longptr++ = '89ab';
        *longptr++ = 'cdef';

        cp = format;

        while (*cp)
         {
           if (*cp != '%')
             {
                /* Ordinary (non-%) character */
                (*func)(farg, *cp++);
                }
         else {
              /*
              ** % character.
             ** Parse the format specification.
              */

              /* Scan the <flags> */
              fzero = fplus = fminus = fblank = fsharp = 0;

           scan:
           switch (*++cp)
              {
               case '+':
                       fplus = 1;
                      goto scan;
              case '-':
                       fminus = 1;
                     goto scan;
              case ' ':
                       fblank = 1;
                     goto scan;
              case '#':
			  fsharp = 1;
			  goto scan;
              case '0':
                       fzero = 1;
                      goto scan;
              }

           /* Scan the field width */
          if (*cp == '*')
             {
               width = *(int *)args;
               args += sizeof(int);
                if (width < 0)
                  {
                   width = -width;
                 fminus = 1;
                 }
               cp++;
               }
           else
                {
               width = 0;
              while (isdigit (*cp))
                   {
                   n = tonumber (*cp++);
                   width = width * 10 + n;
                 }
               }

           /* Scan the precision */
            if (*cp == '.')
             {
               /* '*' instead of digits? */
                if (*++cp == '*')
                   {
                   prec = *(int *)args; args += sizeof(int);
                   cp++;
                   }
               else
                    {
                   prec = 0;
                   while (isdigit (*cp))
                       {
                       n = tonumber (*cp++);
                       prec = prec * 10 + n;
                       }
                   }
               }
           else
                prec = -1;

          /* Scan the length modifier */
          length = 0;
         
            switch (*cp)
                {
               case 'l':
                   length = 1;
                 /* No break */
              case 'h':
                   cp++;
               }

           lzero = 0;
          prefix = EmptyStr;

          switch (fcode = *cp++)
              {
               case 'd':
               case 'u':
                   hradix = 5;
                 goto fixed;
                 
                case 'D':
                   fcode = 'd';
                    length = 1;
                 hradix = 5;
                 goto fixed;
 
                case 'o':
                   hradix = 4;
                 goto fixed;
 
                case 'X':
               case 'x':
                   hradix = 8;
 
                fixed:
                  /* Establish default precision */
                   if (prec < 0)
                       prec = 1;

                   /* Fetch the argument to be printed */
                  if (length)
                     {
                       val = *(long *)args; args += sizeof(long);
                      }
                   else if (fcode == 'd')
                      {
                       val = *(int *)args; args += sizeof(int);
                        }
                   else
                        {
                       val = *(unsigned *)args; args += sizeof(unsigned);
                      }

                   /* If signed conversion, establish sign */
                  if (fcode == 'd')
                       {
                       if (val < 0)
                            {
                           prefix = MinusStr;
                          /*
                          **   Negate, checking in
                            **   advance for possible
                           **   overflow.
                          */
                          if (val != INTEGER_HIBIT)
                               val = -val;
                         }
                       else if (fplus)
                         prefix = PlusStr;
                       else if (fblank)
                            prefix = SpaceStr;
                      }

                   if (fzero)
                      {
                       int n = width - msstrlen (prefix);
                      if (n > prec)
                           prec = n;
                       }

                   /* Set translate table for digits */
                    if (fcode == 'X')
                           tab = UpperHex;
                 else
                            tab = LowerHex;

                 /* Develop the digits of the value */
                   p = bp = buf + MAX_DIGITS;
                  while (val) {
                           lowbit = val & 1;
                           val = (val >> 1) & ~INTEGER_HIBIT;
                          *--bp = tab[val % hradix * 2 + lowbit];
                         val = val / hradix;
                 }

                   /* Calculate padding zero requirement */
                    lzero = bp - p + prec;

                  /* Handle the # flag */
                 if (fsharp && bp != p)
                          switch (fcode) {
                            case 'o':
                                   if (lzero < 1)
                                          lzero = 1;
                                  break;
                          case 'x':
                                   prefix = LHexPrefix;
                                    break;
                          case 'X':
                                   prefix = UHexPrefix;
                                    break;
                          }

                   break;
              case 'c':
                   buf[0] = *(int *)args; args += sizeof(int);
                 bp = &buf[0];
                   p = bp + 1;
                 break;
  
                case 's':
                   bp = *(char **)args; args += sizeof(char **);
                   if (prec < 0)
                       prec = MAX_INTEGER;
                 
                    for (n=0; *bp++ != '\0' && n < prec; n++);
                  
                    p = --bp;
                   bp -= n;
                    break;
  
                case '\0':
                  cp--;
                   break;
  
                default:
                    p = bp = &fcode;
                    p++;
                    break;
  
                }
           
            if (fcode != '\0')
              {
               /* Calculate number of padding blanks */
                int nblank;
             nblank = width
                      - (p - bp)
                      - (lzero < 0 ? 0 : lzero)
                       - msstrlen(prefix);

             /* Blanks on left if required */
                if (! fminus)
                   while (--nblank >= 0)
                       (*func) (farg,' ');

             /* Prefix, if any */
                while (*prefix != '\0')
                 (*func) (farg, *prefix++);

              /* Zeroes on the left */
                while (--lzero >= 0)
                    (*func) (farg, '0');
                
                /* The value itself */
              while (bp < p)
                  (*func) (farg, *bp++);
              
                /* Blanks on the right if required */
               if (fminus)
                 while (--nblank >= 0)
                       (*func) (farg, ' ');
                }
           }
       }

   return 1;
   }

msstrlen(s)  /* msstrlen from K & R */
char *s;
 {
   int n;
  
    for (n=0; *s != '\0'; s++) n++;
 
    return(n);
  }


