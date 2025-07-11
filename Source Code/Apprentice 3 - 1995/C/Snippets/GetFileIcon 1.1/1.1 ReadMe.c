/*
GetFileIcon.c
3/15/95
ver 1.1

First, I'd like to say that I tried to make GetFileIcon (GFI) accessable to as many
developers and development environment platforms possible.  If you are still using
the old universal headers, use the THINK C or CodwWarrior project in the "Old Headers"
folder.  If you use the new universal headers (for PPC & 68k development), check out
the files in the "Universal Headers" folder.

Second, I'd like to say that none of this would have been possible without the aid of
Erik A. Johnson (johnsone@uxh.cso.uiuc.edu).  Erik provided fixes to the code so it could
be compiled in CodwWarrior, implemented the Universal Headers ProcPtr code, and
informed me of several bugs.


Here are some notes that Erik sent me:

Modified: 3/1/95 by eaj  -- removed the 680x0 assembly from FindGenericIconID() and
                            put the data in the globals gGenericFinderIcons and
                            gGenericSysIcons
                         -- removed most of GetFileIcon.h because most of it reproduced
                            the Universal Header files Icons.h and Finder.h (these are
                            now included within the new GetFileIcon.h.
                         -- added a call to PBHGetVolParms() in InOneDesktop() to make
                            sure a volume supports the Desktop Manager before trying to
                            examine the Desktop database in that volume
                         -- modified the calls to ForEachIconDo() (in the routines
                            GetNormalFileIcon(), CopyEachIcon(), Get1IconSuite(), and
                            IsSuiteEmpty()) to use UniversalProcPtr's.

Compiles fine with CW5 (MW C/C++ 1.2).  I haven't tried it with MPW PPCC or THINK C.
To use GetFileIcon(), include GetFileIcon.c in your project, and in your source do
something like:

#include "GetFileIcon.h"

FSSpec  myFileFSSpec;
Handle  myIconSuite;
OSErr   err;
Rect    myRect={0,0,32,32};

err = GetFileIcon( &myFileFSSpec, svAllAvailableData, &myIconSuite );
PlotIconSuite( &myRect, atNone, ttNone, myIconSuite );
*/          
