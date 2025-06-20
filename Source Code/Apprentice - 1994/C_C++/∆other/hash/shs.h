/* @(#)shs.h	10.1 3/25/94 08:04:01 */
/*
 * shs - NIST proposed Secure Hash Standard
 *
 * Written 2 September 1992, Peter C. Gutmann.
 *
 * This file was Modified by:
 *
 *	 Landon Curt Noll  (chongo@toad.com)	chongo <was here> /\../\
 *
 * This code has been placed in the public domain.  Please do not
 * copyright this code.
 *
 * LANDON CURT NOLL DISCLAIMS ALL WARRANTIES WITH  REGARD  TO
 * THIS  SOFTWARE,  INCLUDING  ALL IMPLIED WARRANTIES OF MER-
 * CHANTABILITY AND FITNESS.  IN NO EVENT SHALL  LANDON  CURT
 * NOLL  BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM  LOSS  OF
 * USE,  DATA  OR  PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR  IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * See shsdrvr.c for version and modification history.
 */

#if !defined(SHS_H)
#define SHS_H

#include <sys/types.h>
#include <sys/stat.h>

/*
 * determine if we are checked in
 */
#define SHS_H_WHAT "@(#)"	/* @(#) if checked in */

/*
 * Useful defines/typedefs
 */
typedef unsigned char BYTE;	/* must be a 1 byte unsigned value */
typedef unsigned long UINT;	/* must be a 2 byte unsigned value */
typedef unsigned long ULONG;	/* must be a 4 byte unsigned value */

/* SHS_CHUNKSIZE must be a power of 2 - fixed value defined by the algorithm */
#define SHS_CHUNKSIZE (1<<6)
#define SHS_CHUNKWORDS (SHS_CHUNKSIZE/sizeof(ULONG))

/* SHS_DIGESTSIZE is a the length of the digest as defined by the algorithm */
#define SHS_DIGESTSIZE (20)
#define SHS_DIGESTWORDS (SHS_DIGESTSIZE/sizeof(ULONG))

/* SHS_LOW - where low 32 bits of 64 bit count is stored during final */
#define SHS_LOW 15

/* SHS_HIGH - where high 32 bits of 64 bit count is stored during final */
#define SHS_HIGH 14

/*
 * MAXBLOCK - maximum blocking factor
 *
 * must be power of 2 > SHS_CHUNKSIZE and <= READSIZE and < 2^29
 */
#define MAXBLOCK (SHS_CHUNKSIZE<<9)

/* READSIZE must be a multiple of SHS_CHUNKSIZE >= MAXBLOCK */
#define READSIZE (SHS_CHUNKSIZE<<9)
#define READWORDS (READSIZE/sizeof(ULONG))

/* maximum part of pre_file used - must be <= MAXBLOCK */
#define MAX_PRE_FILE MAXBLOCK

/*
 * COUNT(SHS_INFO*, ULONG) - update the 64 bit count in an SHS_INFO
 *
 * We will count bytes and convert to bit count during the final
 * transform.
 */
#define COUNT(shsinfo, count) {					\
    ULONG tmp_countLo;						\
    tmp_countLo = (shsinfo)->countLo;				\
    if (((shsinfo)->countLo += (count)) < tmp_countLo) {	\
	(shsinfo)->countHi++;					\
    }								\
}

/*
 * SWAP_BYTE_SEX(ULONG *dest, ULONG *src)
 *
 *	dest	- array of SHS_CHUNKWORDS ULONG of fixed data (may be src)
 *	src	- array of SHS_CHUNKWORDS ULONG of what to fix
 *
 * This macro will either switch to the opposite byte sex (Big Endian vs.
 *  Little Endian).
 */
#define SWAP_BYTE_SEX(dest, src) {	/* swap byte sex if needed */	\
    int tmp_i;	/* index */						\
    for (tmp_i=0; tmp_i < SHS_CHUNKWORDS; ++tmp_i) {			\
	((ULONG*)(dest))[tmp_i] =					\
	  (((ULONG*)(src))[tmp_i] << 16) |				\
	  (((ULONG*)(src))[tmp_i] >> 16);				\
	((ULONG*)(dest))[tmp_i] =					\
	  ((((ULONG*)(dest))[tmp_i] & 0xff00ff00UL) >> 8) |		\
	  ((((ULONG*)(dest))[tmp_i] & 0x00ff00ffUL) << 8);		\
    }									\
}

/* 
 * SHS_TRANSFORM(SHS_INFO *a, ULONG *b, ULONG *c)
 *
 * 	a	pointer to our current digest state
 *	b	pointer to SHS_CHUNKSIZE words of byte sex fixed data
 *	c	pointer to SHS_CHUNKSIZE words that do not need to be fixed
 */
#if BYTE_ORDER == BIG_ENDIAN
# define SHS_TRANSFORM(a,b,c)						\
    shsTransform(((SHS_INFO *)(a))->digest, (ULONG *)(c))
#else
# define SHS_TRANSFORM(a,b,c) { 					\
    SWAP_BYTE_SEX((b), (c));						\
    shsTransform(((SHS_INFO *)(a))->digest, (ULONG *)(b));		\
}
#endif

/*
 * ROUNDUP(x,y) - round x up to the next multiple of y
 */
#define ROUNDUP(x,y) ((((x)+(y)-1)/(y))*(y))

/*
 * The structure for storing SHS info
 *
 * We will assume that bit count is a multiple of 8.
 */
typedef struct {
    ULONG digest[SHS_DIGESTWORDS];	/* message digest */
    ULONG countLo;			/* 64 bit count: bits 3-34 */
    ULONG countHi;			/* 64 bit count: bits 35-63 */
    ULONG datalen;			/* length of data in data */
    ULONG data[SHS_CHUNKWORDS];		/* SHS chunk buffer */
} SHS_INFO;

/*
 * elements of the stat structure that we will process
 */
struct hashstat {
    dev_t st_dev;
    ino_t st_ino;
    mode_t st_mode;
    nlink_t st_nlink;
    uid_t st_uid;
    gid_t st_gid;
    off_t st_size;
    time_t st_mtime;
    time_t st_ctime;
};

/*
 * Used to remove arguments in function prototypes for non-ANSI C
 */
#if defined(__STDC__) && __STDC__ == 1
# define PROTO
#endif
#ifdef PROTO
# define P(a) a
#else	/* !PROTO */
# define P(a) ()
#endif	/* ?PROTO */

/* shs.c */
void shsInit P((SHS_INFO*));
void shsUpdate P((SHS_INFO*, BYTE*, ULONG));
void shsfullUpdate P((SHS_INFO*, BYTE*, ULONG));
void shsFinal P((SHS_INFO*));
char *shs_what;

/* shsdual.c */
void dualMain P((int, char**, BYTE*, UINT, char*));
void dualTest P((void));
char *shsdual_what;

/* shsdrvr.c */
void shsPrint P((SHS_INFO*));
ULONG zero[];
char *program;
int debug;
int i_flag;
int q_flag;
int dot_zero;

#endif /* SHS_H */
