/*
 * File: oasgn.r
 * Asgn - perform an assignment when the destination descriptor might
 *  be within a block.
 */
#define Asgn(dest, src) *(dptr)((word *)VarLoc(dest) + Offset(dest)) = src;

/*
 * GeneralAsgn - perform the assignment x := y, where x is known to be
 *  a variable and y is has been dereferenced.
 */
#begdef GeneralAsgn(x, y)

#ifdef EventMon
   body {
      if (!is:null(curpstate->eventmask) &&
	  Testb((word)ToAscii(E_Assign), curpstate->eventmask)) {
	 EVAsgn(&x);
	 }
      }
#endif					/* EventMon */

   type_case x of {
      tvsubs: {
        abstract {
           store[store[type(x).str_var]] = string
           }
        inline {
           if (subs_asgn(&x, (const dptr)&y) == Error)
              runerr(0);
           }
        }
      tvtbl: {
        abstract {
           store[store[type(x).trpd_tbl].tbl_val] = type(y)
           }
        inline {
           tvtbl_asgn(&x, (const dptr)&y);
           }
         }
      kywdevent:
	 body {
	    *VarLoc(x) = y;
	    }
      kywdwin:
	 body {
#ifdef Graphics
	    if (is:null(y))
	       *VarLoc(x) = y;
	    else {
	       if ((!is:file(y)) || !(BlkLoc(y)->file.status & Fs_Window))
		  runerr(140,y);
	       *VarLoc(x) = y;
	       }
#endif					/* Graphics */
	    }
      kywdint: 
	 {
         /*
          * No side effect in the type realm - keyword x is still an int.
          */
         body {
            C_integer i;

            if (!cnv:C_integer(y, i))
               runerr(101, y);
            IntVal(*VarLoc(x)) = i;

#ifdef Graphics
	    if (xyrowcol(&x) == -1)
	       runerr(140,kywd_xwin[XKey_Window]);
#endif					/* Graphics */
	    }
	}
      kywdpos: {
         /*
          * No side effect in the type realm - &pos is still an int.
          */
         body {
            C_integer i;

            if (!cnv:C_integer(y, i))
               runerr(101, y);

#ifdef MultiThread
	    i = cvpos((long)i, StrLen(*(VarLoc(x)+1)));
#else					/* MultiThread */
            i = cvpos((long)i, StrLen(k_subject));
#endif					/* MultiThread */

            if (i == CvtFail)
               fail;
	    IntVal(*VarLoc(x)) = i;

#ifdef EventMon
            EVVal(k_pos, E_Spos);
#endif					/* EventMon */
            }
         }
      kywdsubj: {
         /*
          * No side effect in the type realm - &subject is still a string
          *  and &pos is still an int.
          */
         if !cnv:string(y, *VarLoc(x)) then
            runerr(103, y);
         inline {
#ifdef MultiThread
	    IntVal(*(VarLoc(x)-1)) = 1;
#else					/* MultiThread */
            k_pos = 1;
#endif					/* MultiThread */

#ifdef EventMon
            EVVal(k_pos, E_Spos);
#endif					/* EventMon */
            }
         }
      kywdstr: {
         /*
          *  No side effect in the type realm.
          */
         if !cnv:string(y, *VarLoc(x)) then
            runerr(103, y);
         }
      default: {
        abstract {
           store[type(x)] = type(y)
           }
         inline {

            Asgn(x, y)
            }
         }
      }

#ifdef EventMon
   body {
      EVValD(&y, E_Value);
      }
#endif					/* EventMon */

#enddef


"x := y - assign y to x."

operator{0,1} := asgn(underef x, y)

   if !is:variable(x) then
      runerr(111, x)

   abstract {
      return type(x)
      }

   GeneralAsgn(x, y)

   inline {
      /*
       * The returned result is the variable to which assignment is being
       *  made.
       */
      return x;
      }
end


"x <- y - assign y to x."
" Reverses assignment if resumed."

operator{0,1+} <- rasgn(underef x -> saved_x, y)

   if !is:variable(x) then
      runerr(111, x)

   abstract {
      return type(x)
      }

   GeneralAsgn(x, y)

   inline {
      suspend x;
      }

   GeneralAsgn(x, saved_x)

   inline {
      fail;
      }
end


"x <-> y - swap values of x and y."
" Reverses swap if resumed."

operator{0,1+} <-> rswap(underef x -> dx, underef y -> dy)

   declare {
      tended union block *bp_x, *bp_y;
      word adj1 = 0;
      word adj2 = 0;
      }

   if !is:variable(x) then
      runerr(111, x)
   if !is:variable(y) then
      runerr(111, y)

   abstract {
      return type(x)
      }

   if is:tvsubs(x) && is:tvsubs(y) then
      body {
         bp_x = BlkLoc(x);
         bp_y = BlkLoc(y);
         if (VarLoc(bp_x->tvsubs.ssvar) == VarLoc(bp_y->tvsubs.ssvar) &&
   	  Offset(bp_x->tvsubs.ssvar) == Offset(bp_y->tvsubs.ssvar)) {
            /*
             * x and y are both substrings of the same string, set
             *  adj1 and adj2 for use in locating the substrings after
             *  an assignment has been made.  If x is to the right of y,
             *  set adj1 := *x - *y, otherwise if y is to the right of
             *  x, set adj2 := *y - *x.  Note that the adjustment
             *  values may be negative.
             */
            if (bp_x->tvsubs.sspos > bp_y->tvsubs.sspos)
               adj1 = bp_x->tvsubs.sslen - bp_y->tvsubs.sslen;
            else if (bp_y->tvsubs.sspos > bp_x->tvsubs.sspos)
               adj2 = bp_y->tvsubs.sslen - bp_x->tvsubs.sslen;
   	    }
         }

   /*
    * Do x := y
    */
   GeneralAsgn(x, dy)

   if is:tvsubs(x) && is:tvsubs(y) then
      inline {
         if (adj2 != 0)
            /*
             * Arg2 is to the right of Arg1 and the assignment Arg1 := Arg2 has
             *  shifted the position of Arg2.  Add adj2 to the position of Arg2
             *  to account for the replacement of Arg1 by Arg2.
             */
            bp_y->tvsubs.sspos += adj2;
         }

   /*
    * Do y := x
    */
   GeneralAsgn(y, dx)

   if is:tvsubs(x) && is:tvsubs(y) then
      inline {
         if (adj1 != 0)
            /*
             * Arg1 is to the right of Arg2 and the assignment Arg2 := Arg1
             *  has shifted the position of Arg1.  Add adj2 to the position
             *  of Arg1 to account for the replacement of Arg2 by Arg1.
             */
            bp_x->tvsubs.sspos += adj1;
         }

   inline {
      suspend x;
      }
   /*
    * If resumed, the assignments are undone.  Note that the string position
    *  adjustments are opposite those done earlier.
    */
   GeneralAsgn(x, dx)
   if is:tvsubs(x) && is:tvsubs(y) then
      inline {
         if (adj2 != 0)
           bp_y->tvsubs.sspos -= adj2;
         }

   GeneralAsgn(y, dy)
   if is:tvsubs(x) && is:tvsubs(y) then
      inline {
         if (adj1 != 0)
            bp_x->tvsubs.sspos -= adj1;
         }

   inline {
      fail;
      }
end


"x :=: y - swap values of x and y."

operator{0,1} :=: swap(underef x -> dx, underef y -> dy)
   declare {
      tended union block *bp_x, *bp_y;
      word adj1 = 0;
      word adj2 = 0;
      }

   /*
    * x and y must be variables.
    */
   if !is:variable(x) then
      runerr(111, x)
   if !is:variable(y) then
      runerr(111, y)

   abstract {
      return type(x)
      }

   if is:tvsubs(x) && is:tvsubs(y) then
      body {
         bp_x = BlkLoc(x);
         bp_y = BlkLoc(y);
         if (VarLoc(bp_x->tvsubs.ssvar) == VarLoc(bp_y->tvsubs.ssvar) &&
   	  Offset(bp_x->tvsubs.ssvar) == Offset(bp_y->tvsubs.ssvar)) {
            /*
             * x and y are both substrings of the same string, set
             *  adj1 and adj2 for use in locating the substrings after
             *  an assignment has been made.  If x is to the right of y,
             *  set adj1 := *x - *y, otherwise if y is to the right of
             *  x, set adj2 := *y - *x.  Note that the adjustment
             *  values may be negative.
             */
            if (bp_x->tvsubs.sspos > bp_y->tvsubs.sspos)
               adj1 = bp_x->tvsubs.sslen - bp_y->tvsubs.sslen;
            else if (bp_y->tvsubs.sspos > bp_x->tvsubs.sspos)
               adj2 = bp_y->tvsubs.sslen - bp_x->tvsubs.sslen;
   	    }
         }

   /*
    * Do x := y
    */
   GeneralAsgn(x, dy)

   if is:tvsubs(x) && is:tvsubs(y) then
      inline {
         if (adj2 != 0)
            /*
             * Arg2 is to the right of Arg1 and the assignment Arg1 := Arg2 has
             *  shifted the position of Arg2.  Add adj2 to the position of Arg2
             *  to account for the replacement of Arg1 by Arg2.
             */
            bp_y->tvsubs.sspos += adj2;
         }

   /*
    * Do y := x
    */
   GeneralAsgn(y, dx)

   if is:tvsubs(x) && is:tvsubs(y) then
      inline {
         if (adj1 != 0)
            /*
             * Arg1 is to the right of Arg2 and the assignment Arg2 := Arg1
             *  has shifted the position of Arg1.  Add adj2 to the position
             *  of Arg1 to account for the replacement of Arg2 by Arg1.
             */
            bp_x->tvsubs.sspos += adj1;
         }

   inline {
      return x;
      }
end

/*
 * subs_asgn - perform assignment to a substring. Leave the updated substring
 *  in dest in case it is needed as the result of the assignment.
 */
int subs_asgn(dest, src)
dptr dest;
const dptr src;
   {
   tended struct descrip deststr;
   tended struct descrip srcstr;
   tended struct descrip rsltstr;
   tended struct b_tvsubs *tvsub;

   char *s, *s2;
   word i, len;
   word prelen;   /* length of portion of string before substring */
   word poststrt; /* start of portion of string following substring */
   word postlen;  /* length of portion of string following substring */

   if (!cnv:tmp_string(*src, srcstr))
      ReturnErrVal(103, *src, Error);

   /*
    * Be sure that the variable in the trapped variable points
    *  to a string and that the string is big enough to contain
    *  the substring.
    */
   tvsub = (struct b_tvsubs *)BlkLoc(*dest);
   deref(&tvsub->ssvar, &deststr);
   if (!is:string(deststr))
      ReturnErrVal(103, deststr, Error);
   prelen = tvsub->sspos - 1;
   poststrt = prelen + tvsub->sslen;
   if (poststrt > StrLen(deststr))
      ReturnErrNum(205, Error);

   /*
    * Form the result string.
    *  Start by allocating space for the entire result.
    */
   len = prelen + StrLen(srcstr) + StrLen(deststr) - poststrt;
   Protect(s = alcstr(NULL, len), return Error);
   StrLoc(rsltstr) = s;
   StrLen(rsltstr) = len;
   /*
    * First, copy the portion of the substring string to the left of
    *  the substring into the string space.
    */
   s2 = StrLoc(deststr);
   for (i = 0; i < prelen; i++)
      *s++ = *s2++;
   /*
    * Copy the string to be assigned into the string space,
    *  effectively concatenating it.
    */
   s2 = StrLoc(srcstr);
   for (i = 0; i < StrLen(srcstr); i++)
      *s++ = *s2++;
   /*
    * Copy the portion of the substring to the right of
    *  the substring into the string space, completing the
    *  result.
    */
   s2 = StrLoc(deststr) + poststrt;
   postlen = StrLen(deststr) - poststrt;
   for (i = 0; i < postlen; i++)
      *s++ = *s2++;

   /*
    * Perform the assignment and update the trapped variable.
    */
   if (is:kywdsubj(tvsub->ssvar)) {
      *VarLoc(tvsub->ssvar) = rsltstr;
      k_pos = 1;
      }
   else
      Asgn(tvsub->ssvar, rsltstr)
   tvsub->sslen = StrLen(srcstr);
   return Succeeded;
   }

/*
 * tvtbl_asgn - perform an assignment to a table element trapped variable,
 *  inserting the element in the table if needed.
 */
novalue tvtbl_asgn(dest, src)
dptr dest;
const dptr src;
   {
   /*
    * No allocations are performed until the end, so nothing need be tended.
    */
   union block *bp, *ep, *hook;
   union block **slot;
   struct b_table *tp;
   struct descrip d;
   register uword hn;

   /*
    * If already a table element, not a trapped variable, just assign and
    *  return.
    */
   bp = BlkLoc(*dest);
   if (bp->tvtbl.title == T_Telem) {
       bp->telem.tval = *src;
       return;
    }

   /*
    * Convert the trapped-variable block into a table-element block
    *  and insert it in the table.  Begin by getting the hash number
    *  for the subscripting value and locating the correct hash chain.
    */

   tp = (struct b_table *) bp->tvtbl.clink;
   hn = bp->tvtbl.hashnum;
   slot = hchain((union block *)tp, hn);
   ep = *slot;

   /*
    * Traverse the chain to see if the value is already in the
    *  table.  If it is there, assign to it and return.
    */
   hook = ep;
   while (ep != NULL && ep->telem.hashnum <= hn) {
      if (ep->telem.hashnum == hn
	    && equiv(&ep->telem.tref, &bp->tvtbl.tref)) {
	       bp->telem.tval = *src;
               return;
               }
      hook = ep;
      ep = ep->telem.clink;
      }

   /*
    * The value being assigned is new.  Increment the table
    *  size, convert the table-element trapped-variable block
    *  to a table-element block, and link it into the chain.
    */
   tp->size++;
   if (hook == ep) {			/* it goes at front of chain */
      bp->telem.clink = *slot;
      *slot = bp;
      }
   else {				/* it follows hook */
      bp->telem.clink = hook->telem.clink;
      hook->telem.clink = bp;
      }

   bp->tvtbl.title = T_Telem;		/* mutate b_tvtbl into b_telem */
   bp->telem.tval = *src;		/* set the value */
   d.dword = D_Telem;
   BlkLoc(d) = (union block *)bp;
   MMShow(&d, 'r');			/* record mutation event */

   if (TooCrowded(tp))			/* grow hash table if now too full */
      hgrow((union block *)tp);
   }
