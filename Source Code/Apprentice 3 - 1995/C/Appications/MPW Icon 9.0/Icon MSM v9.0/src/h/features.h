/*
 * features.h -- predefined symbols and &features
 *
 * This file consists entirely of a sequence of conditionalized calls
 *  to the Feature() macro.  The macro is not defined here, but is
 *  defined to different things by the the code that includes it.
 *
 * For the macro call  Feature(guard,symname,kwval)
 * the parameters are:
 *    guard	for the compiler's runtime system, an expression that must
 *		evaluate as true for the feature to be included in &features
 *    symname	predefined name in the preprocessor; "" if none
 *    kwval	value produced by the &features keyword; 0 if none
 *
 * The translator and compiler modify this list of predefined symbols
 * through calls to ppdef().
 */

   Feature(1, "_V9", 0)			/* Version 9 (unconditional) */

#if AMIGA
   Feature(1, "_AMIGA", "Amiga")
#endif					/* AMIGA */
#if ARM
   Feature(1, "_ACORN", "Acorn Archimedes")
#endif					/* ARM */
#if ATARI_ST
   Feature(1, "_ATARI", "Atari ST")
#endif					/* ATARI_ST */
#if VM
   Feature(1, "_CMS", "CMS")
#endif					/* VM */
#if MACINTOSH
   Feature(1, "_MACINTOSH", "Macintosh")
#endif					/* MACINTOSH */
#if MSDOS
#if INTEL_386 || HIGHC_386 || WATCOM || ZTC_386
   Feature(1, "_MSDOS_386", "MS-DOS/386")
#else					/* INTEL_386 || HIGHC_386 ... */
#if MSNT
   Feature(1, "_MS_WINDOWS_NT", "MS Windows NT")
#else					/* MSNT */
   Feature(1, "_MSDOS", "MS-DOS")
#endif					/* MSNT */
#endif					/* INTEL_386 || HIGHC_386 ... */
#endif					/* MSDOS */
#if MVS
   Feature(1, "_MVS", "MVS")
#endif					/* MVS */
#if OS2
   Feature(1, "_OS2", "OS/2")
#endif					/* OS2 */
#if PORT
   Feature(1, "_PORT", "PORT")
#endif					/* PORT */
#if UNIX
   Feature(1, "_UNIX", "UNIX")
#endif					/* VM */
#if VMS
   Feature(1, "_VMS", "VMS")
#endif					/* VMS */

#if COMPILER
   Feature(1, "", "compiled")
#else					/* COMPILER */
   Feature(1, "", "interpreted")
#endif					/* COMPILER */

#if EBCDIC != 1
   Feature(1, "_ASCII", "ASCII")
#else					/* EBCDIC != 1 */
   Feature(1, "_EBCDIC", "EBCDIC")
#endif					/* EBCDIC */

#ifdef Coexpr
   Feature(1, "_CO_EXPRESSIONS", "co-expressions")
#endif					/* Coexpr */
#ifdef DirectExecution
   Feature(1, "_DIRECT_EXECUTION", "direct execution")
#endif					/* DirectExecution */
#ifdef LoadFunc
   Feature(1, "_DYNAMIC_LOADING", "dynamic loading")
#endif					/* LoadFunc */
#ifdef EnvVars
   Feature(1, "", "environment variables")
#endif					/* EnvVars */
#ifdef EventMon
   Feature(1, "_EVENT_MONITOR", "event monitoring")
#endif					/* EventMon */
#ifdef ExecImages
   Feature(1, "_EXECUTABLE_IMAGES", "executable images")
#endif					/* ExecImages */
#ifndef	FixedRegions			/* FixedRegions */
   Feature(1, "_EXPANDABLE_REGIONS", "expandable regions")
#endif					/* FixedRegions */
#ifdef ExternalFunctions
   Feature(1, "_EXTERNAL_FUNCTIONS", "external functions")
#endif					/* ExternalFunctions */
#ifdef FixedRegions
   Feature(1, "_FIXED_REGIONS", "fixed regions")
#endif					/* FixedRegions */
#ifdef KeyboardFncs
   Feature(1, "_KEYBOARD_FUNCTIONS", "keyboard functions")
#endif					/* Keyboard */
#ifdef LargeInts
   Feature(largeints, "_LARGE_INTEGERS", "large integers")
#endif					/* LargeInts */
#ifdef MemMon
   Feature(1, "_MEMORY_MONITOR", "memory monitoring")
#endif					/* MEMMON */
#ifdef MultiThread
   Feature(1, "_MULTITASKING", "multiple programs")
#endif					/* MultiThread */
#ifdef MultiRegion
   Feature(1, "_MULTIREGION", "multiple regions")
#endif					/* MultiRegion */
#ifdef Pipes
   Feature(1, "_PIPES", "pipes")
#endif					/* Pipes */
#ifdef RecordIO
   Feature(1, "_RECORD_IO", "record I/O")
#endif					/* RecordIO */
#ifdef StrInvoke
   Feature(1, "_STRING_INVOKE", "string invocation")
#endif					/* StrInvoke */
#ifdef SystemFnc
   Feature(1, "_SYSTEM_FUNCTION", "system function")
#endif					/* SystemFnc */
#ifdef Visualization
   Feature(1, "_VISUALIZATION", "visualization support")
#endif					/* Visualization */

#ifdef Graphics
   Feature(1, "_GRAPHICS", "graphics")
#endif					/* Graphics */
#ifdef XWindows
   Feature(1, "_X_WINDOW_SYSTEM", "X Windows")
#endif					/* XWindows */
#ifdef MSWindows
#ifdef MSNT
   Feature(1, "_WIN32", "Win32")
#else					/* MSNT */
   Feature(1, "_WIN16", "Win16")
#endif					/* MSNT */
#endif					/* MSWindows */
#ifdef PresentationManager
   Feature(1, "_PRESENTATION_MGR", "Presentation Manager")
#endif					/* PresentationManager */

#ifdef ArmFncs
   Feature(1, "_ARM_FUNCTIONS", "Archimedes extensions")
#endif					/* ArmFncs */
#ifdef DosFncs
   Feature(1, "_DOS_FUNCTIONS", "MS-DOS extensions")
#endif					/* DosFncs */

#ifdef DataConv
   Feature(1,"_BINARYDATA_CONVERSION", "binary data conversions")
#endif					/* DataConv */
