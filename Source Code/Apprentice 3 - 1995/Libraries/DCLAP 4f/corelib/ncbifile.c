/*   ncbifile.c
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE                          
*               National Center for Biotechnology Information
*                                                                          
*  This software/database is a "United States Government Work" under the   
*  terms of the United States Copyright Act.  It was written as part of    
*  the author's official duties as a United States Government employee and 
*  thus cannot be copyrighted.  This software/database is freely available 
*  to the public for use. The National Library of Medicine and the U.S.    
*  Government have not placed any restriction on its use or reproduction.  
*                                                                          
*  Although all reasonable efforts have been taken to ensure the accuracy  
*  and reliability of the software and data, the NLM and the U.S.          
*  Government do not and cannot warrant the performance or results that    
*  may be obtained by using this software or data. The NLM and the U.S.    
*  Government disclaim all warranties, express or implied, including       
*  warranties of performance, merchantability or fitness for any particular
*  purpose.                                                                
*                                                                          
*  Please cite the author in any work or product based on this material.   
*
* ===========================================================================
*
* File Name:  ncbifile.c
*
* Author:  Gish, Kans, Ostell, Schuler
*
* Version Creation Date:   3/4/91
*
* $Revision: 2.35 $
*
* File Description: 
*     portable file routines
*
* Modifications:  
* --------------------------------------------------------------------------
* Date     Name        Description of modification
* -------  ----------  -----------------------------------------------------
* 04-15-93 Schuler     Changed _cdecl to LIBCALL
* 12-20-93 Schuler     Converted ErrPost to ErrPostEx
*
* ==========================================================================
*/

#undef  THIS_MODULE
#define THIS_MODULE g_corelib
#undef  THIS_FILE
#define THIS_FILE  _this_file

#include <ncbi.h>
#include <ncbiwin.h>

#if (defined(OS_DOS) && defined(WIN_DUMB))
#endif

#ifdef OS_MAC
#include <Errors.h>
#include <GestaltEqu.h>
#include <Folders.h>
#include <Strings.h>
#endif

#ifdef OS_UNIX_SUN
#include <sys/file.h>
#include <sun/dkio.h>
#include <sys/buf.h>
#include <scsi/targets/srdef.h>
#define DEFAULT_CDROM "/dev/sr0"
#define DEFAULT_RAW_CDROM "/dev/rsr0"
#endif

#ifdef PROC_MIPS
#define DEFAULT_CDROM "/dev/scsi/sc0d4l0"
#endif

#ifdef OS_UNIX
#ifndef DEFAULT_CDROM
#define DEFAULT_CDROM "/dev/cdrom"
#endif
#endif

#ifdef WIN_MSWIN
static char * localbuf;
static int filecount;
#define LOCAL_BUF_SIZE 2*KBYTE
#endif

#if (defined(OS_DOS) || defined (OS_NT))
#ifdef WIN_DUMB
#include <fcntl.h>         /* for setmode() */
#include <io.h>
#endif
#ifdef COMP_MSC
#include <direct.h>
#ifndef mkdir
#define mkdir _mkdir
#endif
#ifndef stat
#define stat _stat
#endif
#endif
#ifdef COMP_BOR
#include <dir.h>
#endif
#endif

#ifdef OS_VMS
#include <stat.h>   /* fstat function and buffer definition */
#include <fab.h>    /* RFM (Record Format) definitions      */
#ifndef DEFAULT_CDROM
#define DEFAULT_CDROM "cdrom:"
#endif
#endif

extern char *g_corelib;
static char * _this_file = __FILE__;


/*****************************************************************************
*
*   Macintosh file utilities
*
*****************************************************************************/

#ifdef OS_MAC
static short Nlm_MacGetVRefNum (Nlm_CharPtr pathname, OSErr *errptr)

{
  OSErr           err;
  FILE            *f;
  Nlm_Char        filename [FILENAME_MAX];
  Nlm_Char        path [256];
  HParamBlockRec  pbh;
  Nlm_CharPtr     ptr;

  memset (&pbh, 0, sizeof (HParamBlockRec));
  Nlm_StringNCpy (path, pathname, sizeof (path) - 1);
  ptr = Nlm_StringRChr (path, (int) DIRDELIMCHR);
  if (ptr != NULL) {
    ptr++;
    Nlm_StringNCpy (filename, ptr, sizeof (filename) - 1);
    *ptr = '\0';
#if defined(COMP_MPW) || defined(COMP_CODEWAR)
    c2pstr ((char *) path);
#else
#ifdef COMP_THINKC
    CtoPstr ((char *) path);
#endif
#endif
    pbh.volumeParam.ioNamePtr = (StringPtr) path;
    pbh.volumeParam.ioVolIndex = -1;
    err = PBHGetVInfo (&pbh, FALSE);
    if (errptr != NULL) {
      *errptr = err;
    }
    return pbh.volumeParam.ioVRefNum;
  } else {
    if (errptr != NULL) {
      *errptr = noErr;
    }
    return 0;
  }
}

static long Nlm_MacGetDirID (Nlm_CharPtr pathname, short newVRefNum, OSErr *errptr)

{
  OSErr           err;
  FILE            *f;
  Nlm_Char        path [256];
  CInfoPBRec      pbc;
  Nlm_CharPtr     ptr;

  memset (&pbc, 0, sizeof (CInfoPBRec));
  Nlm_StringNCpy (path, pathname, sizeof (path) - 1);
  ptr = Nlm_StringRChr (path, (int) DIRDELIMCHR);
  if (ptr != NULL) {
    ptr++;
    Nlm_StringNCpy (path, pathname, sizeof (path) - 1);
    *ptr = '\0';
#if defined(COMP_MPW) || defined(COMP_CODEWAR)
    c2pstr ((char *) path);
#else
#ifdef COMP_THINKC
    CtoPstr ((char *) path);
#endif
#endif
    pbc.dirInfo.ioNamePtr = (StringPtr) path;
    pbc.dirInfo.ioVRefNum = newVRefNum;
    err = PBGetCatInfo (&pbc, FALSE);
    if (errptr != NULL) {
      *errptr = err;
    }
    return pbc.dirInfo.ioDrDirID;
  } else {
    if (errptr != NULL) {
      *errptr = noErr;
    }
    return 0;
  }
}

static OSErr Nlm_SetDefault (short newVRefNum, long newDirID, short *oldVRefNum, long *oldDirID)

{
  OSErr  error;
  long   procID;

  error = HGetVol (NULL, oldVRefNum, oldDirID);
  if (error == noErr) {
    error = HSetVol (NULL, newVRefNum, newDirID);
  }
  return (error);
}

static OSErr Nlm_RestoreDefault (short oldVRefNum, long oldDirID)

{
  OSErr  error;
  short  defaultVRefNum;
  long   defaultDirID;
  long   defaultProcID;

  error = GetWDInfo (oldVRefNum, &defaultVRefNum, &defaultDirID, &defaultProcID);
  if (error == noErr) {
    if (defaultDirID != fsRtDirID) {
      error = SetVol (NULL, oldVRefNum);
    } else {
      error = HSetVol (NULL, oldVRefNum, oldDirID);
    }
  }
  return (error);
}
#endif

/*****************************************************************************
*
*   FileOpen(filename, mode)
*     if (filename == "stdin" or "stdout" or "stderr"
*           returns those predefined
*           streams on non-windowing systems)
*
*****************************************************************************/

static Nlm_FileOpenHook _hookFile = NULL;

#ifdef COMP_MPW
/*
*  MPWOptimizationErrorBypass was called in order to avoid an apparent MPC C
*  compiler optimization problem that resulted in the newDirID value sometimes
*  appearing to be 0.  Placing any debugging statement (or this dummy function)
*  after the statement that gets newDirID and before the statement that uses it
*  originally appeared to fix the problem.  Upon further investigation, it turns
*  out to be a problem with a UNIX to Mac file server product.  The problem did
*  not occur when compiling under THINK C.
*/

static Nlm_Int2 Nlm_MPWOptimizationErrorBypass (short newVRefNum, long newDirID)
{
}

/*
*  In MPW, if a temporary file is written first, without being created, it is
*  created on the hard disk, rather than in the appropriate path.  This code
*  creates the file in the desired location.
*/

static void Nlm_MPWCreateOutputFile (Nlm_CharPtr pathname, Nlm_CharPtr filename)

{
  OSErr     err;
  FILE      *f;
  Nlm_Char  temp [256];

  f = fopen (filename, "r");
  if (f == NULL) {
    Nlm_StringNCpy (temp, pathname, sizeof (temp));
#if defined(COMP_MPW) || defined(COMP_CODEWAR)
  c2pstr ((char *) temp);
#else
#ifdef COMP_THINKC
  CtoPstr ((char *) temp);
#endif
#endif
    err = Create ((StringPtr) temp, 0, '    ', 'TEXT');
  } else {
    fclose (f);
  }
}

static FILE * LIBCALL  Nlm_MPWFileOpen (Nlm_CharPtr pathname, Nlm_CharPtr mode)

{
  Nlm_Boolean  createfile;
  OSErr        err;
  FILE         *f;
  long         newDirID;
  short        newVRefNum;
  long         oldDirID;
  short        oldVRefNum;
  Nlm_CharPtr  ptr;

  newVRefNum = Nlm_MacGetVRefNum (pathname, &err);
  newDirID = Nlm_MacGetDirID (pathname, newVRefNum, &err);
  ptr = Nlm_StringRChr (pathname, (int) DIRDELIMCHR);
  createfile = (Nlm_Boolean) (strchr (mode, 'w') != NULL);
  if (ptr != NULL) {
    ptr++;
	Nlm_MPWOptimizationErrorBypass (newVRefNum, newDirID);
    err = Nlm_SetDefault (newVRefNum, newDirID, &oldVRefNum, &oldDirID);
	if (createfile) {
	  Nlm_MPWCreateOutputFile (pathname, ptr);
	}
    f = fopen (ptr, mode);
    err = Nlm_RestoreDefault (oldVRefNum, oldDirID);
  } else {
    if (createfile) {
	  Nlm_MPWCreateOutputFile (pathname, pathname);
	}
    f = fopen (pathname, mode);
  }
  return f;
}
#endif

FILE * LIBCALL  Nlm_FileOpen (const char *filename, const char *mode)
{
  FILE      *f;
#ifdef OS_MAC
  OSType    fCreator;
  Nlm_Int2  fError;
  FInfo     fInfo;
  OSType    fType;
  Nlm_Char  temp [256];
#endif

  if (_hookFile != NULL) {
    return _hookFile (filename, mode);
  }
  f = NULL;
#ifdef WIN_DUMB
  if (! StringCmp ("stdin", filename)) {
#ifdef OS_DOS
    if (strchr (mode, 'b') != NULL) {
      setmode (fileno (stdin), O_BINARY);
    }
#endif
    f = stdin;
  } else if (! StringCmp ("stdout", filename)) {
#ifdef OS_DOS
    if (strchr (mode, 'b') != NULL) {
      setmode (fileno (stdout), O_BINARY);
    }
#endif
    f = stdout;
  } else if (! StringCmp("stderr", filename)) {
    f = stderr;
  } else {
#ifdef OS_VMS
    f = fopen (filename, mode);
    if (f) {
      stat_t statbuf;
      if ( fstat(fileno(f),&statbuf) == 0 ) {
        if (statbuf.st_fab_rfm == FAB$C_UDF ) {
          fclose(f);
          f = fopen(filename,mode,"ctx=stm");
        }
      }
    }
#else
    f = fopen (filename, mode);
#endif
  }
#else
#ifdef OS_MAC
  Nlm_StringNCpy (temp, filename, sizeof (temp));
#if defined(COMP_MPW) || defined(COMP_CODEWAR)
    c2pstr ((char *) temp);
#else
#ifdef COMP_THINKC
    CtoPstr ((char *) temp);
#endif
#endif
  fError = GetFInfo ((StringPtr) temp, 0, &fInfo);
  if (fError == 0) {
    fCreator = fInfo.fdCreator;
    fType = fInfo.fdType;
  } else {
    if (strchr (mode, 'b') != NULL) {
      fType = '    ';
    } else {
      fType = 'TEXT';
    }
    fCreator = '    ';
  }
#endif
#ifdef COMP_MPW
  {
	Nlm_Char  localmode [16];
	Nlm_Char  path [PATH_MAX];
    Nlm_StringNCpy (path, filename, sizeof (path) - 1);
    Nlm_StringNCpy (localmode, mode, sizeof (localmode) - 1);
    f = Nlm_MPWFileOpen (path, localmode);
  }
#else
#ifdef OS_VMS

    f = fopen (filename, mode);
    if (f) {
      if ( fstat(fileno(f),&statbuf) == 0 ) {
        if (statbuf.st_fab_rfm == FAB$C_UDF ) {
          fclose(f);
          f = fopen(filename,mode,"ctx=stm");
        }
      }
    }
#else
  f = fopen (filename,mode);
#endif
#endif
#ifdef OS_MAC
  fError = GetFInfo ((StringPtr) temp, 0, &fInfo);
  if (fError == 0) {
    fInfo.fdCreator = fCreator;
    fInfo.fdType = fType;
    fError = SetFInfo ((StringPtr) temp, 0, &fInfo);
  }
#endif
#endif

	if (f == NULL)
		ErrPostEx(SEV_INFO,E_File,E_FOpen,"FileOpen(\"%s\",\"%s\") failed",filename,mode);
		
	return f;
}

/*****************************************************************************
*
*   SetFileOpenHook(hook)
*
*****************************************************************************/

void LIBCALL Nlm_SetFileOpenHook (Nlm_FileOpenHook hook)
{
	_hookFile = hook;
}

/*****************************************************************************
*
*   FileClose(fp)
*
*****************************************************************************/

void LIBCALL  Nlm_FileClose (FILE *stream)
{
	if (stream == NULL)
		return;
    
#ifdef WIN_DUMB
	if (stream==stdin || stream==stdout || stream==stderr)
	{
#ifdef OS_DOS
		setmode(fileno(stream),O_TEXT);
#endif
	}
	else 
	{
		fclose(stream);
    }
#else
	fclose(stream);
#endif
}

/*****************************************************************************
*
*   FileRead(buf, size, fp)
*
*****************************************************************************/
#ifdef WIN16
#include <dos.h> /* dos.h defines the FP_SEG macro */
#endif

Nlm_sizeT LIBCALL  Nlm_FileRead (void *ptr, Nlm_sizeT size, Nlm_sizeT n, FILE *stream)
{
    if ((n != 0) && ((Nlm_Int4)(SIZE_MAX / (Nlm_Int4) n) < (Nlm_Int4) size))
        ErrPostEx(SEV_WARNING,E_Programmer,0,"FileRead: size > SIZE_MAX");
	else if (ptr != NULL && stream != NULL)
		return fread(ptr,size,n,stream);
	return 0;
}

/*****************************************************************************
*
*   FileWrite(buf, size, fp)
*
*****************************************************************************/

Nlm_sizeT LIBCALL  Nlm_FileWrite (const void *ptr, Nlm_sizeT size, Nlm_sizeT n, FILE *stream)
{
	if ((n != 0) && ((Nlm_Int4)(SIZE_MAX / (Nlm_Int4) n) < (Nlm_Int4) size))
		ErrPostEx(SEV_WARNING,E_Programmer,0,"FileWrite:  size > SIZE_MAX");
	else if (ptr != NULL && stream != NULL)
	{
		Nlm_sizeT cnt = fwrite(ptr,size,n,stream);
		if (cnt != n)
			ErrPostEx(SEV_FATAL,E_File,E_FWrite,"File write error");
		return cnt;
  	}
	return 0;
}

/*****************************************************************************
*
*   FilePuts(ptr, fp)
*
*****************************************************************************/
int LIBCALL  Nlm_FilePuts (const char *ptr, FILE *fp)
{
	int retval;

	if ((ptr == NULL) || (fp == NULL))
    	return EOF;
	if ((retval = fputs(ptr,fp)) ==EOF)
		ErrPostEx(SEV_FATAL,E_File,E_FWrite,"File write error");
	return retval;
}

/*****************************************************************************
*
*   FileGets()
*
*****************************************************************************/
char * LIBCALL  Nlm_FileGets (Nlm_CharPtr ptr, Nlm_sizeT size, FILE *fp)
{
	if ((ptr == NULL) || (size <= 0) || (fp == NULL))
		return NULL;
	return fgets(ptr,size,fp);
}


/*****************************************************************************
*
*   FileBuildPath()
*
*****************************************************************************/
Nlm_CharPtr LIBCALL  Nlm_FileBuildPath (Nlm_CharPtr root, Nlm_CharPtr sub_path, Nlm_CharPtr filename)

{
    Nlm_CharPtr tmp;
    Nlm_Boolean dir_start = FALSE;
#ifdef OS_VMS
  Nlm_Boolean had_root = FALSE;
#endif

    if (root == NULL)              /* no place to put it */
        return NULL;

    tmp = root;
    if (*tmp != '\0')                /* if not empty */
    {
#ifndef OS_VMS
        dir_start = TRUE;
#else
        had_root = TRUE;
#endif
        while (*tmp != '\0')
        {
#ifdef OS_VMS
            if (*tmp == '[')
                dir_start = TRUE;
#endif
            tmp++;
        }

        if ((*(tmp - 1) != DIRDELIMCHR) && (dir_start))
        {
            *tmp = DIRDELIMCHR;
            tmp++; *tmp = '\0';
        }
    }

    if (sub_path != NULL)
    {
#ifdef OS_VMS
        if (dir_start)
        {
            *(tmp-1) = '.';
            if (*sub_path == '[')
                sub_path++;
        }
        else if ((had_root) && (*sub_path != '['))
        {
            *tmp = '[';
            tmp++; *tmp = '\0';
        }
#else
        if ((dir_start) && (*sub_path == DIRDELIMCHR))
            sub_path++;
#endif
        tmp = StringMove(tmp, sub_path);
        if (*(tmp-1) != DIRDELIMCHR)
        {
            *tmp = DIRDELIMCHR;
            tmp++; *tmp = '\0';
        }
    }

    if (filename != NULL)
        StringMove(tmp, filename);

    return root;
}

/*****************************************************************************
*
*   FileNameFind()
*
*****************************************************************************/
Nlm_CharPtr LIBCALL Nlm_FileNameFind (Nlm_CharPtr pathname)

{
  Nlm_CharPtr  filename;
  Nlm_Int2     len;

  if (pathname != NULL) {
    len = Nlm_StringLen (pathname);
    filename = &(pathname [len]);
    while (len > 0 && pathname [len - 1] != DIRDELIMCHR) {
      len--;
      filename--;
    }
    return filename;
  } else {
    return NULL;
  }
}

/*****************************************************************************
*
*   FileLength()
*
*****************************************************************************/
Nlm_Int4 LIBCALL Nlm_FileLength (Nlm_CharPtr fileName)

{
#ifdef OS_MAC
  ParamBlockRec  params;
  OSErr          err;
  Nlm_Char       path [256];
  Nlm_Int4       rsult;

  rsult = 0;
  if (fileName != NULL && fileName [0] != '\0') {
    Nlm_StringNCpy (path, fileName, sizeof (path));
    params.fileParam.ioNamePtr = (StringPtr) path;
    params.fileParam.ioVRefNum = 0;
    params.fileParam.ioFDirIndex = 0;
#if defined(COMP_MPW) || defined(COMP_CODEWAR)
    c2pstr ((char *) path);
#else
#ifdef COMP_THINKC
    CtoPstr ((char *) path);
#endif
#endif
    err = PBGetFInfo (&params, FALSE);
    if (err == noErr) {
      rsult = params.fileParam.ioFlLgLen;
    }
  }
  return rsult;
#else
  int          err;
  Nlm_Char     local [256];
  Nlm_Int4     rsult;
  struct stat  sbuf;

  rsult = 0;
  if (fileName != NULL && fileName [0] != '\0') {
    Nlm_StringNCpy (local, fileName, sizeof (local));
    err = stat (local, &sbuf);
    if (err == 0) {
      rsult = sbuf.st_size;
    }
  }
  return rsult;
#endif
}

/*****************************************************************************
*
*   FileDelete()
*
*****************************************************************************/
Nlm_Boolean LIBCALL Nlm_FileRemove (Nlm_CharPtr fileName)

{
  Nlm_Char  local [256];

  if (fileName != NULL && fileName [0] != '\0') {
    Nlm_StringNCpy (local, fileName, sizeof (local));
    return (Nlm_Boolean) (remove (local) == 0);
  } else {
    return FALSE;
  }
}

/*****************************************************************************
*
*   FileRename()
*
*****************************************************************************/
Nlm_Boolean LIBCALL Nlm_FileRename (Nlm_CharPtr oldFileName, Nlm_CharPtr newFileName)

{
  Nlm_Char  localnew [256];
  Nlm_Char  localold [256];

  if (oldFileName != NULL && oldFileName [0] != '\0'
    && newFileName != NULL && newFileName [0] != '\0') {
    Nlm_StringNCpy (localold, oldFileName, sizeof (localold));
    Nlm_StringNCpy (localnew, newFileName, sizeof (localnew));
    return (Nlm_Boolean) (rename (localold, localnew) == 0);
  } else {
    return FALSE;
  }
}

/*****************************************************************************
*
*   FileCreate()
*
*****************************************************************************/
#ifdef WIN_MAC
static OSType Nlm_GetOSType (Nlm_CharPtr str, OSType dfault)

{
  OSType  rsult;

  rsult = dfault;
  if (str != NULL && str [0] != '\0') {
    rsult = *(OSType*) str;
  }
  return rsult;
}
#endif

void LIBCALL Nlm_FileCreate (Nlm_CharPtr fileName, Nlm_CharPtr type, Nlm_CharPtr creator)

{
  FILE      *fp;
#ifdef WIN_MAC
  OSType    fCreator;
  Nlm_Int2  fError;
  OSType    fType;
  Nlm_Char  temp [256];
#endif

  if (fileName != NULL && fileName [0] != '\0') {
#ifdef WIN_MAC
    if (type != NULL || creator != NULL) {
      fp = Nlm_FileOpen (fileName, "r");
      if (fp == NULL) {
        fType = Nlm_GetOSType (type, 'TEXT');
        fCreator = Nlm_GetOSType (creator, '    ');
        Nlm_StringNCpy (temp, fileName, sizeof (temp));
#if defined(COMP_MPW) || defined(COMP_CODEWAR)
    c2pstr ((char *) temp);
#else
#ifdef COMP_THINKC
    CtoPstr ((char *) temp);
#endif
#endif
        fError = Create ((StringPtr) temp, 0, fCreator, fType);
      } else {
        Nlm_FileClose (fp);
      }
    }
#else
    fp = Nlm_FileOpen (fileName, "w");
    if (fp != NULL) {
      Nlm_FileClose (fp);
    }
#endif
  }
}

/*****************************************************************************
*
*   CreateDir(pathname)
*
*****************************************************************************/

Nlm_Boolean LIBCALL  Nlm_CreateDir (Nlm_CharPtr pathname)

{
#ifdef OS_MAC
  long            dirID;
  Nlm_Char        dirname [FILENAME_MAX];
  OSErr           err;
  Nlm_sizeT          len;
  Nlm_Char        path [PATH_MAX];
  HParamBlockRec  pbh;
  Nlm_CharPtr     ptr;
  short           vRefNum;
#endif
#if (defined(OS_DOS) || defined (OS_NT))
  Nlm_sizeT          len;
  Nlm_Char        path [PATH_MAX];
#endif
#ifdef OS_UNIX
  Nlm_sizeT          len;
  mode_t          oldmask;
  Nlm_Char        path [PATH_MAX];
#endif
  Nlm_Boolean     rsult;

  rsult = FALSE;
  if (pathname != NULL && pathname [0] != '\0') {
#ifdef OS_MAC
    Nlm_StringNCpy (path, pathname, sizeof (path) - 1);
    len = Nlm_StringLen (path);
    if (len > 0 && path [len - 1] == DIRDELIMCHR) {
        path [len - 1] = '\0';
    }
    memset (&pbh, 0, sizeof (HParamBlockRec));
    vRefNum = Nlm_MacGetVRefNum (path, &err);
    if (err == noErr) {
      dirID = Nlm_MacGetDirID (path, vRefNum, &err);
      if (err == noErr) {
        ptr = Nlm_StringRChr (path, (int) DIRDELIMCHR);
        if (ptr != NULL) {
          ptr++;
          Nlm_StringNCpy (dirname, ptr, sizeof (dirname) - 1);
#if defined(COMP_MPW) || defined(COMP_CODEWAR)
    c2pstr ((char *) dirname);
#else
#ifdef COMP_THINKC
    CtoPstr ((char *) dirname);
#endif
#endif
          pbh.fileParam.ioNamePtr = (StringPtr) dirname;
          pbh.fileParam.ioVRefNum = vRefNum;
          pbh.fileParam.ioDirID = dirID;
          err = PBDirCreate (&pbh, FALSE);
          rsult = (Nlm_Boolean) (err == noErr || err == dupFNErr);
        }
      }
    }
#endif
#if (defined(OS_DOS) || defined (OS_NT))
    Nlm_StringNCpy (path, pathname, sizeof (path) - 1);
    len = Nlm_StringLen (path);
    if (len > 0 && path [len - 1] == DIRDELIMCHR) {
        path [len - 1] = '\0';
    }
    rsult = (Nlm_Boolean) (mkdir ((char *) path) == 0);
    if (errno == EACCES) { /* it's O.K. if it was already there */
	rsult = TRUE;
    }
#endif
#ifdef OS_UNIX
    oldmask = umask (0000);
    Nlm_StringNCpy (path, pathname, sizeof (path) - 1);
    len = Nlm_StringLen (path);
    if (len > 0 && path [len - 1] == DIRDELIMCHR) {
        path [len - 1] = '\0';
    }
    rsult = (Nlm_Boolean) (mkdir ((char *) path, 0755) == 0);
    if (errno == EEXIST) { /* it's O.K. if it was already there */
	rsult = TRUE;
    }
    umask (oldmask);
#endif
#ifdef OS_VMS
    rsult = (Nlm_Boolean) (mkdir ((char *) pathname, 0755) == 0);
#endif
  }
  return rsult;
}

/*****************************************************************************
*
*   TmpNam()
*
*****************************************************************************/
Nlm_CharPtr LIBCALL Nlm_TmpNam (Nlm_CharPtr s)

{
#ifdef TEMPNAM_AVAIL
    char *filename;
    static Nlm_Char save_filename[L_tmpnam+30];

    /* emulate tmpnam(), except get the benefits of tempnam()'s ability to */
    /* place the files in another directory specified by the environment   */
    /* variable TMPDIR                                                     */

    filename = tempnam("", "");

    if (s == NULL)
    { /* return pointer to static string */
        if (filename != NULL) {
          strcpy ((char *) save_filename, (char *) filename);
          free ((void *) filename);
        } else {
          save_filename [0] = '\0';
        }
        return save_filename;
    } else {
        if (filename != NULL) {
          strcpy ((char *) save_filename, (char *) filename);
          Nlm_StrCpy (s, save_filename);
          free ((void *) filename);
        } else {
          *s = '\0';
        }
        return s;
    }
#else
#ifdef OS_MAC
    static Nlm_Char  directory [PATH_MAX];
    OSErr        err;
    long         gesResponse;
    long         newDirID;
    short        newVRefNum;
    long         oldDirID;
    short        oldVRefNum;
    CInfoPBRec   params;
    Nlm_Char     temp [PATH_MAX];
    Nlm_CharPtr  tmp;
    Nlm_Boolean  useTempFolder;
    char * filename;

    useTempFolder = FALSE;
    if (! Gestalt (gestaltFindFolderAttr, &gesResponse) &&
        (gesResponse & (1 << gestaltFindFolderPresent))) {
      err = FindFolder(kOnSystemDisk, kTemporaryFolderType,
                       kCreateFolder, &newVRefNum, &newDirID);
      if (err == noErr) {
        useTempFolder = TRUE;
        err = Nlm_SetDefault (newVRefNum, newDirID, &oldVRefNum, &oldDirID);
      }
    }
    filename = tmpnam (NULL);
    if (useTempFolder) {
      err = Nlm_RestoreDefault (oldVRefNum, oldDirID);
      temp [0] = '\0';
      params.dirInfo.ioNamePtr = (StringPtr) directory;
      params.dirInfo.ioDrParID = newDirID;
      do {
        params.dirInfo.ioVRefNum = newVRefNum;
        params.dirInfo.ioFDirIndex = -1;
        params.dirInfo.ioDrDirID = params.dirInfo.ioDrParID;
        err = PBGetCatInfo (&params, FALSE);
#if defined(COMP_MPW) || defined(COMP_CODEWAR)
  p2cstr ((StringPtr) directory);
#else
#ifdef COMP_THINKC
  PtoCstr ((StringPtr) directory);
#endif
#endif
        Nlm_StringCat (directory, DIRDELIMSTR);
        Nlm_StringCat (directory, temp);
        Nlm_StringCpy (temp, directory);
      } while (params.dirInfo.ioDrDirID != fsRtDirID);
      tmp = Nlm_StringMove (directory, temp);
      tmp = Nlm_StringMove (tmp, (Nlm_CharPtr) filename);
      if (s == NULL) {
          return (Nlm_CharPtr) directory;
      } else {
          s [0] = '\0';
          Nlm_StringCpy (s, directory);
          return s;
      }
    } else {
      if (s == NULL) {
          return (Nlm_CharPtr) filename;
      } else {
          s [0] = '\0';
          Nlm_StringCpy (s, filename);
          return s;
      }
    }
#else
    char * filename;

    filename = tmpnam (NULL);
    if (s == NULL) {
        return (Nlm_CharPtr) filename;
    } else {
        s [0] = '\0';
        Nlm_StringCpy (s, filename);
        return s;
    }
#endif
#endif
}

/*****************************************************************************
*
*   CD-ROM Ejection Routines
*
*****************************************************************************/

Nlm_Boolean LIBCALL  Nlm_EjectCd(Nlm_CharPtr sVolume, Nlm_CharPtr deviceName,
			Nlm_CharPtr rawDeviceName, 
			Nlm_CharPtr mountPoint,
			Nlm_CharPtr mountCmd)
{
    Nlm_Boolean retval = FALSE;
#ifdef OS_MAC
    OSErr err;
    Nlm_CharPtr prob_area = "Ejection";
    Nlm_Char    temp [64];

    
    Nlm_StringNCpy (temp, sVolume, sizeof (temp) - 1);
    Nlm_StringCat (temp, ":");
    if ((err = Eject((StringPtr) NULL, Nlm_MacGetVRefNum(temp, NULL))) == noErr)
    {
        if ((err = UnmountVol((StringPtr) NULL, Nlm_MacGetVRefNum(temp, NULL))) == noErr)
        	return TRUE;
        
        /* We should still return TRUE if we Eject() successfully but failed to    */
        /* unmount the volume; however, we need to warn them, because a subsequent */
        /* GetFInfo() will result in a bus error, at least with System 7.0.        */
        retval = TRUE;
        prob_area = "Unmounting";
    }

    switch (err) {
    case bdNamErr:
	ErrPostEx(SEV_ERROR,E_File,E_CdEject,"%s error - bad volume name %s", prob_area, sVolume);
	break;
    case extFSErr:
	ErrPostEx(SEV_ERROR,E_File,E_CdEject,"%s error - external file system %s", prob_area, sVolume);
	break;
    case ioErr:
	ErrPostEx(SEV_ERROR,E_File,E_CdEject,"%s error - I/O error %s", prob_area, sVolume);
	break;
    case nsDrvErr:
	ErrPostEx(SEV_ERROR,E_File,E_CdEject,"%s error - No such drive %s", prob_area, sVolume);
	break;
    case nsvErr:
	ErrPostEx(SEV_ERROR,E_File,E_CdEject,"%s error - No such volume %s", prob_area, sVolume);
	break;
    case paramErr:
	ErrPostEx(SEV_ERROR,E_File,E_CdEject,"%s error - No default volume %s", prob_area, sVolume);
	break;
    }
    
    return retval;
#endif /* OS_MAC */

#ifdef OS_UNIX
	char cmd[100];
#endif
#ifdef OS_UNIX_SUN
	int fd;

	if (deviceName == NULL)
	{
		deviceName = DEFAULT_CDROM;
	}

	if (rawDeviceName == NULL)
	{
		rawDeviceName = DEFAULT_RAW_CDROM;
	}

	/* Open the CD-ROM character-based device */
	if ((fd = open(rawDeviceName, O_RDONLY, 0)) < 0)
	{
		ErrPostEx(SEV_ERROR,E_File,E_CdEject,"Ejection error - Unable to open device %s", rawDeviceName);
		return FALSE;
	}

	retval = ioctl(fd, CDROMEJECT, 0) >= 0;
	close (fd);

	if (! retval)
	{
		ErrPostEx(SEV_ERROR,E_File,E_CdEject,"Ejection error - Ioctl failure for %s", rawDeviceName);
    	return FALSE;
	}
#endif /* OS_UNIX_SUN */

#ifdef OS_UNIX
	/* Now try to unmount device using (un)mount-script */
	if (mountCmd != NULL)
	{
		sprintf(cmd, "%s -u %s >/dev/null 2>/dev/null", mountCmd,
				deviceName);
		retval = system(cmd) == 0;
	}
	else {
		if (deviceName != NULL)
		{
			retval = Message(MSG_OKC,
				            "Unmount device <%s> now; select OK when completed",
						    deviceName) != ANS_CANCEL;
		}
		else
		if (sVolume != NULL)
		{
			retval = Message(MSG_OKC,
						    "Unmount volume <%s> now; select OK when completed",
						    sVolume) != ANS_CANCEL;
		}
		else
		{
			retval = Message(MSG_OKC,
							"Unmount CD-ROM now; select OK when completed") !=
							ANS_CANCEL;
		}
	}
#endif /* OS_UNIX */

#ifdef OS_VMS
	char  cmd[100];
	char  tmp[100];
	char* cPtr;


	if ( mountPoint == NULL || *mountPoint == '\0' ) 
		strcpy(tmp,DEFAULT_CDROM); 
	else {
		strcpy(tmp,mountPoint);
		if ( cPtr = strchr(tmp,':') ) *(cPtr+1) = '\0';
	}
	/* 
	** Try to mount device using mount-script 
	*/

	sprintf(cmd, "CD_DISMOUNT/UNLOAD %s",tmp);
	retval = (system(cmd) == 0);

	 Message(MSG_OK,
		"Press the eject button on <%s>.",tmp);

#endif
	
    return retval;
}

Nlm_Boolean LIBCALL  Nlm_MountCd(Nlm_CharPtr sVolume, Nlm_CharPtr deviceName,
			Nlm_CharPtr mountPoint, Nlm_CharPtr mountCmd)
{
	Nlm_Boolean retval = FALSE;

#ifdef OS_UNIX
	char cmd[100];

	if (deviceName == NULL)
	{
		deviceName = DEFAULT_CDROM;
	}

	/* Try to mount device using mount-script */
	if (mountCmd != NULL)
	{
		sprintf(cmd, "%s -m %s %s >/dev/null 2>/dev/null", mountCmd, deviceName,
				mountPoint != NULL ? mountPoint : "");
		retval = system(cmd) == 0;
	}
	else {
		if (deviceName != NULL)
		{
		}
		else
		{
			retval = Message(MSG_OKC,
							"Mount CD-ROM now; select OK when completed") !=
							ANS_CANCEL;
		}
	}
#endif

#ifdef OS_VMS
	char  cmd[100];
	char  tmp[100];
	char* cPtr;


	if ( mountPoint == NULL || *mountPoint == '\0' ) 
		strcpy(tmp,DEFAULT_CDROM); 
	else {
		strcpy(tmp,mountPoint);
		if ( cPtr = strchr(tmp,':') ) *(cPtr+1) = '\0';
	}


	/* Try to mount device using mount-script */

	sprintf(cmd, "CD_MOUNT/MEDIA=CDROM/OVERRIDE=IDENTIFICATION/NOASSIST %s",
          tmp);

	retval = (system(cmd) == 0);

#endif

	return retval;
}

