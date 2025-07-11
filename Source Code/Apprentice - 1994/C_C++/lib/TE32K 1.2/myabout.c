#define LENGTH	20



DoAboutBox(doWait)
int	doWait;
{
Rect			creditRect, lineRect;
GrafPtr			oldPort;
WindowPtr		creditWPtr;
EventRecord		theEvent;
long			iticks;
char			*line1 = "TE32K Demo";
char			*line2 = "Written by Roy Wood";
char			*line3 = "�1992 Silicon Angst Software";
char			*line4 = "122 Britannia Avenue";
char			*line5 = "London, Ontario, Canada N6H 2J5";
char			*line6 = "(519) 438-3177";
int				length,oldLength,countDown,i;
int				x[LENGTH][2],y[LENGTH][2],vx[2],vy[2];
long			finalTicks;

	GetPort(&oldPort);
	
	SetRect(&creditRect, 75,75,425,200);
	creditWPtr = NewWindow((WindowPeek) 0L, &creditRect, "\1x", FALSE, dBoxProc, (WindowPtr) -1L, FALSE, 0L);
	SetPort(creditWPtr);
	
	CenterWindow(creditWPtr);
	ShowWindow(creditWPtr);

MAIN:
	
	BeginUpdate(creditWPtr);
	EndUpdate(creditWPtr);
	
	creditRect = creditWPtr->portRect;
	SetRect(&lineRect, 100,10,350,25);
	
	BackPat(black);
	EraseRect(&creditRect);
	
	TextMode(srcXor);
	TextSize(12);
	TextFont(3);
	TextFace(bold);
	TextBox(line1, strlen(line1), &lineRect, teJustCenter);
	OffsetRect(&lineRect, 0 , 20);
	
	TextSize(10);
	TextBox(line2, strlen(line2), &lineRect, teJustCenter);
	
	TextFace(0);
	
	OffsetRect(&lineRect, 0 , 25);
	TextBox(line3, strlen(line3), &lineRect, teJustCenter);
	OffsetRect(&lineRect, 0 , 15);
	TextBox(line4, strlen(line4), &lineRect, teJustCenter);
	OffsetRect(&lineRect, 0 , 15);
	TextBox(line5, strlen(line5), &lineRect, teJustCenter);
	OffsetRect(&lineRect, 0 , 15);
	TextBox(line6, strlen(line6), &lineRect, teJustCenter);

	PenNormal();
	PenMode(patXor);
	
	GetDateTime(&randSeed);
	
	x[0][0] = 50;
	x[0][1] = 10;
	y[0][0] = 45;
	y[0][1] = 55;
	
	vx[0] = 2;
	vx[1] = -4;
	vy[0] = 3;
	vy[1] = 1;
	
	countDown = 1;
	length = 1;
	oldLength = 0;
	
	creditRect = creditWPtr->portRect;
	creditRect.right = 100;
	
	MoveTo((int) x[0][0],(int) y[0][0]);
	LineTo((int) x[0][1],(int) y[0][1]);
	
	iticks = TickCount();
	
	do
	{
		for (i=0;i<=1;i++)
		{
			x[length][i] = x[oldLength][i] + vx[i];
			
			if (x[length][i] < creditRect.left + 5)
				vx[i] = (vx[i] > 0) ? vx[i] : -vx[i];
			else if (x[length][i] > creditRect.right - 5)
				vx[i] = (vx[i] > 0) ? -vx[i] : vx[i];
			
			y[length][i] = y[oldLength][i] + vy[i];
			
			if (y[length][i] < creditRect.top + 5)
				vy[i] = (vy[i] > 0) ? vy[i] : -vy[i];
			else if (y[length][i] > creditRect.bottom - 5)
				vy[i] = (vy[i] > 0) ? -vy[i] : vy[i];
		}
		
		MoveTo((int) x[length][0],(int) y[length][0]);
		LineTo((int) x[length][1],(int) y[length][1]);	
		
		oldLength = length;
		
		if (++length >= LENGTH)
			length = 0;
		
		
		if (countDown)
			countDown = length;
		
		if (!countDown)
		{
			MoveTo((int) x[length][0],(int) y[length][0]);
			LineTo((int) x[length][1],(int) y[length][1]);	
		}
		
		SystemTask();
		GetNextEvent(everyEvent, &theEvent);
		
		Delay(1L,&finalTicks);
		
		if (theEvent.what==updateEvt && theEvent.message==(long) creditWPtr)
			goto MAIN;
		
	}	while ((!doWait && (theEvent.what != mouseDown))||(doWait && (TickCount() < iticks + doWait)));
	
	
	DisposeWindow(creditWPtr);
	
	SetPort(oldPort);
}

