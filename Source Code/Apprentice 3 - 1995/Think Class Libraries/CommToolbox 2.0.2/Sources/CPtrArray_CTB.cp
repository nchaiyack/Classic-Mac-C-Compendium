/* °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°

	CPtrArray_CThread.cp
	
	CTB Classes list class.
	
	Copyright © 1994-95 Ithran Einhorn. All rights reserved.
	
°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°° */

#include "CPtrArray.h"
#include "CConnection.h"
#include "CFileTransfer.h"
#include "CTermPane.h"

#pragma template_access public
#pragma template CPtrArray<CConnection>
#pragma template CPtrArray<CFileTransfer>
#pragma template CPtrArray<CTermPane>

TCL_DEFINE_CLASS_M1(CPtrArray<CConnection>,CVoidPtrArray);
TCL_DEFINE_CLASS_M1(CPtrArray<CFileTransfer>,CVoidPtrArray);
TCL_DEFINE_CLASS_M1(CPtrArray<CTermPane>,CVoidPtrArray);

#include "CPtrArray.tem"