/***************************************************************************************************
	main.c

		Source code for the 'Until That Day' application.

		This simple application displays a small window showing the date/time remaining
		until a specfied target date/time.

		This is hardly a perfect example of how a Mac application should be built.

		Pardon the sloppy code - this was just slapped together to make it work.

		Almost no error checking is done, so beware.

		If you build a more spiffy version of this application, consider sending
		me a copy. It's not required. I'm just curious what others might do with
		this code.

		Mark W. DeNyse
		March, 1994

		CIS:		 74776,755
		AOL:		 markdenyse
		Internet:    MarkDeNyse@aol.com
		
		Converted to CodeWarrior 6 by Paul Celestin

****************************************************************************************************/

// Assume a #include "MacHeaders" in your project prefix
#include <Traps.h>
#include <Packages.h>

/***************************************************************************************************
													Constants
****************************************************************************************************/
enum {
	kAppleMenuID				= 1,
		kAboutItem				= 1,

	kFileMenuID					= 2,
		kSetTargetDateItem	= 1,
		kQuitItem				= 3
} MenuConstants;

enum {
	kSleepTime					= 1,

	kWindowID					= 128,
	kHandCursID					= 128,
	kGrabbedHandlCursID		= 129,

	kSignature					= '????',
	kDateTimeFileType			= '�MWD'
} MiscConstants;

enum {
	kStrID						= kWindowID,

	kTimeUntilIndex			= 1,
	kUpTimeIndex,
	kOneDayUpTimeIndex,
	kManyDaysUpTimeIndex,
	kAlreadyPastIndex,
	kDateTimeFileNameIndex
} StringConstants;

enum {
	kSetTargetID				= kWindowID,

	kOKButton					= 1,
	kCancelButton,
	kTargetDate,
	kTargetTime
} GetDateTimeConstants;

#define kOneMinute		((unsigned long)60)
#define kOneHour			((unsigned long)3600)
#define kHoursInADay		((unsigned long)24)
#define kOneDay			(kOneHour * kHoursInADay)

#define OK					1
#define Cancel				2

#define	HiShort(longNum)	(short)(((longNum) >> 16) & 0xFFFF)
#define	LoShort(longNum)	(short)((longNum) & 0xFFFF)

/***************************************************************************************************
													Global variables
****************************************************************************************************/
static short				gVRefNum;
static Boolean				gHasWNE;
static WindowPtr			gWindow;
static RgnHandle			gSleepRgn;
static long					gLastDateTime;
static long					gTargetDate;
static NumberParts		gPartsTable;
static DateCacheRecord	gDateCache;


/***************************************************************************************************
													Function Prototypes
****************************************************************************************************/
void				main(void);

static void		Initialize(void);

static Boolean	TrapAvailable(short theTrap);
static void		CopyPString(ConstStr255Param srcString, Str255 destString);
static void		InsertStrAt(Str255 theStr, Str255 insertStr, short index);
static void		MakeTimeString(long timePart, Str255 theStr);
static void		MakeRelativeTimeString(unsigned long theTime, Str255 timeStr);
static void		SetMemory(register Ptr thePtr, register long dataSize, register char initValue);

static Boolean	GetTargetDate(long *targetDate);
static void		ReadFile(long *targetDate);
static void		WriteFile(long targetDate);

static void		DoIdle(Boolean fullRedraw);
static Boolean	DoCommand(long theCommand);
static Boolean	ProcessEvent(void);


/***************************************************************************************************
	TrapAvailable

		Return true if the specified trap exists. Snarfed from the TCL sources.

		-> theTrap		The trap to test for.

		returns:			true if the specified trap exists, else false.

****************************************************************************************************/
static Boolean TrapAvailable(short theTrap)
{
	TrapType tType;
	short    numToolBoxTraps;

				
	tType = (theTrap & 0x800) > 0 ? ToolTrap : OSTrap;
	
	if (NGetTrapAddress(_InitGraf, ToolTrap) == NGetTrapAddress(0xAA6E, ToolTrap))
		numToolBoxTraps = 0x200;
	else
		numToolBoxTraps = 0x400;
	
	if (tType == ToolTrap)
	{
		theTrap &= 0x7FF;
		if (theTrap >= numToolBoxTraps)
			theTrap = _Unimplemented;
	}

	return(NGetTrapAddress(theTrap, tType) != NGetTrapAddress(_Unimplemented, ToolTrap));
}


/***************************************************************************************************
	CopyPString

		Copy a Pascal string. Snarfed from the TCL sources.

		-> srcString			The string to copy
		<- destString			Where to put the copy

****************************************************************************************************/
static void CopyPString(ConstStr255Param srcString, Str255 destString)
{
	BlockMove(srcString, destString, srcString[0] + 1L);
	return;
}


/***************************************************************************************************
	InsertStrAt

		Given a string containing a '%n' token (where n is 1-9), insert a string replacing
		the '%n' token.

		Example:
			CopyPString("\pThe color of the ball is %1.", string1);
			CopyPString("\pred", string2);
			InsertStrAt(string1, string2, 1);

			result: string1 = "\pThe color of the ball is red."


		<-> theStr			The string to insert insertStr into
		 -> insertStr		The string to insert into theStr
		 -> index			The '%n' index in theStr to insert insertStr into.

****************************************************************************************************/
static void InsertStrAt(Str255 theStr, Str255 insertStr, short index)
{
	short				i;
	Str255			newStr;
	StringHandle	theStrHdl;
	char				tokenChar;


	CopyPString(theStr, newStr);

	for (i = 1; i < theStr[0]; i++)
		if ((theStr[i] == '%') && (theStr[i + 1] == ('0' + index)))
		{
			BlockMove((Ptr)theStr + 1,	    (Ptr)newStr + 1,				   	i - 1);
			BlockMove((Ptr)insertStr + 1,  (Ptr)newStr + i,				   	(long)insertStr[0]);
			BlockMove((Ptr)theStr + i + 2, (Ptr)newStr + i + insertStr[0], theStr[0] - i - 1);

			newStr[0] = theStr[0] + insertStr[0] - 2;
			break;
		}

	CopyPString(newStr, theStr);

	return;
}


/***************************************************************************************************
	MakeTimeString

		Given a number, convert it to a string, adding a leading zero if necessary.

		-> timePart			The value to convert
		<- theStr			The string form of timePart

****************************************************************************************************/
static void MakeTimeString(long timePart, Str255 theStr)
{
	theStr[0] = 0;
	NumToString(timePart, theStr);
	if (theStr[0] == 1)											// Need a leading zero?
	{
		theStr[2] = theStr[1];
		theStr[1] = '0';
		theStr[0] = 2;
	}
	return;
}


/***************************************************************************************************
	MakeRelativeTimeString

		Create a string that represents the relative time of theTime, expressed as:
						nn Days hh:mm:ss
			-or-		hh:mm:ss

		The �nn Days� portion of the string is only outputted if the time is greater
		than one day.

		-> theTime		An unsigned long expressing time in seconds
		<- timeStr		Str255 expressing theTime

****************************************************************************************************/
static void MakeRelativeTimeString(unsigned long theTime, Str255 timeStr)
{
	Str255			aStr;
	Str255			bStr;
	unsigned long	days;
	unsigned long	hours;
	unsigned long	minutes;
	unsigned long	seconds;


	timeStr[0] = 0;

	days		= 0L;
	hours		= 0L;
	minutes	= 0L;
	seconds	= 0L;

	// Calculate the days, hours, minutes and seconds.
	if (theTime >= kOneDay)										// Only do days if caller wants it
	{
		days = theTime / kOneDay;								// Calculate days up
		theTime -= (days * kOneDay);							// Leave remainder
	}

	if (theTime >= kOneHour)									// Any hours?
	{
		hours = theTime / kOneHour;							// Calculate hours up
		theTime -= (hours * kOneHour);						// Leave remainder
	}

	if (theTime >= kOneMinute)									// Any minutes?
	{
		minutes = theTime / kOneMinute;
		theTime -= (minutes * kOneMinute);
	}

	seconds = theTime;											// Seconds are what's left

	GetIndString(timeStr, kStrID, kUpTimeIndex);			// Get the hh:mm:ss string

	MakeTimeString(hours, bStr);
	InsertStrAt(timeStr, bStr, 1);							// Insert the hours
	MakeTimeString(minutes, bStr);
	InsertStrAt(timeStr, bStr, 2);							// Insert the minutes
	MakeTimeString(seconds, bStr);
	InsertStrAt(timeStr, bStr, 3);							// Insert the seconds

	if (days != 0)
	{
		GetIndString(aStr, kStrID, (days == 1) ? kOneDayUpTimeIndex : kManyDaysUpTimeIndex);
		InsertStrAt(aStr, timeStr, 1);						// Insert hh:mm:ss into days string
		if (days > 1)
		{
			NumToString(days, bStr);
			InsertStrAt(aStr, bStr, 2);						// Insert the number of days
		}
		CopyPString(aStr, timeStr);							// Now copy into timeStr
	}

	return;
}


/***************************************************************************************************
	SetMemory

		Set a bunch of bytes to the same value.

		-> thePtr			The address of the memory to set
		-> dataSize			The number of bytes to set
		-> initValue		The value to initialize each byte to

****************************************************************************************************/
static void SetMemory(register Ptr thePtr, register long dataSize, register char initValue)
{
	register short	index;


	for (index = 0; index < dataSize; index++)
		thePtr[index] = initValue;
	return;
}


/***************************************************************************************************
	GetTargetDate

		Display a modal dialog that prompts the user to enter a date and time.
		The formats of the date/time can be entered by the user in any form that
		the String2Date/String2Time can decipher. shortDate form is used for the
		date field as the default format. 

		<-> targetDate		On input, the date/time to display in the dialog.
								On output, the date/time the user entered if the user
								pressed the OK button.

		returns:				true if the user clicked the OK button, else false.

****************************************************************************************************/
static Boolean GetTargetDate(long *targetDate)
{
	DialogPtr			theDialog;
	GrafPtr				savePort;
	short					itemHit;
	short					kind;
	Handle				h;
	Rect					r;
	Str255				theStr;
	String2DateStatus	theStatus;
	long					lengthUsed;
	LongDateRec			dateTime;


	GetPort(&savePort);
	theDialog = GetNewDialog(kSetTargetID, nil, (WindowPtr)-1);
	// Should check for an error here�
	SetPort(theDialog);

	GetDItem(theDialog, OK, &kind, &h, &r);
	PenSize(3, 3);
	InsetRect(&r, -4, -4);
	FrameRoundRect(&r, 16, 16);
	PenSize(1, 1);

	IUDateString((unsigned long)*targetDate, shortDate, theStr);
	GetDItem(theDialog, kTargetDate, &kind, &h, &r);
	SetIText(h, theStr);
	SelIText(theDialog, kTargetDate, 0, 9999);

	IUTimeString((unsigned long)*targetDate, shortDate, theStr);
	GetDItem(theDialog, kTargetTime, &kind, &h, &r);
	SetIText(h, theStr);

	do {
		ModalDialog(nil, &itemHit);
	} while ((itemHit != OK) && (itemHit != cancel));

	if (itemHit == OK)
	{
		SetMemory((Ptr)&dateTime, sizeof(dateTime), 0);

		GetDItem(theDialog, kTargetDate, &kind, &h, &r);
		GetIText(h, theStr);
		theStatus = String2Date((Ptr)&(theStr[1]), Length(theStr), &gDateCache, &lengthUsed, &dateTime);
		if (theStatus >= 0)
			Date2Secs(&dateTime.od.oldDate, (unsigned long *)targetDate);

		GetDItem(theDialog, kTargetTime, &kind, &h, &r);
		GetIText(h, theStr);
		theStatus = String2Time((Ptr)&(theStr[1]), Length(theStr), &gDateCache, &lengthUsed, &dateTime);
		if (theStatus >= 0)
			Date2Secs(&dateTime.od.oldDate, (unsigned long *)targetDate);
	}

	DisposDialog(theDialog);
	SetPort(savePort);

	return(itemHit == OK);
}


/***************************************************************************************************
	ReadFile

		Read the file containing the saved 'time till' date. If the file doesn't exist,
		return 0.

		<- targetDate		The date/time read from the file. If the file does not exist
								or could not be opened, 0 is returned.

****************************************************************************************************/
static void ReadFile(long *targetDate)
{
	OSErr	theErr;
	Str63 fileName;
	short	fRefNum;
	long	numBytes;


	*targetDate = 0;

	GetIndString(fileName, kStrID, kDateTimeFileNameIndex);
	theErr = FSOpen(fileName, gVRefNum, &fRefNum);
	if (theErr == noErr)
	{
		numBytes = sizeof(long);
		theErr = FSRead(fRefNum, &numBytes, (Ptr)targetDate);
		theErr = FSClose(fRefNum);
	}

	return;
}


/***************************************************************************************************
	WriteFile

		Save the 'time till' date in a file. If the file does not exist, create it.

		-> targetDate		The date/time to write to the file.

****************************************************************************************************/
static void WriteFile(long targetDate)
{
	OSErr	theErr;
	Str63 fileName;
	short	fRefNum;
	long	numBytes;


	GetIndString(fileName, kStrID, kDateTimeFileNameIndex);						// Get the file name

	theErr = FSOpen(fileName, gVRefNum, &fRefNum);									// Try to open it
	if (theErr == fnfErr)																	// Doesn't exist?
	{
		theErr = Create(fileName, gVRefNum, kDateTimeFileType, kSignature);	// Create it
		if (theErr == noErr)
			theErr = FSOpen(fileName, gVRefNum, &fRefNum);							// Now open it
	}

	if (theErr == noErr)
	{
		numBytes = sizeof(long);
		theErr = FSWrite(fRefNum, &numBytes, (Ptr)&targetDate);					// Write the data
		theErr = FSClose(fRefNum);															// Close the file
	}

	return;
}


/***************************************************************************************************
	DoIdle

		We've got a slice of free time, so update the date/time remaining.

		-> fullRedraw		If true, all parts of the window will be redrawn. If false,
								only remaining date/time string is redrawn.

****************************************************************************************************/
static void DoIdle(Boolean fullRedraw)
{
	Str255		theStr;
	Str32			theStr1;
	long			theDateTime;
	Point			mousePt;
	short			thePart;
	WindowPtr	macWindow;


	SetPort(gWindow);

	GetMouse(&mousePt);														// Set the cursor
	LocalToGlobal(&mousePt);
	thePart = FindWindow(mousePt, &macWindow);
	if (thePart == inContent)
		SetCursor(*GetCursor(kHandCursID));
	else
		InitCursor();

	if (fullRedraw)															// Redraw everything?
	{
		MoveTo(5, 14);
		GetIndString(theStr, kStrID, kTimeUntilIndex);
		IUDateString(gTargetDate, shortDate, theStr1);
		InsertStrAt(theStr, theStr1, 1);
		IUTimeString(gTargetDate, false, theStr1);
		InsertStrAt(theStr, theStr1, 2);
		DrawString(theStr);
	}

	GetDateTime((unsigned long *)&theDateTime);
	if ((unsigned long)theDateTime > (unsigned long)gLastDateTime)
	{
		gLastDateTime = theDateTime;

		if (theDateTime > gTargetDate)
			GetIndString(theStr, kStrID, kAlreadyPastIndex);
		else
			MakeRelativeTimeString(gTargetDate - theDateTime, theStr);

		MoveTo(5, 28);
		DrawString(theStr);
	}

	return;
}


/***************************************************************************************************
	DoCommand

		Handle a menu command.

		-> theCommand		The result from MenuSelect or MenuKey

		returns:				true if the application should quit, else false.

****************************************************************************************************/
static Boolean DoCommand(long theCommand)
{
	short		theMenu;
	short		itemNum;
	Str255	theDA;
	Boolean	running;


	running = true;
	theMenu = HiShort(theCommand);
	itemNum = LoShort(theCommand);

	switch (theMenu)
	{
		case kAppleMenuID:
		{
			if (itemNum == kAboutItem)
				Alert(256, nil);
			else
			{
				GetItem(GetMHandle(kAppleMenuID), itemNum, theDA);
				OpenDeskAcc(theDA);
			}
			break;
		}
		case kFileMenuID:
		{
			if (itemNum == kSetTargetDateItem)
			{
				if (GetTargetDate(&gTargetDate))
				{
					SetPort(gWindow);
					InvalRect(&qd.thePort->portRect);
					WriteFile(gTargetDate);
				}
			}
			else if (itemNum == kQuitItem)
				running = false;
			break;
		}
		default:
		{
			break;
		}
	}
	HiliteMenu(0);

	return(running);
}


/***************************************************************************************************
	ProcessEvent

		Process an event.

		returns:			true if the application should quit, else false.

****************************************************************************************************/
static Boolean ProcessEvent(void)
{
	Boolean		eventResult;
	EventRecord	macEvent;
	Boolean		running;


	running = true;
	if (gHasWNE)
		eventResult = WaitNextEvent(everyEvent, &macEvent, kSleepTime, gSleepRgn);
	else
	{
		SystemTask();
		eventResult = GetNextEvent(everyEvent, &macEvent);
	}

	if (!eventResult)
		DoIdle(false);
	else
	{
		switch (macEvent.what)
		{
			case mouseDown:
			{
				short			thePart;
				WindowPtr	macWindow;
				Rect			dragRect;
				RgnHandle	grayRgn;


				InitCursor();
				thePart = FindWindow(macEvent.where, &macWindow);
				switch (thePart)
				{
					case inMenuBar:
					{
						running = DoCommand(MenuSelect(macEvent.where));
						break;
					}
					case inSysWindow:
					{
						SystemClick(&macEvent, macWindow);
						break;
					}
					case inContent:
					case inDrag:
					{
						grayRgn = GetGrayRgn();
						dragRect = (**grayRgn).rgnBBox;
						InsetRect(&dragRect, 2, 2);
						SetCursor(*GetCursor(kGrabbedHandlCursID));
							DragWindow(macWindow, macEvent.where, &dragRect);
						SetCursor(*GetCursor(kHandCursID));
						break;
					}
					default:
					{
						break;
					}
				}
				break;
			}

			case keyDown:
			{
				InitCursor();
				if (macEvent.modifiers & cmdKey)
					running = DoCommand(MenuKey(macEvent.message & charCodeMask));
				break;
			}

			case updateEvt:
			{
				BeginUpdate((WindowPtr)macEvent.message);
					ClipRect(&qd.thePort->portRect);
					EraseRect(&qd.thePort->portRect);
					DoIdle(true);
				EndUpdate((WindowPtr)macEvent.message);
				break;
			}

			default:
			{
				break;
			}
		}
	}

	return(running);
}



/***************************************************************************************************
	Initialize

		Initialize the toolbox and application.

****************************************************************************************************/
static void Initialize(void)
{
	MenuHandle	theMenu;
	short			index;
	SysEnvRec	theWorld;
	Rect			theRect;
	Ptr			p;
	Itl4Handle	itl4;


	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	InitCursor();
	MaxApplZone();
	FlushEvents(everyEvent - diskMask, 0);

	GetVol(nil, &gVRefNum);

	gHasWNE			= false;
	gWindow			= nil;
	gLastDateTime	= 0;

	ReadFile(&gTargetDate);

	gSleepRgn = NewRgn();
	SetRect(&theRect, 1, 1, 3, 3);
	RectRgn(gSleepRgn, &theRect);

	SysEnvirons(1, &theWorld);
	if ((theWorld.machineType >= 0) && TrapAvailable(_WaitNextEvent))
		gHasWNE = true;

	for (index = kAppleMenuID; index <= kFileMenuID; index++)
	{
		theMenu = GetMenu(index);
		InsertMenu(theMenu, 0);
	}
	AddResMenu(GetMHandle(kAppleMenuID), 'DRVR');
	DrawMenuBar();

	gWindow = GetNewWindow(kWindowID, nil, (WindowPtr)(-1));
	SetPort(gWindow);
	TextFont(applFont);
	TextSize(9);
	TextMode(srcCopy);

	itl4 = (Itl4Handle)IUGetIntl(4);
	if (itl4 != nil)
	{
		p = (Ptr)(*itl4);
		BlockMove((Ptr)(p + (**itl4).defPartsOffset), (Ptr)&gPartsTable, sizeof(NumberParts));
	}

	InitDateCache(&gDateCache);

	return;
}


/***************************************************************************************************
	main

****************************************************************************************************/
void main(void)
{
	Boolean	running;


	Initialize();

	running = true;
	do {
		running = ProcessEvent();
	} while (running);
}
