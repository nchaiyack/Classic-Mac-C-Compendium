/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�

	CPtrArray_CThread.cp
	
	Thread Manager list class.
	
	Copyright � 1994-95 Ithran Einhorn. All rights reserved.
	
같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같� */

#include "CPtrArray.h"
#include "CThread.h"

#pragma template_access public
#pragma template CPtrArray<CThread>

TCL_DEFINE_CLASS_M1(CPtrArray<CThread>,CVoidPtrArray);

#include "CPtrArray.tem"