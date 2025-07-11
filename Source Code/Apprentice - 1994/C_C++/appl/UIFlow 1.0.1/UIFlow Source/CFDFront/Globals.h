#pragma segment Main
/* the basics */
#include	<Types.h>
#include	<Resources.h>
#include	<Quickdraw.h>
#include	<Windows.h>
#include	<OSUtils.h>

/* extras */
#include	<Dialogs.h>
#include 	<Packages.h>
#include	<ToolUtils.h>
#include 	<Memory.h>
#include	<Fonts.h>
#include	<Events.h>
#include	<OSEvents.h>
#include	<Menus.h>
#include	<Desk.h>
#include	<TextEdit.h>
#include	<Scrap.h>
#include 	<Palettes.h>
#include 	<Controls.h>
#include 	<Math.h>
#include	<Errors.h>
#include 	<stdlib.h>
#include 	<Files.h>
#include 	<StdIO.h>
#include 	<FCntl.h>
#include	<CursorCtl.h>
#include	<PrintTraps.h>
#include	<Strings.h>
#include	<String.h>

// UUtilities.cp

#ifndef __UMacApp__
#include <UMacApp.h>
#endif __UMacApp__

#ifndef __UMacAppUtilities__
#include <UMacAppUtilities.h>
#endif __UMacAppUtilities__

#ifndef __UDialog__
#include <UDialog.h>
#endif __UDialog__

#ifndef __UGridView__
#include <UGridView.h>
#endif __UGridView__

#ifndef __UTEView__
#include <UTEView.h>
#endif __UTEView__

//--------------------------------------------------------------------------------------------------
//	Menu Command Numbers
//--------------------------------------------------------------------------------------------------
//	Options Menu elements
const CmdNumber	cExport			=	2000;

const CmdNumber	cOptPrint			=	2001;
const CmdNumber	cOptRelax			=	2002;
const CmdNumber	cOptSolution		=	2003;
const CmdNumber	cOptFlow			=	2004;

const CmdNumber	cRApp				=	2009;
const CmdNumber	cRemote			=	2005;
const CmdNumber	cLocal				=	2006;
	
//	Grid Menu Elements
const CmdNumber	cGridOn			=	2007;
const CmdNumber cGridFine			=	2008;

const short	cFirst						= cExport;													// first menu item
const short	cLast						= cRApp;														// last menu item
//--------------------------------------------------------------------------------------------------
// this is the application definition.
//--------------------------------------------------------------------------------------------------
const OSType kSignature 				= 'SPFW';														// Application signature
const OSType kFileType 				= 'UTXT';														// File-type code used for document files 
																														// created by this application
//	OPTIONS MENU - resource id numbers
const short	kCFDFrontWindowId 	= 1000;
const short 	kOptDimension			=	1001;														// Dialog Box ID Number.
const short	kOptPrint					=	1002;														// Dialog Box ID Number.
const short	kOptSolution				=	1005;														// Dialog Box ID Number.
const short	kWarnMe					=	1006;														// Warning Dialog ID Number
const short	kOptFlow					=	1009;														// Dialog Box ID Number.
const short	kOptRadius				=	1010;														// Dialog Box ID Number.
const short	kOptTurbulence			=	1011;														// Dialog Box ID Number.
const short 	kOptDiffusion			=	1013;														// Dialog Box ID Number.
const short	kOptPremix				= 	1014;														// Dialog Box ID Number.
const short	kOptRelax					= 	1015;														// Dialog Box ID Number.
const short	kOptPressure			= 	1016;														// Dialog Box ID Number.
const short	kWarnDelete				=	1017;														// Dialog Box ID Number.

//	COMMAND NUMBERS
const short 	cMakeBaffle				=	2003;														// make a baffle
const short 	cMakeObstacle			=	2002;														// make an obstacle
const short 	cDragPointCommand 	=	2001;														// drag a boundry point
const short 	cNewSegCommand 	=	2000;														// create a new boundry segment
const short 	cNewSecCommand 	=	2004;														// create a new boundry segment
const short	cDeleteSegCommand	=	2005;
const short	cDeleteSecCommand	=	2006;
const short	cDeleteBaffle			=	2007;
const short	cDeleteObstacle		=	2008;

const short 	cMaxSize 					= 350;															// max geometry dimension
const short 	cGeomViewBorder 	= 13;															// offset from top & left
const int 		cNumRlxFacts 			= 7;		
const int 		cNumTurbConst 		= 13;
const int 		gPensize_H 				= 3;																// horizontal pen size
const int 		gPensize_V 				= 3;																// vertical pen size
const int 		gGridsize_H 				= 1;																// horizontal pen size
const int 		gGridsize_V 				= 1;																// vertical pen size
const short 	cMousePlay 				= 5;																// play on either side of a point

/* the point data structure. */

const int cNumProfiles 					= 11;
const int cMaxProfVals 				= 10;
const int cNumFuelProps 				= 5;
const int cElemSize 						= sizeof(Handle);

const IDType		kOK						= 'OKOK';														// Ok Button
const IDType		kCancel					= 'CNCL';														// Cancel Button
const IDType 	cXCoord 				= 'XCor';														// X Coordinate Field
const IDType 	cYCoord 				= 'YCor';														// Y Coordinate Field
CursHandle 		PointCursor;
CursHandle		MagCursorUp;
CursHandle		MagCursorDn;

int					qdbug					= 0;																// debug window flag
TWindow 			*dbWindow;

//this record contains the initial condition for each boundry
typedef struct WallRecord
	{
	float u;
	float v;
	float w;
	float kenergy;
	float dissip;
	float mixfrac;
	float concfrac;
	float fuelfrac;
	float temp;
	float density;
	float visc;
	} WallRecord;
	
enum e1
	{
	Wall = 1,
	InFlow,
	Symm,
	OutFlow,
	Grid,
	NumSegmentTypes
	} SegmentType;

// 	DrawColors. used by segment class to draw the boundries.  

RGBColor DrawColors[NumSegmentTypes] = {
	{0,0,0},
	{2304, 9984, 45056},
	{65280, 7680, 12032},
	{0x7f00, 65280, 0x7f00},
	{0xfff0, 0xfff0, 0},
	{0,0,0},
	};
				
RGBColor SegmentCenter = {0xffff,0xffff,0x3333};											// Black - Segment Point Color
RGBColor SegmentPoint = 	{0xcccc,0x0,0xffff};												// Cyan - Segment Point Color
RGBColor StartPoint = 	{0x0,0xffff,0x0};														// Green - Segment Point Color
RGBColor GridPoint = {0xeeee,0x0,0x0};															// Red - Grid Point Color
RGBColor GridLineColor = {0,0,0};																	// Black - Grid Line Color
RGBColor FineGridColor = {0xffff,0x0000,0xffff};											// magenta - Fine Grid Line Color
RGBColor ObstacleColor = {0x6666,0xcccc,0xffff};											// Sky Blue - Obstacle Fill
RGBColor BaffleColor = {0x9999,0x6666,0x0000};											// Brown - Baffle Fill
RGBColor WhiteColor = {0xffff, 0xffff, 0xffff};
RGBColor BlackColor = {0, 0, 0};

PixPatHandle SegmentPat[NumSegmentTypes];													// Pix Pattern for the Segments
PixPatHandle SegmentCenterPat;																		// Pix Pattern for the Segment Point
PixPatHandle SegmentPointPat;																			// Pix Pattern for the Segment Point
PixPatHandle StartPointPat;
PixPatHandle PointFramePat;																			// Pix Pattern for the select frame
PixPatHandle GridPointPat;																				// Pix Pattern for grid point
PixPatHandle GridLinePat;																					// Pix Pattern for grid line
PixPatHandle FineGridPat;																					// Pix Pattern for grid line
PixPatHandle ObstaclePat;																				// Pix Pattern for obstacle fill
PixPatHandle BafflePat;																					// Pix Pattern for baffle fill

short pat[NumSegmentTypes]={1,23,3,26};														// the drawing patterns for lines. 
Pattern SegPatterns[NumSegmentTypes];

extended c1, c2, c3, c4, c5, c6, c7, c8;

enum e2
	{
	cDragTool = 0,
	cSegmentTool,
	cGridTool,
	cBaffle,
	cObstacle,
	cData,
	cFineGrid,
	cMagnify
	} ToolType;
	
enum e3	
	{
	cNotOnBoundry = 0,
	cTop,
	cRight,
	cBottom,
	cLeft	
	} BoundrySide;

enum e4																											// bit order for determining the
	{																													// status of the requested dialog box
	cPrintDialog = 1,																							
	cFlowDialog,
	cSolutionDialog,
	cRelaxDialog
	} ModelessDialogStatus;
	
struct gridRC
	{																													// row / column for grid cell
	short row;
	short col;
	};

struct realPt		
	{																													// used in conversion view to real
	float x;
	float y;
	};
	
struct slopeStruct	{																						// equation:  Y = mX + b
	Point		first;																							// left most point
	Point		second;																							// right most point
	extended 	m;																								// slope
	extended	b;																									// y intercept
	Boolean		horizontal;
	Boolean		vertical;
	};
	
typedef struct slopeStruct lineSlope;
