/*  ncbibs.h
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
* File Name:  ncbibs.h
*
* Author:  Jim Ostell
*
* Version Creation Date:  1/1/91
*
* $Revision: 2.3 $
*
* File Description:
*   ByteStore typedefs, prototypes, and defines
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

#ifndef _NCBIBS_
#define _NCBIBS_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct bsunit {             /* for building multiline strings */
	Nlm_Handle str;            /* the string piece */
	Nlm_Int2 len_avail,
		 len;
	struct bsunit PNTR next; }       /* the next one */
Nlm_BSUnit, PNTR Nlm_BSUnitPtr;

typedef struct bytestore {
	Nlm_Int4 seekptr,       /* current position */
		totlen,             /* total stored data length in bytes */
		chain_offset;       /* offset in ByteStore of first byte in curchain */
	Nlm_BSUnitPtr chain,       /* chain of elements */
		curchain;           /* the BSUnit containing seekptr */
} Nlm_ByteStore, PNTR Nlm_ByteStorePtr;

Nlm_VoidPtr LIBCALL Nlm_BSMerge PROTO((Nlm_ByteStorePtr ssp, Nlm_VoidPtr dest));
Nlm_ByteStorePtr LIBCALL Nlm_BSNew PROTO((Nlm_Int4 len));
Nlm_Int2 LIBCALL Nlm_BSSeek PROTO((Nlm_ByteStorePtr bsp, Nlm_Int4 offset, Nlm_Int2 origin));
Nlm_Int4 LIBCALL Nlm_BSTell PROTO((Nlm_ByteStorePtr bsp));
Nlm_Int4 LIBCALL Nlm_BSDelete PROTO((Nlm_ByteStorePtr bsp, Nlm_Int4 len));
Nlm_Int4 LIBCALL Nlm_BSWrite PROTO((Nlm_ByteStorePtr bsp, Nlm_VoidPtr ptr, Nlm_Int4 len));
Nlm_Int4 LIBCALL Nlm_BSInsert PROTO((Nlm_ByteStorePtr bsp, Nlm_VoidPtr ptr, Nlm_Int4 len));
Nlm_Int4 LIBCALL Nlm_BSInsertFromBS PROTO((Nlm_ByteStorePtr bsp, Nlm_ByteStorePtr bsp2, Nlm_Int4 len));
Nlm_Int4 LIBCALL Nlm_BSRead PROTO((Nlm_ByteStorePtr bsp, Nlm_VoidPtr ptr, Nlm_Int4 len));
Nlm_Int2 LIBCALL Nlm_BSGetByte PROTO((Nlm_ByteStorePtr bsp));
Nlm_Int2 LIBCALL Nlm_BSPutByte PROTO((Nlm_ByteStorePtr bsp, Nlm_Int2 value));
Nlm_Int4 LIBCALL Nlm_BSAdd PROTO((Nlm_ByteStorePtr ssp, Nlm_Int4 len));
Nlm_Int4 LIBCALL Nlm_BSLen PROTO((Nlm_ByteStorePtr ssp));
Nlm_ByteStorePtr LIBCALL Nlm_BSFree PROTO((Nlm_ByteStorePtr ssp));
Nlm_ByteStorePtr LIBCALL Nlm_BSDup PROTO((Nlm_ByteStorePtr source));

#define ByteStore Nlm_ByteStore
#define ByteStorePtr Nlm_ByteStorePtr
#define BSMerge Nlm_BSMerge
#define BSNew Nlm_BSNew
#define BSLen Nlm_BSLen
#define BSFree Nlm_BSFree
#define BSSeek Nlm_BSSeek
#define BSTell Nlm_BSTell
#define BSDelete Nlm_BSDelete
#define BSWrite Nlm_BSWrite
#define BSInsert Nlm_BSInsert
#define BSInsertFromBS Nlm_BSInsertFromBS
#define BSDup Nlm_BSDup
#define BSRead Nlm_BSRead
#define BSGetByte Nlm_BSGetByte
#define BSPutByte Nlm_BSPutByte

#ifdef __cplusplus
}
#endif

#endif
