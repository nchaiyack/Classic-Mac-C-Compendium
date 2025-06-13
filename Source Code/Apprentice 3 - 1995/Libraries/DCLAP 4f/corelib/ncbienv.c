/*   ncbienv.c
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
* File Name:  ncbienv.c
*
* Author:  Ostell
*
* Version Creation Date:   7/7/91
*
* $Revision: 1.6 $
*
* File Description: 
*       portable environment functions, companions for ncbimain.c
*
* Modifications:  
* --------------------------------------------------------------------------
* Date     Name        Description of modification
* -------  ----------  -----------------------------------------------------
* 7/7/91   Kans        Multiple configuration files, get and set functions
* 9-20-91  Schuler     GetAppParam takes default value as an argument
* 1-14-94  Epstein     Merged ncbienv.{unx,vms,dos,msw,mac} into a single file
*
* ==========================================================================
*/

#include <ncbi.h>
#include <ncbiwin.h>

#ifdef OS_UNIX
#include <pwd.h>
#endif /* OS_UNIX */
#ifdef OS_MAC
#include <GestaltEqu.h>
#include <Folders.h>
#include <Strings.h>
#endif /* OS_MAC */
#ifdef OS_VMS
#include <unixlib.h>
#endif /* OS_VMS */


typedef struct nlm_env_item {
  struct nlm_env_item  PNTR next;
  Nlm_CharPtr          name;
  Nlm_CharPtr          comment;
  Nlm_CharPtr          value;
} Nlm_env_item, PNTR Nlm_env_itemPtr;

typedef struct nlm_env_sect {
  struct nlm_env_sect  PNTR next;
  Nlm_CharPtr          name;
  Nlm_CharPtr          comment;
  Nlm_Boolean          transientOnly; /* this field used only by Transient fns */
  struct nlm_env_item  PNTR children;
} Nlm_env_sect, PNTR Nlm_env_sectPtr;

typedef struct nlm_env_file {
  struct nlm_env_file  PNTR next;
  Nlm_CharPtr          name;
  Nlm_env_sectPtr      envList;
} Nlm_env_file, PNTR Nlm_env_filePtr;


Nlm_env_filePtr Nlm_transientFileList = NULL;

static Nlm_Boolean Nlm_Qualified PROTO((Nlm_CharPtr path));
static Nlm_Boolean Nlm_TransientLookup PROTO((Nlm_CharPtr file, Nlm_CharPtr section, Nlm_CharPtr type, Nlm_CharPtr dflt, Nlm_CharPtr buf, Nlm_Int2 buflen));
static void Nlm_TransientLogSetApp PROTO((Nlm_CharPtr file, Nlm_CharPtr section, Nlm_CharPtr type, Nlm_CharPtr value));
static void Nlm_FreeEnvData PROTO((Nlm_env_sectPtr esp));
static void Nlm_FreeTransientData PROTO((void));

#ifndef WIN_MSWIN
static FILE *Nlm_OpenConfigFile PROTO((Nlm_CharPtr file, Nlm_Boolean writeMode, Nlm_Boolean create));
static Nlm_CharPtr Nlm_TrimString PROTO((Nlm_CharPtr str));
static Nlm_Boolean Nlm_ReadConfigFile PROTO((FILE *fp));
static Nlm_env_sectPtr Nlm_FindConfigSection PROTO((Nlm_CharPtr section));
static Nlm_env_itemPtr Nlm_FindConfigItem PROTO((Nlm_CharPtr section, Nlm_CharPtr type, Nlm_Boolean create));
static Nlm_Boolean Nlm_WriteConfigFile PROTO((FILE *fp));
static void Nlm_PutComment PROTO((Nlm_CharPtr s, FILE *fp));
static void Nlm_FreeConfigFileData PROTO((void));

#ifdef OS_UNIX
static Nlm_CharPtr saveHome = NULL;
#endif /* OS_UNIX */

static Nlm_env_sectPtr Nlm_envList = NULL;
static Nlm_CharPtr Nlm_lastParamFile = NULL;
static Nlm_CharPtr Nlm_bottomComment = NULL;

/* always FALSE, because this file is trying to emulating MS Windows's  */
/* handling of comments in Param files; however, just change this value */
/* to TRUE to turn this approach around                                 */
static Nlm_Boolean destroyDeadComments = FALSE;

/*****************************************************************************
*
* The "guts" of:
*   Nlm_GetAppParam (file, section, type, dflt, buf, buflen)
*      finds parameters from configuration files
*      this version, searching for configuration file(s) in a
*      platform-dependent basis as handled by Nlm_OpenConfigFile()
*
*      if configuration file is found, tries to read the parameter from it.
*
*****************************************************************************/

static Nlm_Int2 Nlm_WorkGetAppParam (Nlm_CharPtr file, Nlm_CharPtr section, Nlm_CharPtr type, Nlm_CharPtr dflt, Nlm_CharPtr buf, Nlm_Int2 buflen, Nlm_Boolean searchTransient)

{
  Nlm_env_itemPtr  eip;
  FILE             *fp;
  Nlm_env_sectPtr  esp;
  Nlm_Int2         totlen;
  Nlm_Int2         bytesToAppend;

  if (buf != NULL && buflen > 0) {
    *buf = '\0';
    if (searchTransient && Nlm_TransientLookup(file, section, type, dflt, buf, buflen))
    {
      return Nlm_StringLen(buf);
    }
    if (dflt) Nlm_StringNCat (buf, dflt, buflen);
    if (file != NULL && *file != '\0' && section != NULL && *section != '\0') {
      if (Nlm_lastParamFile == NULL ||
          Nlm_StringICmp(Nlm_lastParamFile, file) != 0) {
        Nlm_FreeConfigFileData();
        fp = Nlm_OpenConfigFile (file, FALSE, FALSE);
        if (fp != NULL) {
          Nlm_lastParamFile = Nlm_StringSave(file);
          Nlm_ReadConfigFile (fp);
          Nlm_FileClose (fp);
        }
      }
      if (type != NULL && *type != '\0') {
        eip = Nlm_FindConfigItem (section, type, FALSE);
        if (eip != NULL) {
          *buf = '\0';
          Nlm_StringNCat (buf, eip->value, buflen);
        }
      }
      else { /* return all the types in that section */
        *buf = '\0';
        totlen = 0;
        if ((esp = Nlm_FindConfigSection (section)) != NULL)
        {
          /* traverse the children, allowing the null chars to be inserted */
          /* in between each type-name                                     */
          for (eip = esp->children; eip != NULL; eip = eip->next)
          {
            bytesToAppend = StrLen(eip->name) + 1;
            bytesToAppend = MIN(bytesToAppend, buflen - totlen);
            StrNCpy(&buf[totlen], eip->name, bytesToAppend);
            totlen += bytesToAppend;
          }
          if (totlen > 0 && buf[totlen] == '\0')
            totlen--; /* account for final null character */
        }
        return totlen;
      }
    }
  }

  return Nlm_StringLen(buf);
}

/*****************************************************************************
*
*   Nlm_SetAppParam (file, section, type, value)
*      finds paths for types of data and fills in path in buf
*      this version
*      1)  looks in the current directory for ".filerc", but will not
*          create a new file in this directory.
*      2)  then looks in the home directory for ".filerc".
*      3)  then looks for an environment variable "NCBI" and takes its
*          value as a complete path to a directory containing the
*          configuration file ".filerc".
*      if configuration file is found, tries to write the parameter to it.
*
*****************************************************************************/

Nlm_Boolean LIBCALL Nlm_SetAppParam (Nlm_CharPtr file, Nlm_CharPtr section, Nlm_CharPtr type, Nlm_CharPtr value)

{
  Nlm_env_itemPtr  eip;
  Nlm_env_sectPtr  esp;
  FILE             *fp;
  Nlm_Boolean      rsult;

  rsult = FALSE;
  if (file != NULL && *file != '\0' && section != NULL && *section != '\0') {
    Nlm_TransientLogSetApp (file, section, type, value);
    fp = Nlm_OpenConfigFile (file, FALSE, TRUE);
    if (fp != NULL) {
      if (Nlm_lastParamFile == NULL ||
          Nlm_StringICmp(Nlm_lastParamFile, file) != 0) {
        Nlm_FreeConfigFileData();
        Nlm_lastParamFile = Nlm_StringSave(file);
        Nlm_ReadConfigFile (fp);
      }
      Nlm_FileClose (fp);
      if (type != NULL && *type != '\0')
      {
        eip = Nlm_FindConfigItem (section, type, TRUE);
        if (eip != NULL) {
          if (eip->value != NULL) {
            eip->value = (Nlm_CharPtr) Nlm_MemFree (eip->value);
          }
          eip->value = Nlm_StringSave (value);
          rsult = TRUE;
        }
      }
      else { /* wipe out that section */
        esp = Nlm_FindConfigSection (section);
        if (esp != NULL) { /* kill section by deleting name (leave comments)*/
          esp->name = (Nlm_CharPtr) Nlm_MemFree(esp->name);
          rsult = TRUE;
        }
      }
          
      fp = Nlm_OpenConfigFile (file, TRUE, FALSE);
      if (fp != NULL) {
        Nlm_WriteConfigFile (fp);
        Nlm_FileClose (fp);
      }
    }
  }

  return rsult;
}

#ifdef OS_UNIX
/*****************************************************************************
*
*   Nlm_GetHome (buf, buflen)
*      returns the path of the home directory
*
*****************************************************************************/

#ifndef L_cuserid
#define L_cuserid 9
#endif

static Nlm_Boolean Nlm_GetHome PROTO((Nlm_CharPtr buf, Nlm_Int2 buflen));
static Nlm_Boolean Nlm_GetHome (Nlm_CharPtr buf, Nlm_Int2 buflen)

{
  char           *getlogin PROTO((void));
/*
#ifndef COMP_ACC
  struct passwd  *getpwuid PROTO((uid_t uid));
#endif
*/

  Nlm_CharPtr    cp;
  Nlm_Boolean    found;
  struct passwd  *pswd;
  Nlm_Char       whoiam [L_cuserid + 1];

  if (saveHome != NULL)
  { /* return cached value, if available */
      Nlm_StringNCpy (buf, saveHome, buflen);
      return TRUE;
  }

  whoiam [0] = '\0';
  found = FALSE;
  cp = getlogin ();
  if (cp != NULL && cp [0] != '\0') {
    Nlm_StringNCpy (whoiam, cp, sizeof (whoiam) - 1);
    whoiam [sizeof (whoiam) - 1] = '\0';
    found = TRUE;
  } else {
    setpwent ();
    pswd = getpwuid (getuid ());
    if (pswd != NULL) {
      Nlm_StringNCpy (whoiam, pswd->pw_name, sizeof (whoiam) - 1);
      whoiam [sizeof (whoiam) - 1] = '\0';
      found = TRUE;
    }
  }
  if (found) {
    setpwent ();
    pswd = getpwnam (whoiam);
    if (pswd != NULL) {
        Nlm_StringNCpy (buf, pswd->pw_dir, buflen);
        if (saveHome == NULL)
        {
            saveHome = Nlm_StringSave(pswd->pw_dir);
        }
    } else {
      found = FALSE;
    }
  }
  return found;
}

/*****************************************************************************
*
*   Nlm_OpenConfigFile (file, writeMode, create)
*      returns a file pointer to the specified configuration file.
*      1)  looks in the current directory for ".filerc", but will not
*          create a new file in this directory.
*      2)  then looks in the home directory for ".filerc".
*      3)  then looks for an environment variable "NCBI" and takes its
*          value as a complete path to a directory containing the
*          configuration file "filerc" or ".filerc".
*
*****************************************************************************/

static FILE *Nlm_OpenConfigFile (Nlm_CharPtr file, Nlm_Boolean writeMode, Nlm_Boolean create)

{
  FILE      *fp;
  Nlm_Int2  i;
  Nlm_Int2  len;
  FILE      *newfp;
  Nlm_Char  path [PATH_MAX+1];
  char      *pth;
  Nlm_Char  str [FILENAME_MAX+1];

  fp = NULL;
  if (file != NULL) {
    newfp = NULL;
    Nlm_StringMove (str, ".");
    Nlm_StringNCat (str, file, sizeof (str) - 3);
    if ( ! Nlm_Qualified (str))
    { /* use the user's extension instead of the "rc" extension */
      Nlm_StringCat (str, "rc");
    }
    len = (Nlm_Int2) Nlm_StringLen (str);
    for (i = 0; i < len; i++) {
      str [i] = TO_LOWER (str [i]);
    }
    path[0] = '\0';
    Nlm_StringCpy (path, str);
    fp = Nlm_FileOpen (path, "r");
    if (fp == NULL) {
      path[0] = '\0';
      if (Nlm_GetHome (path, sizeof (path))) {
        Nlm_FileBuildPath(path, NULL, str);
      } else {
        Nlm_StringCpy (path, str);
      }
      fp = Nlm_FileOpen (path, "r");
      if (fp == NULL && create) {
        newfp = Nlm_FileOpen (path, "w");
        Nlm_FileClose (newfp);
        newfp = Nlm_FileOpen (path, "r");
      }
    }
    if (fp == NULL) {
      path[0] = '\0';
      pth = getenv ("NCBI");
      if (pth != NULL) {
        Nlm_FileBuildPath(path, pth, str + 1);
        fp = Nlm_FileOpen (path, "r");
        if (fp == NULL) {
          path[0] = '\0';
          Nlm_FileBuildPath(path, pth, str);
          fp = Nlm_FileOpen (path, "r");
        }
      }
    }
    if (newfp != NULL) {
      if (fp != NULL) {
        Nlm_FileClose (newfp);
        newfp = NULL;
      } else {
        fp = newfp;
      }
    }
    if (writeMode && fp != NULL) {
      Nlm_FileClose (fp);
      fp = Nlm_FileOpen (path, "w");
    }
  }
  return fp;
}

#endif /* OS_UNIX */


#ifdef OS_MAC
/*****************************************************************************
*
*   Nlm_OpenConfigFile (file, writeMode, create)
*      returns a file pointer to the specified configuration file.
*      1)  looks in the System Folder for "file.cnf"
*      2)  then looks in System Folder:Preferences for "file.cnf"
*
*****************************************************************************/

static FILE *Nlm_OpenConfigFile (Nlm_CharPtr file, Nlm_Boolean writeMode, Nlm_Boolean create)

{
  WDPBRec      block;
  Nlm_Char     directory [PATH_MAX];
  long         dirID;
  OSErr        err;
  OSType       fCreator;
  Nlm_Int2     fError;
  FILE         *fp;
  FInfo        finfo;
  OSType       fType;
  long         gesResponse;
  Nlm_Int2     i;
  Nlm_Int2     len;
  CInfoPBRec   params;
  Nlm_Char     str [FILENAME_MAX+1];
  SysEnvRec    sysenv;
  Nlm_Char     temp [PATH_MAX];
  Nlm_CharPtr  tmp;
  short        vRefNum;

  fp = NULL;
  if (file != NULL) {
    Nlm_StringNCpy (str, file, sizeof (str) - 4);
    if ( ! Nlm_Qualified (str) ) {
      Nlm_StringCat (str, ".cnf");
    }
    len = (Nlm_Int2) Nlm_StringLen (str);
    for (i = 0; i < len; i++) {
      str [i] = TO_LOWER (str [i]);
    }
    if (SysEnvirons (curSysEnvVers, &sysenv) == noErr) {
      block.ioNamePtr = NULL;
      block.ioVRefNum = sysenv.sysVRefNum;
      block.ioWDIndex = 0;
      block.ioWDProcID = 0;
      PBGetWDInfo (&block, FALSE);
      dirID = block.ioWDDirID;
      vRefNum = block.ioWDVRefNum;
      temp [0] = '\0';
      params.dirInfo.ioNamePtr = (StringPtr) directory;
      params.dirInfo.ioDrParID = dirID;
      do {
        params.dirInfo.ioVRefNum = vRefNum;
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
      tmp = Nlm_StringMove (tmp, str);
      fp = Nlm_FileOpen (directory, "r");
      if (fp == NULL) {
        if (! Gestalt (gestaltFindFolderAttr, &gesResponse) &&
            (gesResponse & (1 << gestaltFindFolderPresent))) {
          err = FindFolder(kOnSystemDisk, kPreferencesFolderType,
                           kCreateFolder, &vRefNum, &dirID);
          if (err == noErr) {
            params.dirInfo.ioNamePtr = (StringPtr) directory;
            params.dirInfo.ioDrDirID = dirID;
            params.dirInfo.ioVRefNum = vRefNum;
            params.dirInfo.ioFDirIndex = -1;
            err = PBGetCatInfo (&params, FALSE);
#if defined(COMP_MPW) || defined(COMP_CODEWAR)
  p2cstr ((StringPtr) directory);
#else
#ifdef COMP_THINKC
  PtoCstr ((StringPtr) directory);
#endif
#endif
            Nlm_StringCat (temp, directory);
            Nlm_StringCat (temp, DIRDELIMSTR);
            tmp = Nlm_StringMove (directory, temp);
            tmp = Nlm_StringMove (tmp, str);
          } else {
            tmp = Nlm_StringMove (directory, temp);
            tmp = Nlm_StringMove (tmp, "Preferences");
            tmp = Nlm_StringMove (tmp, DIRDELIMSTR);
            tmp = Nlm_StringMove (tmp, str);
          }
        } else {
          tmp = Nlm_StringMove (directory, temp);
          tmp = Nlm_StringMove (tmp, "Preferences");
          tmp = Nlm_StringMove (tmp, DIRDELIMSTR);
          tmp = Nlm_StringMove (tmp, str);
        }
        fp = Nlm_FileOpen (directory, "r");
      }
      if (fp == NULL && create) {
        tmp = Nlm_StringMove (directory, temp);
        tmp = Nlm_StringMove (tmp, str);
        fp = Nlm_FileOpen (directory, "w");
        Nlm_StringCpy (temp, directory);
#if defined(COMP_MPW) || defined(COMP_CODEWAR)
    c2pstr ((char *) temp);
#else
#ifdef COMP_THINKC
    CtoPstr ((char *) temp);
#endif
#endif
        fError = GetFInfo ((StringPtr) temp, 0, &finfo);
        if (fError == 0) {
          finfo.fdCreator = 'ttxt';
          finfo.fdType = 'TEXT';
          fError = SetFInfo ((StringPtr) temp, 0, &finfo);
        }
        Nlm_FileClose (fp);
        fp = Nlm_FileOpen (directory, "r");
      }
      Nlm_StringCpy (temp, directory);
      if (writeMode && fp != NULL) {
        Nlm_FileClose (fp);
#if defined(COMP_MPW) || defined(COMP_CODEWAR)
    c2pstr ((char *) temp);
#else
#ifdef COMP_THINKC
    CtoPstr ((char *) temp);
#endif
#endif
        fType = 'TEXT';
        fCreator = '    ';
        fError = GetFInfo ((StringPtr) temp, 0, &finfo);
        if (fError == 0) {
          fCreator = finfo.fdCreator;
          fType = finfo.fdType;
        }
        fp = Nlm_FileOpen (directory, "w");
        fError = GetFInfo ((StringPtr) temp, 0, &finfo);
        if (fError == 0) {
          finfo.fdCreator = fCreator;
          finfo.fdType = fType;
          fError = SetFInfo ((StringPtr) temp, 0, &finfo);
        }
      }
    }
  }
  return fp;
}
#endif /* OS_MAC */

#ifdef OS_VMS
/*****************************************************************************
*
*   Nlm_GetHome (buf, buflen)
*      returns the path of the home directory
*
*****************************************************************************/

static Nlm_Boolean Nlm_GetHome PROTO((Nlm_CharPtr buf, Nlm_Int2 buflen));
static Nlm_Boolean Nlm_GetHome (Nlm_CharPtr buf, Nlm_Int2 buflen)

{
  StringCpy(buf, getenv("SYS$LOGIN"));
  return TRUE;
}

/*****************************************************************************
*
*   Nlm_OpenConfigFile (file, writeMode, create)
*      returns a file pointer to the specified configuration file.
*      1)  looks in the current directory for "file.cfg", but will not
*          create a new file in this directory.
*      2)  then looks in the home directory for "file.cfg".
*      3)  then looks for an environment variable "NCBI" and takes its
*          value as a complete path to a directory containing the
*          configuration file "file.cfg".
*
*****************************************************************************/

static FILE *Nlm_OpenConfigFile (Nlm_CharPtr file, Nlm_Boolean writeMode, Nlm_Boolean create)

{
  FILE      *fp;
  Nlm_Int2  i;
  Nlm_Int2  len;
  FILE      *newfp;
  Nlm_Char  path [PATH_MAX+1];
  char      *pth;
  Nlm_Char  str [FILENAME_MAX+1];

  fp = NULL;

  if (file != NULL) {
    newfp = NULL;
    Nlm_StringNCpy (str, file, sizeof (str) - 3);
    if ( ! Nlm_Qualified (str) ) {
      Nlm_StringCat (str, ".cfg");
    }
    len = (Nlm_Int2) Nlm_StringLen (str);
    for (i = 0; i < len; i++) {
      str [i] = TO_LOWER (str [i]);
    }
    path[0] = '\0';
    Nlm_StringCpy (path, str);

    fp = Nlm_FileOpen (path, "r");  /* File exists? */
    if (fp == NULL) {
      path[0] = '\0';
      if (Nlm_GetHome (path, sizeof (path))) {
        Nlm_FileBuildPath(path, NULL, str);
      } else {
        Nlm_StringCpy (path, str);
      }
      fp = Nlm_FileOpen (path, "r");   /* File exists? */
      if (fp == NULL && create) {
        newfp = Nlm_FileOpen (path, "w");
        Nlm_FileClose (newfp);
        newfp = Nlm_FileOpen (path, "r");
      }
    }

    if (fp == NULL) {
      path[0] = '\0';
      pth = getenv ("NCBI");
      if (pth != NULL) {
        Nlm_FileBuildPath(path, pth, str);
        fp = Nlm_FileOpen (path, "r");
      }
    }

    if (newfp != NULL) {
      if (fp != NULL) {
        Nlm_FileClose (newfp);
        newfp = NULL;
      } else {
        fp = newfp;
      }
    }

    /*
    ** On VMS if a file is opened for write a new version is created.
    ** This section of code check for "writeMode" and an existing file
    ** if both are true.  Get the currently open file's name and delete
    ** it.  Open a new one in write mode.
    **
    ** Side effects: This will replace the highest existing file version,
    ** but not older version.  There exists the possibility that a user's
    ** custom change may get lost.  A possible workaround for this would
    ** be to have the calling program make a new copy (one higher version)
    ** of the existing file before doing extensive write to the params
    ** file OR keep a static flag in this routine which does  delete the
    ** first time time.
    */

    if (writeMode && fp != NULL) {
      char temp[256];
      fgetname(fp,temp);
      Nlm_FileClose (fp);
      delete(temp);
      fp = Nlm_FileOpen (path, "w");
    }
  }
  return fp;
}

#endif /* OS_VMS */

#ifdef OS_DOS
/*****************************************************************************
*
*   Nlm_OpenConfigFile (file, writeMode, create)
*      returns a file pointer to the specified configuration file.
*      1)  looks in the current directory for "file.cfg"
*      2)  then looks for an environment variable "NCBI" and takes its
*           value as a complete path to a directory containing the
*           configuration file "file.cfg".
*
*****************************************************************************/

static FILE *Nlm_OpenConfigFile (Nlm_CharPtr file, Nlm_Boolean writeMode, Nlm_Boolean create)

{
  FILE      *fp;
  Nlm_Int2  i;
  Nlm_Int2  len;
  Nlm_Char  path [PATH_MAX+1];
  char      *pth;
  Nlm_Char  str [FILENAME_MAX+1];

  fp = NULL;
  if (file != NULL) {
    Nlm_StringNCpy (str, file, sizeof (str) - 4);
    if ( ! Nlm_Qualified (str) ) {
      Nlm_StringCat (str, ".cfg");
    }
    len = (Nlm_Int2) Nlm_StringLen (str);
    for (i = 0; i < len; i++) {
      str [i] = TO_LOWER (str [i]);
    }
    path[0] = '\0';
    Nlm_StringCpy (path, str);
    fp = Nlm_FileOpen (path, "r");
    if (fp == NULL) {
      pth = getenv ("NCBI");
      if (pth != NULL) {
        path[0] = '\0';
        Nlm_FileBuildPath (path, pth, str);
        fp = Nlm_FileOpen (path, "r");
        if (fp == NULL && create) {
          fp = Nlm_FileOpen (path, "w");
          Nlm_FileClose (fp);
          fp = Nlm_FileOpen (path, "r");
        }
      } else if (create) {
        fp = Nlm_FileOpen (path, "w");
        Nlm_FileClose (fp);
        fp = Nlm_FileOpen (path, "r");
      } else {
        path[0] = '\0';
      }
    }
    if (writeMode && fp != NULL) {
      Nlm_FileClose (fp);
      fp = Nlm_FileOpen (path, "w");
    }
  }
  return fp;
}
#endif /* OS_DOS */

/*****************************************************************************
*
*   Nlm_TrimString (str)
*      strips trailing spaces, \r, \n
*
*****************************************************************************/

static Nlm_CharPtr Nlm_TrimString (Nlm_CharPtr str)

{
  Nlm_Char     ch;
  Nlm_CharPtr  spc;
  Nlm_CharPtr  tmp;

  if (str != NULL) {
    ch = *str;
    while (ch == ' ' || ch == '\t') {
      str++;
      ch = *str;
    }
    tmp = str;
    spc = NULL;
    ch = *tmp;
    while (ch != '\0' && ch != '\r' && ch != '\n') {
      if (ch == ' ' || ch == '\t') {
        if (spc == NULL) {
          spc = tmp;
        }
      } else {
        spc = NULL;
      }
      tmp++;
      ch = *tmp;
    }
    *tmp = '\0';
    if (spc != NULL) {
      *spc = '\0';
    }
  }
  return str;
}

/*****************************************************************************
*
*   Nlm_ReadConfigFile (fp)
*      reads parameters from configuration file to memory structure
*
*****************************************************************************/

static Nlm_Boolean Nlm_ReadConfigFile (FILE *fp)

{
  Nlm_Char         ch;
  Nlm_env_itemPtr  eip;
  Nlm_env_sectPtr  esp;
  Nlm_env_itemPtr  lastEip;
  Nlm_env_sectPtr  lastEsp;
  Nlm_CharPtr      mid;
  Nlm_Char         str [256];
  Nlm_CharPtr      tmp;
  Nlm_CharPtr      comment;

  if (fp != NULL) {
    Nlm_envList = NULL;
    esp = NULL;
    lastEsp = NULL;
    eip = NULL;
    lastEip = NULL;
    comment = NULL;
    while (fgets (str, sizeof (str), fp)) {
      ch = *str;
      if (ch != '\n' && ch != '\r') {
        if (ch == ';') { /* comment */
          if (comment == NULL) { /* first comment */
             comment = Nlm_StringSave(str);
          }
          else { /* append to existing comment */
             tmp = (Nlm_CharPtr) Nlm_MemNew(StrLen(comment) + StrLen(str) + 1);
             StrCpy(tmp, comment);
             StrCat(tmp, str);
             comment = (Nlm_CharPtr) Nlm_MemFree(comment);
             comment = tmp;
          }
        } else if (ch == '[') {
          if (esp == NULL) {
            esp = (Nlm_env_sectPtr) Nlm_MemNew (sizeof (Nlm_env_sect));
            lastEsp = esp;
            Nlm_envList = esp;
          } else {
            esp = (Nlm_env_sectPtr) Nlm_MemNew (sizeof (Nlm_env_sect));
            lastEsp->next = esp;
            lastEsp = esp;
          }
          esp->comment = comment;
          comment = NULL;
          tmp = str;
          ch = *tmp;
          while (ch != '\0' && ch != ']') {
            tmp++;
            ch = *tmp;
          }
          *tmp = '\0';
          esp->name = Nlm_StringSave (str + 1);
          eip = NULL;
          lastEip = NULL;
        } else if (esp != NULL) {
          if (eip == NULL) {
            eip = (Nlm_env_itemPtr) Nlm_MemNew (sizeof (Nlm_env_item));
            lastEip = eip;
            esp->children = eip;
          } else {
            eip = (Nlm_env_itemPtr) Nlm_MemNew (sizeof (Nlm_env_item));
            lastEip->next = eip;
            lastEip = eip;
          }
          eip->comment = comment;
          comment = NULL;
          tmp = str;
          mid = str;
          ch = *tmp;
          while (ch != '\0' && ch != '\n') {
            if (ch == '=' && mid == str) {
              mid = tmp;
              *mid++ = '\0';
            }
            tmp++;
            ch = *tmp;
          }
          *tmp = '\0';
          eip->name = Nlm_StringSave (Nlm_TrimString (str));
          eip->value = Nlm_StringSave (Nlm_TrimString (mid));
        }
      }
    }

    /* any comments which appeared after the final key of the final section */
    Nlm_bottomComment = comment;
  }
  return TRUE;
}

static Nlm_env_sectPtr Nlm_FindConfigSection (Nlm_CharPtr section)
{
  Nlm_env_sectPtr esp;

  if (section == NULL)
    return NULL;

  for (esp = Nlm_envList; esp != NULL; esp = esp->next)
  {
    if (esp->name != NULL && Nlm_StringICmp(section, esp->name) == 0)
       return esp;
  }

  return NULL;
}

/*****************************************************************************
*
*   Nlm_FindConfigItem (section, type, create)
*      finds parameter in memory structure
*
*****************************************************************************/

static Nlm_env_itemPtr Nlm_FindConfigItem (Nlm_CharPtr section, Nlm_CharPtr type, Nlm_Boolean create)

{
  Nlm_env_itemPtr  eip;
  Nlm_env_sectPtr  esp;
  Nlm_Boolean      goOn;
  Nlm_env_itemPtr  lastEip;
  Nlm_env_sectPtr  lastEsp;

  eip = NULL;
  if (section != NULL && type != NULL) {
    goOn = TRUE;
    esp = Nlm_envList;
    lastEsp = esp;
    while (esp != NULL && goOn) {
      if (esp->name != NULL && Nlm_StringICmp (section, esp->name) == 0) {
        goOn = FALSE;
      } else {
        lastEsp = esp;
        esp = esp->next;
      }
    }
    if (goOn && create) {
      if (Nlm_envList != NULL) {
        esp = (Nlm_env_sectPtr) Nlm_MemNew (sizeof (Nlm_env_sect));
        if (esp != NULL) {
          esp->name = Nlm_StringSave (section);
          lastEsp->next = esp;
        }
      } else {
        esp = (Nlm_env_sectPtr) Nlm_MemNew (sizeof (Nlm_env_sect));
        if (esp != NULL) {
          esp->name = Nlm_StringSave (section);
        }
        Nlm_envList = esp;
      }
    }
    if (esp != NULL) {
      eip = esp->children;
      if (eip != NULL) {
        goOn = TRUE;
        lastEip = eip;
        while (eip != NULL && goOn) {
          if (eip->name != NULL && Nlm_StringICmp (type, eip->name) == 0) {
            goOn = FALSE;
          } else {
            lastEip = eip;
            eip = eip->next;
          }
        }
        if (goOn && create) {
          eip = (Nlm_env_itemPtr) Nlm_MemNew (sizeof (Nlm_env_item));
          if (eip != NULL) {
            eip->name = Nlm_StringSave (type);
            lastEip->next = eip;
          }
        }
      } else if (create) {
        eip = (Nlm_env_itemPtr) Nlm_MemNew (sizeof (Nlm_env_item));
        if (eip != NULL) {
          eip->name = Nlm_StringSave (type);
          esp->children = eip;
        }
      }
    }
  }
  return eip;
}

/*****************************************************************************
*
*   Nlm_WriteConfigFile (fp)
*      writes parameters to configuration file from memory structure
*
*****************************************************************************/

static Nlm_Boolean Nlm_WriteConfigFile (FILE *fp)

{
  Nlm_env_itemPtr  eip;
  Nlm_env_sectPtr  esp;

  if (Nlm_envList != NULL && fp != NULL) {
    esp = Nlm_envList;
    while (esp != NULL) {
      if (! destroyDeadComments || esp->name != NULL)
      {
        Nlm_PutComment (esp->comment, fp);
      }
      if (esp->name != NULL)
      {
        fputc ('[', fp);
        fputs (esp->name, fp);
        fputs ("]\n", fp);
      }
      eip = esp->children;
      while (eip != NULL) {
        if (! destroyDeadComments)
        {
          Nlm_PutComment (eip->comment, fp);
        }
        if (esp->name != NULL && eip->name != NULL && eip->value != NULL) {
          if (destroyDeadComments)
          {
            Nlm_PutComment (eip->comment, fp);
          }
          fputs (eip->name, fp);
          fputc ('=', fp);
          fputs (eip->value, fp);
          fputc ('\n', fp);
        }
        eip = eip->next;
      }
      if (esp->name != NULL)
      {
        fputc ('\n', fp);
      }
      esp = esp->next;
    }
  }

  if (fp != NULL)
    Nlm_PutComment(Nlm_bottomComment, fp);

  return TRUE;
}

/*****************************************************************************
*
*   Nlm_FreeConfigFileData ()
*      frees parameter structure in memory
*
*****************************************************************************/

static void Nlm_FreeConfigFileData (void)

{
  Nlm_bottomComment = (Nlm_CharPtr) Nlm_MemFree(Nlm_bottomComment);
  if (Nlm_lastParamFile != NULL)
    Nlm_lastParamFile = (Nlm_CharPtr) Nlm_MemFree(Nlm_lastParamFile);

  if (Nlm_envList != NULL) {
    Nlm_FreeEnvData (Nlm_envList);
    Nlm_envList = NULL;
  }
}


/*****************************************************************************
*
*   Nlm_FreeConfigStruct ()
*      frees parameter structure in memory, and perform other cleanup
*
*****************************************************************************/

void LIBCALL Nlm_FreeConfigStruct (void)

{
  Nlm_FreeConfigFileData ();
  Nlm_FreeTransientData ();
#ifdef OS_UNIX
  if (saveHome != NULL)
  {
      saveHome = (Nlm_CharPtr) Nlm_MemFree(saveHome);
  }
#endif /* OS_UNIX */
}


/*****************************************************************************
*
*   Nlm_PutComment()
*      output a comment to the config file
*
*****************************************************************************/

static void Nlm_PutComment (Nlm_CharPtr s, FILE *fp)

{
  if (s != NULL)
    fputs(s, fp);
}

#else /* MS-WIN */
/*****************************************************************************
*
* The "guts" of:
*   Nlm_GetAppParam (file, section, type, buf, buflen)
*      finds parameters from configuration files
*      if configuration file is found, trys to read the parameter from it.
*
*****************************************************************************/

static Nlm_Int2 Nlm_WorkGetAppParam (Nlm_CharPtr file, Nlm_CharPtr section, Nlm_CharPtr type, Nlm_CharPtr dflt, Nlm_CharPtr buf, Nlm_Int2 buflen, Nlm_Boolean searchTransient)

{
  Nlm_Char     path [PATH_MAX+1];
#ifdef DCLAP
	Nlm_Int4     totlen;
#endif

  if (buf != NULL && buflen > 0) {
    *buf = '\0';
    if (searchTransient && Nlm_TransientLookup(file, section, type, dflt, buf, buflen))
    {
      return Nlm_StringLen(buf);
    }
    if (dflt) Nlm_StringNCat (buf, dflt, buflen);
    if (file != NULL && *file != '\0' && section != NULL && *section != '\0') {
      Nlm_StringNCpy (path, file, sizeof (path) - 4);
      if ( ! Nlm_Qualified (path) ) {
        Nlm_StringCat (path, ".INI");
      }
#ifdef DCLAP
			totlen=
#endif
      GetPrivateProfileString (section, type, dflt, buf, buflen, path);
    }
  }
#ifdef DCLAP
/* we want same result here as above for type==NULL */
 if (type==NULL) return totlen;
 else
#endif
  return Nlm_StringLen (buf);
}

/*****************************************************************************
*
*   Nlm_SetAppParam (file, section, type, value)
*      finds paths for types of data and fills in path in buf
*      if configuration file is found, trys to write the parameter to it.
*
*****************************************************************************/

Nlm_Boolean LIBCALL  Nlm_SetAppParam (Nlm_CharPtr file, Nlm_CharPtr section, Nlm_CharPtr type, Nlm_CharPtr value)    /* value */

{
  Nlm_Char     path [PATH_MAX+1];
  Nlm_Boolean  rsult;

  rsult = FALSE;
  if (file != NULL && *file != '\0' && section != NULL && *section != '\0') {
    Nlm_StringNCpy (path, file, sizeof (path) - 4);
    if ( ! Nlm_Qualified (path) ) {
      Nlm_StringCat (path, ".INI");
    }
    Nlm_TransientLogSetApp (file, section, type, value);
    if (WritePrivateProfileString (section, type, value, path)) {
      rsult = TRUE;
    }
  }
  return rsult;
}

/*****************************************************************************
*
*   Nlm_FreeConfigStruct ()
*      frees parameter structure in memory
*
*****************************************************************************/

void LIBCALL  Nlm_FreeConfigStruct (void)

{
  Nlm_FreeTransientData ();
}

#endif /* WIN_MSWIN */


static Nlm_Boolean Nlm_Qualified( Nlm_CharPtr path )
{
  Nlm_Int2 l,k;
  Nlm_CharPtr  p;

  l = Nlm_StrLen(path);
  p = path+l;
  k = 0;
  while (k < l && k <= 4) {
     if (*p-- == '.') return TRUE;
     k++;
  }
  return FALSE;
}


/*****************************************************************************
*
*   Nlm_FindPath (file, section, type, buf, buflen)
*      finds paths for types of data from configuration files.
*      if configuration file is found, tries to read the parameter from it,
*      then appends a directory delimiter character, if necessary.
*
*****************************************************************************/

Nlm_Boolean LIBCALL Nlm_FindPath (Nlm_CharPtr file, Nlm_CharPtr section, Nlm_CharPtr type, Nlm_CharPtr buf, Nlm_Int2 buflen)   /* length of path buffer */

{
  Nlm_Boolean  rsult;

  rsult = FALSE;
  if (buf != NULL && buflen > 0) {
    *buf = '\0';
    if (file != NULL && *file != '\0' && section != NULL &&
        *section != '\0' && type != NULL && *type != '\0') {
      if (Nlm_GetAppParam (file, section, type, "", buf, buflen - 1)) {
        if (*buf != '\0') {
            Nlm_FileBuildPath(buf, NULL, NULL);
        }
        rsult = TRUE;
      }
    }
  }
  return rsult;
}

Nlm_Int2 LIBCALL Nlm_GetAppParam (Nlm_CharPtr file, Nlm_CharPtr section, Nlm_CharPtr type, Nlm_CharPtr dflt, Nlm_CharPtr buf, Nlm_Int2 buflen)
{
    return Nlm_WorkGetAppParam(file, section, type, dflt, buf, buflen, TRUE);
}

Nlm_Boolean LIBCALL Nlm_TransientSetAppParam (Nlm_CharPtr file, Nlm_CharPtr section, Nlm_CharPtr type, Nlm_CharPtr value)

{
  Nlm_env_filePtr  theFile;
  Nlm_env_itemPtr  eip;
  Nlm_env_sectPtr  esp;
  Nlm_env_itemPtr  nextEip;

  if (file == NULL || *file == '\0' || section == NULL || *section == '\0')
    return FALSE;

  for (theFile = Nlm_transientFileList; theFile != NULL; theFile = theFile->next)
  {
    if (StringICmp(theFile->name, file) == 0)
    {
      for (esp = theFile->envList; esp != NULL; esp = esp->next)
      {
        if (esp->name != NULL && StringICmp(esp->name, section) == 0)
        {
          if (type == NULL || type[0] == '\0')
          {
            /* free all children */
            for (eip = esp->children; eip != NULL; eip = nextEip)
            {
              nextEip = eip->next;
              Nlm_MemFree (eip->name);
              Nlm_MemFree (eip->comment);
              Nlm_MemFree (eip->value);
              Nlm_MemFree (eip);
            }
            esp->children = NULL;
            esp->transientOnly = TRUE;
          } else { /* append this type to the section */
            eip = (Nlm_env_itemPtr) MemNew(sizeof(*eip));
            eip->name = StringSave(type);
            eip->value = StringSave(value);
            eip->next = esp->children;
            esp->children = eip;
          }
          return TRUE;
        }
      }
      break;
    }
  }

  /* create the file data structure if needed */
  if (theFile == NULL)
  {
    theFile = (Nlm_env_filePtr) MemNew(sizeof(*theFile));
    theFile->name = StringSave(file);
    theFile->next = Nlm_transientFileList;
    Nlm_transientFileList = theFile;
  }

  /* create the section and type */
  esp = (Nlm_env_sectPtr) MemNew(sizeof(*esp));
  esp->name = StringSave(section);
  esp->next = theFile->envList;
  theFile->envList = esp;
  if (type == NULL || type[0] == '\0')
  {
    esp->transientOnly = TRUE;
  } else { /* create the section */
    esp->transientOnly = FALSE;
    eip = (Nlm_env_itemPtr) MemNew(sizeof(*eip));
    eip->name = StringSave(type);
    eip->value = StringSave(value);
    eip->next = NULL;
    esp->children = eip;
  }

  return TRUE;
}

/* SetAppParam is writing a value to the real config file, so log this value, if
   necessary, into the "transient" data structures */
static void Nlm_TransientLogSetApp (Nlm_CharPtr file, Nlm_CharPtr section, Nlm_CharPtr type, Nlm_CharPtr value)

{
  Nlm_env_filePtr  theFile;
  Nlm_env_itemPtr  eip;
  Nlm_env_sectPtr  esp;

  if (file == NULL || *file == '\0' || section == NULL || *section == '\0')
    return;

  if (type == NULL || type[0] == '\0')
  {
    for (theFile = Nlm_transientFileList; theFile != NULL; theFile = theFile->next)
    {
      if (StringICmp(theFile->name, file) == 0)
      {
        for (esp = theFile->envList; esp != NULL; esp = esp->next)
        {
          if (esp->name != NULL && StringICmp(esp->name, section) == 0)
          { /* delete the section by removing section name */
            esp->name = (Nlm_CharPtr) MemFree(esp->name);
          }
        }
      }
    }
  } else {
    for (theFile = Nlm_transientFileList; theFile != NULL; theFile = theFile->next)
    {
      if (StringICmp(theFile->name, file) == 0)
      {
        for (esp = theFile->envList; esp != NULL; esp = esp->next)
        {
          if (esp->name != NULL && StringICmp(esp->name, section) == 0 &&
              esp->transientOnly)
          { /* append this type to the section */
            eip = (Nlm_env_itemPtr) MemNew(sizeof(*eip));
            eip->name = StringSave(type);
            eip->value = StringSave(value);
            eip->next = esp->children;
            esp->children = eip;
          }
        }
      }
    }
  }
}

static Nlm_Boolean Nlm_TransientLookup (Nlm_CharPtr file, Nlm_CharPtr section, Nlm_CharPtr type, Nlm_CharPtr dflt, Nlm_CharPtr buf, Nlm_Int2 buflen)

{
  Nlm_env_filePtr  theFile;
  Nlm_env_itemPtr  eip;
  Nlm_env_sectPtr  esp;
  Nlm_Int2         totlen;
  Nlm_Int2         bytesToAppend;

  if (file == NULL || *file == '\0' || section == NULL || *section == '\0')
    return FALSE;

  for (theFile = Nlm_transientFileList; theFile != NULL; theFile = theFile->next)
  {
    if (StringICmp(theFile->name, file) == 0)
    {
      for (esp = theFile->envList; esp != NULL; esp = esp->next)
      {
        if (esp->name != NULL && StringICmp(esp->name, section) == 0)
        {
          if (type == NULL || type[0] == '\0')
          { /* concatenate all types (keys) within section */
            *buf = '\0';
            totlen = 0;
            for (eip = esp->children; eip != NULL; eip = eip->next)
            {
              bytesToAppend = StrLen(eip->name) + 1;
              bytesToAppend = MIN(bytesToAppend, buflen - totlen);
              StrNCpy(&buf[totlen], eip->name, bytesToAppend);
              totlen += bytesToAppend;
            }
            if (totlen > 0 && buf[totlen] == '\0')
            {
                totlen--; /* account for final null character */
            }
            /* now append the GetAppParam() data */
            if (! esp->transientOnly)
            { /* GetAppParam data can be trusted ... append it to buf */
              Nlm_WorkGetAppParam(file, section, NULL, "", &buf[totlen],
                                  buflen - totlen, FALSE);
            }
            return TRUE;
          } else {
            for (eip = esp->children; eip != NULL; eip = eip->next)
            {
              if (StringICmp(eip->name, type) == 0)
              {
                *buf = '\0';
                Nlm_StringNCat (buf, eip->value, buflen);
                return TRUE;
              }
            }
            if (esp->transientOnly)
            { /* GetAppParam data cannot be trusted ... use the default */
              *buf = '\0';
              if (dflt) Nlm_StringNCat (buf, dflt, buflen);
              return TRUE;
            }
          }
        }
      }
    }
  }

  /* not found ... GetAppParam() should search the real config file */
  return FALSE;
}

static void Nlm_FreeEnvData (Nlm_env_sectPtr esp)

{
  Nlm_env_itemPtr  eip;
  Nlm_env_itemPtr  nextEip;
  Nlm_env_sectPtr  nextEsp;

  while (esp != NULL) {
    nextEsp = esp->next;
    eip = esp->children;
    while (eip != NULL) {
      nextEip = eip->next;
      Nlm_MemFree (eip->name);
      Nlm_MemFree (eip->comment);
      Nlm_MemFree (eip->value);
      Nlm_MemFree (eip);
      eip = nextEip;
    }
    Nlm_MemFree (esp->name);
    Nlm_MemFree (esp->comment);
    Nlm_MemFree (esp);
    esp = nextEsp;
  }
}


static void Nlm_FreeTransientData (void)
{
  Nlm_env_filePtr efp, nextEfp;

  efp = Nlm_transientFileList;
  while (efp != NULL) {
    nextEfp = efp->next;
    Nlm_FreeEnvData (efp->envList);
    Nlm_MemFree (efp->name);
    Nlm_MemFree (efp);
    efp = nextEfp;
  }
  Nlm_transientFileList = NULL;
}
