/*

append.c -- originally by Mark^Zimmermann
            ported to CW by Ken Long
            updated for CW6 on 950711

This snippet lets you append chosen text files together. Just pick
the first file, and then subsequent choices from the StdFile dialog
are tacked onto the end of it.

*/

// prototypes

void	main(void);
void	AppendFile(short refNum0, Str255 *fnXp, short vRefX);
int		GetTheFiles(Str255 *fnp, short *vRefp);
void	pStrCopy(char *p1, char *p2);
void	GiveUp(void);

//� set up theBufferfer of size 25,600 bytes (= 512 * 50) for copying through;
//� ktheBufferferSize must be an integer, < 32768

#define ktheBufferferSize  25600

char	theBuffer[ktheBufferferSize];

//� set up a little window to give info to the user...very static

WindowRecord myWinRecord;
WindowPtr infoWindow;
Str255 myString;

//� main routine to initialize everything under the sun (is all this
//� really necessary?) and then get the names of the files to join ... if
//� they open ok, start joining them, and quit when the user chooses
//� "Cancel"....

void main() 
{
	Str255 fn0, fnX;
	short vRef0, vRefX, refNum0, i;

	InitGraf (&qd.thePort);
	InitFonts ();
	FlushEvents (everyEvent, 0);
	InitWindows ();
	InitMenus ();
	TEInit ();
	InitDialogs (0L);
	InitCursor ();
	MaxApplZone ();
	
	infoWindow = GetNewWindow (128, &myWinRecord, (WindowPtr) - 1L);
	ShowWindow (infoWindow);
	SetPort (infoWindow);
	TextFont (0);
	for (i = 1; i <= 3; ++i)
	{
		MoveTo (4, 15 * i - 5);
		GetIndString ((StringPtr) myString, 128, i);
		DrawString ((StringPtr) myString);
	}

	if (GetTheFiles (&fn0, &vRef0))
	{
		if (FSOpen ((StringPtr) fn0, vRef0, &refNum0) != noErr)
			GiveUp ();
		while (GetTheFiles (&fnX, &vRefX))
			AppendFile (refNum0, &fnX, vRefX);
		FSClose (refNum0);
	}
}

//� Routine to append the contents of file X to file 0.

void AppendFile(short refNum0, Str255 *fnXp, short vRefX)
{
	short refNumX, err;
	long count;
	
	if (FSOpen ((StringPtr) fnXp, vRefX, &refNumX) != noErr)
		GiveUp ();
	if (SetFPos (refNumX, fsFromStart, 0L) != noErr)
		GiveUp ();
	if (SetFPos (refNum0, fsFromLEOF, 0L) != noErr)
		GiveUp ();
	for (;;)
	{
		count = ktheBufferferSize;
		err = FSRead (refNumX, &count, theBuffer);
		if (err != noErr && err != eofErr)
			GiveUp ();
		if (count == 0)
			break;
		if (FSWrite (refNum0, &count, theBuffer) != noErr)
			GiveUp ();
	}
	FSClose (refNumX);
	return;
}

//� following variables and routine do the standard files dialog
//� to get the name of the file use ... cribbed from the MiniEdit
//� example that comes with LSC....

static Point SFGwhere = { 90, 82 };
static SFReply reply;

int GetTheFiles (Str255 *fnp, short *vRefp)
{
	SFTypeList myTypes;

	myTypes[0]='TEXT';
	SFGetFile( SFGwhere, "\p", 0L, 1, myTypes, 0L, &reply);
	if (reply.good)
	{
		pStrCopy( (char *)reply.fName, (char *)fnp);
		*vRefp = reply.vRefNum;
		return(1);
	}
	else return(0);
}

//� routine to copy a pascal myString from one place to another.... used in
//� above Standard Files routine....

void pStrCopy(char *p1, char *p2)
{
	short len;
	
	len = *p2++ = *p1++;
	while (--len >= 0)
		*p2++ = *p1++;
	return;
}

//� routine to give up with a beep if an error occurs during opening the
//� file or fixing it....

void GiveUp ()
{
	SysBeep (10);
	ExitToShell ();
}
