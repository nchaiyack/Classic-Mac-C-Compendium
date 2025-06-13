// =================================================================================
//	LDEF.r			©1994 Harold Ekstrom, AG Group, Inc. All rights reserved.
// =================================================================================
//	This rez file produces a fat code resource from 68k and PPC code using the sdes
//	template found in MixedMode.r.
//	The value for the first two fields can be found by looking for the string
//	"__procinfo" in the disassembly of the LDEFStub.c file in the PPC project.
//	The second occurrence of this string should be found in the disassembly similar
//	to this:
//
//	Hunk:	Kind=HUNK_GLOBAL_IDATA   Align=4  Class=RW  Name="__procinfo"(1)  Size=4
//	00000000: 00 0E BD 80                                     '....'
//	          ^^^^^^^^^^^ this is the value used below.

#include "MixedMode.r"

type 'LDEF' as 'sdes';

resource 'LDEF' (128) {
	0x000EBD80,									// 68K ProcInfo
	0x000EBD80,									// PowerPC ProcInfo
	$$Resource("LDEFStub.rsrc", 'oCod', 128),	// name, type, and ID of rsrc containing 68k code
	$$Resource("LDEFStub.rsrc", 'pCod', 128)	// name, type, and ID of rsrc containing ppc code
};
