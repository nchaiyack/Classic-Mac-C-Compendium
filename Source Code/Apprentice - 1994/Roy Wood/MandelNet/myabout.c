#define	DURATION		1



DoAboutBox(doWait)
int	doWait;
{
Rect			creditRect, lineRect;
GrafPtr			oldPort;
WindowPtr		creditWPtr;
EventRecord		theEvent;
long			iticks;
char			*line1 = "MandelNet 1.0";
char			*line2 = "Written by Roy Wood";

char			*line3 = "A parallel-processing Mandelbrot generator";
char			*line4 = "for use on AppleTalk networks";

char			*line5 = "©1991 Silicon Angst Software";
char			*line6 = "122 Britannia Avenue";
char			*line7 = "London, Ontario, Canada N6H 2J5";
char			*line8 = "(519) 438-3177";
int				count,i;
int				x[4],y[4],vx[4],vy[4],length,deltay,deltax;

long	endTime;


	GetPort(&oldPort);
	SetRect(&creditRect, 75,75,425,235);
	
	creditWPtr = NewWindow((WindowPeek) 0L, &creditRect, "\1x", FALSE, dBoxProc, (WindowPtr) -1L, FALSE, 0L);
	SetPort(creditWPtr);
	
	CenterWindow(creditWPtr);
	ShowWindow(creditWPtr);

MAINLOOP:

	SetRect(&lineRect, 100,10,350,25);
	
	creditRect = creditWPtr->portRect;
	
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
	
	TextFace(italic);
	OffsetRect(&lineRect, 0 , 20);
	TextBox(line3, strlen(line3), &lineRect, teJustCenter);
	OffsetRect(&lineRect, 0 , 15);
	TextBox(line4, strlen(line4), &lineRect, teJustCenter);
	
	TextFace(0);
	OffsetRect(&lineRect, 0 , 25);
	TextBox(line5, strlen(line5), &lineRect, teJustCenter);
	OffsetRect(&lineRect, 0 , 15);
	TextBox(line6, strlen(line6), &lineRect, teJustCenter);
	OffsetRect(&lineRect, 0 , 15);
	TextBox(line7, strlen(line7), &lineRect, teJustCenter);
	OffsetRect(&lineRect, 0 , 15);
	TextBox(line8, strlen(line8), &lineRect, teJustCenter);
	
	PenNormal();
	PenMode(patXor);
	
	x[0] = x[2] = 50;
	x[1] = x[3] = 10;
	y[0] = y[2] = 45;
	y[1] = y[3] = 55;
	
	vx[0] = vx[2] = 2;
	vx[1] = vx[3] = -4;
	vy[0] = vy[2] = 3;
	vy[1] = vy[3] = -1;
	
	count = 20;
	
	creditRect = creditWPtr->portRect;
	creditRect.right = 100;
	
	MoveTo((int) x[0],(int) y[0]);
	LineTo((int) x[1],(int) y[1]);
	
	iticks = TickCount();
	
	BeginUpdate(creditWPtr);
	EndUpdate(creditWPtr);
	
	do
	{
		if (count)
			for (i=0;i<=1;i++)
			{
				x[i] += vx[i];
				if (x[i] < creditRect.left + 5)
					vx[i] = (vx[i] > 0) ? vx[i] : -vx[i];
				else if (x[i] > creditRect.right - 5)
					vx[i] = (vx[i] < 0) ? vx[i] : -vx[i];
				
				y[i] += vy[i];
				if (y[i] < creditRect.top + 30)
					vy[i] = (vy[i] > 0) ? vy[i] : -vy[i];
				else if (y[i] > creditRect.bottom - 30)
					vy[i] = (vy[i] < 0) ? vy[i] : -vy[i];
					
				count--;
			}
		
		else
			for (i=0;i<=3;i++)
			{
				x[i] += vx[i];
				if (x[i] < creditRect.left + 5)
					vx[i] = (vx[i] > 0) ? vx[i] : -vx[i];
				else if (x[i] > creditRect.right - 5)
					vx[i] = (vx[i] < 0) ? vx[i] : -vx[i];
				
				y[i] += vy[i];
				if (y[i] < creditRect.top + 30)
					vy[i] = (vy[i] > 0) ? vy[i] : -vy[i];
				else if (y[i] > creditRect.bottom - 30)
					vy[i] = (vy[i] < 0) ? vy[i] : -vy[i];
				
			}
		
		MoveTo((int) x[0],(int) y[0]);
		LineTo((int) x[1],(int) y[1]);	
		
		if (!count)
		{
			MoveTo((int) x[2],(int) y[2]);
			LineTo((int) x[3],(int) y[3]);
		}
	
		GetNextEvent(everyEvent, &theEvent);
		
		Delay(DURATION,&endTime);
		
		if (theEvent.what == updateEvt)
			goto MAINLOOP;
		
	}	while ((!doWait && (theEvent.what != mouseDown))||(doWait && (TickCount() < iticks + doWait)));
	
	
	DisposeWindow(creditWPtr);
	
	SetPort(oldPort);
}

