/****************************************************************
*	NCSA Telnet for the Macintosh								*
*																*
*	National Center for Supercomputing Applications				*
*	Software Development Group									*
*	152 Computing Applications Building							*
*	605 E. Springfield Ave.										*
*	Champaign, IL  61820										*
*																*
*	Copyright (c) 1986-1994,									*
*	Board of Trustees of the University of Illinois				*
****************************************************************/

#ifdef	MPW
#pragma load ":obj:sys.dump"
#endif

#define	NCSA_ENC

// Think C has dangerous pattern always on, with no easy way to 
//	disable it.  So, I've isolated the problem to this macro so
//	it can be changed when Think C supports the Universal Headers.
//	--JMB 2/94
#ifdef	THINK_C
#define	PATTERN(x) x	// Hosed Pattern definition
#define NO_UNIVERSAL	// No universal headers
#define	M68K			// Compiling for 68K
#endif

#ifdef	MPW
#define PATTERN(x) &x	// Good Pattern definition
#define	NO_UNIVERSAL	// I haven't installed the Uni headers for my MPW yet
#define	M68K			// Compiling for 68K
#endif

#ifdef	__MWERKS__
#define PATTERN(x) &x	// Good Pattern definition
#endif

#if defined(powerc) || defined(__powerc)
#define __powerpc__
#endif

// Using settings of NO_UNIVERSAL and __powerpc__, set up our SIMPLE_UPP macro

#ifdef	NO_UNIVERSAL
#define	SIMPLE_UPP(routine,what)		\
ProcPtr routine##UPP = (ProcPtr)&routine

#define	PROTO_UPP(routine, what)			\
extern ProcPtr routine##UPP

#else	// ifdef NO_UNIVERSAL

#define PROTO_UPP(routine, what)		\
extern what##UPP routine##UPP;

// Have universal headers, compiling PPC
#ifdef	__powerpc__
#define SIMPLE_UPP(routine,what)        \
RoutineDescriptor routine##RD =	\
BUILD_ROUTINE_DESCRIPTOR(upp##what##ProcInfo,routine);\
what##UPP routine##UPP=(what##UPP)&routine##RD	\

#else	// ifdef __powerpc__

// Have universal headers, compiling 68K
#define	SIMPLE_UPP(routine,what)		\
what##UPP routine##UPP=(what##UPP)&routine

#endif	// ifdef __powerpc__
#endif	// ifdef NO_UNIVERSAL

#include "typedefs.h"
#include "globaldefs.h"
#include "resrcdefs.h"
#include <MacTCPCommonTypes.h>
#include "Preferences.h"