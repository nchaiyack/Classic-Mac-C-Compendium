/*   ncbifile.h
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
* File Name:  ncbifile.h
*
* Author:  Gish, Kans, Ostell, Schuler
*
* Version Creation Date:   1/1/91
*
* $Revision: 2.10 $
*
* File Description: 
*   	prototypes for portable file routines
*
* Modifications:  
* --------------------------------------------------------------------------
* Date     Name        Description of modification
* -------  ----------  -----------------------------------------------------
* 04-15-93 Schuler     Changed _cdecl to LIBCALL
*
*
* ==========================================================================
*/

#ifndef _NCBIFILE_
#define _NCBIFILE_

#ifdef __cplusplus
extern "C" {
#endif

typedef FILE * (LIBCALLBACK *Nlm_FileOpenHook) PROTO((const char *filename, const char *mode));

FILE * LIBCALL Nlm_FileOpen PROTO((const char *filename, const char *mode));
void LIBCALL Nlm_FileClose PROTO((FILE *stream));
Nlm_sizeT LIBCALL Nlm_FileRead PROTO((void *ptr, Nlm_sizeT size, Nlm_sizeT n, FILE *stream));
Nlm_sizeT LIBCALL Nlm_FileWrite PROTO((const void *ptr, Nlm_sizeT size, Nlm_sizeT n, FILE *stream));
int LIBCALL Nlm_FilePuts PROTO((const char *ptr, FILE * fp));
char * LIBCALL Nlm_FileGets PROTO((char *ptr, Nlm_sizeT size, FILE * fp));
Nlm_CharPtr LIBCALL Nlm_FileBuildPath PROTO((Nlm_CharPtr root, Nlm_CharPtr sub_path, Nlm_CharPtr filename));
Nlm_CharPtr LIBCALL Nlm_FileNameFind PROTO((Nlm_CharPtr pathname));
Nlm_Int4 LIBCALL Nlm_FileLength PROTO((Nlm_CharPtr fileName));
Nlm_Boolean LIBCALL Nlm_FileRemove PROTO((Nlm_CharPtr fileName));
Nlm_Boolean LIBCALL Nlm_FileRename PROTO((Nlm_CharPtr oldFileName, Nlm_CharPtr newFileName));
void LIBCALL Nlm_FileCreate PROTO((Nlm_CharPtr fileName, Nlm_CharPtr type, Nlm_CharPtr creator));
Nlm_Boolean LIBCALL Nlm_CreateDir PROTO((Nlm_CharPtr pathname));
Nlm_CharPtr LIBCALL Nlm_TmpNam PROTO((Nlm_CharPtr s));
Nlm_Boolean LIBCALL Nlm_EjectCd PROTO((Nlm_CharPtr sVolume, Nlm_CharPtr deviceName, Nlm_CharPtr rawDeviceName, Nlm_CharPtr mountPoint, Nlm_CharPtr mountCmd));
Nlm_Boolean LIBCALL Nlm_MountCd PROTO((Nlm_CharPtr sVolume, Nlm_CharPtr deviceName, Nlm_CharPtr mountPoint, Nlm_CharPtr mountCmd));
void LIBCALL Nlm_SetFileOpenHook PROTO((Nlm_FileOpenHook hook));

#define FileOpen Nlm_FileOpen
#define FileClose Nlm_FileClose
#define FileRead Nlm_FileRead
#define FileWrite Nlm_FileWrite
#define FilePuts Nlm_FilePuts
#define FileGets Nlm_FileGets
#define FileBuildPath Nlm_FileBuildPath
#define FileNameFind Nlm_FileNameFind
#define FileLength Nlm_FileLength
#define FileRemove Nlm_FileRemove
#define FileRename Nlm_FileRename
#define FileCreate Nlm_FileCreate
#define CreateDir Nlm_CreateDir
#define TmpNam Nlm_TmpNam
#define EjectCd Nlm_EjectCd
#define MountCd Nlm_MountCd

#ifdef __cplusplus
}
#endif

#endif
