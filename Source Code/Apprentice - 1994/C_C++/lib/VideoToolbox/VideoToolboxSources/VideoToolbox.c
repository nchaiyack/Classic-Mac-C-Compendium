/*
VideoToolbox.c
Use this file to ask THINK C to precompile the VideoToolbox.h header creating
"VideoToolbox.pre". (To create "VideoToolboxMATLAB.pre" use VideoToolboxMATLAB.c
instead of this file.)

INSTRUCTIONS:
While you have this file open, select Precompile from THINK C’s Source menu.
Save the result as “VideoToolbox.pre” in the VideoToolboxSources folder or in
your project's folder (or any subfolder within it). You may then speed up the
compilation of any project roughly tenfold by typing the line

#include "VideoToolbox.pre"

into the Edit:Options:THINK C:Prefix window. 

WARNINGS:
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
2/27/93	dgp	Wrote it.
4/22/93	dgp	Updated comments.
9/8/93	dgp	Updated comments.
9/12/93	dgp	Updated comments.
9/15/93	dgp	Updated comments.
*/
#include "VideoToolbox.h"
