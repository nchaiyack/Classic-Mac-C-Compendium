/*	NAME:
		ExtensionShell.h

	WRITTEN BY:
		Dair Grant
				
	DESCRIPTION:
		Header file for ExtensionShell.c

	___________________________________________________________________________
*/
#ifndef __EXTENSIONSHELL__
#define __EXTENSIONSHELL__
//=============================================================================
//		Include files
//-----------------------------------------------------------------------------
#include <QuickDraw.h>
#include <Traps.h>





//=============================================================================
//		Defines
//-----------------------------------------------------------------------------
#define GrafSize				206							// From IM I-209. Total QDraw storage
#define ADD_GRAFSIZE			(GrafSize - 130)			// 130 bytes in shown fields
#define kInitGrafTrap   		_InitGraf					// 0xA86E
#define kUnimplementedTrap		_Unimplemented				// 0xA89F





//=============================================================================
//		Structures
//-----------------------------------------------------------------------------
typedef struct	{
	char			filler[ADD_GRAFSIZE];			// Internal QD storage
	long			randSeed;
	BitMap			screenBits;
	Cursor			arrow;
	Pattern			dkGray;
	Pattern			ltGray;
	Pattern			gray;
	Pattern			black;
	Pattern			white;
	GrafPtr			thePort;
} FakeQD;





//=============================================================================
//		Function prototypes
//-----------------------------------------------------------------------------
TrapType	GetTrapType(int trapNum);


#endif
