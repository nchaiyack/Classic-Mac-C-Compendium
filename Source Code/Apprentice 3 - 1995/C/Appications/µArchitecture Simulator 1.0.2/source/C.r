/*
Copyright � 1993,1994 by Fabrizio Oddone
��� ��� ��� ��� ��� ��� ��� ��� ��� ���
This source code is distributed as freeware: you can copy, exchange, modify this
code as you wish. You may include this code in any kind of application: freeware,
shareware, or commercial, provided that full credits are given.
You may not sell or distribute this code for profit.
*/

#define SystemSevenOrLater 1

#include "SysTypes.r"
#include "Types.r"
#include "StdAlert.r"

/*----------------------------CSTR � C-Style String--------------------------------*/
type 'CSTR' {
		cstring;
};


#ifdef	__USVersion__
#include "USAC.r"
#include "VersUSA.r"
#include "GeneralUsageUSA.r"
//INCLUDE "USAC.rsrc";
#endif	__USVersion__

#ifdef	__VersioneItaliana__
#include "ItaliaC.r"
#include "VersItalia.r"
#include "GeneralUsageItalia.r"
//INCLUDE "ItaliaC.rsrc";
#endif	__VersioneItaliana__

