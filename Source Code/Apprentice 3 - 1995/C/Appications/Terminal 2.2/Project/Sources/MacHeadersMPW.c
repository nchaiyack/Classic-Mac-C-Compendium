/*
	Terminal 2.0
	"MacHeadersMPW.c"

	Compile this file, it will create "MacHeadersMPW"
	containing the precompiled headers needed
*/


#include <Controls.h>
#include <CType.h>
#include <Desk.h>
#include <Devices.h>
#include <Dialogs.h>
#include <DiskInit.h>
#include <Errors.h>
#include <Files.h>
#include <Fonts.h>
#include <Memory.h>
#include <Menus.h>
#include <Packages.h>
#include <OSUtils.h>
#include <Resources.h>
#include <Scrap.h>
#include <Script.h>
#include <Serial.h>
#include <SetJmp.h>
#include <StandardFile.h>
#include <StdArg.h>
#include <String.h>
#include <Strings.h>
#include <SysEqu.h>
#include <ToolUtils.h>
#include <Types.h>
#include <Windows.h>

/* Macros not found in MPW, but used by THINK C */

/* typedef unsigned char Byte; */	/* Is now defined in "Types.h" of MPW 3.2 */
enum { FALSE, TRUE };
#define topLeft(r)	(((Point *) &(r))[0])
#define botRight(r)	(((Point *) &(r))[1])
#define CtoPstr		c2pstr
#define PtoCstr		p2cstr

/* Low memory globals are real variables in THINK C,
but are only address constants in MPW */

#define Time			(*(long *)0x20C)
#define Ticks			(*(long *)0x16A)
#define SCCRd			(*(Ptr *)0x1D8)
#define SFSaveDisk		(*(short *)0x214)
#define CurDirStore		(*(long *)0x398)
#define CurStackBase	(*(Ptr *)0x908)
#define GrayRgn			(*(RgnHandle *)0x9EE)
#define MemErr			(*(short *)0x220)
#define MBarHeight		(*(short *)0xBAA)

/* The QuickDraw global variables structure does not
exist in THINK C, so use a macro to generate the names */

#define QD(x)	(qd.x)

/*
	Attention:
	
	� int		4 bytes in MPW and 2 bytes in THINK C
	� \r		Line feed in MPW, carriage return in THINK C
	� \n		Carriage return in MPW, line feed in THINK C
	� char		4 bytes on stack if passed to a function
	� short		4 bytes on stack if passed to a function
*/

/* Precompile all definitions */

#pragma dump ":(Objects):MacHeadersMPW"
