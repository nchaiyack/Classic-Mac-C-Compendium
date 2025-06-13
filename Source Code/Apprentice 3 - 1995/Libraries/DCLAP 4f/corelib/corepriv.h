/*   corepriv.h
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
* File Name:  ncbi.h
*
* Author:  Gish, Kans, Ostell, Schuler
*
* Version Creation Date:   02/23/94
*
* $Revision: 2.0 $
*
* File Description:  Privated definitions used internally by CoreLib
*
* Modifications:  
* --------------------------------------------------------------------------
* Date     Name        Description of modification
* -------  ----------  -----------------------------------------------------
*
*
* ==========================================================================
*/

#ifndef __COREPRIV_H__
#define __COREPRIV_H__

extern char * g_corelib_module;
#define THIS_MODULE g_corelib_module

#include <ncbi.h>
#include <ncbiwin.h>

/*
		Not much here yet.  I will start adding things once all the makefiles
		have been modified to copy this file to the build directory (not the
		include directory).
*/

#endif

