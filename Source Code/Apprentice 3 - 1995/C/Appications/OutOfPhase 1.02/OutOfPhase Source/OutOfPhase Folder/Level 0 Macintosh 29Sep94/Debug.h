/* Debug.h */

#ifndef Included_Debug_h
#define Included_Debug_h

/* Debug module depends on: */
/* MiscInfo.h */
/* Definitions */
/* Audit */

/* error handling utilities */

/* in order to use this debugging package, define the following macros */
/* in the MiscInfo.h file: */
/* #define DEBUG (1)  defines the EXECUTE and ERROR macros */
/* #define DEBUG (0)  eliminates EXECUTE and ERROR macros */
/* #define ALWAYSRESUME (1)  makes the 'resume' button always available, even */
/*  if 'ForceAbort' was passed to PRERR */
/* #define ALWAYSRESUME (0)  makes 'ForceAbort' suppress the resume button */
/* #define THINKC_DEBUGGER (1)  adds two breakpoints to the PRERR function, one */
/*  just before it calls ExitToShell() for 'Quit' button, the other just before */
/*  the function returns to caller for 'Resume' button. */
/* #define THINKC_DEBUGGER (0)  eliminates the breakpoints */

#define ForceAbort (0)
#define AllowResume (1)

/* error printing routine.  Message should be a null-terminated string */
void							PRERR(int AbortFlag, char* Message);

/* initialize the local memory buffer used by PRERR in emergency situations */
/* for internal use only */
void							Eep_InitPRERR(void);

/* shut down the PRERR stuff.  for internal use only */
void							Eep_ShutdownPRERR(void);

/* debugging macros */
#if DEBUG
	/* EXECUTE(function) executes the function if debugging is enabled */
	/* in a code block, it can be called like a function:  EXECUTE(parameter); */
	/* in declarations, the semi-colon must be included as part of the parameter: */
	/*  EXECUTE(parameter;) */
	#define EXECUTE(function)  function
	/* ERROR(condition,function) executes the function if condition is TRUE */
	/* there is no reason to call this anywhere other than a code block */
	#define ERROR(condition,function)  if (condition) EXECUTE(function)
#else
	/* these versions ignore their parameters and generate no code */
	#define EXECUTE(function)
	#define ERROR(condition,param)
#endif

#endif
