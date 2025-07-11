#include "MixedMode.r"

type 'WDEF' as 'sdes';

resource 'WDEF' (128) {
	0x00003BB0,										// 68K ProcInfo
	0x00003BB0,										// PowerPC ProcInfo
	$$Resource("InfinityWDEF.rsrc", 'oCod', 128),	// name, type, and ID of rsrc containing 68k code
	$$Resource("InfinityWDEF.rsrc", 'pCod', 128)	// name, type, and ID of rsrc containing ppc code
};
