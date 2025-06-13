#include "CPtrArray.h"
#include "CPrimitive.h"

#pragma template_access public
#pragma template CPtrArray<CPrimitive>

TCL_DEFINE_CLASS_M1(CPtrArray<CPrimitive>, CVoidPtrArray);

#include "CPtrArray.tem"