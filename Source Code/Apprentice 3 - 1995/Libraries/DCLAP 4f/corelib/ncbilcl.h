/*   ncbilcl.h
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
* File Name:  ncbilcl.h
*
* Author:  Gish, Kans, Ostell, Schuler
*
* Version Creation Date:   1/1/91
*
* $Revision: 2.7 $
*
* File Description:
*		system dependent header
*		MS Windows version for CodeWarrior C++
*
* Modifications:
* --------------------------------------------------------------------------
* Date     Name        Description of modification
* -------  ----------  -----------------------------------------------------
* May 95   d.gilbert   codewarrior c++
* ==========================================================================
*/
#ifndef _NCBILCL_
#define _NCBILCL_

/* PLATFORM DEFINITION FOR IBM PC UNDER MS WINDOWS WITH CodeWarrior C++ */

#define COMP_BOR
#define COMP_CWI
#define WIN_MSWIN
#define WIN32CW

// the following definitions were added for WindowsNT
#ifdef WIN32CW

#define OS_DOS
#define PROC_I80X86
#define PASCAL	__stdcall

#undef NULL
#define NULL  0 /* ((void*)0) */

#undef FAR
#define FAR

#undef NEAR
#define NEAR

#undef CDECL
#define CDECL	__cdecl


#else

#ifdef WIN32
#define OS_WINNT
#define NEAR
#ifdef i386
#define PROC_I80X86
#define PROC_I80_386
#define PASCAL	__stdcall
#define CDECL	__cdecl
#endif
#ifdef MIPS
#define PROC_MIPS
#define PASCAL
#define CDECL
#endif
#ifdef ALPHA
#define PROC_ALPHA
#define PASCAL
#define CDECL
#endif

#else
#if defined(WIN32BOR)

/* dgg hack */

/*#define WIN16 << bad for ncbimem memory routines*/

#define OS_DOS

#define PROC_I80X86

#define FAR

#define NEAR

#define PASCAL	__stdcall

#define CDECL	__cdecl

#else
#define WIN16
#define OS_DOS
#define PROC_I80X86
#define FAR		__far
#define NEAR	__near
#define PASCAL	__pascal
#define CDECL	__cdecl
#define EXPORT  __export
#endif
#endif
#endif

/*----------------------------------------------------------------------*/
/*      Desired or available feature list                               */
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/*      #includes                                                       */
/*----------------------------------------------------------------------*/
#define __MSC__

#include <stddef.h>
#include <types.h> /*<sys\types.h>*/
#include <limits.h>
#include <stat.h> /*<sys\stat.h> */
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <float.h>

/*----------------------------------------------------------------------*/
/*      Missing ANSI-isms                                               */
/*----------------------------------------------------------------------*/
#ifndef FILENAME_MAX
#define FILENAME_MAX 63
#endif

/*----------------------------------------------------------------------*/
/*      Aliased Logicals, Datatypes                                     */
/*----------------------------------------------------------------------*/
#define PNTR * /* FAR * */
#define HNDL * /* NEAR * */

typedef int (FAR PASCAL *Nlm_FnPtr)();
#define FnPtr           Nlm_FnPtr

/*----------------------------------------------------------------------*/
/*      Misc Macros                                                     */
/*----------------------------------------------------------------------*/
#define PROTO(x)	x	/* Function prototypes are real */
#define VPROTO(x)	x	/* Prototype for variable argument list */
#define DIRDELIMCHR	'\\'
#define DIRDELIMSTR	"\\"
#define CWDSTR	"."

#define KBYTE	(1024)
#define MBYTE	(1048576L)

#define IS_LITTLE_ENDIAN
#define TEMPNAM_AVAIL

/*----------------------------------------------------------------------*/
/*      For importing Microsoft C code                                  */
/*----------------------------------------------------------------------*/
#define _near near
#define _far far
#define _huge huge
#define _cdecl cdecl
#define _pascal pascal
typedef unsigned int UINT;
typedef unsigned long ULONG;
typedef unsigned short USHORT;
typedef unsigned char UCHAR;

/*----------------------------------------------------------------------*/
/*      Macros for Floating Point                                       */
/*----------------------------------------------------------------------*/
#define EXP2(x) exp((x)*LN2)
#define LOG2(x) (log(x)*(1./LN2))
#define EXP10(x) exp((x)*LN10)
#define LOG10(x) (log10(x))

/*----------------------------------------------------------------------*/
/*      Macros Defining Limits                                          */
/*----------------------------------------------------------------------*/
#if defined(WIN32) || defined(WIN32BOR)
#define MAXALLOC	0x7F000000 /* Largest permissible memory request */
#else
#define MAXALLOC	0x10000 /* Largest permissible memory request */
#endif

#endif
