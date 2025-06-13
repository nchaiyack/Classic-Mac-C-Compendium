	/************************************************************************/
	/*																		*/
	/*							DisplayGraphicsKludge						*/
	/*																		*/
	/*		DisplayGraphicsKludge is a procedure that will display an area	*/
	/* of memory in a dialog box as a graphic.  The purpose of this is to	*/
	/* allow vanilla C programs that use a text window (for printf & scanf)	*/
	/* access to the Mac's graphics without learning the Mac Toolboxes.		*/
	/*		The image data is interpreted as one byte per pixel.  The image	*/
	/* is displayed using a 256 uniform level gray scale, where 0 = black	*/
	/* and 255 = white.														*/
	/*																		*/
	/*		DisplayGraphicsKludge doesn't do any error checking on the		*/
	/* parameters passed to it.  If you crash without anything displayed,	*/
	/* you passed an invalid pointer to your data.  If the dialog window	*/
	/* is not the right size or is not located in the upper left of the		*/
	/* monitor, you passed bad HPixel and/or VPixel values.  If the window	*/
	/* title is weird, you may have passed a C string instead of a Pascal	*/
	/* string.																*/
	/*																		*/
	/*		DisplayGraphicsKludge is passed 4 parameters:					*/
	/*	Str255	theImageTitle	A Pascal String used as the window title &	*/
	/*							the page title if printed.  Pascal strings	*/
	/*							in Think C have this format: "\pMy Title"	*/
	/*																		*/
	/*	Ptr		theDataPtr		Pointer to the data (address of the data).	*/
	/*							Cast your pointer to Ptr, if necessary.		*/
	/*							The data must be one byte per pixel.		*/
	/*																		*/
	/*	short	HPixels			# of horizontal pixels (pixels/row).  There	*/
	/*							is no error checking, make sure it's right!	*/
	/*																		*/
	/*	short	VPixels			# of vertical pixels (rows).  There is no	*/
	/*							error checking, make sure it's right!		*/
	/*																		*/
	/*		DisplayGraphicsKludge was written by->							*/
	/*																		*/
	/*		Samuel Herschbein												*/
	/*		Center for Bioengineering WD-12									*/
	/*		University of Washington										*/
	/*		Seattle, WA  98195												*/
	/*		(206) 543-9757													*/
	/*		Internet: sam@bioeng.washington.edu								*/
	/*																		*/
	/*		DisplayGraphicsKludge was developed using Think C 6.0.  If you	*/
	/* are using another C compiler, you may need to add some #includes		*/
	/* that Think C includes automatically.									*/
	/*																		*/
	/*		This code was written to be as readable and easy to understand	*/
	/* as possible.  It is in not compact or quick, and it has no tricks.	*/
	/*																		*/
	/************************************************************************/


	/************************************************************************/
	/*																		*/
	/*		If you're using Think C and have the Project prefix set to 		*/
	/*			#include <Macheaders.c>										*/
	/*	you can delete the following list of #includes (assuming you have	*/
	/* not modified the standard MacHeaders).								*/
	/*																		*/
	/************************************************************************/
#include	"Controls.h"
#include	"Dialogs.h"
#include	"Fonts.h"
#include	"Memory.h"
#include	"Quickdraw.h"
#include	"PrintTraps.h"
#include	"THINK.h"
#include	"ToolUtils.h"
#include	"Types.h"
#include	"Windows.h"


	/************************************************************************/
	/*																		*/
	/*		The following #includes must always be present in Think C.	 	*/
	/*																		*/
	/************************************************************************/
#include	"Errors.h"
#include	"Palettes.h"
#include	"QDOffscreen.h"
#include	"StandardFile.h"
#include	<stdio.h>



void	DisplayGraphicsKludge(	Str255		theImageTitle,					/* title for the window	(as a Pascal String)			*/
								Ptr			theDataPtr,						/* address of where the actual pixel values are			*/
								short		HPixels,						/* # of horizontal pixels (pixels per row)				*/
								short		VPixels)						/* # of vertical pixels (# rows)						*/
	/************************************************************************/
	/*																		*/
	/*	Str255	theImageTitle	A Pascal String used as the window title &	*/
	/*							the page title if printed.  Pascal strings	*/
	/*							in Think C have this format: "\pMy Title"	*/
	/*																		*/
	/*	Ptr		theDataPtr		Pointer to the data (address of the data).	*/
	/*							Cast your pointer to Ptr, if necessary.		*/
	/*							The data must be one byte per pixel.		*/
	/*																		*/
	/*	short	HPixels			# of horizontal pixels (pixels/row).  There	*/
	/*							is no error checking, make sure it's right!	*/
	/*																		*/
	/*	short	VPixels			# of vertical pixels (rows).  There is no	*/
	/*							error checking, make sure it's right!		*/
	/*																		*/
	/************************************************************************/
{
#define ScreenEdgeSlop		3												/* # pixels from screen edge							*/
#define MenuBarHeight		20												/* since we run on any system, can't use GetMBarHeight	*/
#define WindowTitleSize		20												/* we could calculate this, but since we can't calc ^^	*/
#define TopSlop				MenuBarHeight+WindowTitleSize+ScreenEdgeSlop	/* keeps it off menu bar (unless you're using a Radius)	*/
#define	BottomSlop			ScreenEdgeSlop									/* keeps it off edge of screen							*/
#define	SideSlop			ScreenEdgeSlop									/* keeps it off edge of screen							*/
#define	ButtonTopSlop		10												/* # pixels from button to edge of image/window			*/
#define	ButtonSideSlop		20												/* # pixels from button to edge of window/button		*/

#define	PrintTitleOffset	25												/* # pixels for title (move image down on print page)	*/


																			/* since this DITL is constructed in memory:			*/
																			/* CHANGES MUST BE MADE PRECISELY, OR ELSE!!!			*/
																			/* these #defines are directly from theDITLData below !	*/
#define	OKButtonItemID		1												/* OK Button is first item in DITL						*/
#define SaveButtonItemID	2												/* Save Button is second item in DITL					*/
#define	PrintButtonItemID	3												/* Print button is third item in DITL					*/
#define OKButtonWidth		58												/* width of OKButton - from DITL below!!!				*/
#define SaveButtonWidth		58												/* width of SaveButton - from DITL below!!!				*/
#define PrintButtonWidth	58												/* width of PrintButton - from DITL below!!!			*/
#define ButtonHeight		20												/* height of Buttons - from DITL below!!!				*/


char			theDITLData [58]= {											/* array size						*/
								0x00, 0x02,									/* number of items - 1				*/
																			/* 1st item - OK Button				*/
								0x00, 0x00, 0x00, 0x00,							/* ProcPtr						*/
								0x00, 0x00,										/* Display Rect - top = 0		*/
								0x00, 0x00,										/* Display Rect - left = 0		*/
								0x00, 0x14,										/* Display Rect - bottom = 20	*/
								0x00, 0x3a,										/* Display Rect - right	 = 58	*/
								0x04,											/* Item Type					*/
								0x02,											/* Button Title Length			*/
								'O','K',										/* Button Title					*/
																			/* 2nd Item - Save Button			*/
								0x00, 0x00, 0x00, 0x00,							/* ProcPtr						*/
								0x00, 0x00,										/* Display Rect - top = 0		*/
								0x00, 0x00,										/* Display Rect - left = 0		*/
								0x00, 0x14,										/* Display Rect - bottom = 20	*/
								0x00, 0x3a,										/* Display Rect - right = 58	*/
								0x04,											/* Item Type					*/
								0x05,											/* Button Title Length			*/
								'S','a','v','e','É',							/* Button Title					*/
								0x00,											/* pad to even word				*/
																			/* 3rd Item - Print Button			*/
								0x00, 0x00, 0x00, 0x00,							/* ProcPtr						*/
								0x00, 0x00,										/* Display Rect - top = 0		*/
								0x00, 0x00,										/* Display Rect - left = 0		*/
								0x00, 0x14,										/* Display Rect - bottom = 20	*/
								0x00, 0x3a,										/* Display Rect - right = 58	*/
								0x04,											/* Item Type					*/
								0x05,											/* Button Title Length			*/
								'P','r','i','n','t',							/* Button Title					*/
								0x00,											/* pad to even word				*/
							  };											/* end of DITL 						*/

GrafPtr			SavedPort;													/* port set when our code entered						*/

PixMapHandle	thePixMapHandle;											/* the PixMap (Mac image data structure)				*/
CTabHandle		theCTabHandle;												/* our custom made gray scale color table				*/
PaletteHandle	thePaletteHandle;											/* Gray Scale Palette									*/
unsigned short	theColor;													/* used setting up color table							*/
PicHandle		thePicHandle;												/* used for saving pictures								*/

DialogPtr		theDialogPtr;												/* dialog box (window) graphics displayed in			*/
Handle			theDITLHandle;												/* item list for dialog box (we make it in memory)		*/
unsigned short	DITLByte;													/* used making dialog item list DITL					*/
Rect			theDialogRect;												/* rectangle defining dialog window's size				*/
short			theDialogMinWidth;											/* gotta leave enough room for the buttons...			*/
short			theItemHit;													/* item clicked on in dialog box						*/
Handle			theItemHandle;												/* Handle to an item in dialog box						*/
Rect			theItemRect;												/* bounding rectangle of item							*/
short			theItemType;												/* item's type											*/
Rect			OKButtonRect;												/* final location rect, used for outlining button		*/

CursHandle		iBeamCursHandle;											/* to replace text window's cursor						*/
CursHandle		WatchCursHandle;											/* reminder whilst printing								*/

THPrint			thePrintRecordHandle;										/* holds printing info									*/
short			CopyNum;													/* copy number being printed							*/
TPPrPort		thePrintPort;												/* where we'll print to									*/
Rect			thePrintImageRect;											/* same as PixMap's bounds to start						*/
Rect			thePrintDestinationRect;									/* rect to print image into								*/
short			PaperRectHorzCenter;										/* horizontal center point of printing Rect				*/
short			ImageRectHorzCenter;										/* horizontal center point of image Rect				*/
TPrStatus		PrPicStatus;												/* printing status if spooled							*/
Boolean			PrintBlewUp;												/* Printing Status										*/
Boolean			PrintCancelled;												/* user clicked cancel in Page Setup or Print			*/

Point			theSFPoint;													/* where to put Save As dialog							*/
SFReply			theSFReply;													/* reply from SFPutFile									*/
OSErr			theOSErr;													/* for all the file manager calls						*/
short			theFileRefNum;												/* file reference number								*/
char			ZerosBuffer [512];											/* for header of PICT file								*/
short			i;															/* what good program doesn't have a variable i in it?	*/
long			theCount;													/* Number of bytes to write and written					*/


	thePixMapHandle = NewPixMap();											/* get a new pixmap data structure						*/
	if (thePixMapHandle == nil)												/* nil means we didn't have enough memory				*/
	  {
		printf("\007There isn't enough free memory to display the image!\n\n");
		return;																/* bail outta here										*/
	  }
	DisposePtr((Ptr) &(**thePixMapHandle).baseAddr);						/* get rid of the allocated data area					*/
	(**thePixMapHandle).baseAddr = nil;										/* in case we error out setting up						*/
	(**thePixMapHandle).rowBytes = HPixels + 0x8000;						/* finish setting up PixMap								*/
	SetRect(&(**thePixMapHandle).bounds, 0, 0, HPixels, VPixels);			/* set bounding rectangle								*/
	(**thePixMapHandle).pixelSize = 8;										/* total # bits per pixel								*/
	(**thePixMapHandle).cmpCount = 1;										/* 1 component per pixel								*/
	(**thePixMapHandle).cmpSize = 8;										/* 8 bits per component									*/
	theCTabHandle = (CTabHandle) NewHandle(8 + (256 * 8));					/* allocate our own 256 color table						*/
	if (theCTabHandle == nil)
	  {
		DisposePixMap(thePixMapHandle);										/* we're dead, get rid of PixMap's memory				*/
		printf("\007There isn't enough free memory to display the image!\n\n");
		return;																/* bail outta here										*/
	  }
	HNoPurge((Handle) theCTabHandle);										/* make sure it can't move around in memory				*/
	(**theCTabHandle).ctSeed = GetCTSeed();									/* gives us a unique number								*/
	(**theCTabHandle).ctFlags = 0x4001;										/* 0 in high bit=PixMap, 1=Graphics Device				*/
	(**theCTabHandle).ctSize = 255;											/* # colors - 1 (256 - 1)								*/
	for (theColor = 0; theColor < 256; theColor++) 							/* setup color table to uniform gray scale				*/
	  {
		(**theCTabHandle).ctTable[theColor].value = theColor;				/* set value of color 0..255							*/
		(**theCTabHandle).ctTable[theColor].rgb.red = theColor << 8;		/* set red = green = blue								*/
		(**theCTabHandle).ctTable[theColor].rgb.green = theColor << 8;
		(**theCTabHandle).ctTable[theColor].rgb.blue = theColor << 8;
	  }
	DisposCTable((**thePixMapHandle).pmTable);								/* get rid of allocated color table (2 colors only!)	*/
	(**thePixMapHandle).pmTable = theCTabHandle;							/* put gray scale color table into PixMap				*/
																			/* create new palette using our gray scale color table	*/
	thePaletteHandle = NewPalette (256, theCTabHandle, pmAnimated + pmExplicit, 0);
	if (thePaletteHandle == nil)
	  {
		DisposePixMap(thePixMapHandle);										/* we're dead, get rid of PixMap & color table's  mem	*/
		printf("\007There isn't enough free memory to display the image!\n\n");
		return;																/* bail outta here										*/
	  }
	
	theDITLHandle = NewHandle(sizeof(theDITLData));							/* get a Handle to fit DITL data						*/
	if (theDITLHandle == nil)
	  {
		DisposePixMap(thePixMapHandle);										/* we're dead, get rid of PixMap & color table's  mem	*/
		printf("\007There isn't enough free memory to display the image!\n\n");
		return;																/* bail outta here										*/
	  }
	HLock(theDITLHandle);													/* so our copy doesn't blow up...						*/
	for (DITLByte = 0; DITLByte < sizeof(theDITLData); DITLByte++)			/* copy DITL data to Handle's mem						*/
		*(*theDITLHandle + DITLByte) = theDITLData[DITLByte];
	
	theDialogRect = (**thePixMapHandle).bounds;								/* start with the image's boundaries - TopLeft=0,0		*/
	theDialogRect.bottom += ButtonHeight + (2 * ButtonTopSlop);				/* enlarge vertically to fit buttons					*/
	theDialogMinWidth = OKButtonWidth + SaveButtonWidth + PrintButtonWidth + (4 * ButtonSideSlop);	/* minimum width to fit buttons + space between	*/
	if (theDialogRect.right < theDialogMinWidth)
		theDialogRect.right = theDialogMinWidth;							/* image is smaller than width, enlarge dialog box		*/
	OffsetRect(&theDialogRect, SideSlop, TopSlop);							/* position in top-left corner of monitor				*/
	
	theDialogPtr = NewCDialog(nil, &theDialogRect, theImageTitle, FALSE, documentProc, (WindowPtr) -1, FALSE, 12345, theDITLHandle);
	if (theDialogPtr == nil)
	  {
		DisposePixMap(thePixMapHandle);										/* we're dead, get rid of PixMap & color table's  mem	*/
		DisposePalette(thePaletteHandle);									/* get rid of Palette Handle							*/
		DisposeHandle(theDITLHandle);										/* get rid of DITL Handle								*/
		printf("\007There isn't enough free memory to display the image!\n\n");
		return;																/* bail outta here										*/
	  }

	(**thePixMapHandle).baseAddr = theDataPtr;								/* fill in with address of pixel data					*/
	
																			/* move the OK button to its proper place in window		*/			
	GetDItem(theDialogPtr, OKButtonItemID, &theItemType, &theItemHandle, &OKButtonRect);
	MoveControl((ControlHandle) theItemHandle, (theDialogRect.right - theDialogRect.left) - OKButtonWidth - ButtonSideSlop,
												(theDialogRect.bottom - theDialogRect.top) - ButtonHeight - ButtonTopSlop);
	OffsetRect(&OKButtonRect, (theDialogRect.right - theDialogRect.left) - OKButtonWidth - ButtonSideSlop,
								(theDialogRect.bottom - theDialogRect.top) - ButtonHeight - ButtonTopSlop);
	SetDItem(theDialogPtr, OKButtonItemID, theItemType, theItemHandle, &OKButtonRect);
	InsetRect(&OKButtonRect, -4, -4);										/* now we'll use this for outlining the OK button		*/
	
																			/* move the Print button to its proper place in window	*/			
	GetDItem(theDialogPtr, PrintButtonItemID, &theItemType, &theItemHandle, &theItemRect);
	MoveControl((ControlHandle) theItemHandle, ButtonSideSlop, (theDialogRect.bottom - theDialogRect.top) - ButtonHeight - ButtonTopSlop);
	OffsetRect(&theItemRect, ButtonSideSlop, (theDialogRect.bottom - theDialogRect.top) - ButtonHeight - ButtonTopSlop);
	SetDItem(theDialogPtr, PrintButtonItemID, theItemType, theItemHandle, &theItemRect);

																			/* move the Save button to its proper place in window	*/			
	GetDItem(theDialogPtr, SaveButtonItemID, &theItemType, &theItemHandle, &theItemRect);
	MoveControl((ControlHandle) theItemHandle, ((theDialogRect.right - theDialogRect.left) / 2) - (SaveButtonWidth / 2),
										(theDialogRect.bottom - theDialogRect.top) - ButtonHeight - ButtonTopSlop);
	OffsetRect(&theItemRect, ((theDialogRect.right - theDialogRect.left) / 2) - (SaveButtonWidth / 2),
										(theDialogRect.bottom - theDialogRect.top) - ButtonHeight - ButtonTopSlop);
	SetDItem(theDialogPtr, SaveButtonItemID, theItemType, theItemHandle, &theItemRect);

	SetPalette(theDialogPtr, thePaletteHandle, FALSE);						/* use our gray scale palette (colors)					*/
	ShowWindow(theDialogPtr);												/* make it visible now									*/
	DrawControls(theDialogPtr);												/* draw the buttons										*/

	iBeamCursHandle = GetCursor(iBeamCursor);								/* get I Beam (text) cursor								*/
	WatchCursHandle = GetCursor(watchCursor);								/* get watch cursor										*/
	do
	  {
		InitCursor();														/* change cursor to normal arrow						*/
		GetPort(&SavedPort);												/* get current drawing port								*/
		SetPort(theDialogPtr);												/* set drawing port to the front window					*/
									
		ForeColor(blackColor);												/* make sure we draw in black							*/
		BackColor(whiteColor);												/* make sure the background is white					*/
		PenNormal ();														/* all black pattern, 1x1 pixel							*/
		PenSize (3, 3);														/* for outlining OK button								*/
		FrameRoundRect(&OKButtonRect, 16, 16);								/* draw OK button outline								*/
		PenNormal ();														/* back to normal again									*/
																			/* copy the PixMap into the window						*/
		CopyBits((BitMap*)*thePixMapHandle, &theDialogPtr->portBits, &(**thePixMapHandle).bounds, &(**thePixMapHandle).bounds, srcCopy, nil);
		SetPort(SavedPort);													/* return drawing port to current port					*/
		ModalDialog(nil, &theItemHit);										/* handle dialog's events, tell us about button clicks	*/
		switch (theItemHit)
		  {
			case PrintButtonItemID:												/* the print button was pressed							*/
				PrintBlewUp = TRUE;												/* assume something blew up								*/
				PrintCancelled = FALSE;											/* assume user didn't cancel out						*/
				PrOpen();														/* open the printer driver (whatever the printer is)	*/
				if (PrError() == noErr)											/* check for printer driver errors						*/
				  {
					thePrintRecordHandle = (THPrint) NewHandle(sizeof(TPrint));	/* get mem for print data record						*/
					if (thePrintRecordHandle != nil)
					  {
						PrintDefault(thePrintRecordHandle);						/* set data record to default values for printer		*/
						PrValidate(thePrintRecordHandle);						/* make sure the record is valid according to driver	*/
						if (!PrStlDialog(thePrintRecordHandle))					/* put up Page Setup dialog, check for cancel			*/
							PrintCancelled = TRUE;								/* user cancelled										*/
						else
						  {
							if (!PrJobDialog(thePrintRecordHandle))				/* put up Print dialog box, check for cancel			*/
								PrintCancelled = TRUE;							/* user cancelled										*/
							else
							  {
								SetCursor(&**WatchCursHandle);					/* set cursor to watch cursor							*/
								(**thePrintRecordHandle).prJob.iFstPage = 1;	/* we know there's only one page...						*/
								(**thePrintRecordHandle).prJob.iLstPage = 1;	/* we know there's only one page...						*/
																				/* loop for each copy, not the best way, quick & dirty	*/
								for (CopyNum = 1; CopyNum <= (**thePrintRecordHandle).prJob.iCopies; CopyNum++)
								  {
									if (PrError() == noErr)						/* check for errors first								*/
									  {
										thePrintPort = PrOpenDoc(thePrintRecordHandle, nil, nil);	/* open a GrafPort to write into					*/	
										if (PrError() == noErr)
										  {
											PrOpenPage(thePrintPort, nil);							/* open a page for printing							*/
											if (PrError() == noErr)
											  {
												thePrintImageRect = (**thePixMapHandle).bounds;		/* start with the PixMap's rect						*/
												InsetRect(&thePrintImageRect, 0, 1);				/* inset 1 vertically to get rid of artifact...		*/
												thePrintDestinationRect = thePrintImageRect;		/* set destination rect same as source				*/
												PaperRectHorzCenter = ((**thePrintRecordHandle).rPaper.right + (**thePrintRecordHandle).rPaper.left) / 2;
												ImageRectHorzCenter = thePrintDestinationRect.right / 2;
																									/* offset destination rect so it's centered horz &	*/
																									/*   leave room for title							*/
												OffsetRect(&thePrintDestinationRect, PaperRectHorzCenter - ImageRectHorzCenter, PrintTitleOffset);
												PenNormal;											/* set pen to normal								*/
												TextFont(helvetica);								/* select helvetica font							*/
												TextFace(bold);										/* make the font bold								*/
												TextSize(12);										/* set to 12 point									*/
																									/* move pen so title will be centered on page		*/
												MoveTo(((((**thePrintRecordHandle).rPaper.right - (**thePrintRecordHandle).rPaper.left) / 2) +
														(**thePrintRecordHandle).rPaper.left) - ((StringWidth(theImageTitle)) / 2), 10);
												DrawString(theImageTitle);							/* draw the title									*/
																									/* draw the image									*/
												CopyBits((BitMap*)*thePixMapHandle, &((GrafPtr)thePrintPort)->portBits, &thePrintImageRect, &thePrintDestinationRect, srcCopy, nil);
											  }
											PrClosePage(thePrintPort);								/* close the page									*/
										  }
										PrCloseDoc(thePrintPort);									/* close the GrafPort								*/
										PrintBlewUp = FALSE;										/* we made it, all OK								*/
																									/* if spooled, start the spooled printing			*/
										if (((**thePrintRecordHandle).prJob.bJDocLoop == bSpoolLoop) && (PrError() == noErr))
											PrPicFile(thePrintRecordHandle, nil, nil, nil, &PrPicStatus);
									  }
								  }
							  }
						  }
					  }
				  }
				PrClose();															/* close the printer driver								*/
				if (PrintBlewUp && !PrintCancelled)
					SysBeep(3);														/* all the user gets is a beep if something screwed up!	*/
				break;
			case SaveButtonItemID:													/* save button was pressed								*/
				theSFPoint.h = 64;													/* somewhere away from the menu bar						*/
				theSFPoint.v = 64;
				SFPutFile(theSFPoint, "\pSave PICT File As:", "\pUntitled", nil, &theSFReply);
				if (theSFReply.good)
				  {
					SetCursor(&**WatchCursHandle);									/* set cursor to watch cursor, loop does an InitCursor to clean up	*/
					theOSErr = Create(theSFReply.fName, theSFReply.vRefNum, 'ttxt', 'PICT');	/* create file											*/
					if ((theOSErr != noErr) && (theOSErr != dupFNErr))							/* we'll tolerate a file exists error					*/
						SysBeep(1);
					else
					  {
						if (theOSErr == dupFNErr)
						  {
							theOSErr = FSDelete(theSFReply.fName, theSFReply.vRefNum);			/* file exists, delete it								*/
							if (theOSErr == noErr)
								theOSErr = Create(theSFReply.fName, theSFReply.vRefNum, 'ttxt', 'PICT');	/* create it again							*/
						  }
						if (theOSErr == noErr)													/* this could be from original or 2nd create, or delete	*/
						  {
							theOSErr = FSOpen(theSFReply.fName, theSFReply.vRefNum, &theFileRefNum);		/* open the file							*/
							if (theOSErr == noErr)
							  {
								for (i = 0; i < 512; i++)										/* header array is 512 bytes of zeros					*/
									ZerosBuffer[i] = 0;											/* zero out the header array							*/
								theCount = 512;													/* # bytes to write out									*/
								theOSErr = FSWrite(theFileRefNum, &theCount, &ZerosBuffer);		/* write the header										*/
								if (theOSErr == noErr)
								  {
									GetPort(&SavedPort);										/* get current drawing port								*/
									SetPort(theDialogPtr);										/* set drawing port to the front window					*/
									ClipRect(&(**thePixMapHandle).bounds);						/* clip to the copybits rect							*/
									thePicHandle = OpenPicture(&(**thePixMapHandle).bounds);	/* open the picture to save a copy						*/
									PenNormal ();												/* black, solid pattern, 1x1 pixel						*/
									ForeColor(blackColor);										/* make sure we draw in black							*/
									BackColor(whiteColor);										/* make sure the background is white					*/
																								/* copy the PixMap into the window						*/
									CopyBits((BitMap*)*thePixMapHandle, &theDialogPtr->portBits, &(**thePixMapHandle).bounds, &(**thePixMapHandle).bounds, srcCopy, nil);
									ClosePicture();												/* we're done saving the picture						*/
									SetPort(SavedPort);											/* return drawing port to current port					*/
									HLock((Handle) thePicHandle);								/* lock the handle so pointer correct during FSWrite	*/
									theCount = GetHandleSize((Handle) thePicHandle);			/* get the size of the PICT								*/
									theOSErr = FSWrite(theFileRefNum, &theCount, *thePicHandle);/* write out the PICT									*/
									if (theOSErr != noErr)
										SysBeep(1);												/* just a beep on errors								*/
									KillPicture(thePicHandle);									/* get rid of the memory used by the picture			*/
								  }
								theOSErr = FSClose(theFileRefNum);								/* if we opened it, close regardless of other errors	*/
								if (theOSErr != noErr)
									SysBeep(1);													/* just a beep on errors								*/
							  }
						  }
					  }
				  }
				break;
		  }
	  }
	while (theItemHit != OKButtonItemID);				/* keep looping until the OK button is hit				*/

	DisposeDialog(theDialogPtr);						/* we're done, get rid of Dialog AND theDITLHandle		*/
	DisposePixMap(thePixMapHandle);						/* we're done, get rid of PixMap & color table's  mem	*/
	DisposePalette(thePaletteHandle);					/* we're done, get rid of Palette Handle				*/
	SetCursor(&**iBeamCursHandle);						/* set cursor to I Beam (text) cursor					*/

}








