/* @(#)md5.h	10.1 3/25/94 08:04:11 */
/*
 * md5 - RSA Data Security, Inc. MD5 Message-Digest Algorithm
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
 * See md5drvr.c for version and modification history.
 */

/*
 ***********************************************************************
 ** Copyright (C) 1990, RSA Data Security, Inc. All rights reserved.  **
 **								      **
 ** License to copy and use this software is granted provided that    **
 ** it is identified as the "RSA Data Security, Inc. MD5 Message-     **
 ** Digest Algorithm" in all material mentioning or referencing this  **
 ** software or this function.					      **
 **								      **
 ** License is also granted to make and use derivative works	      **
 ** provided that such works are identified as "derived from the RSA  **
 ** Data Security, Inc. MD5 Message-Digest Algorithm" in all          **
 ** material mentioning or referencing the derived work.	      **
 **								      **
 ** RSA Data Security, Inc. makes no representations concerning       **
 ** either the merchantability of this software or the suitability    **
 ** of this software for any particular purpose.  It is provided "as  **
 ** is" without express or implied warranty of any kind.              **
 **								      **
 ** These notices must be retained in any copies of any part of this  **
 ** documentation and/or software.				      **
 ***********************************************************************
 */

#if !defined(MD5_H)
#define MD5_H

#include <sys/types.h>
#include <sys/stat.h>

/*
 * determine if we are checked in
 */
#define MD5_H_WHAT "@(#)"		/* will be @(#) or @(#) */

/*
 * Useful defines/typedefs
 */
typedef unsigned char BYTE;
typedef unsigned long ULONG;
typedef unsigned int UINT;

/* MD5_CHUNKSIZE must be a power of 2 - fixed value defined by the algorithm */
#define MD5_CHUNKSIZE	(1<<6)
#define MD5_CHUNKWORDS (MD5_CHUNKSIZE/sizeof(ULONG))

/* MD5_DIGESTSIZE is a the length of the digest as defined by the algorithm */
#define MD5_DIGESTSIZE	(16)
#define MD5_DIGESTWORDS (MD5_DIGESTSIZE/sizeof(ULONG))

/* MD5_LOW - where low 32 bits of 64 bit count is stored during final */
#define MD5_LOW 14

/* MD5_HIGH - where high 32 bits of 64 bit count is stored during final */
#define MD5_HIGH 15

/*
 * MAXBLOCK - maximum blocking factor
 *
 * must be power of 2 > MD5_CHUNKSIZE and < READSIZE and < 2^29
 */
#define MAXBLOCK (MD5_CHUNKSIZE<<7)

/* READSIZE must be a multiple of MD5_CHUNKSIZE >= MAXBLOCK */
#define READSIZE (MD5_CHUNKSIZE<<9)
#define READWORDS (READSIZE/sizeof(ULONG))

/* maximum size of pre_file that is used <= MAXBLOCK */
#define MAX_PRE_FILE MAXBLOCK

/*
 * COUNT(MD5_CTX*, ULONG) - update the 64 bit count in an MD5_CTX
 *
 * We will count bytes and convert to bit count during the final
 * transform.
 */
#define COUNT(md5info, count) {					\
    long tmp_countLo;						\
    tmp_countLo = (md5info)->countLo;				\
    if (((md5info)->countLo += (count)) < tmp_countLo) {	\
	(md5info)->countHi++;					\
    }								\
}

/*
 * ROUNDUP(x,y) - round x up to the next multiple of y
 */
#define ROUNDUP(x,y) ((((x)+(y)-1)/(y))*(y))

/*
 * Data structure for MD5 (Message-Digest) computation
 */
typedef struct {
    BYTE digest[MD5_DIGESTSIZE];	/* actual digest after MD5Final call */
    ULONG countLo;		/* 64 bit count: bits 3-34 */
    ULONG countHi;		/* 64 bit count: bits 35-63 (64-66 ignored) */
    ULONG datalen;		/* length of data in inp.inp_BYTE */
    ULONG sub_block;		/* length of current partial block or 0 */
    union {
	BYTE inp_BYTE[MD5_CHUNKSIZE];		       /* BYTE chunk buffer */
	ULONG inp_ULONG[MD5_CHUNKWORDS];  /* ULONG chunk buffer */
    } inp;
    ULONG buf[MD5_DIGESTWORDS];	       /* scratch buffer */
} MD5_CTX;
#define in_byte inp.inp_BYTE
#define in_ulong inp.inp_ULONG

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
 * Turn off prototypes if requested
 */
#if (defined(NOPROTO) && defined(PROTO))
# undef PROTO
#endif

/*
 * Used to remove arguments in function prototypes for non-ANSI C
 */
#ifdef PROTO
# define P(a) a
#else	/* !PROTO */
# define P(a) ()
#endif	/* ?PROTO */

/* md5.c */
void MD5Init P((MD5_CTX*));
void MD5Update P((MD5_CTX*, BYTE*, UINT));
void MD5fullUpdate P((MD5_CTX*, BYTE*, UINT));
void MD5Final P((MD5_CTX*));
char *MD5_what;

/* md5dual.c */
void dualMain P((int, char**, BYTE*, UINT, char*));
void dualTest P((void));
char *MD5dual_what;

/* md5drvr.c */
void MD5Print P((MD5_CTX*));
ULONG zero[];
char *program;
int i_flag;
int q_flag;
int dot_zero;

#endif /* MD5_H */
