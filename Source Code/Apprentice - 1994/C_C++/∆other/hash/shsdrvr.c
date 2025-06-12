/* %W% %G% %U% */
/*
 * shsdrvr - shs driver code
 *
 * This file was written by:
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
 ***
 *
 * NOTE: The version information below refers to all shs code, not
 *	 just this file.  In particular, this file was created by
 *	 Landon Curt Noll.
 *
 * Version 1.1: 02 Sep 1992?			original authors
 *     This code is based on code by Peter C. Gutmann.  Much thanks goes
 *     to Peter C. Gutman (pgut1@cs.aukuni.ac.nz) , Shawn A. Clifford
 *     (sac@eng.ufl.edu), Pat Myrto (pat@rwing.uucp) and others who wrote
 *     and/or worked on the original code.
 *
 * Version 2.1:	31 Dec 1993		Landon Curt Noll   (chongo@toad.com)
 *     Reformatted, performance improvements and bug fixes
 *
 * Version 2.2:	02 Jan 1994		Landon Curt Noll   (chongo@toad.com)
 *     fixed -p usage
 *     better error messages
 *     added -c help
 *     added -c 0	(concatenation)
 *     reordered -i stat buffer pre-pending
 *
 * Version 2.3:	03 Jan 1994		Landon Curt Noll   (chongo@toad.com)
 *     added -c 1	(side by side)
 *     added -c 2	(even force to be odd)
 *     added -c x	(shs dual test suite)
 *     changed -c help to be -c h
 *     changed -c operand to type[,opt[,...]]
 *     prefix & string ABI now can take arbitrary binary data
 *     fixed memory leak
 *     fixed even/odd byte split bug
 *     added -P file
 *     added -q
 *     added UNROLL_LOOPS to control shs.c loop unrolling
 *     major performance improvements
 *
 * Version 2.4: 05 Jan 1994		Landon Curt Noll   (chongo@toad.com)
 *     renamed calc mode to dual mode
 *     removed all -c code
 *     added -d		(dual digests, space separated)
 *     rewrote most of the file, string and stream logic using shsdual code
 *
 * Version 2.5: 08 Jan 1994		Landon Curt Noll   (chongo@toad.com)
 *     added (new) -c	(print 0x in front of digests)
 *     removed st_blksize and st_blocks from -i preprocessing data
 *     only print .0 suffix if -i and digesting a file
 *     non-zero edit codes are now unique
 *     changed the dual test suite (shorter, added non alpha numeric chars)
 *     -i requires filenames
 *     fixed @(#) what string code
 *     boolean logic simplication by Rich Schroeppel (rcs@cs.arizona.edu)
 *     on the fly in a circular buffer by Colin Plumb (colin@nyx10.cs.du.edu)
 *
 * Version 2.6: 11 Jan 1994		Landon Curt Noll   (chongo@toad.com)
 *     Merged the shs and md5 Makefiles to build both in the same directory
 *     alignment and byte order performance improvements
 *     eliminate wateful memory copies
 *     shs transform contains no function calls
 *     beta release
 *
 * Version 2.7: 13 Jan 1994		Landon Curt Noll   (chongo@toad.com)
 *     code cleanup
 *     chunk is now 64 bytes, block is determined by blocking factor
 *     magic 64 and 64 related values defined in terms of #defines
 *     added blocking code (-b block_len)
 *     added xor feedback code (-f)
 *     added xor feedback and block options to performance test
 *     performance improvements
 *
 * Version 2.8: 16 Jan 1994		Landon Curt Noll   (chongo@toad.com)
 *     code cleanup
 *     performance improvements
 *     removed blocking and feedback code
 *     count bytes in driver, convert to 64 bit count in final transform
 *     added debug mode
 *     handle read errors and EOF better
 *     prefix strings not multiple of 64 bytes in length do not slow down hash
 *     renumbered exit codes
 *     fixed dual digest split bug
 *     byte sex swapping is now controlled thru the SHS_TRANSFORM macro
 *     shsTransform() is now called via the SHS_TRANSFORM macro
 *
 * Version 2.9: 12 Feb 1994		Landon Curt Noll   (chongo@toad.com)
 *     prep for beta release
 *     removed all feedback code
 *
 * Version 2.10: 25 Mar 1994		Landon Curt Noll   (chongo@toad.com)
 *     must_align catchs signal to detect misaligned access
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "shs.h"

/* size of test in megabytes */
#define TEST_MEG 16

/* number of chunks to process */
#define TEST_CHUNKS (TEST_MEG*1024*1024/READSIZE)

/* SHS test suite strings */
#define ENTRY(str) {(BYTE *)str, NULL, sizeof(str)-1}
struct shs_test {
    BYTE *ro_data;	/* read only string data or NULL to test */
    BYTE *data;		/* data or NULL to test */
    int len;		/* length of data */
} shs_test_data[] = {
    ENTRY(""),
    ENTRY("a"),
    ENTRY("abc"),
    ENTRY("message digest"),
    ENTRY("abcdefghijklmnopqrstuvwxyz"),
    ENTRY("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"),
    ENTRY("12345678901234567890123456789012345678901234567890123456789012345678901234567890")
};
#define MAX_SHS_TEST_DATA (sizeof(shs_test_data)/sizeof(shs_test_data[0]))

/* shs test filenames */
char *shs_test_file[] = {
    "file1",
    "file2",
};
#define MAX_SHS_TEST_FILE (sizeof(shs_test_file)/sizeof(shs_test_file[0]))

/* where the test files are located by default */
#if !defined(TLIB)
#define TLIB "."
#endif

/* Prototypes of the static functions */
static void shsStream P((BYTE*, UINT, FILE*, SHS_INFO*));
static void shsFile P((BYTE*, UINT, char*, SHS_INFO*));
static void shsOutput P((char*, int, SHS_INFO*));
static int shsPreFileRead P((char*, BYTE**));
static void shsTestSuite P((void));
static void shsHelp P((void));
void main P((int, char**));

/* global variables */
char *program;			/* our name */
static int c_flag = 0;		/* 1 => print C style digest with leading 0x */
int debug = 0;			/* 1 => add debug */
int i_flag = 0;			/* 1 => process inode & filename */
int q_flag = 0;			/* 1 => print the digest only */
int dot_zero = 0;		/* 1 => print .0 after the digest */
ULONG zero[SHS_CHUNKWORDS];	/* block of zeros */


/*
 * shsStream - digest a open file stream
 */
static void
shsStream(pre_str, pre_len, stream, dig)
    BYTE *pre_str;		/* data prefix or NULL */
    UINT pre_len;		/* length of pre_str */
    FILE *stream;		/* the stream to process */
    SHS_INFO *dig;		/* current digest */
{
    ULONG data[READWORDS];	/* our read buffer */
    int bytes;			/* bytes last read */
    int ret;			/* partial fread return value */

    /*
     * pre-process prefix if needed
     */
    if (pre_str != NULL) {
	shsUpdate(dig, pre_str, pre_len);
	COUNT(dig, pre_len);
    }

    /*
     * if we have a partial chunk, try to read until we have a full chunk
     */
    clearerr(stream);
    if (dig->datalen > 0) {

        /* determine what we have so far */
        bytes = dig->datalen;

	/* try to read what we need to fill the chunk */
	while (bytes < SHS_CHUNKSIZE) {

	    /* try to read what we need */
	    ret = fread((char*)data+bytes, 1, SHS_CHUNKSIZE-bytes, stream);

	    /* carefully examine the result */
	    if (ret < 0 || ferror(stream)) {
		/* error processing */
		fprintf(stderr, "%s: ", program);
		perror("read #3 error");
		exit(1);
	    } else if (ret == 0 || feof(stream)) {
		/* EOF processing */
		COUNT(dig, SHS_CHUNKSIZE-dig->datalen);
		shsUpdate(dig, (BYTE *)data+dig->datalen, 
		  SHS_CHUNKSIZE-dig->datalen);
		return;
	    }

	    /* note that we have more bytes */
	    bytes += ret;
        }
        COUNT(dig, SHS_CHUNKSIZE-dig->datalen);
        shsUpdate(dig, (BYTE *)data+dig->datalen, SHS_CHUNKSIZE-dig->datalen);
    }

    /*
     * process the contents of the file
     */
    while ((bytes = fread((char *)data, 1, READSIZE, stream)) > 0) {

	/*
	 * if we got a partial read, try to read up to a full chunk
	 */
	while (bytes < READSIZE) {

	    /* try to read more */
	    ret = fread((char *)data+bytes, 1, READSIZE-bytes, stream);

	    /* carefully examine the result */
	    if (ret < 0 || ferror(stream)) {
	    	/* error processing */
	    	fprintf(stderr, "%s: ", program);
	    	perror("read #1 error");
	    	exit(2);
	    } else if (ret == 0 || feof(stream)) {
	    	/* EOF processing */
	    	shsUpdate(dig, (BYTE *)data, bytes);
	    	COUNT(dig, bytes);
	    	return;
	    }

	    /* note that we have more bytes */
	    bytes += ret;
	}

	/*
	 * digest the read
	 */
	shsfullUpdate(dig, (BYTE *)data, bytes);
	COUNT(dig, bytes);
    }

    /*
     * watch for errors
     */
    if (bytes < 0 || ferror(stream)) {
	/* error processing */
	fprintf(stderr, "%s: ", program);
	perror("read #2 error");
	exit(3);
    }
    return;
}


/*
 * shsFile - digest a file
 */
static void
shsFile(pre_str, pre_len, filename, dig)
    BYTE *pre_str;		/* string prefix or NULL */
    UINT pre_len;		/* length of pre_str */
    char *filename;		/* the filename to process */
    SHS_INFO *dig;		/* current digest */
{
    FILE *inFile;		/* the open file stream */
    struct stat buf;		/* stat or lstat of file */
    struct hashstat hashbuf;	/* stat data to digest */
    struct hashstat hashlbuf;	/* lstat data to digest */
    ULONG filename_len;		/* length of the filename */

    /*
     * open the file
     */
    inFile = fopen(filename, "rb");
    if (inFile == NULL) {
	fprintf(stderr, "%s: cannot open %s: ", program, filename);
	perror("");
	return;
    }

    /*
     * pre-process prefix if needed
     */
    if (pre_str == NULL) {
	if (i_flag) {
	    filename_len = strlen(filename);
	    shsUpdate(dig, (BYTE *)filename, filename_len);
	    COUNT(dig, filename_len);
#if defined(DEBUG)
	    if (debug) {
		fprintf(stderr, "DEBUG: filename_len:%d count:%d\n",
		    filename_len, dig->countLo);
	    }
#endif
	}
    } else {
	if (i_flag) {
	    shsUpdate(dig, pre_str, pre_len);
	    filename_len = strlen(filename);
	    shsUpdate(dig, (BYTE *)filename, filename_len);
	    COUNT(dig, filename_len+pre_len);
#if defined(DEBUG)
	    if (debug) {
		fprintf(stderr, "DEBUG: pre_len:%d filename_len:%d count:%d\n",
		    pre_len, filename_len, dig->countLo);
	    }
#endif
	} else {
	    shsUpdate(dig, pre_str, pre_len);
	    COUNT(dig, pre_len);
	}
    }

    /*
     * digest file stat and lstat
     */
    if (i_flag) {
	if (fstat(fileno(inFile), &buf) < 0) {
	    printf("%s can't be stated.\n", filename);
	    return;
	}
	hashbuf.st_dev = buf.st_dev;
	hashbuf.st_ino = buf.st_ino;
	hashbuf.st_mode = buf.st_mode;
	hashbuf.st_nlink = buf.st_nlink;
	hashbuf.st_uid = buf.st_uid;
	hashbuf.st_gid = buf.st_gid;
	hashbuf.st_size = buf.st_size;
	hashbuf.st_mtime = buf.st_mtime;
	hashbuf.st_ctime = buf.st_ctime;
#if defined(DEBUG)
	if (debug) {
	    fprintf(stderr, 
	      "DEBUG: dev:%d ino:%d mode:%o nlink:%d uid:%d gid:%d\n",
	      hashbuf.st_dev, hashbuf.st_ino, hashbuf.st_mode,
	      hashbuf.st_nlink, hashbuf.st_uid, hashbuf.st_gid);
	    fprintf(stderr, 
	      "DEBUG: size:%d mtime:%d ctime:%d\n",
	      hashbuf.st_size, hashbuf.st_mtime, hashbuf.st_ctime);
	}
#endif
	shsUpdate(dig, (BYTE *)&hashbuf, sizeof(hashbuf));
	if (lstat(filename, &buf) < 0) {
	    printf("%s can't be lstated.\n", filename);
	    return;
	}
	hashlbuf.st_dev = buf.st_dev;
	hashlbuf.st_ino = buf.st_ino;
	hashlbuf.st_mode = buf.st_mode;
	hashlbuf.st_nlink = buf.st_nlink;
	hashlbuf.st_uid = buf.st_uid;
	hashlbuf.st_gid = buf.st_gid;
	hashlbuf.st_size = buf.st_size;
	hashlbuf.st_mtime = buf.st_mtime;
	hashlbuf.st_ctime = buf.st_ctime;
#if defined(DEBUG)
	if (debug) {
	    fprintf(stderr, 
	      "DEBUG: ldev:%d lino:%d mode:%o lnlink:%d luid:%d lgid:%d\n",
	      hashlbuf.st_dev, hashlbuf.st_ino, hashlbuf.st_mode,
	      hashlbuf.st_nlink, hashlbuf.st_uid, hashlbuf.st_gid);
	    fprintf(stderr, 
	      "DEBUG: lsize:%d lmtime:%d lctime:%d\n",
	      hashlbuf.st_size, hashlbuf.st_mtime, hashlbuf.st_ctime);
	}
#endif
	shsUpdate(dig, (BYTE *)&hashlbuf, sizeof(hashlbuf));

	/*
	 * pad with zeros to process file data faster
	 */
	if (dig->datalen > 0) {
#if defined(DEBUG)
	    if (debug) {
		fprintf(stderr, 
		  "DEBUG: pad_len:%d\n", SHS_CHUNKSIZE - dig->datalen);
	    }
#endif
	    COUNT(dig, sizeof(hashbuf) + sizeof(hashlbuf) + 
	          SHS_CHUNKSIZE - dig->datalen);
	    shsUpdate(dig, (BYTE *)zero, SHS_CHUNKSIZE - dig->datalen);
	} else {
	    COUNT(dig, sizeof(hashbuf) + sizeof(hashlbuf));
	}
#if defined(DEBUG)
	if (debug) {
	    fprintf(stderr, "DEBUG: datalen:%d count:%d\n", 
	      dig->datalen, dig->countLo);
	}
#endif
    }

    /*
     * process the data stream
     */
    shsStream(NULL, 0, inFile, dig);
    fclose(inFile);
}


/*
 * shsOutput - output the digest
 */
static void
shsOutput(str, quot, dig)
    char *str;		/* print string after digest, NULL => none */
    int quot;		/* 1 => surround str with a double quotes */
    SHS_INFO *dig;	/* current digest */
{
    /*
     * finalize the digest
     */
    shsFinal(dig);
#if defined(DEBUG)
    if (debug) {
	fprintf(stderr, 
	  "DEBUG: 64 bit count: 0x%08x%08x\n",
	  ((dig->countHi << 3) | (dig->countLo >> 29)), (dig->countLo << 3));
    }
#endif

    /*
     * print the digest
     */
    shsPrint(dig);
    if (str && !q_flag) {
	if (quot) {
	    printf(" \"%s\"\n", str);
	} else {
	    printf(" %s\n", str);
	}
    } else {
	putchar('\n');
    }
    fflush(stdout);
}


/*
 * shsPrint - print a digest in hex
 *
 * Prints message digest buffer in shsInfo as 40 hexadecimal digits. Order is
 * from low-order byte to high-order byte of digest. Each byte is printed
 * with high-order hexadecimal digit first.
 *
 * If -c, then print a leading "0x".  If -i, then print a trailing ".0".
 */
void
shsPrint(shsInfo)
    SHS_INFO *shsInfo;
{
    if (c_flag) {
	fputs("0x", stdout);
    }
    printf("%08lx%08lx%08lx%08lx%08lx",
	shsInfo->digest[0], shsInfo->digest[1], shsInfo->digest[2],
	shsInfo->digest[3], shsInfo->digest[4]);
    if (dot_zero) {
	fputs(".0", stdout);
    }
}


/*
 * shsTimeTrial - measure the speed of SHS
 *
 * Measures user time required to digest TEST_MEG megabytes of characters.
 *
 * This function will time blocking and under xor feedback mode if they
 * are set.
 */
static void
shsTimeTrial()
{
    ULONG data[READWORDS];	/* test buffer */
    SHS_INFO shsInfo;		/* hash state */
    struct rusage start;	/* test start time */
    struct rusage stop;		/* test end time */
    double usrsec;		/* duration of test in user seconds */
    unsigned int i;

    /*
     * initialize test data
     */
    for (i = 0; i < READSIZE; i++) {
	((BYTE *)data)[i] = (BYTE)(i & 0xFF);
    }

    /*
     * announce test
     */
    if (!q_flag) {
	printf("shs time trial for %d megs of test data ...", TEST_MEG);
	fflush(stdout);
    }

    /* 
     * digest data in READSIZE byte chunks
     */
    getrusage(RUSAGE_SELF, &start);
    shsInit(&shsInfo);
    for (i=0; i < TEST_CHUNKS; ++i) {
	shsfullUpdate(&shsInfo, (BYTE *)data, READSIZE);
    }
    COUNT(&shsInfo, READSIZE*TEST_CHUNKS);
    shsFinal(&shsInfo);
    getrusage(RUSAGE_SELF, &stop);

    /*
     * announce the test results
     */
    usrsec = (stop.ru_utime.tv_sec - start.ru_utime.tv_sec) +
    	   (double)(stop.ru_utime.tv_usec - start.ru_utime.tv_usec)/1000000.0;
    if (!q_flag) {
	putchar('\n');
    }
    shsPrint(&shsInfo);
    if (q_flag) {
	putchar('\n');
    } else {
	printf(" is digest of test data\n");
	printf("user seconds to process test data: %.2f\n", usrsec);
	printf("characters processed per user second: %d\n",
	    (int)((double)TEST_MEG*1024.0*1024.0/usrsec));
    }
}


/*
 * shsTestSuite - run a standard suite of test data
 */
static void
shsTestSuite()
{
    struct shs_test *t;		/* current shs test */
    struct stat buf;		/* stat of a test file */
    SHS_INFO digest;		/* test digest */
    char **f;			/* current file being tested */
    int i;

    /*
     * copy our test strings into writable data
     */
    for (i=0, t=shs_test_data; i < MAX_SHS_TEST_DATA; ++i, ++t) {
	if (t->ro_data != NULL) {
	    t->data = (BYTE *)malloc(t->len + 1);
	    if (t->data == NULL) {
		fprintf(stderr, "%s: malloc #4 failed\n", program);
		exit(4);
	    }
	    strcpy((char *)t->data, (char *)t->ro_data);
        }
    }

    /*
     * print test header
     */
    puts("shs test suite results:");

    /*
     * find all of the test files
     */
    for (i=0, f=shs_test_file; i < MAX_SHS_TEST_FILE; ++i, ++f) {
	if (stat(*f, &buf) < 0) {
	    /* no file1 in this directory, cd to the test suite directory */
	    if (chdir(TLIB) < 0) {
		fflush(stdout);
		fprintf(stderr,
		    "%s: cannot find %s or %s/%s\n", program, *f, TLIB, *f);
		return;
	    }
	}
    }

    /*
     * try all combinations of test strings as prefixes and data
     */
    for (i=0, t=shs_test_data; i < MAX_SHS_TEST_DATA; ++i, ++t) {
	shsInit(&digest);
	shsUpdate(&digest, t->data, t->len);
	COUNT(&digest, t->len);
	shsOutput((char *)t->ro_data, 1, &digest);
    }

    /*
     * try the files with all test strings as prefixes
     */
    for (i=0, f=shs_test_file; i < MAX_SHS_TEST_FILE; ++i, ++f) {
	shsInit(&digest);
	shsFile(NULL, 0, *f, &digest);
	shsOutput(*f, 0, &digest);
    }
    exit(0);
}


/*
 * shsPreFileRead - read and process a prepend file
 *
 * Returns the length of pre_str, and modifies pre_str to
 * point at the malloced prepend data.
 */
static int
shsPreFileRead(pre_file, buf)
    char *pre_file;		/* form pre_str from file pre_file */
    BYTE **buf;			/* pointer to pre_str pointer */
{
    struct stat statbuf;	/* stat for pre_file */
    int pre_len;		/* length of pre_file to be used */
    int bytes;			/* bytes read from pre_file */
    FILE *pre;			/* pre_file descriptor */

    /* obtain the length that we will use */
    if (stat(pre_file, &statbuf) < 0) {
	fprintf(stderr, "%s: unpable to find prepend file %s\n",
	    program, pre_file);
	exit(5);
    }
    pre_len = statbuf.st_size;
    if (pre_len > MAX_PRE_FILE) {
	/* don't use beyond MAX_PRE_FILE in size */
	pre_len = MAX_PRE_FILE;
    }

    /* malloc our pre string */
    *buf = (BYTE *)malloc(pre_len+1);
    if (*buf == NULL) {
	fprintf(stderr, "%s: malloc #3 failed\n", program);
	exit(6);
    }

    /* open our pre_file */
    pre = fopen(pre_file, "rb");
    if (pre == NULL) {
	fprintf(stderr, "%s: unable to open prepend file %s\n",
	  program, pre_file);
	exit(7);
    }

    /* read our pre_file data */
    bytes = fread((char *)(*buf), 1, pre_len, pre);
    if (bytes != pre_len) {
	fprintf(stderr,
	  "%s: unable to read %d bytes from prepend file %s\n",
	  program, pre_len, pre_file);
	exit(8);
    }

    /* return our length */
    return (pre_len);
}


/*
 * shsHelp - print shs help message and exit
 */
static void
shsHelp()
{
    fprintf(stderr,
      "%s [-cd%shiqtx][-p prefix][-P pfile][-s str] [file ...]\n", 
#if defined(DEBUG)
      "D",
#else
      "",
#endif
      program);
    fprintf(stderr,
      "    -c          print C style digests with a leading 0x\n");
    fprintf(stderr,
      "    -d          dual digests of even and odd indexed bytes\n");
#if defined(DEBUG)
    fprintf(stderr,
      "    -D          debug mode\n");
#endif
    fprintf(stderr,
      "    -h          prints this message\n");
    fprintf(stderr,
      "    -i          process inode and filename as well as file data\n");
    fprintf(stderr,
      "    -p prefix   prepend str to data before digesting\n");
    fprintf(stderr,
      "    -P pfile    prepend the file 'str' to data before digesting\n");
    fprintf(stderr,
      "    -q          print only the digest\n");
    fprintf(stderr,
      "    -r          reverse feedback mode\n");
    fprintf(stderr,
      "    -s str      prints digest and contents of string\n");
    fprintf(stderr,
      "    -t          prints time statistics for %dM chars\n", TEST_MEG);
    fprintf(stderr,
      "    -v          print version\n");
    fprintf(stderr,
      "    -x          execute an extended standard suite of test data\n");
    fprintf(stderr,
      "    file        print digest and name of file\n");
    fprintf(stderr,
      "    (no args)   print digest of stdin\n");
    exit(0);
}


/*
 * main - shs main control function
 */
void
main(argc, argv)
    int argc;			/* arg count */
    char **argv;		/* the args */
{
    SHS_INFO digest;		/* our current digest */
    BYTE *pre_str = NULL;	/* pre-process this data first */
    char *pre_file = NULL;	/* pre-process this file first */
    char *data_str = NULL;	/* data is this string, not a file */
    UINT pre_str_len;		/* length of pre_str or pre_file */
    UINT data_str_len;		/* length of data_str */
    int d_flag = 0;		/* 1 => dual digest mode */
    int t_flag = 0;		/* 1 => -t was given */
    int x_flag = 0;		/* 1 => -x was given */
    extern char *optarg;	/* argument to option */
    extern int optind;		/* option index */
    int c;

    /*
     * parse args
     */
    program = argv[0];
    while ((c = getopt(argc, argv, "cdDihp:P:qs:tvx")) != -1) {
        switch (c) {
        case 'c':
	    c_flag = 1;
	    break;
        case 'd':
	    d_flag = 1;
	    break;
        case 'D':
#if defined(DEBUG)
	    debug = 1;
#else
	    fprintf(stderr, "%s: not compiled with -DDEBUG\n", program);
	    exit(9);
	    /*NOTREACHED*/
#endif
	    break;
	case 'h':
	    shsHelp();
	    /*NOTREACHED*/
	    break;
	case 'i':
            i_flag = 1;
            break;
	case 'p':
	    pre_str = (BYTE *)optarg;
	    break;
	case 'q':
	    q_flag = 1;
	    break;
	case 'P':
	    pre_file = optarg;
	    break;
        case 's':
            data_str = optarg;
            break;
	case 't':
	    t_flag = 1;
	    break;
	case 'v':
	    printf("%s: version 2.%s.%s%s %s\n",
	        program, "%R%", "%L%",
	        (strcmp(shs_what,"@(#)") == 0 &&
	         strcmp("%Z%","@(#)") == 0 &&
	         strcmp(shsdual_what,"@(#)") == 0 &&
	         strcmp(SHS_H_WHAT,"@(#)") == 0) ? "" : "+",
	        "%D%");
	    exit(0);
	case 'x':
	    x_flag = 1;
	    break;
	default:
	    shsHelp();
	    break;
        }
    }
    /* arg checking */
    if (data_str && optind != argc) {
	fprintf(stderr, "%s: -s is not compatible with digesting files\n",
	    program);
	exit(10);
    }
    if (i_flag && optind == argc) {
	fprintf(stderr, "%s: -i works only on filenames\n", program);
	exit(11);
    }

    /*
     * process -x if needed
     */
    if (x_flag) {
        if (d_flag) {
            dualTest();
	} else {
	    shsTestSuite();
	}
	exit(0);
    }

    /*
     * process -t if needed
     */
    if (t_flag) {
	shsTimeTrial();
	exit(0);
    }

    /*
     * process -P or -p if needed
     */
    if (pre_str && pre_file) {
	fprintf(stderr, "%s: -p and -P conflict\n", program);
	exit(12);
    }
    if (pre_file) {
	pre_str_len = shsPreFileRead(pre_file, &pre_str);
    } else if (pre_str) {
        pre_str_len = strlen((char *)pre_str);
    } else {
        pre_str_len = 0;
    }
    if (pre_str_len > MAX_PRE_FILE) {
	fprintf(stderr, "%s: prefix may not be longer than %d bytes\n",
	    program, MAX_PRE_FILE);
    	exit(13);
    }

    /*
     * if -d, perform dual digest processing instead
     */
    if (d_flag) {
	dualMain(argc, argv, pre_str, pre_str_len, data_str);

    /*
     * if no -d, process string, stdin or files
     */
    } else {

	/*
	 * case: digest a string
	 */
	if (data_str != NULL) {
	    data_str_len = strlen(data_str);
	    shsInit(&digest);
	    shsUpdate(&digest, pre_str, pre_str_len);
	    shsUpdate(&digest, (BYTE *)data_str, data_str_len);
	    COUNT(&digest, pre_str_len + data_str_len);
	    shsOutput(data_str, 1, &digest);

	/*
	 * case: digest stdin
	 */
	} else if (optind == argc) {
	    shsInit(&digest);
	    shsStream(pre_str, pre_str_len, stdin, &digest);
	    shsOutput(NULL, 0, &digest);

	/*
	 * case: digest files
	 */
	} else {
	    if (i_flag) {
		dot_zero = 1;
	    }
	    for (; optind < argc; optind++) {
		shsInit(&digest);
		shsFile(pre_str, pre_str_len, argv[optind], &digest);
		shsOutput(argv[optind], 0, &digest);
	    }
	}
    }

    /* all done */
    exit(0);
}
