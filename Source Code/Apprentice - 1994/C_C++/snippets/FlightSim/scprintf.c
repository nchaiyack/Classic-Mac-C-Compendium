#include "flight.h"


extern	GrafPtr			windManPort;


static	long	rtrnAdrs;
static	char	buffer[256];


int		My_printf(char	*shwStr);
OSErr	SendPort(char *buffer, int length);


tprintf()
{
	asm	{
		move.l	(sp)+,rtrnAdrs
		}
	sprintf(buffer);
	SendPort(buffer, strlen(buffer));
	asm	{
		move.l	rtrnAdrs,a0
		jmp		(a0)
		}
}

scprintf()
{
	asm	{
		move.l	(sp)+,rtrnAdrs
		}
	sprintf(buffer);
	My_printf(buffer);
	asm	{
		move.l	rtrnAdrs,a0
		jmp		(a0)
		}
}

flprintf()
{
	asm	{
		move.l	(sp)+,rtrnAdrs
		}
	sprintf(buffer);
/*	AppendTempFile(buffer);*/
	asm	{
		move.l	rtrnAdrs,a0
		jmp		(a0)
		}
}

static
_My_printf(shwStr)
char	*shwStr;
{
	char	buffer[256];
static	Rect	dispRect = {67, 54, 87, 166};

	ClipRect(&dispRect);
	EraseRect(&dispRect);
	MoveTo(dispRect.left + 10, dispRect.bottom - 4);
	strcpy(buffer, shwStr);
	ctop(buffer);
	DrawString(buffer);
}

static
My_printf(shwStr)
char	*shwStr;
{
	char	buffer[256];
static	Rect	dispRect = {21, 220, 40, 510};
	GrafPtr	savePort;

	GetPort(&savePort);
	SetPort(windManPort);
		ClipRect(&dispRect);
		EraseRect(&dispRect);
		MoveTo(230, 36);
		strcpy(buffer, (char *)shwStr);
		ctop(buffer);
		DrawString(buffer);
	SetPort(savePort);
}

static	IOParam		portParam;

/*** open printer port ***/
OpenSerPort()
{
	portParam.ioCompletion = 0L;
	portParam.ioNamePtr = (StringPtr)("\p.BOut");
	portParam.ioRefNum = 0;
	portParam.ioPermssn = fsWrPerm;
	portParam.ioMisc = 0L;

	PBOpen(&portParam, FALSE);
/*	if (portParam.ioResult)
		if (portParam.ioResult EQ -98)
			GenralAlert("\PAppleTalk connected; won't be able to print");
		else
			GenralAlert("\PError opening printer port");
*/
}

static
OSErr
SendPort(buffer, length)
char	*buffer;
int		length;
{
	portParam.ioBuffer = buffer;
	portParam.ioReqCount = length;
	portParam.ioPosMode = fsAtMark;

	PBWrite(&portParam, FALSE);
	buffer[portParam.ioActCount] = '\0';
	return(portParam.ioResult);
}


#define	KEYMAP		((long *)0x174)

KWait()
{
Boolean	ShiftDown();
Boolean	OptionDown();

	if (OptionDown())
		return;
	while(!ShiftDown());
	while(ShiftDown());
}

Boolean
ShiftDown()
{
	return(KEYMAP[1] & 1);
}

Boolean
OptionDown()
{
	return(KEYMAP[1] & 4);
}

Boolean
CommandDown()
{
	return((KEYMAP[1] & 0x8000) ? TRUE:FALSE);
}

Boolean
CapsDown()
{
	return(KEYMAP[1] & 2);
}

Boolean
CmndPeriod()
{
	EventRecord 	keyEvent;

	if (GetNextEvent(keyDownMask, &keyEvent))	/* ROM */
		if (keyEvent.modifiers & cmdKey)
			if ((keyEvent.message & charCodeMask) EQ '.')
				return(TRUE);

	return(FALSE);
}

/*** has the mouse button been pressed? ***/
Boolean
MousePress()
{
	EventRecord	msEvent;

	return(GetNextEvent(mDownMask, &msEvent));
}
