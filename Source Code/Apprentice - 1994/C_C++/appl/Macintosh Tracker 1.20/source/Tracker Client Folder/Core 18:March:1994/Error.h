/* Error.h */

#pragma once

#ifndef COMPILING_ERROR_C
	extern MyBoolean GlobalErrorStatus;
#endif

#define ResetErrorCheck(glop) GlobalErrorStatus = False
#define SetErrorStatus(glop) GlobalErrorStatus = True
#define ErrorOccurred(glop) (GlobalErrorStatus)
