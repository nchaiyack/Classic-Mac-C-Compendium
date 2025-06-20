/* DrawCommand.h */

#ifndef Included_DrawCommand_h
#define Included_DrawCommand_h

/* DrawCommand module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Screen */
/* DataMunging */
/* Memory */

#include "Screen.h"

/* draw a command with no parameters.  the string is null terminated and not disposed. */
OrdType						DrawCommandNoParams(WinType* Window, OrdType X, OrdType Y,
										FontType Font, FontSizeType FontSize, MyBoolean ActuallyDraw,
										MyBoolean GreyedOut, char* String);

/* draw a command with one parameter.  first string is null terminated and not */
/* disposed, but the second string is a non-null-terminated heap block which is */
/* disposed.  if the second one is NIL, then it is ignored */
OrdType						DrawCommand1Param(WinType* Window, OrdType X, OrdType Y,
										FontType Font, FontSizeType FontSize, MyBoolean ActuallyDraw,
										MyBoolean GreyedOut, char* String, char* Argument1);

/* draw a command with 2 parameters */
OrdType						DrawCommand2Params(WinType* Window, OrdType X, OrdType Y,
										FontType Font, FontSizeType FontSize, MyBoolean ActuallyDraw,
										MyBoolean GreyedOut, char* String, char* Argument1,
										char* Argument2);

/* draw a command that has one parameter, but has line feeds in it. */
/* first string is null terminated and not disposed, but the second string is */
/* a non-null-terminated heap block which is disposed.  if the second one is NIL, */
/* then it is ignored */
OrdType						DrawCommand1ParamWithLineFeeds(WinType* Window, OrdType X, OrdType Y,
										FontType Font, FontSizeType FontSize, MyBoolean ActuallyDraw,
										MyBoolean GreyedOut, char* String, char* Argument1);

#endif
