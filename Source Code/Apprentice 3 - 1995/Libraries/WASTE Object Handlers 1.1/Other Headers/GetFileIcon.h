/* ------------------ GetFileIcon.h ------------------- */

#ifndef __GETFILEICON__
#define __GETFILEICON__

#include <Icons.h>
#include <Finder.h>

#ifdef __cplusplus
extern "C" {
#endif

pascal OSErr GetFileIcon( FSSpec *thing, IconSelectorValue iconSelector, Handle *theSuite);
Boolean IsSuiteEmpty( Handle theSuite );

#ifdef __cplusplus
}
#endif

#endif // __GETFILEICON__
