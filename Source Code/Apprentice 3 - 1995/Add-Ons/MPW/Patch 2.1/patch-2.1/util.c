#include "EXTERN.h"
#include "common.h"
#include "INTERN.h"
#include "util.h"
#include "backupfile.h"
#include <stdarg.h>


#ifndef HAVE_STRERROR
static char *
private_strerror (errnum)
     int errnum;
{
  extern char *sys_errlist[];
  extern int sys_nerr;

  if (errnum > 0 && errnum <= sys_nerr)
    return sys_errlist[errnum];
  return "Unknown system error";
}
#define strerror private_strerror
#endif // !HAVE_STRERROR


// Rename a file, copying it if necessary.
int
move_file (const char* from, const char* to)
{
//    char bakname[512];
    Reg1 char *s;
    Reg2 int i;
    Reg3 int fromfd;
//    bool doExchange = false;

    // to stdout?

    if (strEQ(to, "-")) {
#ifdef DEBUGGING
	if (debug & 4)
	    say2("Moving %s to stdout.\n", from);
#endif
	fromfd = open(from, 0);
	if (fromfd < 0)
	    pfatal2("internal error, can't reopen %s", from);
	while ((i=read(fromfd, buf, sizeof buf)) > 0)
	    if (write(1, buf, i) != 1)
		pfatal1("write failed");
	Close(fromfd);
	return 0;
    }

#if 0
    if (origprae) {
	Strcpy(bakname, origprae);
	Strcat(bakname, to);
    } else {
	Strcpy(bakname, to);
    	Strcat(bakname, simple_backup_suffix);
    }
#endif

#ifdef macintosh
    {
    FSSpec toFss, backFss;
    char* bakname;
    OSErr err = FSMakeFSSpec(0, 0, CToPStr(to), &toFss);
    if (err == noErr) {			// Output file exists
//	BlockMoveData(&toFss, &backFss, sizeof(FSSpec));
	FSpMakeUnique(&toFss, &backFss);
	say3("Moving %s to %s.\n", to, bakname = FSFullPathName(&backFss));
/*
	if ((err = FSpRename(&toFss, backFss.name)) != noErr) {
	    say4("Can't backup %s, output is in %s: %d\n", to, from, err);
	    return -1;
	}
*/
	if (rename(to, bakname) < 0) {
	    say4("Can't backup %s, output is in %s: %s\n", to, from,
		 strerror(errno));
	    return -1;
	}
	free(bakname);
//	doExchange = true;
    }
    }
#else
    if (stat(to, &filestat) == 0) {	// output file exists
	dev_t to_device = filestat.st_dev;
	ino_t to_inode  = filestat.st_ino;
	char *simplename = bakname;
	
	for (s=bakname; *s; s++) {
	    if (*s == '/')
		simplename = s+1;
	}
	/* Find a backup name that is not the same file.
	   Change the first lowercase char into uppercase;
	   if that isn't sufficient, chop off the first char and try again.  */
	while (stat(bakname, &filestat) == 0 &&
		to_device == filestat.st_dev && to_inode == filestat.st_ino) {
	    // Skip initial non-lowercase chars.
	    for (s=simplename; *s && !islower(*s); s++) ;
	    if (*s)
		*s = toupper(*s);
	    else
		Strcpy(simplename, simplename+1);
	}
	while (unlink(bakname) >= 0) ;	// while() is for benefit of Eunice
#ifdef DEBUGGING
	if (debug & 4)
	    say3("Moving %s to %s.\n", to, bakname);
#endif
	if (rename(to, bakname) < 0) {
	    say4("Can't backup %s, output is in %s: %s\n", to, from,
		 strerror(errno));
	    return -1;
	}
	while (unlink(to) >= 0) ;
    }
#endif

#ifdef DEBUGGING
    if (debug & 4)
	say3("Moving %s to %s.\n", from, to);
#endif
    if (rename(from, to) < 0) {		// different file system?
	Reg4 int tofd;
	
	tofd = creat(to, 0666);
	if (tofd < 0) {
	    say4("Can't create %s, output is in %s: %s\n",
	      to, from, strerror(errno));
	    return -1;
	}
	fsetfileinfo((char*) to, 'MPS ', 'TEXT');
	fromfd = open(from, 0);
	if (fromfd < 0)
	    pfatal2("internal error, can't reopen %s", from);
	while ((i=read(fromfd, buf, sizeof buf)) > 0)
	    if (write(tofd, buf, i) != i)
		pfatal1("write failed");
	Close(fromfd);
	Close(tofd);
    }
    Unlink((char*) from);
    return 0;
}


// Copy a file.
void
copy_file (const char* from, const char* to)
{
    Reg3 int tofd;
    Reg2 int fromfd;
    Reg1 int i;
    
    tofd = creat(to, 0666);
    if (tofd < 0)
	pfatal2("can't create %s", to);
    fsetfileinfo((char*) to, 'MPS ', 'TEXT');
    fromfd = open(from, 0);
    if (fromfd < 0)
	pfatal2("internal error, can't reopen %s", from);
    while ((i=read(fromfd, buf, sizeof buf)) > 0)
	if (write(tofd, buf, i) != i)
	    pfatal2("write to %s failed", to);
    Close(fromfd);
    Close(tofd);
}


// Allocate a unique area for a string.
char*
savestr (const char* s)
{
    Reg3 char* rv;
    Reg2 const char* t;

    if (!s)
	s = "Oops";
    t = s;
    while (*t++)
    	;
    rv = malloc((MEM) (t - s));
    if (rv == Nullch) {
	if (using_plan_a)
	    out_of_mem = TRUE;
	else
	    fatal1("out of memory\n");
    } else {
	t = rv;
	while ( (*((char*) t++) = *s++) != 0 )
		;
    }
    return rv;
}


// Vanilla terminal output (buffered).
void
say (const char* format, ...)
{
	va_list args;

	va_start(args, format);
	vfprintf(stderr, format, args);
//	Fflush(stderr);
	va_end(args);
}


// Terminal output, pun intended.
void				// very void
fatal (const char* format, ...)
{
	va_list args;

	fprintf(stderr, "# patch: **** ");
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	my_exit(1);
}


// Say something from patch, something from the system, then silence . . .
void				// very void
pfatal (const char* format, ...)
{
	int errnum = errno;
	va_list args;

	fprintf(stderr, "# patch: **** ");
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fprintf(stderr, ": %s\n", strerror(errnum));
	my_exit(1);
}


// Get a response from the user, somehow or other.
void
ask (const char* format, ...)
{
    int ttyfd;
    int r;
    bool tty2 = isatty(kStderr);
    va_list args;

    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);
    Fflush(stderr);
    write(kStderr, buf, strlen(buf));
    if (tty2) {				// might be redirected to a file
	r = read(kStderr, buf, sizeof buf);
    }
    else if (isatty(kStdout)) {		// this may be new file output
	Fflush(stdout);
	write(kStdout, buf, strlen(buf));
	r = read(kStdout, buf, sizeof buf);
    }
#if 0
    else if ((ttyfd = open("/dev/tty", 2)) >= 0 && isatty(ttyfd)) {
					// might be deleted or unwriteable
	write(ttyfd, buf, strlen(buf));
	r = read(ttyfd, buf, sizeof buf);
	Close(ttyfd);
    }
#endif
    else if (isatty(kStdin)) {		// this is probably patch input
	Fflush(stdin);
	write(kStdin, buf, strlen(buf));
	r = read(kStdin, buf, sizeof buf);
    }
    else {				// no terminal at all--default it
	buf[0] = '\n';
	r = 1;
    }
    if (r <= 0)
	buf[0] = 0;
    else
	buf[r] = '\0';
    if (!tty2)
	say1(buf);
}


// How to handle certain events when not in a critical region.
void
set_signals (int reset)
{
#ifndef lint
#ifdef macintosh
    static RETSIGTYPE (*intval)(int);

    if (!reset) {
	intval = signal(SIGINT, SIG_IGN);
	if (intval != SIG_IGN)
//	    intval = (RETSIGTYPE(*)())my_exit;
	    intval = my_exit;
    }
    Signal(SIGINT, intval);
#else
    static RETSIGTYPE (*hupval)(),(*intval)();

    if (!reset) {
	hupval = signal(SIGHUP, SIG_IGN);
	if (hupval != SIG_IGN)
	    hupval = (RETSIGTYPE(*)())my_exit;
	intval = signal(SIGINT, SIG_IGN);
	if (intval != SIG_IGN)
	    intval = (RETSIGTYPE(*)())my_exit;
    }
    Signal(SIGHUP, hupval);
    Signal(SIGINT, intval);
#endif
#endif
}


// How to handle certain events when in a critical region.
void
ignore_signals()
{
#ifndef lint
#ifndef macintosh
    Signal(SIGHUP, SIG_IGN);
#endif
    Signal(SIGINT, SIG_IGN);
#endif
}


// Make sure we'll have the directories to create a file.
// If `striplast' is TRUE, ignore the last element of `filename'.
void
makedirs (char* filename, bool striplast)
{
    char tmpbuf[256];
    Reg2 char *s = tmpbuf;
    char *dirv[20];		// Point to the NULs between elements.
    Reg3 int i;
    Reg4 int dirvp = 0;		// Number of finished entries in dirv.

#ifdef macintosh
#if 1
	say("makedirs %s %d\n", filename, striplast);
	my_exit(1);
#else
	FSSpec spec;

    // Copy `filename' into `tmpbuf' with a NUL instead of a slash
    // between the directories.
    while (*filename) {
	if (*filename == ':') {
	    filename++;
	    dirv[dirvp++] = s;
	    *s++ = '\0';
	} else
	    *s++ = *filename++;
    }
    *s = '\0';
    dirv[dirvp] = s;
    if (striplast)
	dirvp--;
    if (dirvp < 0)
	return;

    strcpy(buf, "mkdir");
    s = buf;
    for (i=0; i<=dirvp; i++) {
	struct stat sbuf;

	if (stat(tmpbuf, &sbuf) && errno == ENOENT) {
	    while (*s) s++;
	    *s++ = ' ';
	    strcpy(s, tmpbuf);
	}
	*dirv[i] = ':';
    }
    if (s != buf)
	say(buf);

/*
	err = DirCreate(vRefNum, parentDirID, directoryName, createdDirID);
	err = FSMakeFSSpec(0, 0, fileName, &spec);
	err = FSpDirCreate(&spec, smSystemScript, &createdDirID)
*/
#endif
#else
    // Copy `filename' into `tmpbuf' with a NUL instead of a slash
    // between the directories.
    while (*filename) {
	if (*filename == '/') {
	    filename++;
	    dirv[dirvp++] = s;
	    *s++ = '\0';
	}
	else {
	    *s++ = *filename++;
	}
    }
    *s = '\0';
    dirv[dirvp] = s;
    if (striplast)
	dirvp--;
    if (dirvp < 0)
	return;

    strcpy(buf, "mkdir");
    s = buf;
    for (i=0; i<=dirvp; i++) {
	struct stat sbuf;

	if (stat(tmpbuf, &sbuf) && errno == ENOENT) {
	    while (*s) s++;
	    *s++ = ' ';
	    strcpy(s, tmpbuf);
	}
	*dirv[i] = '/';
    }
    if (s != buf)
	system(buf);
#endif
}


// Make filenames more reasonable.
char*
fetchname (char* at, int strip_leading, int assume_exists)
{
    char *fullname;
    char *name;
    Reg1 char *t;
    char tmpbuf[200];
    int sleading = strip_leading;

    if (!at)
	return Nullch;
    while (isspace(*at))
	at++;
#ifdef DEBUGGING
    if (debug & 128)
	say4("fetchname(�%s� %d %d)\n",at,strip_leading,assume_exists);
#endif
    if (strnEQ(at, "dev:null", 8))	// so files can be created by diffing
	return Nullch;			//   against /dev/null.
    name = fullname = t = savestr(at);

    // Strip off up to `sleading' leading colons and null terminate.
    for (; *t && !isspace(*t); t++)
	if (*t == ':')
	    if (--sleading >= 0)
		name = t + 1;
    *t = '\0';

    /* If no -p option was given (957 is the default value!),
       we were given a relative pathname,
       and the leading directories that we just stripped off all exist,
       put them back on.  */
    if (strip_leading == 957 && name != fullname + 1 && *fullname == ':') {
	name[-1] = '\0';
#ifdef DEBUGGING
	if (debug & 128)
	    say("fetchname: �%s� ", fullname);
#endif
	if (stat(fullname, &filestat) == 0 && S_ISDIR(filestat.st_mode)) {
	    name[-1] = ':';
	    name = fullname;
#ifdef DEBUGGING
	    if (debug & 128)
		say("IS a dir\n");
#endif
	}
#ifdef DEBUGGING
	else if (debug & 128)
	    say("is NOT a dir (name = �%s�)\n", name);
#endif
    }

    name = savestr(name);
    free(fullname);

    if (stat(name, &filestat) && !assume_exists) {
	char* filebase = basename(name);
	int pathlen = filebase - name;

	// Put any leading path into `tmpbuf'.
	strncpy(tmpbuf, name, pathlen);

#if 0
#define try(f, a1, a2) (Sprintf(tmpbuf + pathlen, f, a1, a2), stat(tmpbuf, &filestat) == 0)
	if (   try("RCS/%s%s", filebase, RCSSUFFIX)
	    || try("RCS/%s"  , filebase,         0)
	    || try(    "%s%s", filebase, RCSSUFFIX)
	    || try("SCCS/%s%s", SCCSPREFIX, filebase)
	    || try(     "%s%s", SCCSPREFIX, filebase))
	  return name;
#endif
	free(name);
	name = Nullch;
    }

#ifdef DEBUGGING
    if (debug & 128)
	say(name ? "%s�%s�\n\n" : "%sNULL\n\n", "fetchname -> ", name);
#endif
    return name;
}


void*
xmalloc (size_t size)
{
  register char* p = malloc(size);
  if (!p)
    fatal("out of memory");
  return p;
}
