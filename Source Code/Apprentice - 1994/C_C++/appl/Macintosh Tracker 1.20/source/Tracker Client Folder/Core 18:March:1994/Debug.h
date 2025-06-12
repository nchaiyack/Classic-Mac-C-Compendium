/* Debug.h */

#pragma once

/* error handling utilities */

/* in order to use this debugging package, define the following macros */
/* in the prefix: */
/* #define DEBUG  defines the EXECUTE and ERROR macros */
/* #define ALWAYSRESUME  makes the 'resume' button always available, even */
/*  if 'ForceAbort' was passed to PRERR */
/* #define THINKC_DEBUGGER  adds two breakpoints to the PRERR function, one */
/*  just before it calls ExitToShell() for 'Quit' button, the other just before */
/*  the function returns to caller for 'Resume' button. */

#define ForceAbort (0)
#define AllowResume (1)

pascal void		MyResumeProc(void);
void		PRERR(short AbortFlag, void* Message);
void		InitPRERR(void);
void		SetErrorFunction(void (*Erf)(void));

/* debugging macros */
#ifdef DEBUG
	/* EXECUTE(function) executes the function */
	/* in a code block, it can be called like a function:  EXECUTE(parameter); */
	/* in declarations, the semi-colon must be included as part of the parameter: */
	/*  EXECUTE(parameter;) */
	#define EXECUTE(function)  function
	/* ERROR(condition,function) executes the function if condition is TRUE */
	/* there is no reason to call this anywhere other than a code block */
	#define ERROR(condition,function)  if (condition) EXECUTE(function)
	/* checks the range of access of a handle to see if it is within the handle's size */
	/* AccessSize is how large an 'object' will be accessed.  Pass sizeof(type) to it. */
	void			HRNGCHK(void* TheHandle, void* EffectiveAddress, signed long AccessSize);
	/* checks the range of access of a ptr to see if it is within the ptr's size */
	/* AccessSize is how large an 'object' will be accessed.  Pass sizeof(type) to it. */
	void			PRNGCHK(void* ThePointer, void* EffectiveAddress, signed long AccessSize);
#else
	/* these versions ignore their parameters and generate no code */
	#define EXECUTE(function)
	#define ERROR(condition,param)
	#define HRNGCHK(Hand,Addr,Mode)
	#define PRNGCHK(Poin,Addr,Mode)
#endif
