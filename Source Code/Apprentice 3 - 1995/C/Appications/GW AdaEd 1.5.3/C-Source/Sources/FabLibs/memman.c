/*
ANSI C _working_ equivalents for malloc, free, realloc, calloc;
as a free bonus, you get ffcalloc() and fgetallocsize()
unexplicably missing from the standard libraries.

malloc & free implementation based on Kernighan&Ritchie, 2nd ed.;
realloc, calloc, ffcalloc, fgetallocsize by Fabrizio Oddone.

2/1/95
Modified the realloc routine in order to behave as described in
section 7.10.3.4 of ANSI/ISO-C;
that is:
if ptr is a null pointer, realloc() allocates size bytes,
else if size is 0, the memory block pointed to by ptr is freed.
*/


#include	<StdLib.h>
#include	<String.h>
#include	"UtilsSys7.h"
#include	"memman.h"

//typedef long Align;

typedef union header Header;

union header {
	struct {
		Header *ptr;	// next block if on free list
		size_t	size;		// size of this block (not bytes but MEMORY_UNITs)
		} s;
//	Align	x, y;
	};

#define	USETEMPORARYMEMORY	1
//#define	USE64BITUNITS	1

#ifdef	USE64BITUNITS
#define	MEMORY_UNIT	sizeof(Header)
#else
#define	MEMORY_UNIT	4	//sizeof(Header)
#endif

static Header base;	// empty list to get started
static Header *freep = NULL;	// start of free list

static Header *morecore(size_t nu);

#define	HDR(p)	((Header *)p)

void *fmalloc(size_t nbytes)
{
Ptr	p;
Header *prevp;
size_t	nunits, tempsize;

//DebugStr("\pNow entering fmalloc");
nunits = (nbytes + sizeof(Header) - 1) / MEMORY_UNIT + 1;
if ((prevp = freep) == NULL) {	// no free list yet
	base.s.ptr = freep = prevp = &base;
	base.s.size = 0;
	}

for (p = (Ptr)prevp->s.ptr; ; prevp = HDR(p), p = (Ptr)HDR(p)->s.ptr) {
	tempsize = HDR(p)->s.size;
	if (tempsize >= nunits) {	// big enough
		if (
#ifdef	USE64BITUNITS
		tempsize == nunits
#else
		tempsize <= nunits + 1
#endif
		)	// exactly
			prevp->s.ptr = HDR(p)->s.ptr;
		else {	// allocate tail end
			HDR(p)->s.size -= nunits;
			//p = (Header *)((Ptr)p + p->s.size * MEMORY_UNIT);
			p += HDR(p)->s.size * MEMORY_UNIT;
			HDR(p)->s.size = nunits;
			}
		freep = prevp;
		return (p + sizeof(Header));
		}
	if (HDR(p) == freep)	// wrapped around free list
		if ((p = (Ptr)morecore(nunits)) == NULL)
			return NULL;	// none left
	}
}

/* NALLOC determines the amount of memory asked to the Mac OS */
/* the allocated block is then handled by malloc */
/* currently morecore asks "at least 64Kbytes" (you may want smaller blocks) */

#define	NALLOC	(65536UL / MEMORY_UNIT)	// minimum # units to request

static Header *morecore(size_t nu)
{
Header *up;

if (nu < NALLOC)
	nu = NALLOC;
if (NULL == (up = (Header *)
#ifdef USETEMPORARYMEMORY
	NewHandleGeneral
#else
	NewPtr
#endif
		(nu * MEMORY_UNIT)))
	return NULL;	// no space left

#ifdef USETEMPORARYMEMORY
HLock((Handle)up);
#endif
up = (Header *)*(Handle)up;
up->s.size = nu;
ffree(up + 1);
return freep;
}

void ffree(void *bp)
{
Header *p, *tmp;

bp = HDR(bp) - 1;	// point to block header
// !(bp > p && bp < p->s.ptr)
//for (p = freep; bp <= p || bp >= p->s.ptr; p = p->s.ptr)
//	if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
//		break;	// freed block at start or end of arena

p = freep;
tmp = p->s.ptr;
while (bp <= p || bp >= tmp) {
	if (p >= tmp && (bp > p || bp < tmp))
		break;	// freed block at start or end of arena
	p = p->s.ptr;
	tmp = p->s.ptr;
	}

if ((Ptr)bp + HDR(bp)->s.size * MEMORY_UNIT == (Ptr)tmp) {	// join to upper neighbor
	HDR(bp)->s.size += tmp->s.size;
	HDR(bp)->s.ptr = tmp->s.ptr;
	}
else
	HDR(bp)->s.ptr = tmp;
if ((Ptr)p + p->s.size * MEMORY_UNIT == (Ptr)bp) {	// join to lower neighbor
	p->s.size += HDR(bp)->s.size;
	p->s.ptr = HDR(bp)->s.ptr;
	}
else
	p->s.ptr = bp;
freep = p;
}

/* reallocates and copies the old block only when necessary */

void *frealloc(void *ap, size_t s)
{
Ptr	bp;
Header *p, *new_block;
void *r = ap;
size_t	cursize, nunits, temp;

if (ap) {
	if (s) {
		bp = (Ptr)ap - sizeof(Header);
		nunits = (s + sizeof(Header) - 1) / MEMORY_UNIT + 1;
		cursize = HDR(bp)->s.size;
		if (nunits
		#ifndef	USE64BITUNITS
			+ 1
		#endif
			< cursize) {
		
			HDR(bp)->s.size = nunits;
			bp += nunits * MEMORY_UNIT;
			HDR(bp)->s.size = cursize - nunits;
			ffree(bp + sizeof(Header));
			}
		else if (nunits > cursize) {
			for (p = freep; !(bp > (Ptr)p && bp < (Ptr)p->s.ptr); p = p->s.ptr)
				if (p >= p->s.ptr && (bp > (Ptr)p || bp < (Ptr)p->s.ptr))
					break;	// freed block at start or end of arena
			temp = p->s.ptr->s.size + cursize;
			if ((bp + cursize * MEMORY_UNIT == (Ptr)p->s.ptr) && (temp >= nunits)) {
		// we can expand the block upwards in memory
				if (
		#ifdef	USE64BITUNITS
				temp == nunits
		#else
				temp <= nunits + 1
		#endif
				) {
		// there is an adjacent free block matching exactly our needs
					HDR(bp)->s.size = temp;
					p->s.ptr = p->s.ptr->s.ptr;
					}
				else {
		// we need to split the adjacent block
					new_block = (Header *) (bp + nunits * MEMORY_UNIT);
					new_block->s.ptr = p->s.ptr->s.ptr;
					new_block->s.size = temp - nunits;
					HDR(bp)->s.size = nunits;
					p->s.ptr = new_block;
					}
				freep = p;
				}
			else {
		// expanding not possible; reallocate, copy and free the old block
				if (r = fmalloc(s)) {
					BlockMoveData(ap, r, cursize * MEMORY_UNIT - sizeof(Header));
					ffree(ap);
					}
				}
			}
		}
	else {
		ffree(ap);
		r = NULL;
		}
	}
else
	r = fmalloc(s);

return r;
}

size_t fgetallocsize(const void *ap)
{
return ((Header *)ap - 1)->s.size * MEMORY_UNIT - sizeof(Header);
}

void *fcalloc(size_t nelem, size_t nsize)
{
void *p;
size_t	nbytes = nelem * nsize;

if (p = fmalloc(nbytes)) {
// memset is poorly implemented, so we use a better one
	MyZeroBuffer(p, numOfLongs(nbytes + 3));
	//(void) memset(p, 0, nbytes);
	}
return p;
}

void *ffcalloc(size_t s)
{
void *p;

if (p = fmalloc(s)) {
// memset is poorly implemented, so we use a better one
	MyZeroBuffer(p, numOfLongs(s + 3));
	//(void) memset(p, 0, nbytes);
	}
return p;
}
