/*
 * File: ralc.r
 *  Contents: allocation routines
 */

extern word alcnum;

#ifndef MultiThread
word coexp_ser = 2;	/* serial numbers for co-expressions; &main is 1 */
word list_ser = 1;	/* serial numbers for lists */
word set_ser = 1;	/* serial numbers for sets */
word table_ser = 1;	/* serial numbers for tables */
#endif					/* MultiThread */

/*
 * Note: function calls beginning with "MM" are just empty macros
 * unless MemMon is defined.
 */

/*
 * AlcBlk - allocate a block.
 */
#begdef AlcBlk(var, struct_nm, t_code, nbytes)
#ifdef FixedRegions
{
   struct region *rp = NULL;
#endif					/* FixedRegions */
#ifdef MultiThread
   MMAlc((word)nbytes,t_code);
#endif					/* MultiThread */
   /*
    * See if there is enough room in the block region.
    */
   if (DiffPtrs(blkend,blkfree) < nbytes) {
#ifdef FixedRegions
      /*
       * See if there is enough room in *any* block region
       */
      for (rp = curblock->next; rp; rp = rp->next) {
	 if (DiffPtrs(rp->end,rp->free) >= nbytes) {
	    var = (struct struct_nm *)rp->free;
	    rp->free += nbytes;
	    var->title = t_code;
	    break;
	    }
         }
      if (!rp) {
         for (rp = curblock->prev; rp; rp = rp->prev) {
	    if (DiffPtrs(rp->end,rp->free) >= nbytes) {
	       var = (struct struct_nm *)rp->free;
	       rp->free += nbytes;
	       var->title = t_code;
	       break;
	       }
            }
         }
      if (!rp)
#endif					/* FixedRegions */
      if (!collect(Blocks,nbytes))
          return NULL;
      }
   /*
    * If monitoring, show the allocation.
    */
#ifndef MultiThread
   MMAlc((word)nbytes,t_code);
#endif

   /*
    * Decrement the free space in the block region by the number of bytes
    *  allocated and return the address of the first byte of the allocated
    *  block.
    */
   blktotal += nbytes;
#ifdef FixedRegions
   if (!rp) {
#endif					/* FixedRegions */
   var = (struct struct_nm *)blkfree;
   blkfree += nbytes;
   var->title = t_code;
#ifdef FixedRegions
   }
}
#endif					/* FixedRegions */
#enddef

/*
 * AlcFixBlk - allocate a fixed length block.
 */
#define AlcFixBlk(var, struct_nm, t_code)\
   AlcBlk(var, struct_nm, t_code, sizeof(struct struct_nm))

/*
 * AlcVarBlk - allocate a variable-length block.
 */
#begdef AlcVarBlk(var, struct_nm, t_code, n_desc)
   {
   register uword size;

   /*
    * Variable size blocks are declared with one descriptor, thus
    *  we need add in only n_desc - 1 descriptors.
    */
   size = sizeof(struct struct_nm) + (n_desc - 1) * sizeof(struct descrip);
   AlcBlk(var, struct_nm, t_code, size)
   var->blksize = size;
   }
#enddef

/*
 * alcactiv - allocate a co-expression activation block.
 */

struct astkblk *alcactiv()
   {
   struct astkblk *abp;

   abp = (struct astkblk *)malloc((msize)sizeof(struct astkblk));

#ifdef FixedRegions
   /*
    * If malloc failed, attempt to free some co-expression blocks and retry.
    */
   if (abp == NULL) {
      collect(Static,0);
      abp = (struct astkblk *)malloc((msize)sizeof(struct astkblk));
      }
#endif                                  /* FixedRegions */

   if (abp == NULL)
      ReturnErrNum(305, NULL);
   abp->nactivators = 0;
   abp->astk_nxt = NULL;
   return abp;
   }

#ifdef LargeInts
/*
 * alcbignum - allocate an n-digit bignum in the block region
 */

struct b_bignum *alcbignum(n)
word n;
   {
   register struct b_bignum *blk;
   register uword size;

   size = sizeof(struct b_bignum) + ((n - 1) * sizeof(DIGIT));
   /* ensure whole number of words allocated */
   size = (size + WordSize - 1) & -WordSize;
   AlcBlk(blk, b_bignum, T_Lrgint, size);
   blk->blksize = size;
   blk->msd = blk->sign = 0;
   blk->lsd = n - 1;
   return blk;
   }
#endif					/* LargeInts */

/*
 * alccoexp - allocate a co-expression stack block.
 */

#if COMPILER
struct b_coexpr *alccoexp()
   {
   struct b_coexpr *ep;
   static int serial = 2; /* main co-expression is allocated elsewhere */

#ifdef ATTM32
   ep = (struct b_coexpr *)coexp_salloc(); /* 3B2/15/4000 stack */
#else                                   /* ATTM32 */
   ep = (struct b_coexpr *)malloc((msize)stksize);
#endif                                  /* ATTM32 */

#ifdef FixedRegions
   /*
    * If malloc failed or if there have been too many co-expression allocations
    * since a collection, attempt to free some co-expression blocks and retry.
    */

   if (ep == NULL || alcnum > AlcMax) {

      collect(Static,0);

#ifdef ATTM32           /* not needed, but here to play it safe */
      ep = (struct b_coexpr *)coexp_salloc(); /* 3B2/15/4000 stack */
#else                                   /* ATTM32 */
      ep = (struct b_coexpr *)malloc((msize)stksize);
#endif                                  /* ATTM32 */

      }
#endif                                  /* FixedRegions */

   if (ep == NULL)
      ReturnErrNum(305, NULL);

#ifdef FixedRegions
   alcnum++;                    /* increment allocation count since last g.c. */
#endif                                  /* FixedRegions */

   ep->title = T_Coexpr;
   ep->size = 0;
   ep->id = serial++;
   ep->nextstk = stklist;
   ep->es_tend = NULL;
   ep->file_name = "";
   ep->line_num = 0;
   ep->freshblk = nulldesc;
   ep->es_actstk = NULL;
   stklist = ep;
   MMStat((char *)ep, stksize, 'X');
   return ep;
   }
#else					/* COMPILER */
#ifdef MultiThread
/*
 * If this is a new program being loaded, an icodesize>0 gives the
 * hdr.hsize and a stacksize to use; allocate
 * sizeof(progstate) + icodesize + mstksize
 * Otherwise (icodesize==0), allocate a normal stksize...
 */
struct b_coexpr *alccoexp(icodesize, stacksize)
long icodesize, stacksize;
#else					/* MultiThread */
struct b_coexpr *alccoexp()
#endif					/* MultiThread */

   {
   struct b_coexpr *ep;

#ifdef MultiThread
   if (icodesize>0) {
#ifdef ATTM32
Deliberate Syntax Error
#else					/* ATTM32 */
      ep = (struct b_coexpr *)
	malloc((msize)(stacksize+
		       icodesize+
		       sizeof(struct progstate)+
		       sizeof(struct b_coexpr)));
#endif					/* ATTM32 */
      }
   else
#endif					/* MultiThread */

#ifdef ATTM32
   ep = (struct b_coexpr *)coexp_salloc(); /* 3B2/15/4000 stack */
#else                                   /* ATTM32 */
   ep = (struct b_coexpr *)malloc((msize)stksize);
#endif                                  /* ATTM32 */

   /*
    * If malloc failed or if there have been too many co-expression allocations
    * since a collection, attempt to free some co-expression blocks and retry.
    */

#ifdef FixedRegions
   if (ep == NULL || alcnum > AlcMax) {
#else                                   /* FixedRegions */
   if (ep == NULL) {
#endif                                  /* Fixed Regions */

      collect(Static, 0);

#ifdef MultiThread
      if (icodesize>0) {
#ifdef ATTM32
Deliberate Syntax Error
#else					/* ATTM32 */
         ep = (struct b_coexpr *)
	    malloc((msize)(mstksize+icodesize+sizeof(struct progstate)));
#endif					/* ATTM32 */
         }
      else
#endif					/* MultiThread */

#ifdef ATTM32
	 ep = (struct b_coexpr *)coexp_salloc(); /* 3B2/15/4000 stack */
#else                                   /* ATTM32 */
         ep = (struct b_coexpr *)malloc((msize)stksize);
#endif                                  /* ATTM32 */
      }
      if (ep == NULL) 
         ReturnErrNum(305, NULL);

#ifdef FixedRegions
   alcnum++;		/* increment allocation count since last g.c. */
#endif                                  /* FixedRegions */

   ep->title = T_Coexpr;
   ep->es_actstk = NULL;
   ep->size = 0;
#ifdef MultiThread
   ep->es_pfp = NULL;
   ep->es_gfp = NULL;
   ep->es_argp = NULL;
   ep->tvalloc = NULL;

   if (icodesize>0)
      ep->id = 1;
   else
#endif					/* MultiThread */
      ep->id = coexp_ser++;
   ep->nextstk = stklist;
   ep->es_tend = NULL;

#ifdef MultiThread
   /*
    * Initialize program state to self for &main; curpstate for others.
    */
   if(icodesize>0) ep->program = (struct progstate *)(ep+1);
   else ep->program = curpstate;
#endif					/* MultiThread */

   stklist = ep;
#ifdef MultiThread
   if (icodesize>0)
      MMStat((char *)ep, stacksize, E_Coexpr);
   else
#endif					/* MultiThread */
   MMStat((char *)ep, stksize, E_Coexpr);
   return ep;
   }
#endif					/* COMPILER */

/*
 * alccset - allocate a cset in the block region.
 */

struct b_cset *alccset()
   {
   register struct b_cset *blk;
   register int i;

   AlcFixBlk(blk, b_cset, T_Cset)
   blk->size = -1;              /* flag size as not yet computed */

   /*
    * Zero the bit array.
    */
   for (i = 0; i < CsetSize; i++)
     blk->bits[i] = 0;
   return blk;
   }

/*
 * alcfile - allocate a file block in the block region.
 */

struct b_file *alcfile(fd, status, name)
FILE *fd;
int status;
dptr name;
   {
   tended struct descrip tname = *name;
   register struct b_file *blk;

   AlcFixBlk(blk, b_file, T_File)
   blk->fd = fd;
   blk->status = status;
   blk->fname = tname;
   return blk;
   }

/*
 * alchash - allocate a hashed structure (set or table header) in the block
 *  region.
 */
union block *alchash(tcode)
int tcode;
   {
   register int i;
   register struct b_set *ps;
   register struct b_table *pt;
   word serial;

   if (tcode == T_Table) {
      serial = table_ser++;
      AlcFixBlk(pt, b_table, T_Table);
      ps = (struct b_set *)pt;
      }
   else {	/* tcode == T_Set */
      serial = set_ser++;
      AlcFixBlk(ps, b_set, T_Set);
      }
   ps->size = 0;
   ps->id = serial;
   ps->mask = 0;
   for (i = 0; i < HSegs; i++)
      ps->hdir[i] = NULL;
   return (union block *)ps;
   }

/*
 * alcsegment - allocate a slot block in the block region.
 */

struct b_slots *alcsegment(nslots)
word nslots;
   {
   uword size;
   register struct b_slots *blk;

   size = sizeof(struct b_slots) + WordSize * (nslots - HSlots);
   AlcBlk(blk, b_slots, T_Slots, size);
   blk->blksize = size;
   while (--nslots >= 0)
      blk->hslots[nslots] = NULL;
   return blk;
   }

/*
 * alclist - allocate a list header block in the block region.
 *
 *  Forces a g.c. if there's not enough room for the whole list.
 */

struct b_list *alclist(size)
uword size;
   {
   register struct b_list *blk;

   if (!blkreserve((word)(sizeof(struct b_list) + sizeof (struct b_lelem)
      + (size - 1) * sizeof(struct descrip)))) return NULL;
   AlcFixBlk(blk, b_list, T_List)
   blk->size = size;
   blk->id = list_ser++;
   blk->listhead = NULL;
   blk->listtail = NULL;
   return blk;
   }

/*
 * alclstb - allocate a list element block in the block region.
 */

struct b_lelem *alclstb(nslots, first, nused)
uword nslots, first, nused;
   {
   register struct b_lelem *blk;
   register word i, size;

   AlcVarBlk(blk, b_lelem, T_Lelem, nslots)
   blk->nslots = nslots;
   blk->first = first;
   blk->nused = nused;
   blk->listprev = NULL;
   blk->listnext = NULL;
   /*
    * Set all elements to &null.
    */
   for (i = 0; i < nslots; i++)
      blk->lslots[i] = nulldesc;
   return blk;
   }

/*
 * alcreal - allocate a real value in the block region.
 */

struct b_real *alcreal(val)
double val;
   {
   register struct b_real *blk;

   AlcFixBlk(blk, b_real, T_Real)

#ifdef Double
/* access real values one word at a time */
   { int *rp, *rq;
     rp = (int *) &(blk->realval);
     rq = (int *) &val;
     *rp++ = *rq++;
     *rp   = *rq;
   }
#else                                   /* Double */
   blk->realval = val;
#endif                                  /* Double */

   return blk;
   }

/*
 * alcrecd - allocate record with nflds fields in the block region.
 */

struct b_record *alcrecd(nflds, recptr)
int nflds;
union block *recptr;
   {
   tended union block *trecptr = recptr;
   register struct b_record *blk;
   register int i;

   AlcVarBlk(blk, b_record, T_Record, nflds)
   blk->recdesc = trecptr;
   blk->id = (((struct b_proc *)recptr)->recid)++;
   return blk;
   }

/*
 * alcrefresh - allocate a co-expression refresh block.
 */

#if COMPILER
struct b_refresh *alcrefresh(na, nl, nt, wrk_sz)
int na;
int nl;
int nt;
int wrk_sz;
   {
   struct b_refresh *blk;

   AlcVarBlk(blk, b_refresh, T_Refresh, na + nl)
   blk->nlocals = nl;
   blk->nargs = na;
   blk->ntemps = nt;
   blk->wrk_size = wrk_sz;
   return blk;
   }
#else					/* COMPILER */
struct b_refresh *alcrefresh(entryx, na, nl)
word *entryx;
int na, nl;
   {
   int size;
   struct b_refresh *blk;

   AlcVarBlk(blk, b_refresh, T_Refresh, na + nl);
   blk->ep = entryx;
   blk->numlocals = nl;
   return blk;
   }
#endif					/* COMPILER */

/*
 * alcselem - allocate a set element block.
 */

struct b_selem *alcselem(mbr,hn)
uword hn;
dptr mbr;

   {
   tended struct descrip tmbr = *mbr;
   register struct b_selem *blk;

   AlcFixBlk(blk, b_selem, T_Selem)
   blk->clink = NULL;
   blk->setmem = tmbr;
   blk->hashnum = hn;
   return blk;
   }

/*
 * alcstr - allocate a string in the string space.
 */

char *alcstr(s, slen)
register char *s;
register word slen;
   {
   tended struct descrip ts;
   register char *d;
   register uword fspace;
   char *ofree;
#ifdef FixedRegions
   struct region *rp = NULL;
#endif					/* FixedRegions */

#ifdef MultiThread
   StrLen(ts) = slen;
   StrLoc(ts) = s;
#ifdef EventMon
   if (!noMTevents)
#endif					/* EventMon */
      MMStr(slen);
   s = StrLoc(ts);
#endif					/* MultiThread */
   /*
    * See if there is enough room in the string space.
    */
   fspace = DiffPtrs(strend,strfree);
   if (fspace < slen) {
#ifdef FixedRegions
      /*
       * See if there is enough room in *any* string space.
       */
      for (rp = curstring->next; rp; rp = rp->next) {
	 if (DiffPtrs(rp->end,rp->free) >= slen) {
	    ofree = d = (char *)rp->free;
	    rp->free += slen;
	    break;
	  }
       }
      if (!rp) {
	 for (rp = curstring->prev; rp; rp = rp->prev) {
	    if (DiffPtrs(rp->end,rp->free) >= slen) {
	       ofree = d = (char *)rp->free;
	       rp->free += slen;
	       break;
	       }
	    }
         }
      if (!rp) {
#endif					/* FixedRegions */
         StrLen(ts) = slen;
         StrLoc(ts) = s;
         if (!collect(Strings,slen))
            return NULL;
         s = StrLoc(ts);
#ifdef FixedRegions
         }
#endif					/* FixedRegions */
      }

   strtotal += slen;

   /*
    * Copy the string into the string space, saving a pointer to its
    *  beginning.  Note that s may be null, in which case the space
    *  is still to be allocated but nothing is to be copied into it.
    */
#ifdef FixedRegions
   if (!rp)
#endif					/* FixedRegions */
      ofree = d = strfree;
   if (s) {
      while (slen-- > 0)
         *d++ = *s++;
      }

   else
      d += slen;
#ifdef FixedRegions
   if (!rp)
#endif					/* FixedRegions */
      strfree = d;
   return ofree;
   }

/*
 * alcsubs - allocate a substring trapped variable in the block region.
 */

struct b_tvsubs *alcsubs(len, pos, var)
word len, pos;
dptr var;
   {
   tended struct descrip tvar = *var;
   register struct b_tvsubs *blk;

   AlcFixBlk(blk, b_tvsubs, T_Tvsubs)
   blk->sslen = len;
   blk->sspos = pos;
   blk->ssvar = tvar;
   return blk;
   }

/*
 * alctelem - allocate a table element block in the block region.
 */

struct b_telem *alctelem()
   {
   register struct b_telem *blk;

   AlcFixBlk(blk, b_telem, T_Telem)
   blk->hashnum = 0;
   blk->clink = NULL;
   blk->tref = nulldesc;
   blk->tval = nulldesc;
   return blk;
   }

/*
 * alctvtbl - allocate a table element trapped variable block in the block
 *  region.
 */

struct b_tvtbl *alctvtbl(tbl, ref, hashnum)
register dptr tbl, ref;
uword hashnum;
   {
   tended struct descrip ttbl = *tbl;
   tended struct descrip tref = *ref;
   register struct b_tvtbl *blk;

   AlcFixBlk(blk, b_tvtbl, T_Tvtbl)
   blk->hashnum = hashnum;
   blk->clink = BlkLoc(ttbl);
   blk->tref = tref;
   blk->tval = nulldesc;
   return blk;
   }

/*
 * blkreserve - ensure that n bytes are available in the block region.
 */
char *blkreserve (nbytes)
word nbytes;
{
   if (DiffPtrs(blkend,blkfree) < nbytes)
      if (!collect(Blocks,nbytes))
	 return NULL;
   return blkfree;
}

/*
 * deallocate - return a block to the heap.
 *
 *  The block must be the one that is at the very end of the block region.
 */
novalue deallocate (bp)
union block *bp;
{
   word nbytes;

   nbytes = BlkSize(bp);
   if ((char *)bp + nbytes != blkfree) {
#ifdef FixedRegions
      /*
       * The block must be at the very end of *any* block region.
       */
      struct region *rp;
      for(rp = curblock->next; rp; rp = rp->next)
         if ((char *)bp + nbytes == rp->free) {
            rp->free = (char *)bp;
	    blktotal -= nbytes;
	    MMAlc(-nbytes, 0);
            return;
            }
      for(rp = curblock->prev; rp; rp = rp->prev)
         if ((char *)bp + nbytes == rp->free) {
            rp->free = (char *)bp;
	    blktotal -= nbytes;
	    MMAlc(-nbytes, 0);
            return;
            }
#endif					/* FixedRegions */
      syserr ("deallocation botch");
      }
   blkfree = (char *)bp;
   blktotal -= nbytes;
   MMAlc(-nbytes, 0);
}

/*
 * strreserve - ensure that n bytes are available in the string region.
 */
char *strreserve (nbytes)
word nbytes;
{
   if (DiffPtrs(strend,strfree) < nbytes)
      if (!collect(Strings,nbytes))
	 return NULL;
   return strfree;
}
