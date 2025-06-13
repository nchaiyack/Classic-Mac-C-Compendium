/*********************************************************************
Project	:	XL				-	XCMDs for everyone
File		:	XLTest.c		-	The testing gear
Author	:	Matthias Neeracher
Started	:	10Sep92								Language	:	MPW C
Modified	:	10Sep92	MN	
Last		:	10Sep92
*********************************************************************/

#include <Files.h>
#include <Memory.h>
#include <QuickDraw.h>
#include <Resources.h>

#include	<stdio.h>

#include "XL.h"

void main(int argc, char ** argv)
{
	short					res;
	struct XCmdBlock 	cmd;
	Handle				xcmd;
	Boolean				fn;
	
	InitGraf((Ptr) &qd.thePort);
	
#ifdef XLDEBUG
	XLDebug = xl_DebugAll;
#endif

	if (argc < 3) {
		fprintf(stderr, "Usage: XLTest file (xcmd|xfcn) [args...].\n");
		
		exit(1);
	}
		
	res = openrfperm(argv[1], 0, fsRdPerm);
	
	if (res == -1) {
		fprintf(stderr, "# File not found: %s\n", argv[1]);
		
		exit(1);
	}
	
	if (xcmd = get1namedresource('XCMD', argv[2]))
		fn = false;
	else if (xcmd = get1namedresource('XFCN', argv[2]))
		fn = true;
	else {
		fprintf(stderr, "# Resource not found: %s\n", argv[2]);
		
		CloseResFile(res);
		
		exit(1);
	}

	cmd.paramCount = 0;
	for (argv += 3; *argv; ++argv)
		PtrToHand(*argv, cmd.params+cmd.paramCount++, strlen(*argv)+1);
		
	XLCall(xcmd, XLDefaultGlue, &cmd);
	
	if (cmd.returnValue)	{
		HLock(cmd.returnValue);
		printf("Function returned %s.\n", *cmd.returnValue);
	}
}
