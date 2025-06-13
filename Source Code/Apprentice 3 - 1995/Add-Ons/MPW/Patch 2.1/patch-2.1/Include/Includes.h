/***
 *
 * Includes.h - v 2.0.1.3 95-05-13
 *
 ***/

//#define DEBUGGING

#define VOIDUSED 7
#include "config.h"

// shut lint up about the following when return value ignored

#define Signal	(void)signal
#define Unlink	(void)unlink
#define Lseek	(void)lseek
#define Fseek	(void)fseek
#define Fstat	(void)fstat
#define Pclose	(void)pclose
#define Close	(void)close
#define Fclose	(void)fclose
#define Fflush	(void)fflush
#define Sprintf	(void)sprintf
#define Mktemp	(void)mktemp
#define Strcpy	(void)strcpy
#define Strcat	(void)strcat

#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <signal.h>

// constants

// AIX predefines these.
#ifdef TRUE
#undef TRUE
#endif
#ifdef FALSE
#undef FALSE
#endif
enum { FALSE = 0, TRUE };

enum {
	MAXHUNKSIZE	= 100000,	// is this enough lines?
	INITHUNKMAX	= 125,		// initial dynamic allocation size
	MAXLINELEN	= 1024,
	BUFFERSIZE	= 1024
};

#define SCCSPREFIX	"s."
#define GET		"get %s"
#define GET_LOCKED	"get -e %s"
#define SCCSDIFF	"get -p %s | diff - %s >/dev/null"

#define RCSSUFFIX	",v"
#define CHECKOUT	"co %s"
#define CHECKOUT_LOCKED	"co -l %s"
#define RCSDIFF		"rcsdiff %s > /dev/null"

// handy definitions

#define Null(t)		((t)0)
#define Nullch		Null(char *)
#define Nullfp		Null(FILE *)
#define Nulline		Null(LINENUM)

#define Ctl(ch)		((ch) & 037)

#define strNE(s1,s2)	(strcmp(s1, s2))
#define strEQ(s1,s2)	(!strcmp(s1, s2))
#define strnNE(s1,s2,l)	(strncmp(s1, s2, l))
#define strnEQ(s1,s2,l)	(!strncmp(s1, s2, l))

// typedefs

typedef char bool;
typedef long LINENUM;			// must be signed
typedef unsigned MEM;			// what to feed malloc

// more includes

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "util.h"
#include "patchlevel.h"
#include "version.h"

char* mktemp (const char* name);

#if !defined(S_ISDIR) && defined(S_IFDIR)
#define	S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif
#if !defined(S_ISREG) && defined(S_IFREG)
#define	S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif


#ifdef macintosh

int		isatty		(int fd);
char*		MakeFileName	(size_t size, const char* dir, const char* name);
pascal OSErr	GetDirID	(short vRefNum, long dirID, StringPtr name,
				 long* theDirID, bool* isDirectory);
OSErr		ChangeDirectory	(const char* path);
char*		UnixToMac	(const char* path);
void		FSpMakeUnique	(const FSSpec* orig, FSSpec* spec);
StringPtr	CToPStr		(const char* s);
char*		FSFullPathName	(const FSSpec* spec);

#define	chmod(n,m)	/* chmod(name, mode) */

#endif
