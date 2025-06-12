/*
VideoToolboxMATLAB.c
Use this file to ask THINK C to precompile the VideoToolbox.h header with the
MATLAB option enabled to create "VideoToolboxMATLAB.pre". (To create plain old
VideoToolbox.pre, use VideoToolbox.c instead of this file.) Enabling the MATLAB
option is only useful if you're creating MATLAB code resources, i.e. subroutines
for the MATLAB program.

INSTRUCTIONS:
While you have this file open, select Precompile from THINK C’s Source menu.
Save the result as “VideoToolboxMATLAB.pre” in the VideoToolboxSources folder or in
your project's folder (or any subfolder within it). For each suitable project,
type the line

#include "VideoToolboxMATLAB.pre"

into the Edit:Options:THINK C:Prefix window. 

WARNINGS:
VideoToolboxMATLAB.pre is only appropriate for THINK C projects that are used to
build external code resources, i.e. "MEX" subroutines, for MATLAB.

Precompilation ignores the contents of the THINK C Prefix window.

Unfortunately THINK C doesn't check whether a precompiled header is out of date.
If you update any of the THINK C, Standard C, or VideoToolbox.h headers, then you
should recreate the precompiled header. Also, the precompiled header will
reflect the compiler and language settings in effect at the time of
precompilation (e.g. size of int and double), so if you have different projects
with different settings then you may need various versions of the precompiled
header. I don’t know which compiler settings matter; the THINK C manual doesn’t
say. If in doubt, recompile the header.

You can add this file to your project. It won't generate any code or affect your
project in any way. It merely keeps this file handy.

HISTORY:
9/15/93	dgp	Wrote it, based on VIdeoToolbox.c.
*/
#ifndef MATLAB
	#define MATLAB 1
#endif
#include "VideoToolbox.h"
