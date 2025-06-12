///--------------------------------------------------------------------------------------
// StringUtils.h
///--------------------------------------------------------------------------------------


#ifndef __STRINGUTILS__
#define __STRINGUTILS__

#ifndef __TYPES__
#include <Types.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif


void PStrCpy(Str255 srcStr, Str255 dstStr);
void PStrCat(Str255 srcStr, Str255 dstStr);


#ifdef __cplusplus
};
#endif
#endif

