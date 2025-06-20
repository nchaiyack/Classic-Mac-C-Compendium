/*
 * Icon configuration.
 */

/*
 * System-specific definitions are in define.h
 */

/*
 *  A number of symbols are defined here.  Some are specific to individual
 *  to operating systems.  Examples are:
 *
 *	MSDOS		MS-DOS for PCs
 *	UNIX		any UNIX system
 *	VMS		VMS for the VAX
 *
 *  These are defined to be 1 or 0 depending on which operating system
 *  the installation is being done under.  They are all defined and only
 *  one is defined to be 1.  (They are used in the form #if VAX || MSDOS.)
 *
 *  There also are definitions of symbols for specific computers and
 *  versions of operating systems.  These include:
 *
 *	SUN		code specific to the Sun Workstation
 *	MICROSOFT	code specific to the Microsoft C compiler for MS-DOS
 *
 *  Other definitions may occur for different configurations. These include:
 *
 *	DeBug		debugging code
 *	MultiThread	support for multiple programs under the interpreter
 *
 *  Other definitions perform configurations that are common to several
 *  systems. An example is:
 *
 *	Double		align reals at double-word boundaries
 *
 */

/*
 * If COMPILER is not defined, code for the interpreter is compiled.
 */

#ifndef COMPILER
#define COMPILER 0
#endif

/*
 * The following definitions insure that all the symbols for operating
 * systems that are not relevant are defined to be 0 -- so that they
 * can be used in logical expressions in #if directives.
 */

#ifndef PORT
#define PORT 0
#endif					/* PORT */

#ifndef AMIGA
#define AMIGA 0
#endif					/* AMIGA */

#ifndef ARM
#define ARM 0
#endif					/* ARM */

#ifndef ATARI_ST
#define ATARI_ST 0
#endif					/* ATARI_ST */

#ifndef MACINTOSH
#define MACINTOSH 0
#endif					/* MACINTOSH */

#ifndef MSDOS
#define MSDOS 0
#endif					/* MSDOS */

#ifndef MVS
#define MVS 0
#endif					/* MVS */

#ifndef OS2
#define OS2 0
#endif					/* OS2 */

#ifndef OS2_32
#define OS2_32 0
#endif					/* OS32 */

#ifndef UNIX
#define UNIX 0
#endif					/* UNIX */

#ifndef VM
#define VM 0
#endif					/* VM */

#ifndef VMS
#define VMS 0
#endif					/* VMS */


/*
 * The following definitions serve to cast common conditionals is
 *  a positive way, while allowing defaults for the cases that
 *  occur most frequently.  That is, if co-expressions are not supported,
 *  NoCoexpr is defined in define.h, but if they are supported, no
 *  definition is needed in define.h; nonetheless subsequent conditionals
 *  can be cast as #ifdef Coexpr.
 */

/*
 *  The mysterious Garden of Eden; not supported under the compiler.
 */

#ifndef NoCoexpr
#undef Coexpr
#define Coexpr
#endif					/* NoCoexpr */

#ifdef NoCoexpr
#undef MultiThread
#undef EventMon
#undef Eve
#endif					/* NoCoexpr */

#if COMPILER
#undef Eve
#undef MultiThread
#undef Visualization
#undef EventMon
#endif					/* COMPILER */

#ifdef Eve
#undef EventMon
#undef MultiThread
#undef MultiRegion
#undef Visualization
#define EventMon
#define MultiThread
#define MultiRegion
#define Visualization
#endif					/* Eve */

#ifndef NoEnvVars
#undef EnvVars
#define EnvVars
#endif					/* NoEnvVars */

#ifndef NoStrInvoke
#undef StrInvoke
#define StrInvoke
#endif					/* NoStrInvoke */

#ifndef NoLargeInts
#undef LargeInts
#define LargeInts
#endif					/* NoLargeInts */

#ifdef EventMon
#undef EventProc
#undef MemMon
#undef MultiThread
#define MemMon
#define EventProc
#define MultiThread
#define InterpMon
#endif					/* EventMon */

#ifndef NoInterpMon
#undef InterpMon
#define InterpMon
#endif					/* InterpMon */

#ifndef ExpandableRegions
#undef FixedRegions
#define FixedRegions
#undef IconAlloc
#ifndef NoMultiRegion
#undef MultiRegion
#define MultiRegion
#endif					/* NoMultiRegion */
#endif					/* ExpandableRegions */

/*
 *  Did you understand all that?  Well, get a load of this...
 */

#ifndef NoEventProc
#undef EventProc
#define EventProc
#endif					/* EventProc */

#ifdef MultiThread
#undef MultiRegion
#define MultiRegion
#endif					/* MultiThread */

#ifdef MultiRegion
#ifdef ExpandableRegions
deliberate syntax error
#else					/* NoFixedRegions */
#undef FixedRegions
#define FixedRegions
#endif					/* NoFixedRegions */
#endif					/* MultiRegion */

#ifndef RefPath
#define RefPath BinPath
#endif					/* RefPath */

#if UNIX
#ifndef NoShellHeader
#undef ShellHeader
#define ShellHeader
#endif					/* NoShellHeader */
#endif					/* UNIX */

/*
 * Names for standard environment variables, if they are supported.
 * The standard names are used unless they are overridden.
 */

#ifdef EnvVars
 
#ifndef MEMMON
#define MEMMON "MEMMON"
#endif

#ifndef NOERRBUF
#define NOERRBUF "NOERRBUF"
#endif

#ifndef TRACE
#define TRACE "TRACE"
#endif

#ifndef COEXPSIZE
#define COEXPSIZE "COEXPSIZE"
#endif

#ifndef STRSIZE
#define STRSIZE "STRSIZE"
#endif

#ifndef HEAPSIZE
#define HEAPSIZE "HEAPSIZE"
#endif

#ifndef BLOCKSIZE
#define BLOCKSIZE "BLOCKSIZE"
#endif

#ifndef BLKSIZE
#define BLKSIZE "BLKSIZE"
#endif

#ifndef STATSIZE
#define STATSIZE "STATSIZE"
#endif

#ifndef STATINCR
#define STATINCR "STATINCR"
#endif

#ifndef MSTKSIZE
#define MSTKSIZE "MSTKSIZE"
#endif

#ifndef QLSIZE
#define QLSIZE "QLSIZE"
#endif

#ifndef ICONCORE
#define ICONCORE "ICONCORE"
#endif

#ifndef IPATH
#define IPATH "IPATH"
#endif

#endif					/* EnvVars */

#ifdef MSWindows
#undef Graphics
#define Graphics 1
#endif					/* MSWindows */
#ifdef MacGraph
#undef Graphics
#define Graphics 1
#endif					/* MacGraph */

#ifdef Graphics
#ifndef NoXpmFormat
#if UNIX
#undef HaveXpmFormat
#define HaveXpmFormat
#endif					/* UNIX */
#endif					/* NoXpmFormat */
#ifndef MSWindows
#ifndef PresentationManager
#ifndef MacGraph
#undef XWindows
#define XWindows 1
#endif					/* MacGraph */
#endif					/* PresentationManager */
#endif					/* MSWindows */
#undef LineCodes
#define LineCodes
#undef Polling
#define Polling
#ifndef NoIconify
#define Iconify
#endif					/* NoIconify */
#ifndef MaxStatSize
/*
 * Bump up MaxStatSize for X programs; they're stat-hungry
 */
#define MaxStatSize 100000
#endif					/* MaxStatSize */
#ifndef ICONC_XLIB
#define ICONC_XLIB "-lX11"
#endif					/* ICONC_XLIB */
#endif					/* Graphics */

#ifndef NoExternalFunctions
#undef ExternalFunctions
#define ExternalFunctions
#endif					/* NoExternalFunctions */

/*
 * EBCDIC == 0 corresponds to ASCII.  EBCDIC == 1 corresponds to EBCDIC
 *  collating sequence, while EBCDIC == 2 provides the ASCII collating
 *  sequence for EBCDIC systems.
 */
#ifndef EBCDIC
#define EBCDIC 0
#endif					/* EBCDIC */

/*
 * Other defaults.
 */

#ifdef DeBug
#undef DeBugTrans
#undef DeBugLinker
#undef DeBugIconx
#define DeBugTrans
#define DeBugLinker
#define DeBugIconx
#endif					/* DeBug */

#ifndef AllocType
#define AllocType unsigned int
#endif					/* AllocType */

#ifndef ErrorExit
#define ErrorExit 1
#endif					/* ErrorExit */

#ifndef NormalExit
#define NormalExit 0
#endif					/* NormalExit */

#ifndef Hz
#define Hz 60
#endif					/* Hz */

#ifndef MaxHdr
#define MaxHdr 4096
#endif					/* MaxHdr */

#ifndef MaxPath
#define MaxPath 200
#endif					/* MaxPath */

#ifndef StackAlign
#define StackAlign 2
#endif					/* StackAlign */

#ifndef SysTime
#define SysTime <time.h>
#endif					/* SysTime */

#ifndef WordBits
#define WordBits 32
#endif					/* WordBits */

#ifndef IntBits
#define IntBits WordBits
#endif					/* IntBits */

#ifndef SourceSuffix
#define SourceSuffix ".icn"
#endif					/* SourceSuffix */

/*
 * Representations of directories. LocalDir is the "current working directory".
 *  SourceDir is where the source file is. 
 */

#define LocalDir ""
#define SourceDir (char *)NULL

#ifndef TargetDir
#define TargetDir LocalDir
#endif					/* TargetDir */


#ifndef Pipes
#if ARM || OS2 || UNIX || VMS
#define Pipes
#endif					/* ARM || OS2 || UNIX || VMS */
#endif					/* Pipes */

#if UNIX || VMS
#define SystemFnc
#endif					/* UNIX || VMS */


/*
 * Default sizing and such.
 */

#define WordSize sizeof(word)

#ifndef ByteBits
#define ByteBits 8
#endif					/* ByteBits */

/*
 * Define the size of the units in MemMon (allocation history) files.
 */
#ifdef MemMon
#ifndef MMUnits
#define MMUnits WordSize
#endif					/* MMUnits */
#endif					/* MemMon */

#ifdef EventMon
#undef MMUnits
#define MMUnits 1
#endif					/* EventMon */

/*
 * Change the name of gcvt() if we're supplying our own version,
 * to avoid complaints under VMS and others with shared libraries.
 */

#ifndef NoIconGcvt
#undef IconGcvt
#define IconGcvt
#define gcvt icon_gcvt
#endif					/* IconGcvt */

/*
 *  The following definitions depend on whether or not the ANSI C standard
 *  is supported.
 */

#ifdef Standard
#undef StandardC
#undef StandardPP
#undef StandardLib
#define StandardC
#define StandardPP
#define StandardLib
#endif					/* Standard */

#ifdef StandardPP
#define Cat(x,y) x##y
#define Lit(x) #x
#else					/* StandardPP */
#define Ident(x) x
#define Cat(x,y) Ident(x)y
#define Lit(x) "x"
#endif					/* StandardPP */

#ifdef StandardC
#undef Prototypes
#define Prototypes
#undef VoidType
#define VoidType
#define Bell '\a'
#else					/* StandardC */
#if EBCDIC
#define Bell '\x2F'
#else					/* EBCDIC */
#define Bell '\007'
#endif					/* EBCDIC */
#endif					/* StandardC */

#ifdef StandardLib
#undef SysMem
#define SysMem
#endif					/* StandardLib */

#if IntBits != WordBits
#undef SysMem
#endif					/* Intbits != WordBits */

#ifdef SysMem
#undef memcopy
#undef memfill
#define memcopy(to,from,len) memcpy(to,from,len)
#define memfill(to,from,len) memset(to,from,len)
#endif					/* SysMem */

#ifdef SysOpt
#define optindex optind
#endif					/* SysOpt */

/*
 * Provide definition to use void if it's supported (mainly avoids error
 *  messages with some C compilers.  Note: typedef does not work, so care
 *  is needed not to use compound declarations.
 */

#ifdef VoidType
#define novalue void
#define noargs void
#else					/* VoidType */
#define novalue int
#define noargs
#endif					/* VoidType */

/*
 * And something to handle a cast problem for signal().
 */

#ifndef SigFncCast
#ifdef VoidType
#define SigFncCast (void (*)(int))
#else					/* VoidType */
#define SigFncCast 
#endif					/* VoidType */
#endif					/* SigFncCast */

/*
 * Customize output if not pre-defined.
 */

#ifndef TraceOut
#define TraceOut(s) fprintf(stderr,s)
#endif					/* TraceOut */

#if EBCDIC
#define BackSlash "\xe0"
#else					/* EBCDIC */
#define BackSlash "\\"
#endif					/* EBCDIC */

#ifndef WriteBinary
#define WriteBinary "wb"
#endif					/* WriteBinary */

#ifndef ReadBinary
#define ReadBinary "rb"
#endif					/* ReadBinary */

#ifndef ReadEndBinary
#define ReadEndBinary "r+b"
#endif					/* ReadEndBinary */

#ifndef WriteText
#define WriteText "w"
#endif					/* WriteText */

#ifndef ReadText
#define ReadText "r"
#endif					/* ReadText */

/*
 * "hidden" is used to declare static functions;  "hidden" normally == "static",
 *  except with compilers that can't handle "static" in prototypes.
 */

#ifndef hidden
#define hidden static
#endif					/* hidden */

/*
 * The "Params" macro allows a single declaration to be used with both old- and
 *  new-style compilers.
 */

#ifdef Prototypes
#define Params(a) a
#else					/* Prototypes */
#define Params(a) ()
#endif					/* Prototypes */

/*
 * The following code is operating-system dependent [@config.01].
 *  Any configuration stuff that has to be done at this point.
 */

#if PORT
   /* Probably nothing is needed. */
Deliberate Syntax Error
#endif					/* PORT */

#if VMS
#define ExecSuffix ".exe"
#define ObjSuffix ".obj"
#define LibSuffix ".olb"
#endif					/* VMS */

#if AMIGA || ARM || ATARI_ST || MACINTOSH || MVS || VM
#endif					/* AMIGA || ARM || ... */

#if MSDOS || OS2

/*
 *  Define compiler-specific symbols to be zero if not already
 *  defined.
 */

#ifndef MICROSOFT
#define MICROSOFT 0
#endif					/* MICROSOFT */

#ifndef CSET2
#define CSET2 0
#endif                                  /* CSet/2 */

#ifndef CSET2V2
#define CSET2V2 0
#endif					/* CSet/2 version 2 */

#ifndef TURBO
#define TURBO 0
#endif					/* TURBO */

#ifndef HIGHC_386
#define HIGHC_386 0
#endif					/* HIGHC_386 */

#ifndef INTEL_386
#define INTEL_386 0
#endif					/* INTEL_386 */

#ifndef WATCOM
#define WATCOM 0
#endif					/* WATCOM */

#ifndef ZTC_386
#define ZTC_386 0
#endif					/* ZTC_386 */

#ifndef MSNT
#define MSNT 0
#endif					/* MSNT */

#if HIGHC_386
/*
 * MetaWare's HighC 386 macro putc doesn't handle putc('\n') correctly -
 * sometimes a CR is not written out before the LF.  So, redefine
 * macro putc to actually issue an fputc.
 */
#undef putc
#define putc(c,f) fputc(c,f)
#endif					/* HIGHC_386 */
#endif					/* MSDOS || OS2 */


#if MACINTOSH
#if LSC
/*
 * LightSpeed C requires that #define tokens appear after prototypes
 */
#define index strchr
#define malloc mlalloc
#define rindex strrchr
#define unlink remove
#endif					/* LSC */
#endif					/* MACINTOSH */

#if MVS || VM
 
/*
 *  Define compiler-specific symbols to be zero if not already
 *  defined.
 */
 
#ifndef SASC
#define SASC 0
#endif                                  /* SASC */
 
#ifndef WATERLOO_C_V3_0
#define WATERLOO_C_V3_0 0
#endif                                  /* WATERLOO_C_V3_0 */
#endif                                  /* MVS || VM */

#if UNIX || (MACINTOSH && MPW)

#undef WriteBinary
#define WriteBinary "w"

#undef ReadBinary
#define ReadBinary "r"

#undef ReadEndBinary
#define ReadEndBinary "r+"

#ifndef NoHardWiredPaths
#define HardWiredPaths
#endif					/* NoHardWiredPaths */

#ifndef NoHeader
#undef Header
#define Header
#endif					/* NoHeader */

#endif					/* UNIX || (MACINTOSH && MPW) */

/*
 * End of operating-system specific code.
 */


#ifndef DiffPtrs
#define DiffPtrs(p1,p2) (word)((p1)-(p2))
#endif					/* DiffPtrs */

#ifndef AllocReg
#define AllocReg(n) malloc((msize)n)
#endif					/* AllocReg */

#define MaxFileName 256

#ifndef EVENTMON
#define EVENTMON "EVENTMON"
#endif					/* EVENTMON */

#ifndef MEMMON
#define MEMMON "MEMMON"
#endif					/* MEMMON */

#ifndef RttSuffix
#define RttSuffix ".r"
#endif					/* RttSuffix */

#ifndef DBSuffix
#define DBSuffix ".db"
#endif					/* DBSuffix */

#ifndef PPInit
#define PPInit ""
#endif					/* PPInit */

#ifndef PPDirectives
#define PPDirectives {"passthru", PpKeep},
#endif					/* PPDirectives */

#ifndef NoRanlib
#define Ranlib
#endif					/* NoRanlib */

#ifndef EventMon
#if IntBits == 16
#define NoSrcColumnInfo
#endif					/* IntBits == 16 */
#endif					/* EventMon */

#ifndef NoSrcColumnInfo
#define SrcColumnInfo
#endif					/* NoSrcColumnInfo */

#ifndef ExecSuffix
#define ExecSuffix ""
#endif					/* ExecSuffix */

#ifndef CSuffix
#define CSuffix ".c"
#endif					/* CSuffix */

#ifndef HSuffix
#define HSuffix ".h"
#endif					/* HSuffix */

#ifndef ObjSuffix
#define ObjSuffix ".o"
#endif					/* ObjSuffix */

#ifndef LibSuffix
#define LibSuffix ".a"
#endif					/* LibSuffix */

#ifndef CComp
#define CComp "cc"
#endif					/* CComp */

#ifndef COpts
#define COpts "" 
#endif					/* COpts */

/*
 * Note, size of the hash table is a power of 2:
 */
#define IHSize 128
#define IHasher(x)	(((unsigned int)x)&(IHSize-1))	


#if COMPILER

/*
 * Code for the compiler.
 */

#undef MultiThread		/* no way -- interpreter only */
#undef EventMon			/* presently not supported in the compiler */
#undef EventProc		/* presently not supported in the compiler */
#undef ExecImages               /* interpreter only */
#undef InterpMon

#else					/* COMPILER */

/*
 * Code for the interpreter.
 */

#ifndef IcodeSuffix
#define IcodeSuffix ""
#endif					/* IcodeSuffix */

#ifndef IcodeASuffix
#define IcodeASuffix ""
#endif					/* IcodeASuffix */

#ifndef U1Suffix
#define U1Suffix ".u1"
#endif					/* U1Suffix */

#ifndef U2Suffix
#define U2Suffix ".u2"
#endif					/* U2Suffix */

#ifndef USuffix
#define USuffix ".u"
#endif					/* USuffix */

#endif					/* COMPILER */

#ifndef IconOptions

#if ARM || MPW || UNIX
#define IconOptions "C:ELS:Tce:f:mn:o:p:r:stuv:x"
#endif					/* ARM || MPW || UNIX */

#if MSDOS
#define IconOptions "C:ELS:XITce:f:mn:o:p:r:stuv:x"
#endif					/* MSDOS */

#if OS2
#define IconOptions "C:ELS:Tce:f:in:o:p:r:stuv:x"
#endif					/* OS2 */

#endif					/* Options */

#if EBCDIC
#define CUsage "<-C C-comp> <-T> <-c> <-f{adelns}> <-n{acest}>\n\
 <-o ofile> <-p C-opts> <-r runtime> <-s> <-t> <-u> <-v i>"
#else                                   /* EBCDIC */
#ifndef CUsage
#if UNIX
#define CUsage "[-C C-comp] [-E] [-T] [-c] [-f{adelns}] [-m] [-n{acest}]\n\
 [-o ofile] [-p C-opts] [-r runtime] [-s] [-t] [-u] [-v i]"
#else					/* UNIX */
#define CUsage "[-C C-comp] [-E] [-T] [-c] [-f{adelns}] [-n{acest}]\n\
 [-o ofile] [-p C-opts] [-r runtime] [-s] [-t] [-u] [-v i]"
#endif					/* UNIX */
#endif					/* CUsage */
#endif					/* EBCDIC */

#ifndef TUsage

#if EBCDIC
#define TUsage "<-cstu> <-fs> <-e efile> <-o ofile>"
#endif                                   /* EBCDIC */

#if ARM || UNIX
#define TUsage "[-cmstuE] [-fs] [-e efile] [-o ofile]"
#endif					/* ARM || UNIX */

#if MPW
#define TUsage "[-cstuE] [-fs] [-e efile] [-o ofile]"
#endif					/* MPW */

#if MVS || VM
#define TUsage "<-cstuE> <-fs> <-e efile> <-o ofile>"
#endif					/* MVS || VM */

#if MSDOS
#define TUsage "[-cstuEIX] [-fs] [-e efile] [-o ofile]"
#endif					/* MSDOS */

#endif					/* TUsage */

#if MSDOS || OS2
#undef DirectExecution
#define DirectExecution
#endif					/* MSDOS || OS2 */

#ifdef Header
#undef DirectExecution
#define DirectExecution
#endif					/* Header */

/*
 * TEMPORARY
 */

/*
 *  Vsizeof is for use with variable-sized (i.e., indefinite)
 *   structures containing an array of descriptors declared of size 1
 *   to avoid compiler warnings associated with 0-sized arrays.
 */

#define Vsizeof(s)	(sizeof(s) - sizeof(struct descrip))

/*
 * Other sizeof macros:
 *
 *  Wsizeof(x)	-- Size of x in words.
 *  Vwsizeof(x) -- Size of x in words, minus the size of a descriptor.	Used
 *   when structures have a potentially null list of descriptors
 *   at their end.
 */
#define Wsizeof(x)	((sizeof(x) + sizeof(word) - 1) / sizeof(word))
#define Vwsizeof(x)	((sizeof(x) - sizeof(struct descrip) +\
			   sizeof(word) - 1) / sizeof(word))
