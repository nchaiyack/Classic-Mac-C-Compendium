/*
 * cnv.r -- Conversion routines:
 *
 * cnv_c_dbl, cnv_c_int, cnv_c_str, cnv_cset, cnv_ec_int,
 * cnv_eint, cnv_int, cnv_real, cnv_str, cnv_tcset, cnv_tstr, deref
 * strprc, bi_strprc
 *
 * Service routines: itos, ston, radix, cvpos
 *
 * Philosophy: certain redundancy is present which could be avoided,
 * and nested conversion calls are avoided due to the importance of
 * minimizing these routines' costs.
 *
 * Assumed: the C compiler must handle assignments of C integers to
 * C double variables and vice-versa.  Hopefully production C compilers
 * have managed to eliminate bugs related to these assignments.
 */

#if !EBCDIC
#define tonum(c)	(isdigit(c) ? (c)-'0' : 10+(((c)|(040))-'a'))
#endif					/* EBCDIC */

/*
 * Prototypes for static functions.
 */
hidden novalue cstos Params((unsigned int *cs, dptr dp, char *s));
hidden novalue itos  Params((C_integer num, dptr dp, char *s));
hidden int     ston  Params((dptr sp, union numeric *result));

/*
 * cnv_c_dbl - cnv:C_double(*s, *d), convert a value directly into a C double
 */
int cnv_c_dbl(s, d)
dptr s;
double *d;
   {
#ifdef LargeInts
   tended	/* need to be tended if ston allocates largeint blocks */
#endif					/* LargeInts */
     struct descrip result, cnvstr;
     char sbuf[MaxCvtLen];

   union numeric numrc;

#ifdef EventMon
   EVValD(s, E_Aconv);
   EVValD(&rzerodesc, E_Tconv);
#endif					/* EventMon */

   type_case *s of {
      real: {
         GetReal(s, *d);

#ifdef EventMon
         EVValD(s, E_Nconv);
#endif					/* EventMon */

         return 1;
         }
      integer: {
#ifdef LargeInts
         if (Type(*s) == T_Lrgint)
            *d = bigtoreal(s);
         else
#endif					/* LargeInts */
            *d = IntVal(*s);

#ifdef EventMon
            EVValD(s, E_Sconv);
#endif					/* EventMon */

         return 1;
         }
      string: {
         /* fall through */
         }
      cset: {
        tmp_str(sbuf, s, &cnvstr);
        s = &cnvstr;
        }
      default: {

#ifdef EventMon
        EVValD(s, E_Fconv);
#endif					/* EventMon */

        return 0;
        }
      }

   /*
    * s is now an string.
    */
   switch( ston(s, &numrc) ) {
      case T_Integer:
         *d = numrc.integer;

#ifdef EventMon
         EVValD(s, E_Sconv);
#endif					/* EventMon */

         return 1;
#ifdef LargeInts
      case T_Lrgint:
         result.dword = D_Lrgint;
	 BlkLoc(result) = (union block *)numrc.big;
         *d = bigtoreal(&result);

#ifdef EventMon
         EVValD(s, E_Sconv);
#endif					/* EventMon */

         return 1;
#endif					/* LargeInts */
      case T_Real:
         *d = numrc.real;

#ifdef EventMon
         EVValD(s, E_Sconv);
#endif					/* EventMon */
         return 1;
      default:

#ifdef EventMon
         EVValD(s, E_Fconv);
#endif					/* EventMon */

         return 0;
      }
  }

/*
 * cnv_c_int - cnv:C_integer(*s, *d), convert a value directly into a C_integer
 */
int cnv_c_int(s, d)
dptr s;
C_integer *d;
   {
#ifdef LargeInts
   tended  /* tended since ston now allocates blocks */
#endif						/* LargeInts */
      struct descrip cnvstr, result;			/* not tended */
   union numeric numrc;
   char sbuf[MaxCvtLen];

#ifdef EventMon
   EVValD(s, E_Aconv);
   EVValD(&zerodesc, E_Tconv);
#endif					/* EventMon */


   type_case *s of {
      integer: {
#ifdef LargeInts
         if (Type(*s) == T_Lrgint) {

#ifdef EventMon
            EVValD(s, E_Fconv);
#endif					/* EventMon */

            return 0;
            }
#endif					/* LargeInts */
         *d = IntVal(*s);

#ifdef EventMon
         EVValD(s, E_Nconv);
#endif					/* EventMon */

         return 1;
         }
      real: {
         double dbl;
         GetReal(s,dbl);
         if (dbl > MaxLong || dbl < MinLong) {

#ifdef EventMon
            EVValD(s, E_Fconv);
#endif					/* EventMon */

            return 0;
            }
         *d = dbl;

#ifdef EventMon
         EVValD(s, E_Sconv);
#endif					/* EventMon */

         return 1;
         }
      string: {
         /* fall through */
         }
      cset: {
        tmp_str(sbuf, s, &cnvstr);
        s = &cnvstr;
        }
      default: {

#ifdef EventMon
         EVValD(s, E_Fconv);
#endif					/* EventMon */

         return 0;
         }
      }

   /*
    * s is now a string.
    */
   switch( ston(s, &numrc) ) {
      case T_Integer: {
         *d = numrc.integer;

#ifdef EventMon
         EVValD(s, E_Sconv);
#endif					/* EventMon */

         return 1;
	 }
      case T_Real: {
         double dbl = numrc.real;
         if (dbl > MaxLong || dbl < MinLong) {

#ifdef EventMon
            EVValD(s, E_Fconv);
#endif					/* EventMon */

            return 0;
            }
         *d = dbl;

#ifdef EventMon
         EVValD(s, E_Sconv);
#endif					/* EventMon */

         return 1;
         }
      default:

#ifdef EventMon
         EVValD(s, E_Fconv);
#endif					/* EventMon */

         return 0;
      }
   }

/*
 * cnv_c_str - cnv:C_string(*s, *d), convert a value into a C (and Icon) string
 */
int cnv_c_str(s, d)
dptr s;
dptr d;
   {
   /*
    * Get the string to the end of the string region and append a '\0'.
    */
#ifdef FixedRegions
   if (!is:string(*s)) {
      if (!cnv_str(s, d)) {
         return 0;
         }
      }
   else {

#ifdef EventMon
      EVValD(s, E_Aconv);
      EVValD(&emptystr, E_Tconv);
      EVValD(s, E_Nconv);
#endif					/* EventMon */

      *d = *s;
      }

   /*
    * See if the end of d is already at the end of the string region
    * and (in FixedRegions) there is room for one more byte.
    */
   if ((StrLoc(*d) + StrLen(*d) == strfree) && (strfree != strend)) {
      Protect(alcstr("\0", 1), fatalerr(0,NULL));
      ++StrLen(*d);
      }
   else {
      register word slen = StrLen(*d);
      register char *sp, *dp;
      Protect(dp = alcstr(NULL,slen+1), fatalerr(0,NULL));
      StrLen(*d) = StrLen(*d)+1;
      sp = StrLoc(*d);
      StrLoc(*d) = dp;
      while (slen-- > 0)
         *dp++ = *sp++;
      *dp = '\0';
      }

#else						/* FixedRegions */

   if (is:string(*s)) {

#ifdef EventMon
      EVValD(s, E_Aconv);
      EVValD(&emptystr, E_Tconv);
      EVValD(s, E_Nconv);
#endif					/* EventMon */

     /*
       * See if the end of s is already at the end of the string region.
       */
      if (StrLoc(*s) + StrLen(*s) == strfree)
         *d = *s;
      else {
         Protect(StrLoc(*d) = alcstr(StrLoc(*s),StrLen(*s)), fatalerr(0,NULL));
         StrLen(*d) = StrLen(*s);
         }
      }
   else if (!cnv_str(s, d)) {
      return 0;
      }

   Protect(alcstr("\0", 1), fatalerr(0,NULL));
   ++StrLen(*d);

#endif						/* FixedRegions */

   return 1;
   }

/*
 * cnv_cset - cnv:cset(*s, *d), convert to a cset
 */
int cnv_cset(s, d)
dptr s, d;
   {
   tended struct descrip str;
   char sbuf[MaxCvtLen];
   register C_integer l;
   register char *s1;        /* does not need to be tended */

#ifdef EventMon
   EVValD(s, E_Aconv);
   EVValD(&csetdesc, E_Tconv);
#endif					/* EventMon */


   if (is:cset(*s)) {
      *d = *s;

#ifdef EventMon
      EVValD(s, E_Nconv);
#endif					/* EventMon */

      return 1;
      }
   /*
    * convert to a string and then add its contents to the cset
    */
   if (tmp_str(sbuf, s, &str)) {
      Protect(BlkLoc(*d) = (union block *)alccset(), fatalerr(0,NULL));
      d->dword = D_Cset;
      s1 = StrLoc(str);
      l = StrLen(str);
      while(l--) {
         Setb(*s1, *d);
	 s1++;
         }

#ifdef EventMon
         EVValD(d, E_Sconv);
#endif					/* EventMon */

      return 1;
      }
   else {

#ifdef EventMon
      EVValD(s, E_Fconv);
#endif					/* EventMon */

      return 0;
      }
  }

/*
 * cnv_ec_int - cnv:(exact)C_integer(*s, *d), convert to an exact C integer
 */
int cnv_ec_int(s, d)
dptr s;
C_integer *d;
   {
#ifdef LargeInts
   tended  /* tended since ston now allocates blocks */
#endif						/* LargeInts */
      struct descrip cnvstr;			/* not tended */
   union numeric numrc;
   char sbuf[MaxCvtLen];

#ifdef EventMon
   EVValD(s, E_Aconv);
   EVValD(&zerodesc, E_Tconv);
#endif					/* EventMon */


   type_case *s of {
      integer: {
#ifdef LargeInts
         if (Type(*s) == T_Lrgint) {

#ifdef EventMon
            EVValD(s, E_Fconv);
#endif					/* EventMon */

            return 0;
            }
#endif					/* LargeInts */
         *d = IntVal(*s);

#ifdef EventMon
         EVValD(s, E_Nconv);
#endif					/* EventMon */

         return 1;
         }
      string: {
         /* fall through */
         }
      cset: {
        tmp_str(sbuf, s, &cnvstr);
        s = &cnvstr;
        }
      default: {

#ifdef EventMon
         EVValD(s, E_Fconv);
#endif					/* EventMon */

         return 0;
         }
      }

   /*
    * s is now a string.
    */
   if (ston(s, &numrc) == T_Integer) {
      *d = numrc.integer;

#ifdef EventMon
      EVValD(s, E_Sconv);
#endif					/* EventMon */

      return 1;
      }
   else {

#ifdef EventMon
      EVValD(s, E_Fconv);
#endif					/* EventMon */

      return 0;
      }
   }

/*
 * cnv_eint - cnv:(exact)integer(*s, *d), convert to an exact integer
 */
int cnv_eint(s, d)
dptr s, d;
   {
#ifdef LargeInts
   tended  /* tended since ston now allocates blocks */
#endif						/* LargeInts */
      struct descrip cnvstr;			/* not tended */
   char sbuf[MaxCvtLen];
   union numeric numrc;
   int status;

#ifdef EventMon
   EVValD(s, E_Aconv);
   EVValD(&zerodesc, E_Tconv);
#endif					/* EventMon */


   type_case *s of {
      integer: {
         *d = *s;

#ifdef EventMon
         EVValD(s, E_Nconv);
#endif					/* EventMon */

         return 1;
         }
      string: {
         /* fall through */
         }
      cset: {
        tmp_str(sbuf, s, &cnvstr);
        s = &cnvstr;
        }
      default: {

#ifdef EventMon
         EVValD(s, E_Fconv);
#endif					/* EventMon */

        return 0;
        }
      }

   /*
    * s is now a string.
    */
   switch (ston(s, &numrc)) {
      case T_Integer:
         MakeInt(numrc.integer, d);

#ifdef EventMon
         EVValD(s, E_Sconv);
#endif					/* EventMon */

	 return 1;
#ifdef LargeInts
      case T_Lrgint:
         d->dword = D_Lrgint;
	 BlkLoc(*d) = (union block *)numrc.big;

#ifdef EventMon
         EVValD(d, E_Sconv);
#endif					/* EventMon */

         return 1;
#endif				/* LargeInts */
      default:

#ifdef EventMon
         EVValD(s, E_Fconv);
#endif					/* EventMon */

         return 0;
      }
   }

/*
 * cnv_int - cnv:integer(*s, *d), convert to integer
 */
int cnv_int(s, d)
dptr s, d;
   {
#ifdef LargeInts
   tended   /* tended since ston now allocates blocks */
#endif						/* LargeInts */
      struct descrip cnvstr;			/* not tended */
   char sbuf[MaxCvtLen];
   union numeric numrc;

#ifdef EventMon
   EVValD(s, E_Aconv);
   EVValD(&zerodesc, E_Tconv);
#endif					/* EventMon */

   type_case *s of {
      integer: {
         *d = *s;

#ifdef EventMon
         EVValD(s, E_Nconv);
#endif					/* EventMon */

         return 1;
         }
      real: {
         double dbl;
         GetReal(s,dbl);
         if (dbl > MaxLong || dbl < MinLong) {
#ifdef LargeInts
            if (realtobig(s, d) == Succeeded) {

#ifdef EventMon
               EVValD(d, E_Sconv);
#endif					/* EventMon */

               return 1;
               }
            else {

#ifdef EventMon
               EVValD(s, E_Fconv);
#endif					/* EventMon */

               return 0;
               }
#else					/* LargeInts */

#ifdef EventMon
            EVValD(s, E_Fconv);
#endif					/* EventMon */


            return 0;
#endif					/* LargeInts */
	    }
         MakeInt((word)dbl,d);

#ifdef EventMon
         EVValD(d, E_Sconv);
#endif					/* EventMon */

         return 1;
         }
      string: {
         /* fall through */
         }
      cset: {
        tmp_str(sbuf, s, &cnvstr);
        s = &cnvstr;
        }
      default: {

#ifdef EventMon
         EVValD(s, E_Fconv);
#endif					/* EventMon */

        return 0;
        }
      }

   /*
    * s is now a string.
    */
   switch( ston(s, &numrc) ) {
#ifdef LargeInts
      case T_Lrgint:
         d->dword = D_Lrgint;
	 BlkLoc(*d) = (union block *)numrc.big;

#ifdef EventMon
         EVValD(d, E_Sconv);
#endif					/* EventMon */

	 return 1;
#endif					/* LargeInts */
      case T_Integer:
         MakeInt(numrc.integer,d);

#ifdef EventMon
         EVValD(d, E_Sconv);
#endif					/* EventMon */

         return 1;
      case T_Real: {
         double dbl = numrc.real;
         if (dbl > MaxLong || dbl < MinLong) {
#ifdef LargeInts
            if (realtobig(s, d) == Succeeded) {

#ifdef EventMon
               EVValD(d, E_Sconv);
#endif					/* EventMon */

               return 1;
               }
            else {

#ifdef EventMon
               EVValD(s, E_Fconv);
#endif					/* EventMon */

               return 0;
               }
#else					/* LargeInts */

#ifdef EventMon
            EVValD(s, E_Fconv);
#endif					/* EventMon */

            return 0;
#endif					/* LargeInts */
	    }
         MakeInt((word)dbl,d);

#ifdef EventMon
         EVValD(d, E_Sconv);
#endif					/* EventMon */

         return 1;
         }
      default:

#ifdef EventMon
         EVValD(s, E_Fconv);
#endif					/* EventMon */

          return 0;
       }
   }

/*
 * cnv_real - cnv:real(*s, *d), convert to real
 */
int cnv_real(s, d)
dptr s, d;
   {
   double dbl;
   if (cnv_c_dbl(s, &dbl)) {
      Protect(BlkLoc(*d) = (union block *)alcreal(dbl), fatalerr(0,NULL));
      d->dword = D_Real;
      return 1;
      }
   else
      return 0;
   }

/*
 * cnv_str - cnv:string(*s, *d), convert to a string
 */
int cnv_str(s, d)
dptr s, d;
   {
   char sbuf[MaxCvtLen];

#ifdef EventMon
   EVValD(s, E_Aconv);
   EVValD(&emptystr, E_Tconv);
#endif					/* EventMon */


   type_case *s of {
      string: {
         *d = *s;

#ifdef EventMon
         EVValD(s, E_Nconv);
#endif					/* EventMon */

         return 1;
         }
      integer: {
#ifdef LargeInts
         if (Type(*s) == T_Lrgint) {
            word slen;
            word dlen;

            slen = (BlkLoc(*s)->bignumblk.lsd - BlkLoc(*s)->bignumblk.msd +1);
            dlen = slen * NB * 0.3010299956639812;	/* 1 / log2(10) */
	    bigtos(s,d);
	    }
         else
#endif					/* LargeInts */
         itos(IntVal(*s), d, sbuf);
	 }
      real: {
         double res;
         GetReal(s, res);
         rtos(res, d, sbuf);
         }
      cset:
         cstos(BlkLoc(*s)->cset.bits, d, sbuf);
      default: {

#ifdef EventMon
         EVValD(s, E_Fconv);
#endif					/* EventMon */

         return 0;
         }
      }
   Protect(StrLoc(*d) = alcstr(StrLoc(*d), StrLen(*d)), fatalerr(0,NULL));

#ifdef EventMon
   EVValD(d, E_Sconv);
#endif					/* EventMon */

   return 1;
   }

/*
 * cnv_tcset - cnv:tmp_cset(*s, *d), convert to a temporary cset
 */
int cnv_tcset(cbuf, s, d)
struct b_cset *cbuf;
dptr s, d;
   {
   struct descrip tmpstr;
   char sbuf[MaxCvtLen];
   register char *s1;
   C_integer l;

#ifdef EventMon
   EVValD(s, E_Aconv);
   EVValD(&csetdesc, E_Tconv);
#endif					/* EventMon */

   if (is:cset(*s)) {
      *d = *s;

#ifdef EventMon
      EVValD(s, E_Nconv);
#endif					/* EventMon */

      return 1;
      }
   if (tmp_str(sbuf, s, &tmpstr)) {
      for (l = 0; l < CsetSize; l++) 
          cbuf->bits[l] = 0;
      d->dword = D_Cset;
      BlkLoc(*d) = (union block *)cbuf;
      s1 = StrLoc(tmpstr);
      l = StrLen(tmpstr);
      while(l--) {
         Setb(*s1, *d);
	 s1++;
         }

#ifdef EventMon
      EVValD(d, E_Sconv);
#endif					/* EventMon */

      return 1;
      }
   else {

#ifdef EventMon
      EVValD(s, E_Fconv);
#endif					/* EventMon */

      return 0;
      }
   }

/*
 * cnv_tstr - cnv:tmp_string(*s, *d), convert to a temporary string
 */
int cnv_tstr(sbuf, s, d)
char *sbuf;
dptr s;
dptr d;
   {

#ifdef EventMon
   EVValD(s, E_Aconv);
   EVValD(&emptystr, E_Tconv);
#endif					/* EventMon */

   if (is:string(*s)) {
      *d = *s;

#ifdef EventMon
      EVValD(s, E_Nconv);
#endif					/* EventMon */

      return 1;
      }
   else if (tmp_str(sbuf, s, d)) {

#ifdef EventMon
      EVValD(d, E_Sconv);
#endif					/* EventMon */

       return 1;
       }
   else {

#ifdef EventMon
      EVValD(s, E_Fconv);
#endif					/* EventMon */

       return 0;
       }
   }

/*
 * deref - dereference a descriptor.
 */
novalue deref(s, d)
dptr s, d;
   {
   /*
    * no allocation is done, so nothing need be tended.
    */
   register union block *bp;
   struct descrip v;
   register union block **ep;
   int res;

   if (!is:variable(*s)) {
      *d = *s;
      }
   else type_case *s of {
      tvsubs: {
         /*
          * A substring trapped variable is being dereferenced.
          *  Point bp to the trapped variable block and v to
          *  the string.
          */
         bp = BlkLoc(*s);
         deref(&bp->tvsubs.ssvar, &v);
         if (!is:string(v))
            fatalerr(103, &v);
         if (bp->tvsubs.sspos + bp->tvsubs.sslen - 1 > StrLen(v))
            fatalerr(205, NULL);
         /*
          * Make a descriptor for the substring by getting the
          *  length and pointing into the string.
          */
         StrLen(*d) = bp->tvsubs.sslen;
         StrLoc(*d) = StrLoc(v) + bp->tvsubs.sspos - 1;
        }

      tvtbl: {
         bp = BlkLoc(*s);
         if (bp->tvtbl.title == T_Telem) {
            /*
             * The tvtbl has been converted to a telem and is
             *  in the table.  Replace the descriptor pointed to
             *  by d with the value of the element.
             */
             *d = bp->telem.tval;
             }
         else {
	    /*
	     * Look up the element in the table.
	     */
	    ep = memb(bp->tvtbl.clink,&bp->tvtbl.tref,bp->tvtbl.hashnum,&res);
            if (res == 1)
	       *d = (*ep)->telem.tval;			/* found; use value */
            else
               *d = bp->tvtbl.clink->table.defvalue;	/* nope; use default */
            }
        }

      kywdint:
      kywdpos:
      kywdsubj:
      kywdevent:
      kywdwin:
      kywdstr:
         *d = *VarLoc(*s);

      default:
         /*
          * An ordinary variable is being dereferenced.
          */
         *d = *(dptr)((word *)VarLoc(*s) + Offset(*s));
      }
   }

/*
 * tmp_str - Convert to temporary string.
 */
int tmp_str(sbuf, s, d)
char *sbuf;
dptr s;
dptr d;
   {
   type_case *s of {
      string:
         *d = *s;
      integer: {
#ifdef LargeInts
         if (Type(*s) == T_Lrgint) {
            word slen;
            word dlen;

            slen = (BlkLoc(*s)->bignumblk.lsd - BlkLoc(*s)->bignumblk.msd +1);
            dlen = slen * NB * 0.3010299956639812;	/* 1 / log2(10) */
	    bigtos(s,d);
	    }
         else
#endif					/* LargeInts */
         itos(IntVal(*s), d, sbuf);
	 }
      real: {
         double res;
         GetReal(s, res);
         rtos(res, d, sbuf);
         }
      cset:
         cstos(BlkLoc(*s)->cset.bits, d, sbuf);
      default:
         return 0;
      }
   return 1;
   }

/*
 * dp_pnmcmp - do a string comparison of a descriptor to the procedure 
 *   name in a pstrnm struct; used in call to qsearch().
 */
int dp_pnmcmp(pne,dp)
struct pstrnm *pne;
struct descrip *dp;
{
   struct descrip d;
   StrLen(d) = strlen(pne->pstrep);
   StrLoc(d) = pne->pstrep;
   return lexcmp(&d,dp);
}

/*
 * bi_strprc - convert a string to a (built-in) function or operator.
 */
struct b_proc *bi_strprc(s, arity)
dptr s;
C_integer arity;
   {
   C_integer i;
   char *fnc;
   struct pstrnm *pp;

   if (!StrLen(*s))
      return NULL;

   /*
    * See if the string represents an operator. In this case the arity
    *  of the operator must match the one given.
    */
   if (!isalpha(*StrLoc(*s))) {
      for (i = 0; i < op_tbl_sz; ++i)
	 if (eq(s, &op_tbl[i].pname) && (arity == op_tbl[i].nparam ||
					 op_tbl[i].nparam == -1))
	    return &op_tbl[i];
      return NULL;
      }

   /*
    * See if the string represents a built-in function.
    */
#if COMPILER
   for (i = 0; i < n_globals; ++i)
      if (eq(s, &gnames[i]))
	 return builtins[i];  /* may be null */
#else					/* COMPILER */
   pp = (struct pstrnm *)qsearch((char *)s,(char *)pntab,pnsize,
				 sizeof(struct pstrnm),dp_pnmcmp);
   if (pp!=NULL)
      return (struct b_proc *)pp->pblock;
#endif					/* !COMPILER */

   return NULL;
   }

/*
 * strprc - convert a string to a procedure.
 */
struct b_proc *strprc(s, arity)
dptr s;
C_integer arity;
   {
   C_integer i;
   char *fnc;

   /*
    * See if the string is the name of a global variable.
    */
   for (i = 0; i < n_globals; ++i)
      if (eq(s, &gnames[i]))
         if (is:proc(globals[i]))
            return (struct b_proc *)BlkLoc(globals[i]);
         else
            return NULL;

   return bi_strprc(s,arity);
   }

/*
 * Service routines
 */

/*
 * itos - convert the integer num into a string using s as a buffer and
 *  making q a descriptor for the resulting string.
 */

static novalue itos(num, dp, s)
C_integer num;
dptr dp;
char *s;
   {
   register char *p;
   long ival;
   static char *maxneg = MaxNegInt;

   p = s + MaxCvtLen - 1;
   ival = num;

   *p = '\0';
   if (num >= 0L)
      do {
	 *--p = ival % 10L + '0';
	 ival /= 10L;
	 } while (ival != 0L);
   else {
      if (ival == -ival) {      /* max negative value */
	 p -= strlen (maxneg);
	 sprintf (p, "%s", maxneg);
         }
      else {
	ival = -ival;
	do {
	   *--p = '0' + (ival % 10L);
	   ival /= 10L;
	   } while (ival != 0L);
	*--p = '-';
	}
      }

   StrLen(*dp) = s + MaxCvtLen - 1 - p;
   StrLoc(*dp) = p;
   }


/*
 * ston - convert a string to a numeric quantity if possible.
 * Returns a typecode or CvtFail.  Its answer is in the dptr,
 * unless its a double, in which case its in the union numeric
 * (we do this to avoid allocating a block for a real
 * that will later be used directly as a C_double).
 */
static int ston(sp, result)
dptr sp;
union numeric *result;
   {
   register char *s = StrLoc(*sp), *end_s;
   register int c;
   int realflag = 0;	/* indicates a real number */
   char msign = '+';    /* sign of mantissa */
   char esign = '+';    /* sign of exponent */
   double mantissa = 0; /* scaled mantissa with no fractional part */
   long lresult = 0;	/* integer result */
   int scale = 0;	/* number of decimal places to shift mantissa */
   int digits = 0;	/* total number of digits seen */
   int sdigits = 0;	/* number of significant digits seen */
   int exponent = 0;	/* exponent part of real number */
   double fiveto;	/* holds 5^scale */
   double power;	/* holds successive squares of 5 to compute fiveto */
   int err_no;
   char *ssave;         /* holds original ptr for bigradix */

   if (StrLen(*sp) == 0)
      return CvtFail;
   end_s = s + StrLen(*sp);
   c = *s++;

   /*
    * Skip leading white space.
    */
   while (isspace(c))
      if (s < end_s)
         c = *s++;
      else
         return CvtFail;

   /*
    * Check for sign.
    */
   if (c == '+' || c == '-') {
      msign = c;
      c = (s < end_s) ? *s++ : ' ';
      }

   ssave = s - 1;   /* set pointer to beginning of digits in case it's needed */

   /*
    * Get integer part of mantissa.
    */
   while (isdigit(c)) {
      digits++;
      if (mantissa < Big) {
	 mantissa = mantissa * 10 + (c - '0');
         lresult = lresult * 10 + (c - '0');
	 if (mantissa > 0.0)
	    sdigits++;
	 }
      else
	 scale++;
      c = (s < end_s) ? *s++ : ' ';
      }

   /*
    * Check for based integer.
    */
   if (c == 'r' || c == 'R') {
      int rv;
#ifdef LargeInts
      rv = bigradix((int)msign, (int)mantissa, s, end_s, result);
      if (rv == Error)
         fatalerr(0, NULL);
#else					/* LargeInts */
      rv = radix((int)msign, (int)mantissa, s, end_s, result);
#endif					/* LargeInts */
      return rv;
      }

   /*
    * Get fractional part of mantissa.
    */
   if (c == '.') {
      realflag++;
      c = (s < end_s) ? *s++ : ' ';
      while (isdigit(c)) {
	 digits++;
	 if (mantissa < Big) {
	    mantissa = mantissa * 10 + (c - '0');
	    lresult = lresult * 10 + (c - '0');
	    scale--;
	    if (mantissa > 0.0)
	       sdigits++;
	    }
         c = (s < end_s) ? *s++ : ' ';
	 }
      }

   /*
    * Check that at least one digit has been seen so far.
    */
   if (digits == 0)
      return CvtFail;

   /*
    * Get exponent part.
    */
   if (c == 'e' || c == 'E') {
      realflag++;
      c = (s < end_s) ? *s++ : ' ';
      if (c == '+' || c == '-') {
	 esign = c;
         c = (s < end_s) ? *s++ : ' ';
	 }
      if (!isdigit(c))
	 return CvtFail;
      while (isdigit(c)) {
	 exponent = exponent * 10 + (c - '0');
         c = (s < end_s) ? *s++ : ' ';
	 }
      scale += (esign == '+') ? exponent : -exponent;
      }

   /*
    * Skip trailing white space and make sure there is nothing else left
    *  in the string. Note, if we have already reached end-of-string,
    *  c has been set to a space.
    */
   while (isspace(c) && s < end_s)
      c = *s++;
   if (!isspace(c))
      return CvtFail;

   /*
    * Test for integer.
    */
   if (!realflag && !scale && mantissa >= MinLong && mantissa <= MaxLong) {
      result->integer = (msign == '+' ? lresult : -lresult);
      return T_Integer;
      }

#ifdef LargeInts
   /*
    * Test for bignum.
    */
#if COMPILER
   if (largeints)
#endif					/* COMPILER */
      if (!realflag) {
         int rv;
         rv = bigradix((int)msign, 10, ssave, end_s, result);
         if (rv == Error)
            fatalerr(0, NULL);
         return rv;
         }
#endif					/* LargeInts */

   if (!realflag)
      return CvtFail;		/* don't promote to real if integer format */

   /*
    * Rough tests for overflow and underflow.
    */
   if (sdigits + scale > LogHuge)
      return CvtFail;

   if (sdigits + scale < -LogHuge) {
      result->real = 0.0;
      return T_Real;
      }

   /*
    * Put the number together by multiplying the mantissa by 5^scale and
    *  then using ldexp() to multiply by 2^scale.
    */

   exponent = (scale > 0)? scale : -scale;
   fiveto = 1.0;
   power = 5.0;
   for (;;) {
      if (exponent & 01)
	 fiveto *= power;
      exponent >>= 1;
      if (exponent == 0)
	 break;
      power *= power;
      }
   if (scale > 0)
      mantissa *= fiveto;
   else
      mantissa /= fiveto;

   err_no = 0;
   mantissa = ldexp(mantissa, scale);
   if (err_no > 0 && mantissa > 0)
      /*
       * ldexp caused overflow.
       */
      return CvtFail;

   if (msign == '-')
      mantissa = -mantissa;
   result->real = mantissa;
   return T_Real;
   }

#if COMPILER || !(defined LargeInts)
/*
 * radix - convert string s in radix r into an integer in *result.  sign
 *  will be either '+' or '-'.
 */
int radix(sign, r, s, end_s, result)
int sign;
register int r;
register char *s;
register char *end_s;
union numeric *result;
   {
   register int c;
   long num;

   if (r < 2 || r > 36)
      return CvtFail;
   c = (s < end_s) ? *s++ : ' ';
   num = 0L;
   while (isalnum(c)) {
      c = tonum(c);
      if (c >= r)
	 return CvtFail;
      num = num * r + c;
      c = (s < end_s) ? *s++ : ' ';
      }

   /*
    * Skip trailing white space and make sure there is nothing else left
    *  in the string. Note, if we have already reached end-of-string,
    *  c has been set to a space.
    */
   while (isspace(c) && s < end_s)
      c = *s++;
   if (!isspace(c))
      return CvtFail;

   result->integer = (sign == '+' ? num : -num);

   return T_Integer;
   }
#endif					/* COMPILER || !(defined LargeInts) */


/*
 * cvpos - convert position to strictly positive position
 *  given length.
 */

word cvpos(pos, len)
long pos;
register long len;
   {
   register word p;

   /*
    * Make sure the position is in the range of an int. (?)
    */
   if ((long)(p = pos) != pos)
      return CvtFail;
   /*
    * Make sure the position is within range.
    */
   if (p < -len || p > len + 1)
      return CvtFail;
   /*
    * If the position is greater than zero, just return it.  Otherwise,
    *  convert the zero/negative position.
    */
   if (pos > 0)
      return p;
   return (len + p + 1);
   }

/*
 * rtos - convert the real number n into a string using s as a buffer and
 *  making a descriptor for the resulting string.
 */
novalue rtos(n, dp, s)
double n;
dptr dp;
char *s;
   {

   s++; 			/* leave room for leading zero */
/*
 * The following code is operating-system dependent [@rconv.01]. Convert real
 *  number to string.
 *
 * If IconGcvt is defined, icon_gcvt() is actually called, due to a #define
 *  in config.h.
 */

#if PORT
   gcvt(n, Precision, s);
Deliberate Syntax Error
#endif					/* PORT */

#if HIGHC_386
   sprintf(s,"%.*g", Precision, n);
#else					/* HIGHC_386 */
#if AMIGA || ARM || ATARI_ST || MSDOS || OS2 || UNIX || VMS
   gcvt(n, Precision, s);
#endif                                  /* AMIGA || ARM || ATARI_ST || ... */
#endif					/* HIGHC_386 */

#if MACINTOSH
   sprintf(s,"%.20g",n);
#endif					/* MACINTOSH */

#if VM || MVS
   sprintf(s,"%.*g", Precision, n);
   {
     char *ep = strstr(s, "e+");
     if (ep) memmove(ep+1, ep+2, strlen(ep+2)+1);
   }
#endif					/* VM || MVS */

/*
 * End of operating-system specific code.
 */
   
   /*
    * Now clean up possible messes.
    */
   while (*s == ' ')			/* delete leading blanks */
      s++;
   if (*s == '.') {			/* prefix 0 to initial period */
      s--;
      *s = '0';
      }
   else if (strcmp(s, "-0.0") == 0)	/* negative zero */
      s++;
   else if (!index(s, '.') && !index(s,'e') && !index(s,'E'))
         strcat(s, ".0");		/* if no decimal point or exp. */
   if (s[strlen(s) - 1] == '.')		/* if decimal point is at end ... */
      strcat(s, "0");
   StrLen(*dp) = strlen(s);
   StrLoc(*dp) = s;
   }

/*
 * cstos - convert the cset bit array pointed at by cs into a string using
 *  s as a buffer and making a descriptor for the resulting string.
 */

static novalue cstos(cs, dp, s)
unsigned int *cs;
dptr dp;
char *s;
   {
   register unsigned int w;
   register int j, i;
   register char *p;

   p = s;
   for (i = 0; i < CsetSize; i++) {
      if (cs[i])
	 for (j=i*IntBits, w=cs[i]; w; j++, w >>= 1)
	    if (w & 01)
	       *p++ = FromAscii((char)j);
      }
   *p = '\0';

   StrLen(*dp) = p - s;
   StrLoc(*dp) = s;
   }
