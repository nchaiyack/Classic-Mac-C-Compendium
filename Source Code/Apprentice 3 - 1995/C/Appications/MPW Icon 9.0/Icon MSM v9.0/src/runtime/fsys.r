/*
 * File: fsys.r
 *  Contents: close, chdir, exit, getenv, open, read, reads, remove, rename,
 *  [save], seek, stop, [system], where, write, writes, [getch, getche,
 *  kbhit]
 */

#if MICROSOFT || SCO_XENIX
#define BadCode
#endif					/* MICROSOFT || SCO_XENIX */

#ifdef XENIX_386
#define register
#endif					/* XENIX_386 */
/*
 * The following code is operating-system dependent [@fsys.01]. Include
 *  system-dependent files and declarations.
 */

#if PORT
   /* nothing to do */
Deliberate Syntax Error
#endif					/* PORT */

#if AMIGA || ATARI_ST || MSDOS || MVS || OS2 || UNIX || VM || VMS
   /* nothing to do */
#endif					/* AMIGA || ATARI_ST || ... */

#if MACINTOSH && MPW
extern int MPWFlush(FILE *f);
#define fflush(f) MPWFlush(f)
#endif					/* MACINTOSH && MPW*/

/*
 * End of operating-system specific code.
 */


"close(f) - close file f."

function{1} close(f)

   if !is:file(f) then
      runerr(105, f)

   abstract {
      return file ++ integer
      }

   body {
      FILE *fp;

      fp = BlkLoc(f)->file.fd;

      /*
       * Close f, using fclose, pclose, or wclose as appropriate.
       */

#ifdef Graphics
      /*
       * Close window if windows are supported.
       */

      pollctr >>= 1;
      pollctr++;
      if (BlkLoc(f)->file.status & Fs_Window) {
	 if (BlkLoc(f)->file.status != Fs_Window) { /* not already closed? */
	    BlkLoc(f)->file.status = Fs_Window;
	    wclose((wbp) fp);
	    }
	 return f;
	 }
      else
#endif					/* Graphics */

#if ARM || OS2 || UNIX || VMS
      /*
       * Close pipe if pipes are supported.
       */

      if (BlkLoc(f)->file.status & Fs_Pipe) {
	 BlkLoc(f)->file.status = 0;
	 return C_integer((pclose(fp) >> 8) & 0377);
	 }
      else
#endif					/* ARM || OS2 || UNIX || VMS */

      fclose(fp);
      BlkLoc(f)->file.status = 0;

      /*
       * Return the closed file.
       */
      return f;
      }
end


"exit(i) - exit process with status i, which defaults to 0."

function{} exit(status)
   if !def:C_integer(status, NormalExit) then
      runerr(0)
   inline {
      c_exit((int)status);
      }
end


"getenv(s) - return contents of environment variable s."

#ifndef EnvVars
function{0} getenv(s)
   abstract {
      return empty_type
      }
   inline {
      fail;
      }
#else					/* EnvVars */
function{0,1} getenv(s)

   /*
    * Make a C-style string out of s
    */
   if !cnv:C_string(s) then
      runerr(103,s)
   abstract {
      return string
      }

   inline {
      register char *p;
      long l;

      if ((p = getenv(s)) != NULL) {	/* get environment variable */
	 l = strlen(p);
	 Protect(p = alcstr(p,l),runerr(0));
	 return string(l,p);
	 }
      else 				/* fail if not in environment */
	 fail;

      }
#endif					/* EnvVars */
end


#ifdef Graphics
"open(s1, s2, ...) - open file named s2 with options s2"
" and attributes given in trailing arguments."
function{0,1} open(fname, spec, attr[n])
#else						/* Graphics */
#ifdef OpenAttributes
"open(fname, spec, attrstring) - open file fname with specification spec."
function{0,1} open(fname, spec, attrstring)
#else						/* OpenAttributes */
"open(fname, spec) - open file fname with specification spec."
function{0,1} open(fname, spec)
#endif						/* OpenAttributes */
#endif						/* Graphics */
   declare {
      tended struct descrip filename;
      }

   /*
    * fopen and popen require a C string, but it looks terrible in
    *  error messages, so convert it to a string here and use a local
    *  variable (fnamestr) to store the C string.
    */
   if !cnv:string(fname) then
      runerr(103, fname)

   /*
    * spec defaults to "r".
    */
   if !def:tmp_string(spec, letr) then
      runerr(103, spec)

#ifdef OpenAttributes
   /*
    * Convert attrstr to a string, defaulting to "".
    */
   if !def:C_string(attrstring, emptystr) then
      runerr(103, attrstring)
#endif					/* OpenAttributes */

   abstract {
      return file
      }

   body {
      tended char *fnamestr;
      register word slen;
      register int i;
      register char *s;
      int status;
      char mode[4];
      extern FILE *fopen();
      FILE *f;
      struct b_file *fl;

#ifdef Graphics
      int j, err_index = -1;
      tended struct b_list *hp;
      tended struct b_lelem *bp;
#endif					/* Graphics */

/*
 * The following code is operating-system dependent [@fsys.02].  Make
 *  declarations as needed for opening files.
 */

#if PORT
Deliberate Syntax Error
#endif					/* PORT */

#if AMIGA || MACINTOSH
   /* nothing is needed */
#endif					/* AMIGA || MACINTOSH */

#if ARM
      extern FILE *popen(const char *, const char *);
      extern int pclose(FILE *);
#endif					/* ARM */

#if ATARI_ST || MSDOS || MVS || OS2 || VM
      char untranslated;
#endif					/* ATARI_ST || MSDOS || ... */

#if MACINTOSH
#if LSC
      char untranslated;
#endif					/* LSC */
#endif					/* MACINTOSH */

#if OS2 || UNIX || VMS
      extern FILE *popen();
#endif					/* OS2 || UNIX || VMS */

/*
 * End of operating-system specific code.
 */

      /*
       * get a C string for the file name
       */
      if (!cnv:C_string(fname, fnamestr))
	 runerr(103,fname);

      status = 0;

/*
 * The following code is operating-system dependent [@fsys.03].  Provide
 *  declaration for untranslated line-termination mode, if supported.
 */

#if PORT
   /* nothing to do */
Deliberate Syntax Error
#endif					/* PORT */

#if AMIGA
   /* translated mode could be supported, but is not now */
#endif					/* AMIGA */

#if ARM || UNIX || VMS
   /* nothing to do */
#endif					/* ARM || UNIX || VMS */

#if ATARI_ST || MSDOS || MVS || OS2 || VM
      untranslated = 0;
#endif					/* ATARI_ST || MSDOS || ... */

#if MACINTOSH
#if LSC
      untranslated = 0;
#endif					/* LSC */
#endif					/* MACINTOSH */

/*
 * End of operating-system specific code.
 */

      /*
       * Scan spec, setting appropriate bits in status.  Produce a
       *  run-time error if an unknown character is encountered.
       */
      s = StrLoc(spec);
      slen = StrLen(spec);
      for (i = 0; i < slen; i++) {
	 switch (*s++) {
	    case 'a':
	    case 'A':
	       status |= Fs_Write|Fs_Append;
	       continue;
	    case 'b':
	    case 'B':
	       status |= Fs_Read|Fs_Write;
	       continue;
	    case 'c':
	    case 'C':
	       status |= Fs_Create|Fs_Write;
	       continue;
	    case 'r':
	    case 'R':
	       status |= Fs_Read;
	       continue;
	    case 'w':
	    case 'W':
	       status |= Fs_Write;
	       continue;

/*
 * The following code is operating-system dependent [@fsys.04].  Handle
 * untranslated line-terminator mode, pipes, and/or window modes if supported.
 */

#if PORT
	    case 't':
	    case 'T':
	    case 'u':
	    case 'U':
	       continue;			/* no-op */
Deliberate Syntax Error
#endif					/* PORT */

#if AMIGA
	    case 't':
	    case 'T':
	    case 'u':
	    case 'U':
	       continue;			/* no-op */
#endif					/* AMIGA */

#if ARM || UNIX || VMS
	    case 't':
	    case 'T':
	    case 'u':
	    case 'U':
	       continue;			/* no-op */
	    case 'p':
	    case 'P':
	       status |= Fs_Pipe;
	       continue;
#endif					/* ARM || UNIX || VMS */

#if ATARI_ST || MSDOS || OS2 || SASC
	    case 't':
	    case 'T':

#ifdef RecordIO
               status &= ~Fs_Record;
#endif                                  /* RecordIO */

	       untranslated = 0;
	       continue;			/* no-op */

#if OS2
	    case 'p':
	    case 'P':
	       status |= Fs_Pipe;
#endif					/* OS2 */

	       continue;
	    case 'u':
	    case 'U':
	       untranslated = 1;

#ifdef RecordIO
	       status &= ~Fs_Record;
#endif					/* RecordIO */

	       continue;
#endif					/* ATARI_ST || MSDOS || ... */

#ifdef RecordIO
	    case 's':
	    case 'S':
	       untranslated = 1;
	       status |= Fs_Record;
	       continue;
#endif					/* RecordIO */

#if MACINTOSH
#if LSC
	    case 't':
	    case 'T':
	       untranslated = 0;
	       continue;
	    case 'u':
	    case 'U':
	       untranslated = 1;
	       continue;
#endif					/* LSC */
#if MPW
	    case 't':
	    case 'T':
	    case 'u':
	    case 'U':
	       continue;			/* no-op */
#endif					/* MPW */
#endif					/* MACINTOSH */

   /*
    * End of operating-system specific code.
    */

	    case 'x':
	    case 'X':
	    case 'g':
	    case 'G':
#ifdef Graphics
	       status |= Fs_Window | Fs_Read | Fs_Write;
	       continue;
#else					/* Graphics */
	       fail;
#endif					/* Graphics */

	    default:
	       runerr(209, spec);
	    }
	 }

      /*
       * Construct a mode field for fopen/popen.
       */
      mode[0] = '\0';
      mode[1] = '\0';
      mode[2] = '\0';
      mode[3] = '\0';

      if ((status & (Fs_Read|Fs_Write)) == 0)	/* default: read only */
	 status |= Fs_Read;
      if (status & Fs_Create)
	 mode[0] = 'w';
      else if (status & Fs_Append)
	 mode[0] = 'a';
      else if (status & Fs_Read)
	 mode[0] = 'r';
      else
	 mode[0] = 'w';

/*
 * The following code is operating-system dependent [@fsys.05].  Handle open
 *  modes.
 */

#if PORT
      if ((status & (Fs_Read|Fs_Write)) == (Fs_Read|Fs_Write))
	 mode[1] = '+';
Deliberate Syntax Error
#endif					/* PORT */

#if AMIGA || ARM || UNIX || VMS
      if ((status & (Fs_Read|Fs_Write)) == (Fs_Read|Fs_Write))
	 mode[1] = '+';
#endif					/* AMIGA || ARM || UNIX || VMS */

#if ATARI_ST
      if ((status & (Fs_Read|Fs_Write)) == (Fs_Read|Fs_Write)) {
	 mode[1] = '+';
	 mode[2] = untranslated ? 'b' : 'a';
	 }
      else mode[1] = untranslated ? 'b' : 'a';
#endif					/* ATARI_ST */

#if MSDOS || OS2
      if ((status & (Fs_Read|Fs_Write)) == (Fs_Read|Fs_Write)) {
	 mode[1] = '+';
#if CSET2
         /* we don't have the 't' in C Set/2 */
         if (untranslated) mode[2] = 'b';
         } /* End of if - open file for reading or writing */
      else if (untranslated) mode[1] = 'b';
#else					/* CSET2 */
	 mode[2] = untranslated ? 'b' : 't';
	 }
      else mode[1] = untranslated ? 'b' : 't';
#endif					/* CSET2 */
#endif					/* MSDOS || OS2 */

#if MACINTOSH
#if LSC
      untranslated = 0;
#endif					/* LSC */
#endif					/* MACINTOSH */

#if MVS || VM
      if ((status & (Fs_Read|Fs_Write)) == (Fs_Read|Fs_Write)) {
	 mode[1] = '+';
	 mode[2] = untranslated ? 'b' : 0;
	 }
      else mode[1] = untranslated ? 'b' : 0;
#endif					/* MVS || VM */

/*
 * End of operating-system specific code.
 */

      /*
       * Open the file with fopen or popen.
       */

#ifdef OpenAttributes
#if SASC
#ifdef RecordIO
	 f = afopen(fnamestr, mode, status & Fs_Record ? "seq" : "",
		    attrstring);
#else					/* RecordIO */
	 f = afopen(fnamestr, mode, "", attrstring);
#endif					/* RecordIO */
#endif					/* SASC */

#else					/* OpenAttributes */

#ifdef Graphics
      if (status & Fs_Window) {
	 /*
	  * allocate an empty event queue for the window
	  */
	 Protect(hp = alclist(0), runerr(0));
	 Protect(bp = alclstb(MinListSlots, (word)0, 0), runerr(0));
	 hp->listhead = hp->listtail = (union block *) bp;

	 /*
	  * loop through attributes, checking validity
	  */
	 for (j = 0; j < n; j++) {
	    if (is:null(attr[j]))
	       attr[j] = emptystr;
	    if (!is:string(attr[j]))
	       runerr(109, attr[j]);
	    }

	 f = (FILE *)wopen(fnamestr, hp, attr, n, &err_index);
	 if (f == NULL) {
	    if (err_index != -1) runerr(145, attr[err_index]);
	    else fail;
	    }
	 } else
#endif					/* Graphics */

#if ARM || OS2 || UNIX || VMS
      if (status & Fs_Pipe) {
	 if (status != (Fs_Read|Fs_Pipe) && status != (Fs_Write|Fs_Pipe))
	    runerr(209, spec);
	 f = popen(fnamestr, mode);
	 }
      else
#endif					/* ARM || OS2 || UNIX || VMS */

	 f = fopen(fnamestr, mode);
#endif					/* OpenAttributes */

      /*
       * Fail if the file cannot be opened.
       */
      if (f == NULL)
	 fail;

#if MACINTOSH
#if MPW
      {
	 void SetFileToMPWText(const char *fname);

	 if (status & Fs_Write)
	    SetFileToMPWText(fnamestr);
      }
#endif					/* MPW */
#endif					/* MACINTOSH */

      /*
       * Return the resulting file value.
       */
      StrLen(filename) = strlen(fnamestr);
      StrLoc(filename) = fnamestr;

      Protect(fl = alcfile(f, status, &filename), runerr(0));
#ifdef Graphics
      /*
       * link in the Icon file value so this window can find it
       */
      if (status & Fs_Window) {
	 ((wbp)f)->window->filep.dword = D_File;
	 BlkLoc(((wbp)f)->window->filep) = (union block *)fl;
	 if (is:null(lastEventWin))
	    lastEventWin = ((wbp)f)->window->filep;
	 }
#endif					/* Graphics */
      return file(fl);
      }
end


"read(f) - read line on file f."

function{0,1} read(f)
   /*
    * Default f to &input.
    */
   if is:null(f) then
      inline {
	 f.dword = D_File;
	 BlkLoc(f) = (union block *)&k_input;
	 }
   else if !is:file(f) then
      runerr(105, f)

   abstract {
      return string
      }

   body {
      register word slen, rlen;
      register char *sp;
      int status;
      static char sbuf[MaxReadStr];
      tended struct descrip s;
      FILE *fp;

      /*
       * Get a pointer to the file and be sure that it is open for reading.
       */
      fp = BlkLoc(f)->file.fd;
      status = BlkLoc(f)->file.status;
      if ((status & Fs_Read) == 0)
	 runerr(212, f);

#ifdef StandardLib
      if (status & Fs_Writing) {
	 fseek(fp, 0L, SEEK_CUR);
	 BlkLoc(f)->file.status &= ~Fs_Writing;
	 }
      BlkLoc(f)->file.status |= Fs_Reading;
#endif					/* StandardLib */

#ifdef PresentationManager
      /* if file is &input, then make sure our console is open and read
         from it, unless input redirected */
      if (fp == stdin && !(PMFlags & StdInRedirect)) {
        fp = OpenConsole();
        status = Fs_Window | Fs_Read | Fs_Write;
        } /* End of if - console input */
#endif					/* PresentationManager */

      /*
       * Use getstrg to read a line from the file, failing if getstrg
       *  encounters end of file. [[ What about -2?]]
       */
      StrLen(s) = 0;
      do {
#ifdef Graphics
	 pollctr >>= 1;
	 pollctr++;
	 if (status & Fs_Window) {
	    slen = wgetstrg(sbuf,MaxReadStr,fp);
	    if (slen == -1)
	       runerr(141);
	    if (slen < -1)
	       runerr(143);
	    }
	 else
#endif					/* Graphics */

#ifdef RecordIO
	 if ((slen = (status & Fs_Record ? getrec(sbuf, MaxReadStr, fp) :
					   getstrg(sbuf, MaxReadStr, fp)))
	     == -1) fail;
#else					/* RecordIO */
	 if ((slen = getstrg(sbuf,MaxReadStr,fp)) == -1)
	    fail;
#endif					/* RecordIO */

	 /*
	  * Allocate the string read and make s a descriptor for it.
	  */
	 rlen = slen < 0 ? (word)MaxReadStr : slen;

#ifdef FixedRegions
	 Protect(strreserve(rlen), runerr(0));
	 if (StrLen(s) > 0 && !InRange(strbase,StrLoc(s),strfree)) {
	    Protect(strreserve(StrLen(s)+rlen), runerr(0));
	    Protect((StrLoc(s) = alcstr(StrLoc(s),StrLen(s))), runerr(0));
	    }
#endif					/* FixedRegions */

	 Protect(sp = alcstr(sbuf,rlen), runerr(0));
	 if (StrLen(s) == 0)
	    StrLoc(s) = sp;
	 StrLen(s) += rlen;
	 } while (slen < 0);
      return s;
      }
end


"reads(f,i) - read i characters on file f."

function{0,1} reads(f,i)
   /*
    * Default f to &input.
    */
   if is:null(f) then
      inline {
	 f.dword = D_File;
	 BlkLoc(f) = (union block *)&k_input;
	 }
   else if !is:file(f) then
      runerr(105, f)

   /*
    * i defaults to 1 (read a single character)
    */
   if !def:C_integer(i,1L) then
      runerr(101, i)

   abstract {
      return string
      }

   body {
      long tally;
      int status;
      FILE *fp;
      tended struct descrip s;

      /*
       * Get a pointer to the file and be sure that it is open for reading.
       */
      fp = BlkLoc(f)->file.fd;
      status = BlkLoc(f)->file.status;
      if ((status & Fs_Read) == 0)
	 runerr(212, f);

#ifdef StandardLib
      if (status & Fs_Writing) {
	 fseek(fp, 0L, SEEK_CUR);
	 BlkLoc(f)->file.status &= ~Fs_Writing;
	 }
      BlkLoc(f)->file.status |= Fs_Reading;
#endif					/* StandardLib */

#ifdef PresentationManager
      /* if file is &input, then make sure our console is open and read
         from it, unless input redirected */
      if (fp == stdin && !(PMFlags & StdInRedirect)) {
        fp = OpenConsole();
        status = Fs_Read | Fs_Write | Fs_Window;
        } /* End of if - console input */
#endif					/* PresentationManager */

      /*
       * Be sure that a positive number of bytes is to be read.
       */
      if (i <= 0) {
	 irunerr(205, i);

	 errorfail;
	 }

      /*
       * For now, assume we can read the full number of bytes.
       */
      Protect(StrLoc(s) = alcstr(NULL, i), runerr(0));
      StrLen(s) = 0;

#if AMIGA
      /*
       * The following code is special for Lattice 4.0 -- it was different
       *  for Lattice 3.10.  It probably won't work correctly with other
       *  C compilers.
       */
      if (IsInteractive(_ufbs[fileno(fp)].ufbfh)) {
	 if ((i = read(fileno(fp),StrLoc(s),i)) <= 0)
	    fail;
	 StrLen(s) = i;
	 /*
	  * We may not have used the entire amount of storage we reserved.
	  */
	 MMStr(DiffPtrs(StrLoc(s) + i, strfree));
	 strtotal += DiffPtrs(StrLoc(s) + i, strfree);
	 strfree = StrLoc(s) + i;
	 return s;
	 }
#endif					/* AMIGA */

#ifdef Graphics
      pollctr >>= 1;
      pollctr++;
      if (status & Fs_Window) {
	 tally = wlongread(StrLoc(s),sizeof(char),i,fp);
	 if (tally == -1)
	    runerr(141);
	 else if (tally < -1)
	    runerr(143);
	 }
      else
#endif					/* Graphics */
      tally = longread(StrLoc(s),sizeof(char),i,fp);

      if (tally == 0)
	 fail;
      StrLen(s) = tally;
      /*
       * We may not have used the entire amount of storage we reserved.
       */
      MMStr(DiffPtrs(StrLoc(s) + tally, strfree));
      strtotal += DiffPtrs(StrLoc(s) + tally, strfree);
      strfree = StrLoc(s) + tally;
      return s;
      }
end


"remove(s) - remove the file named s."

function{0,1} remove(s)

   /*
    * Make a C-style string out of s
    */
   if !cnv:C_string(s) then
      runerr(103,s)
   abstract {
      return null
      }

   inline {
      if (unlink(s) != 0)
	 fail;
      return nulldesc;
      }
end


"rename(s1,s2) - rename the file named s1 to have the name s2."

function{0,1} rename(s1,s2)

   /*
    * Make C-style strings out of s1 and s2
    */
   if !cnv:C_string(s1) then
      runerr(103,s1)
   if !cnv:C_string(s2) then
      runerr(103,s2)

   abstract {
      return null
      }

   body {
/*
 * The following code is operating-system dependent [@fsys.06].  Rename the
 *  file, and fail if unsuccessful.
 */

#if PORT
   /* need something */
Deliberate Syntax Error
#endif					/* PORT */

#if AMIGA || ARM || ATARI_ST || MACINTOSH || MSDOS || MVS || OS2 || VM || VMS
      {
      if (rename(s1,s2) != 0)
	 fail;
      }
#endif					/* AMIGA || ARM || ATARI_ST ... */

#if UNIX
      if (link(s1,s2) != 0)
	 fail;
      if (unlink(s1) != 0) {
	 unlink(s2);	/* try to undo partial rename */
	 fail;
	 }
#endif					/* UNIX */

/*
 * End of operating-system specific code.
 */

      return nulldesc;
      }
end

#ifdef ExecImages

"save(s) - save the run-time system in file s"

function{0,1} save(s)

   if !cnv:C_string(s) then
      runerr(103,s)

   abstract {
      return integer
      }

   body {
      char sbuf[MaxCvtLen];
      int f, fsz;

      dumped = 1;

      /*
       * Open the file for the executable image.
       */
      f = creat(s, 0777);
      if (f == -1)
	 fail;
      fsz = wrtexec(f);
      /*
       * It happens that most wrtexecs don't check the system call return
       *  codes and thus they'll never return -1.  Nonetheless...
       */
      if (fsz == -1)
	 fail;
      /*
       * Return the size of the data space.
       */
      return C_integer fsz;
      }
end
#endif					/* ExecImages */


"seek(f,i) - seek to offset i in file f."
" [[ What about seek error ? ]] "

function{0,1} seek(f,o)

   /*
    * f must be a file
    */
   if !is:file(f) then
      runerr(105,f)

   /*
    * o must be an integer and defaults to 1.
    */
   if !def:C_integer(o,1L) then
      runerr(0)

   abstract {
      return file
      }

   body {
      FILE *fd;

      fd = BlkLoc(f)->file.fd;
      if (BlkLoc(f)->file.status == 0)
	 fail;

#ifdef Graphics
      pollctr >>= 1;
      pollctr++;
      if (BlkLoc(f)->file.status & Fs_Window)
	 fail;
#endif					/* Graphics */

      if (o > 0) {
/* fseek returns a non-zero value on error for CSET2, not -1 */
#if CSET2
	 if (fseek(fd, o - 1, SEEK_SET))
#else
	 if (fseek(fd, o - 1, SEEK_SET) == -1)
#endif					/* CSET2 */
	    fail;
	 }
      else {

#if CSET2
/* unreliable seeking from the end in CSet/2 on a text stream, so
   we will fixup seek-from-end to seek-from-beginning */
	long size;
	long save_pos;

	/* save the position in case we have to reset it */
	save_pos = ftell(fd);
	/* seek to the end and get the file size */
	fseek(fd, 0, SEEK_END);
	size = ftell(fd);
	/* try to accomplish the fixed-up seek */
	if (fseek(fd, size + o, SEEK_SET)) {
	   fseek(fd, save_pos, SEEK_SET);
	   fail;
	   }  /* End of if - seek failed, reset position */
#else
	 if (fseek(fd, o, SEEK_END) == -1)
	    fail;
#endif					/* CSET2 */
	 }
#ifdef StandardLib
      BlkLoc(f)->file.status &= ~(Fs_Reading | Fs_Writing);
#endif					/* StandardLib */
      return f;
      }
end


#ifdef SystemFnc

"system(s) - execute string s as a system command."

function{1} system(s)
   /*
    * Make a C-style string out of s
    */
   if !cnv:C_string(s) then
      runerr(103,s)

   abstract {
      return integer
      }

   inline {
      /*
       * Pass the C string to the system() function and return
       * the exit code of the command as the result of system().
       * Note, the expression on a "return" may not have side effects,
       * so the exit code must be returned via a variable.
       */
      C_integer i;

#ifdef Graphics
      pollctr >>= 1;
      pollctr++;
#endif					/* Graphics */

/*
 * The following code is operating-system dependent [@fsys.12].  Perform system
 *  call.  Should not get here unless system(s) is supported.
 */

#if PORT
Deliberate Syntax Error
#endif					/* PORT */

#if AMIGA || OS2 || UNIX
      i = ((system(s) >> 8) & 0377);
#endif					/* AMIGA || OS2 || ... */

#if MSDOS
#if HIGHC_386
      i = (C_integer)system(s);
#else					/* HIGHC_386 */
      i = ((system(s) >> 8) & 0377);
#endif					/* HIGHC_386 */
#endif					/* MSDOS */

#if ARM
      i = (C_integer)system(s);
#endif					/* ARM */

#if ATARI_ST || VMS
      i = system(s);
#endif					/* ATARI_ST || VMS */

#if MACINTOSH
   /* Should not get here */
#endif					/* MACINTOSH */

#if MVS || VM
#if SASC && MVS
   {
      char *wprefix;
      wprefix = malloc(strlen(s)+5);
		     /* hope this will do no harm... */
      sprintf(wprefix,"tso:%s",s);
      i = (C_integer)system(wprefix);
      free(wprefix);
   }
#else					/* SASC && MVS */
   i = (C_integer)system(s);
#endif					/* SASC && MVS */
#endif					/* MVS || VM */

/*
 * End of operating-system specific code.
 */
      return C_integer i;
      }
end

#endif					/* SystemFnc */


"where(f) - return current offset position in file f."

function{0,1} where(f)

   if !is:file(f) then
      runerr(105,f)

   abstract {
      return integer
      }

   body {
      FILE *fd;
      long ftell();
      long pos;

      fd = BlkLoc(f)->file.fd;

      if ((BlkLoc(f)->file.status == 0))
	 fail;

#ifdef Graphics
      pollctr >>= 1;
      pollctr++;
      if (BlkLoc(f)->file.status & Fs_Window)
	 fail;
#endif					/* Graphics */

      pos = ftell(fd) + 1;
#ifdef StandardLib
      if (pos == 0)
	 fail;	/* may only be effective on ANSI systems */
#endif					/* StandardLib */

      return C_integer pos;
      }
end

/*
 * stop(), write(), and writes() differ in whether they stop the program
 *  and whether they output newlines. The macro GenWrite is used to
 *  produce all three functions.
 */
#define False 0
#define True 1

#begdef DefaultFile(error_out)
   inline {
#if error_out
      if ((k_errout.status & Fs_Write) == 0)
	 runerr(213);
      else {
#ifndef PresentationManager
	 f = k_errout.fd;
#else					/* PresentationManager */
         f = (PMFlags & StdErrRedirect) ? k_errout.fd : OpenConsole();
#endif					/* PresentationManager */
	 }
#else					/* error_out */
      if ((k_output.status & Fs_Write) == 0)
	 runerr(213);
      else {
#ifndef PresentationManager
	 f = k_output.fd;
#else					/* PresentationManager */
         f = (PMFlags & StdOutRedirect) ? k_output.fd : OpenConsole();
#endif					/* PresentationManager */
	 }
#endif					/* error_out */
      }
#enddef					/* DefaultFile */

#begdef Finish(retvalue, nl, terminate)
#if nl
   /*
    * Append a newline to the file.
    */
#ifdef Graphics
   pollctr >>= 1;
   pollctr++;
   if (status & Fs_Window)
      wputc('\n',(wbp)f);
   else
#endif					/* Graphics */
#ifdef RecordIO
      if (!(status & Fs_Record))
#endif					/* RecordIO */
	 putc('\n', f);
#endif					/* nl */

   /*
    * Flush the file.
    */
#ifdef Graphics
   if (!(status & Fs_Window)) {
#endif					/* Graphics */
#ifdef RecordIO
      if (status & Fs_Record)
	 flushrec(f);
#endif					/* RecordIO */

      if (ferror(f))
	 runerr(214);
      fflush(f);

#ifdef Graphics
      }
#ifdef PresentationManager
    /* must be writing to a window, then, if it is not the console,
       we have to set the background mix mode of the character bundle 
       back to LEAVEALONE so the background is no longer clobbered */
    else if (f != ConsoleBinding) {
      /* have to set the background mode back to leave-alone */
      ((wbp)f)->context->charBundle.usBackMixMode = BM_LEAVEALONE;
      /* force the reload on next use */
      ((wbp)f)->window->charContext = NULL;
      } /* End of else if - not the console window we're writing to */
#endif					/* PresentationManager */
#endif					/* Graphics */


#if terminate
	    c_exit(ErrorExit);
#else					/* terminate */
	    return retvalue;
#endif					/* terminate */
#enddef					/* Finish */

#begdef GenWrite(name, nl, terminate)

#name "(a,b,...) - write arguments"
#if !nl
   " without newline terminator"
#endif					/* nl */
#if terminate
   " (starting on error output) and stop"
#endif					/* terminate */
"."

#if terminate
function {} name(x[nargs])
#else					/* terminate */
function {1} name(x[nargs])
#endif					/* terminate */

   declare {
      FILE *f = NULL;
      word status =
#if terminate
#ifndef PresentationManager
	k_errout.status;
#else					/* PresentationManager */
        (PMFlags & StdErrRedirect) ? k_errout.status : Fs_Read | Fs_Write | Fs_Window;
#endif					/* PresentationManager */
#else					/* terminate */
#ifndef PresentationManager
	k_output.status;
#else					/* PresentationManager */
        (PMFlags & StdOutRedirect) ? k_output.status : Fs_Read | Fs_Write | Fs_Window;
#endif					/* PresentationManager */
#endif					/* terminate */

#ifdef BadCode
      struct descrip temp;
#endif					/* BadCode */
      }

#if terminate
   abstract {
      return empty_type
      }
#endif					/* terminate */

   len_case nargs of {
      0: {
#if !terminate
	 abstract {
	    return null
	    }
#endif					/* terminate */
	 DefaultFile(terminate)
	 body {
	    Finish(nulldesc, nl, terminate)
	    }
	 }

      default: {
#if !terminate
	 abstract {
	    return type(x)
	    }
#endif					/* terminate */
	 /*
	  * See if we need to start with the default file.
	  */
	 if !is:file(x[0]) then
	    DefaultFile(terminate)

	 body {
	    tended struct descrip t;
	    register word n;

	    /*
	     * Loop through the arguments.
	     */
	    for (n = 0; n < nargs; n++) {
	       if (is:file(x[n])) {	/* Current argument is a file */
#if nl
		  /*
		   * If this is not the first argument, output a newline to the
		   * current file and flush it.
		   */
		  if (n > 0) {

		     /*
		      * Append a newline to the file and flush it.
		      */
#ifdef Graphics
		     pollctr >>= 1;
		     pollctr++;
		     if (status & Fs_Window) {
			wputc('\n',(wbp)f);
			wflush((wbp)f);
			  }
		     else {
#endif					/* Graphics */
#ifdef RecordIO
			if (status & Fs_Record)
			   flushrec(f);
			else
#endif					/* RecordIO */

			   putc('\n', f);

			if (ferror(f))
			   runerr(214);
			fflush(f);
#ifdef Graphics
			}
#endif					/* Graphics */
		     }
#endif					/* nl */

#ifdef PresentationManager
                 /* have to put the background mix back on the current file */
                 if (f != NULL && (status & Fs_Window) && f != ConsoleBinding) {
                   /* set the background back to leave-alone */
                   ((wbp)f)->context->charBundle.usBackMixMode = BM_LEAVEALONE;
                   /* unload the context from this window */
                   ((wbp)f)->window->charContext = NULL;
                   } /* End of if - not the console we're writing to */
#endif					/* PresentationManager */

		  /*
		   * Switch the current file to the file named by the current
		   * argument providing it is a file.
		   */
		  status = BlkLoc(x[n])->file.status;
		  if ((status & Fs_Write) == 0)
		     runerr(213, x[n]);
		  f = BlkLoc(x[n])->file.fd;
#ifdef PresentationManager
                  if ((f == stdout && !(PMFlags & StdOutRedirect)) ||
                      (f == stderr && !(PMFlags & StdErrRedirect))) {
                    f = OpenConsole();
                    status = Fs_Read | Fs_Write | Fs_Window;
                    } /* End of if - have to catch console output */
                  else if (status & Fs_Window) {
                    /* have to set the background to overpaint - the one
                       difference between XDrawString and write(s) */
                    ((wbp)f)->context->charBundle.usBackMixMode = BM_OVERPAINT;
                    /* unload the context from the window so it will be reloaded */
                    ((wbp)f)->window->charContext = NULL;
                    } /* End of elseif - window */
#endif					/* PresentationManager */
		  }
	       else {
		  /*
		   * Convert the argument to a string, defaulting to a empty
		   *  string.
		   */
		  if (!def:tmp_string(x[n],emptystr,t))
		     runerr(109, x[n]);

		  /*
		   * Output the string.
		   */
#ifdef Graphics
		  if (status & Fs_Window)
		     wputstr((wbp)f, StrLoc(t), StrLen(t));
		  else
#endif					/* Graphics */
#ifdef RecordIO
		     if ((status & Fs_Record ? putrec(f, &t) :
					     putstr(f, &t)) == Failed)
#else					/* RecordIO */
		     if (putstr(f, &t) == Failed)
#endif					/* RecordIO */
			runerr(214, x[n]);
		  }
	       }

	    Finish(x[n-1], nl, terminate)
	    }
	 }
      }
end
#enddef					/* GenWrite */

GenWrite(stop,	 True,	True)  /* stop(s, ...) - write message and stop */
GenWrite(write,  True,	False) /* write(s, ...) - write with new-line */
GenWrite(writes, False, False) /* writes(s, ...) - write with no new-line */

#ifdef KeyboardFncs
#if UNIX
int keyboard_error;
#endif					/* UNIX */

"getch() - return a character from console."

function{0,1} getch()
   abstract {
      return string;
      }
   body {
      int i;
#if UNIX
      keyboard_error = 0;
#endif					/* UNIX */
#ifndef PresentationManager
      i = getch();
#else					/* PresentationManager */
      struct descrip res;
      if (wgetchne((wbp)OpenConsole(), &res) < 0) fail;
      i = *StrLoc(res);
#endif					/* PresentationManager */
#if UNIX
      if (keyboard_error) runerr(keyboard_error);
#endif					/* UNIX */
      if (i<0 || i>255)
	 fail;
      return string(1, (char *)&allchars[FromAscii(i) & 0xFF]);
      }
end

"getche() -- return a character from console with echo."

function{0,1} getche()
   abstract {
      return string;
      }
   body {
      int i;
#if UNIX
      keyboard_error = 0;
#endif					/* UNIX */
#ifndef PresentationManager
      i = getche();
#else					/* PresentationManager */
      struct descrip res;
      if (wgetche((wbp)OpenConsole(), &res) < 0) fail;
      i = *StrLoc(res);
#endif					/* PresentationManager */
#if UNIX
      if (keyboard_error) runerr(keyboard_error);
#endif					/* UNIX */
      if (i<0 || i>255)
	 fail;
      return string(1, (char *)&allchars[FromAscii(i) & 0xFF]);
      }
end


"kbhit() -- Check to see if there is a keyboard character waiting to be read."

function{0,1} kbhit()
   abstract {
      return null
      }
   inline {
      int rv;
#if UNIX
      keyboard_error = 0;
#endif					/* UNIX */
#ifndef PresentationManager
      rv = kbhit();
#else					/* PresentationManager */
     /* make sure we're up-to-date event wise */

     /* look in the console's icon event list for either a string or event > 60k */

     fail;
#endif					/* PresentationManager */
#if UNIX
      if (keyboard_error) runerr(keyboard_error);
#endif					/* UNIX */
      if (rv) {
	 return nulldesc;
	 }
      else fail;
      }
end
#endif					/* KeyboardFncs */

"chdir(s) - change working directory to s."
function{0,1} chdir(s)

   if !cnv:C_string(s) then
      runerr(103,s)
   abstract {
      return null
      }
   inline {

/*
 * The following code is operating-system dependent [@fsys.01].
 *  Change directory.
 */

#if PORT
Deliberate Syntax Error
#endif					/* PORT */

#if AMIGA || ARM || MACINTOSH || MVS || VM
      runerr(121);
#endif					/* AMIGA || ARM || MACINTOSH ... */

#if ATARI_ST || MSDOS || OS2 || UNIX || VMS
#if MSNT
      int nt_chdir(char *);
      if (nt_chdir(s) != 0)
#else					/* MSNT */
      if (chdir(s) != 0)
#endif					/* MSNT */
	 fail;
      return nulldesc;
#endif					/* ATARI_ST || MSDOS || ... */

/*
 * End of operating-system specific code.
 */
   }
end

#if MSNT
#passthru #include <direct.h>
int nt_chdir(char *s)
{
    return chdir(s);
}
#endif					/* MSNT */

"delay(i) - delay for i milliseconds."

function{1} delay(n)

   if !cnv:C_integer(n) then
      runerr(101,n)
   abstract {
      return null
      }

   inline {
      if (idelay(n) == Failed)
        fail;
#ifdef Graphics
      pollctr >>= 1;
      pollctr++;
#endif					/* Graphics */
      return nulldesc;
      }
end

"flush(f) - flush file f."

function{1} flush(f)
   if !is:file(f) then
      runerr(105, f)
   abstract {
      return type(f)
      }

   body {
      FILE *fp;
      fp = BlkLoc(f)->file.fd;

#ifdef Graphics
      pollctr >>= 1;
      pollctr++;

#ifndef PresentationManager
      if (BlkLoc(f)->file.status & Fs_Window)
	 wflush((wbp)fp);
      else
#else
       if (!(BlkLoc(f)->file.status & Fs_Window))
#endif					/* PresentationManager */
#endif					/* Graphics */
	 fflush(fp);

      /*
       * Return the flushed file.
       */
      return f;
      }
end
