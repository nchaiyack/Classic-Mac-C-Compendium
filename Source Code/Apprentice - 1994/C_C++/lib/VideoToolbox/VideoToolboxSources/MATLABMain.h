/*
MATLABMain.h
This header allocates space for QuickDraw globals for use by THINK C MATLAB
projects. It should be #included in the main C file of each THINK C MATLAB
project, i.e. in the C file that contains "main" or "user_fcn". It must be
included in exactly one source file of each project, as the THINK C Linker will
otherwise complain.

Some older programs instead #define the symbol "MAIN" before #including
the VideoToolbox.h header. If MAIN is defined then VideoToolbox.h #includes
MATLABMain.h. This older approach is not recommended for new programs
because it fails if you precompile the VideoToolbox.h header.

It might seem dangerous that VideoToolbox.h can include this file and that
this file, in turn, includes VideoToolbox.h. Actually it's fine because
VideoToolbox.h has preprocessor directives that guarantee that it will only
be included once.

HISTORY:
93		dhb wrote it
9/15/93	dgp	extracted it from VideoToolbox.h
*/
#pragma once
#include "VideoToolbox.h"	// defines "THINK_C_QD"
#if MATLAB && defined(THINK_C)
	THINK_C_QD mex_qd;	/*  Allocate storage for QuickDraw globals. */
#endif
