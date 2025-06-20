/*
 *  fmonitr.r -- mmout, mmpause, mmshow, EvGet
 *
 *   This file contains event monitoring code.
 *
 *   Much of this code is contingent on the definition of MemMon (memory
 *   monitoring) and EventMon (event monitoring). Memory monitoring came
 *   first and normally is defined in all implementations of Icon. It also
 *   is a supported feature with various visualization tools. (See the
 *   directory "memmon", which is parallel to this one.) Event monitoring is
 *   more recent and is stil experimental. It normally is not enabled. Memory
 *   monitoring is a subset of event monitoring.
 *
 *   When MemMon or EventMon is undefined, most of the "MMxxxx" and "EVxxxx"
 *   entry points are defined as null macros in monitor.h.
 *
 *   See monitor.h for important definitions and for the interaction between
 *   MemMon and Eventmon.
 */


#ifdef MemMon

/*
 * Prototypes.
 */

novalue mmrefresh		Params((noargs));
hidden	novalue mmsizes		Params((int c));
hidden	novalue mmstatic	Params((noargs));

#ifdef EventMon
#define evforget()
#else					/* EventMon */
hidden	novalue etvalue		Params((word n, int c));
novalue evcmd			Params((word addr, word len, int c));
hidden	novalue evdec		Params((uword n));
hidden	novalue evforget	Params((noargs));
hidden	novalue evnewline	Params((noargs));
#endif					/* EventMon */


static char typech[MaxType+1];	/* output character for each type */
static char *monname = NULL;	/* output file name */

#ifdef EventMon
int noMTevents;			/* don't produce events in EVAsgn */

union { 			/* clock ticker -- keep in sync w/ interp.r */
   unsigned short s[4];		/* four counters */
   unsigned long l[2];		/* two longs are easier to check */
} ticker;
unsigned long oldtick;		/* previous sum of the two longs */

#if UNIX
/*
 * Global state used by EVTick()
 */
word oldsum = 0;
#endif					/* UNIX */

#else					/* EventMon */

FILE *monfile = NULL;		/* output file pointer */

static word llen = 0;		/* current output line length */

/* Define size of curvalue table, and bias needed to access it. */
/* Assumes all type codes are printable characters (or space).   */
/* Smaller table is used if not EBCDIC.                          */
#if !EBCDIC
#define CurSize (127 - ' ')
#define CurBias ' '
#else					/* !EBCDIC */
#define CurSize 256
#define CurBias 0
#endif					/* !EBCDIC */

static word curvalue[CurSize];	/* current length for each output character */

/* line limit: start a new line when a command goes beyond this column */
#define LLIM 70

/* evchar(c): output character c and update the column counter */
#define evchar(c) (llen++,putc((c),monfile))

/* evspace(): output unneeded whitespace whitespace following a command */
/*  define as "evchar(' ')" for readable files, or as "0" for compact ones */
#define evspace() 0

/*
 * evseparate(): output either a space or a newline depending on spacing
 *  requirements
 */
#define evseparate() if (llen >= LLIM) evnewline(); else evchar(' ');

#endif					/* EventMon */


"mmout(s) - write the given string to the MemMon file."

function{1} mmout(s)
   if !def:C_string(s, "") then
      runerr(103, s)
   abstract {
      return null
      }
   inline {
      MMOut("", s);
      return nulldesc;
      }
end


"mmpause(s) - pause MemMon displaying string s."

function{1} mmpause(s)
   if !def:C_string(s, "") then
      runerr(103, s)
   abstract {
      return null
      }
   inline {
      MMOut("; ", s[0] ? s : "programmed pause");
      return nulldesc;
      }
end


"mmshow(x,s) - alter MemMon display of x depending on s."

function{1} mmshow(x, s)

   if !def:string(s, emptystr) then
      runerr(103, s)
   abstract {
      return null
      }
   body {
      register word i, j, d;
      register union block *bp, *ep;
      char c;
      struct b_slots *seg;

      if (StrLen(s) == 0)
	 c = '\0';
      else
	 c = *StrLoc(s);
      MMShow(&x,c);
      switch (Type(x)) {
      case T_List:
            bp = BlkLoc(x);
            for (bp = bp->list.listhead; bp != NULL; bp = bp->lelem.listnext) {
	       x.dword = D_Lelem;
               BlkLoc(x) = bp;
               MMShow(&x, c);
               }
	    break;
      case T_Set:
      case T_Table:
	    d = (Type(x) == T_Set) ? D_Selem : D_Telem;
            bp = BlkLoc(x);
            for (i = 0; i < HSegs && (seg = bp->table.hdir[i]) != NULL; i++) {
	       x.dword = D_Slots;
               BlkLoc(x) = (union block *)seg;
               MMShow(&x, c);
               for (j = segsize[i] - 1; j >= 0; j--) {
	          x.dword = d;
                  for (ep = seg->hslots[j]; ep != NULL; ep = ep->telem.clink) {
                     BlkLoc(x) = ep;
                     MMShow(&x, c);
                     }
                  }
               }
            break;
         }

      return nulldesc;
      }
end

#ifdef MultiThread
#ifdef EventMon

static char scopechars[] = "+:-^";

/*
 * Special event function for E_Assign; allocates out of monitor's heap.
 */
novalue EVAsgn(dx)
dptr dx;
{
   int i;
   dptr procname;
   struct progstate *parent = curpstate->parent;
   struct region *rp = curpstate->stringregion;

#if COMPILER
   procname = &(PFDebug(*pfp)->proc->pname);
#else					/* COMPILER */
   procname = &((&BlkLoc(*argp)->proc)->pname);
#endif					/* COMPILER */
   /*
    * call get_name, allocating out of the monitor if necessary.
    */
   curpstate->stringregion = parent->stringregion;
   parent->stringregion = rp;
   noMTevents++;
   i = get_name(dx,&(parent->eventval));

   if (i == GlobalName) {
      if (strreserve(StrLen(parent->eventval) + 1) == NULL)
	 syserr("event monitoring out-of-memory error");
      StrLoc(parent->eventval) =
	 alcstr(StrLoc(parent->eventval), StrLen(parent->eventval));
      alcstr("+",1);
      StrLen(parent->eventval)++;
      }
   else if (i == StaticName || i == LocalName || i == ParamName) {
      if (!strreserve(StrLen(parent->eventval) + StrLen(*procname) + 1))
	 syserr("event monitoring out-of-memory error");
      StrLoc(parent->eventval) =
	 alcstr(StrLoc(parent->eventval), StrLen(parent->eventval));
      alcstr(scopechars+i,1);
      alcstr(StrLoc(*procname), StrLen(*procname));
      StrLen(parent->eventval) += StrLen(*procname) + 1;
      }
   else if (i == Error) {
      noMTevents--;
      return; /* should be more violent than this */
      }

   parent->stringregion = curpstate->stringregion;
   curpstate->stringregion = rp;
   noMTevents--;
   actparent(E_Assign);
}


/*
 * event(x, y, C) -- generate an event at the program level.
 */

"event(x, y, C) - create event with event code x and event value y."

function{0,1} event(x,y,ce)
   body {
      struct progstate *dest;

      if (is:null(x)) {
	 x = curpstate->eventcode;
	 if (is:null(y)) y = curpstate->eventval;
	 }
      if (is:null(ce) && is:coexpr(curpstate->parentdesc))
	 ce = curpstate->parentdesc;
      else if (!is:coexpr(ce)) runerr(118,ce);
      dest = BlkLoc(ce)->coexpr.program;
      dest->eventcode = x;
      dest->eventval = y;
      if (mt_activate(&(dest->eventcode),&result,
			 (struct b_coexpr *)BlkLoc(ce)) == A_Cofail) {
         fail;
         }
       return result;
      }
end

/*
 * EvGet(c) - user function for reading event streams.
 * This implementation requires MultiThread and EventMon.
 */

"EvGet(c,flag) - read through the next event token having a code matched "
" by cset c."

/*
 *  EvGet returns the code of the matched token.  These keywords are also set:
 *    &eventcode     token code
 *    &eventvalue    token value
 */
function{0,1} EvGet(cs,flag)
   if !def:cset(cs,fullcs) then
      runerr(104,cs)

   body {
      register int c;
      tended struct descrip dummy;
      struct progstate *p;

      /*
       * Be sure an eventsource is available
       */
      if (!is:coexpr(curpstate->eventsource))
         runerr(118,curpstate->eventsource);

      /*
       * If our event source is a child of ours, assign its event mask.
       */
      p = BlkLoc(curpstate->eventsource)->coexpr.program;
      if (p->parent == curpstate)
	 p->eventmask = cs;

#ifdef Graphics
      if (Testb((word)ToAscii(E_MXevent), cs) &&
	  is:file(kywd_xwin[XKey_Window])) {
	 wbp _w_ = (wbp)BlkLoc(kywd_xwin[XKey_Window])->file.fd;
	 pollctr = pollevent();
	 if (pollctr == -1)
	    fatalerr(141, NULL);
	 if (BlkLoc(_w_->window->listp)->list.size > 0) {
	    c = wgetevent(_w_, &curpstate->eventval);
	    if (c == 0) {
	       StrLen(curpstate->eventcode) = 1;
	       StrLoc(curpstate->eventcode) =
		  (char *)&allchars[FromAscii(E_MXevent) & 0xFF];
	       return curpstate->eventcode;
	       }
	    else if (c == -1)
	       runerr(141);
	    else
	       runerr(143);
	    }
	 }
#endif					/* Graphics */

      /*
       * Loop until we read an event allowed.
       */
      while (1) {
         /*
          * Activate the event source to produce the next event.
          */
	 dummy = cs;
	 if (mt_activate(&dummy, &curpstate->eventcode,
			 (struct b_coexpr *)BlkLoc(curpstate->eventsource)) == A_Cofail) fail;

	 deref(&curpstate->eventcode, &curpstate->eventcode);

	 if (!is:string(curpstate->eventcode) ||
	     StrLen(curpstate->eventcode) != 1) {
	    /*
	     * this event is out-of-band data; return or reject it
	     * depending on whether flag is null.
	     */
	    if (!is:null(flag))
	       return curpstate->eventcode;
	    else continue;
	    }
	 return curpstate->eventcode;
	 }
      }
end

#endif					/* EventMon */
#endif					/* MultiThread */

/*
 *  EVInit(exename,outname) - initialization.
 *
 *  Event monitoring is activated if one of the environment variables EVENTMON
 *  or MEMMON is non-null, depending on which type of monitoring is configured.
 *  The environment variable names the output file;  or, under implementations
 *  that support pipes, a value beginning with "|" specifies a command to which
 *  the output is piped.
 *
 *  Monitoring can also be activated by the -E option on the iconx command
 *  line, in which case outname is nonnull and overrides any environment
 *  setting.
 *
 *  If monitoring is defined on a system lacking environment variables,
 *  monitoring is always activated and output is to the file "eventmon.out"
 *  if outname does not specify a different file.
 */

novalue EVInit(exename,outname)
char *exename;
char *outname;
   {
   int i;


   /*
    * Initialize the typech array, which is used if either file-based
    * or MT-based event monitoring is enabled.
    */

   for (i = 0; i <= MaxType; i++)
      typech[i] = '?';	/* initialize with error character */

#ifdef LargeInts
   typech[T_Lrgint]  = E_Lrgint;	/* long integer */
#endif					/* LargeInts */

   typech[T_Real]    = E_Real;		/* real number */
   typech[T_Cset]    = E_Cset;		/* cset */
   typech[T_File]    = E_File;		/* file block */
   typech[T_Record]  = E_Record;	/* record block */
   typech[T_Tvsubs]  = E_Tvsubs;	/* substring trapped variable */
   typech[T_External]= E_External;	/* external block */
   typech[T_List]    = E_List;		/* list header block */
   typech[T_Lelem]   = E_Lelem;		/* list element block */
   typech[T_Table]   = E_Table;		/* table header block */
   typech[T_Telem]   = E_Telem;		/* table element block */
   typech[T_Tvtbl]   = E_Tvtbl;		/* table elem trapped variable*/
   typech[T_Set]     = E_Set;		/* set header block */
   typech[T_Selem]   = E_Selem;		/* set element block */
   typech[T_Slots]   = E_Slots;		/* set/table hash slots */
   typech[T_Coexpr]  = E_Coexpr;	/* co-expression block (static) */
   typech[T_Refresh] = E_Refresh;	/* co-expression refresh block */


   /*
    * codes used elsewhere but not shown here:
    *    in the static region: E_Alien = alien (malloc block)
    *    in the static region: E_Free = free
    *    in the string region: E_String = string
    */
#ifdef MultiThread
   strcpy(curpstate->exename, exename);
#endif					/* MultiThread */



#ifdef EventMon
#if UNIX
   /*
    * Call profil(2) to enable program counter profiling.  We use the smallest
    *  allowable scale factor in order to minimize the number of counters;
    *  we assume that the text of iconx does not exceed 256K and so we use
    *  four bins.  One of these four bins will be incremented every system
    *  clock tick (typically 4 to 20 ms).
    *
    *  Take your local profil(2) man page with a grain of salt.  All the systems
    *  we tested really maintain 16-bit counters despite what the man pages say.
    *  Some also say that a scale factor of two maps everything to one counter;
    *  that is believed to be a no-longer-correct statement dating from the days
    *  when the maximum program size was 64K.
    *
    *  The reference to EVInit below just obtains an arbitrary address within
    *  the text segment.
    */
   profil(ticker.s, sizeof(ticker.s), (int) EVInit & ~0x3FFFF, 2);
#endif					/* UNIX */
#endif					/* EventMon */
   }
/*
 *  EVSetup() - Set up storage information.
 */
novalue EVSetup()
   {

   if (!EventStream)
      return;
   mmrefresh();				/* show current state */
   fflush(monfile);			/* force it out */
   }

/*
 * EVTerm(n, part2) - terminate memory monitoring.
 *  The error message for n and part2 are concatentated to form an explanatory
 *  message.
 */

novalue EVTerm(n, part2)
int n;
char *part2;
   {
   FILE *f;
   char part1[40];

   if (!EventStream)
      return;
   if (n > 0)
      sprintf(part1,"Run-time error %d: ",n);
   else
      part1[0] = '\0';
   if (part2 == NULL)
      part2 = "";
   evnewline();
   mmsizes('=');		/* make a final check on region sizes */

   if (*part1 || *part2)	/* if any reason given, write it as comment */
      fprintf(monfile, "# %s%s\n", part1, part2);

   f = monfile;
   monfile = NULL;	/* so we don't try to show the freeing of the buffer */

#ifdef Pipes
   if (monname[0] == '|')
      pclose(f);
   else
#endif					/* Pipes */
      fclose(f);
   }

/*
 * MMStat(a, n, c) - note static block at a, length n, represented by char 'c'.
 * Output values are in basic units (typically words).
 */
novalue MMStat(a, n, c)
char *a;
word n;
int c;
   {

#ifndef FixedRegions
   if (!EventStream)
      return;
   evcmd(DiffPtrs(a, statbase) / MMUnits, n / MMUnits, c);
#endif					/* FixedRegions */

   }

/*
 * MMAlc(len, type) - note an allocation at the end of the block region.
 *
 *  If len is negative, it's a deallocation, and the type doesn't matter.
 */

novalue MMAlc(len, type)
word len;
int type;
   {
   if (len < 0) {
#ifdef EventMon
      EVVal(-len / MMUnits, E_BlkDeAlc);
#else					/* EventMon */
      evcmd((word)-1, -len / MMUnits, E_BlkDeAlc);
#endif					/* EventMon */
      }
   else
      EVVal(len / MMUnits, typech[type]);
   }

/*
 * MMStr(len) - note a string allocation at the end of the string region.
 *
 *  If len is negative, it's a deallocation.
 */

novalue MMStr(slen)
word slen;
   {
   if (slen > 0) {
      EVVal(slen, E_String);
      }
   else if (slen < 0)
#ifdef EventMon
      EVVal(-slen, E_StrDeAlc);
#else					/* EventMon */
      evcmd((word)-1, -slen, E_StrDeAlc);
#endif					/* EventMon */
   }

/*
 * MMBGC() - begin garbage collection.
 */

novalue MMBGC(region)
int region;
   {
#ifndef EventMon
   if (!EventStream)
      return;
   mmsizes('=');			/* write current sizes */
   fprintf(monfile, "%d{\n", region);	/* indicate start of g.c. */
   fflush(monfile);
   evforget();				/* clear memory of block sizes */
#endif					/* EventMon */
   }

/*
 * MMEGC() - end garbage collection.
 */

novalue MMEGC()
   {
#ifndef EventMon
   if (!EventStream)
      return;
   evnewline();
   fprintf(monfile, "}\n");	/* indicate end of marking */
   mmrefresh();			/* redraw regions after compaction */
   fprintf(monfile, "!\n");	/* indicate end of g.c. */
   fflush(monfile);
#endif					/* EventMon */
   }

#ifndef EventMon
/*
 * MMMark(block, type) - mark indicated block during garbage collection.
 */

novalue MMMark(block, type)
char *block;
int type;
   {
   if (!EventStream)
      return;
   evcmd(DiffPtrs(block, blkbase) / MMUnits, (word)BlkSize(block) / MMUnits,
      typech[type]);
   }

/*
 * MMSMark - Mark String.
 */

novalue MMSMark(saddr, slen)
char *saddr;
word slen;
   {
   if (!EventStream)
      return;
   evcmd(DiffPtrs(saddr, strbase), slen, E_String);
   }

/*
 * MMOut(prefix, msg) - write the prefix and message to the MemMon output file.
 */

novalue MMOut(prefix, msg)
char *prefix, *msg;
   {
   if (!EventStream)
      return;
   evnewline();
   fprintf(monfile, "%s%s\n", prefix, msg);
   }

/*
 * MMShow(d, colr) - redraw string or block d, specifying the color character.
 */

novalue MMShow(d, colr)
dptr d;
int colr;
   {
   char *block;
   uword addr;
   word len;
   char cmd, tch;

   if (!EventStream)
      return;
   if (colr == '\0')
      colr = 'r';		/* default color is 'r' (redraw) */

   if (Qual(*d)) {
      /*
       *  Show a string.
       */
      if (!InRange(strbase,StrLoc(*d),strend))
         return;	/* ignore if outside string region */
      addr = DiffPtrs(StrLoc(*d), strbase);
      len = StrLen(*d);
      cmd = '$';
      tch = E_String;
      }
   else if (Type(*d)==T_Coexpr) {
      /*
       *  Show a co-expression block, which will be in the static region.
       */
      block = (char *)BlkLoc(*d);
      addr = DiffPtrs(block, statbase) / MMUnits;
      len = BlkSize(block) / MMUnits;
      cmd = 'Y';
      tch = typech[T_Coexpr];
      }
   else if (Pointer(*d)) {
      /*
       *  Show object in the block region.
       */
      block = (char *)BlkLoc(*d);
      if (!InRange(blkbase,block,blkfree))
         return;	/* ignore if outside block region */
      addr = DiffPtrs(block, blkbase) / MMUnits;
      len = BlkSize(block) / MMUnits;
      cmd = '%';
      tch = typech[Type(*d)];
      }

   if (llen+5 >= LLIM)		/* allow extra room; this will be a long one */
      evnewline();

   evdec(addr);			/* address */
   evchar(E_Offset);

#ifdef EventMon
   evchar('"');
#endif					/* EventMon */

   etvalue(len, cmd);		/* length, and $ Y or % command */
   evchar(colr);		/* color flag */
   evchar(tch);			/* block type character */

#ifdef EventMon
   evchar('"');
   evchar(E_Highlight);
#endif					/* EventMon */

   if (llen >= LLIM)
      evnewline();
   else
      evspace();
   }
#endif					/* EventMon */

/*
 * mmrefresh() - redraw screen, initially or after garbage collection.
 */

novalue mmrefresh()
   {
   char *p;
   word n;

#ifdef EventMon
   /*
    * If the monitor is asking for E_EndCollect events, then it
    * can handle these memory allocation "redraw" events.
    */
  if (!is:null(curpstate->eventmask) &&
       Testb((word)ToAscii(E_EndCollect), curpstate->eventmask)) {
      for (p = blkbase; p < blkfree; p += n)
	 MMAlc(n = BlkSize(p), (int)BlkType(p));/* block region */
      MMStr(DiffPtrs(strfree, strbase));	/* string region */
      }

#else					/* EventMon */

   evnewline();
   mmsizes('<');			/* signal start of screen refresh */
   evnewline();
   evforget();				/* clear memory of past sizes */
   mmstatic();				/* show the static region */
   evnewline();
   for (p = blkbase; p < blkfree; p += n)
      MMAlc(n = BlkSize(p), (int)BlkType(p));/* block region */
   evnewline();
   MMStr(DiffPtrs(strfree, strbase));	/* string region */
   evnewline();

   fprintf(monfile, ">\n");		/* signal end of refresh */
   mmsizes('=');			/* confirm region sizes */
   evforget();				/* clear memory of past sizes */
#endif					/* EventMon */
   }

/*
 *  mmstatic() - recap the static region (stack, coexprs, aliens, free)
 *   (this function is empty under FixedRegions)
 */
static novalue mmstatic()
   {
#ifndef FixedRegions
   HEADER *p;
   char *a;
   int h;
   word n;

   for (p = (HEADER *)statbase; (uword)p < (uword)(HEADER *)statfree;
      p += p->s.bsize) {
         a = (char *)(p + 1);
         n = (p->s.bsize - 1) * sizeof(HEADER);
         h = *(int *)a;
         if (h == T_Coexpr)
            MMStat(a, n, E_Coexpr);		/* co-expression block */
         else if (h == FREEMAGIC)
            MMStat(a, n, E_Free);		/* free block */
         else
            MMStat(a, n, E_Alien);		/* alien block */
         }
   a = (char *)p;
   if (a < statend)
      MMStat(a, (word)(statend-a), E_Free);/* rest of static region is free */
#endif					/* FixedRegions */
   }

/*
 * mmsizes(c) - output current region sizes, with initial character c.
 * If c is '<', the unit size is written ahead of it.
 */
static novalue mmsizes(c)
int c;
   {

#ifdef EventMon
   /* static region; show as full, actual amount is unknown */
   EVVal(statbase, E_Base);
   EVVal(DiffPtrs(statend, statbase), E_Used);
   EVVal(DiffPtrs(statend, statbase), E_Size);
   /* string region */
   EVVal(strbase, E_Base);
   EVVal(DiffPtrs(strfree, strbase), E_Used);
   EVVal(DiffPtrs(strend, strbase), E_Size);
   /* block region */
   EVVal(blkbase, E_Base);
   EVVal(DiffPtrs(blkfree, blkbase), E_Used);
   EVVal(DiffPtrs(blkend, blkbase), E_Size);
#else					/* EventMon */

   evnewline();
   if (c == '<')
      fprintf(monfile,"%d%c\n", MMUnits, c);
   else
      fprintf(monfile, "%c ", c);

#if VMS
   fprintf(monfile, "%ld:%ld/%ld %ld:%ld/%ld %ld:%ld/%ld\n",
#else						/* VMS */
   fprintf(monfile, "%lu:%lu/%lu %lu:%lu/%lu %lu:%lu/%lu\n",
#endif						/* VMS */

      /* static region; show as full, actual amount is unknown */
      (unsigned long)statbase,
      (unsigned long)DiffPtrs(statend, statbase),
      (unsigned long)DiffPtrs(statend, statbase),
      /* string region */
      (unsigned long)strbase,
      (unsigned long)DiffPtrs(strfree, strbase),
      (unsigned long)DiffPtrs(strend, strbase),
      /* block region */
      (unsigned long)blkbase,
      (unsigned long)DiffPtrs(blkfree, blkbase),
      (unsigned long)DiffPtrs(blkend, blkbase));

   if (c == '=')
      fprintf(monfile,"\n");
#endif					/* EventMon */

   }

#ifndef EventMon
/*
 * evcmd(addr, len, c) - output a memmon command.
 *  If addr is < 0, it is omitted.
 *  If len matches the previous value for command c, it is also omitted.
 *  If the output fills the line, a following newline is written.
 */

novalue evcmd(addr, len, c)
word addr, len;
int c;
   {
   if (!EventStream)
      return;
   if (addr >= 0) {
      evdec((uword)addr);
      evchar(E_Offset);
      }
   etvalue(len, c);
   if (llen >= LLIM)
      evnewline();
   else
      evspace();
   }

/*
 * etvalue(n, c) - output length n with character c.
 * Omit the length if it matches the previous value for c.
 */
static novalue etvalue(n, c)
word n;
int c;
   {
   if (n != curvalue[c-CurBias])
      evdec((uword)(curvalue[c-CurBias] = n));
   evchar(c); 
   }

/*
 * evdec(n) - output a decimal value, updating the line length.
 */
static novalue evdec (n)
uword n;
   {
   if (n > 9)
      evdec(n / 10);
   n %= 10;
   evchar('0'+(int)n);
   }

/*
 * evnewline() - output a newline and reset the line length.
 */
static novalue evnewline()
   {
   if (llen > 0)  {
      putc('\n', monfile);
      llen = 0;
      }
   }

/*
 * evforget() - clear the history of remembered lengths.
 */
static novalue evforget()
   {
   int c;

   for (c = 0; c < CurSize; c++)
      curvalue[c] = -1;
   }
#endif					/* EventMon */

#else					/* MemMon */
static char x;			/* avoid empty module */
#endif					/* MemMon */
