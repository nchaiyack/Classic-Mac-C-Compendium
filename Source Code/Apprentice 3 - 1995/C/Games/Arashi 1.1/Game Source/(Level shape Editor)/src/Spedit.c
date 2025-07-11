/*/
     Project Arashi: Spedit.c
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, October 28, 1992, 23:37
     Created: Sunday, February 5, 1989, 22:32

     Copyright � 1989-1992, Juri Munkki
/*/

#include <Math.h>
#define	PI 3.1415926535

#define MAXSEGS	48
#define	ANGLES	120
#define OLDANGLES 64
typedef struct
{
	int		numsegs;
	char	wraps;
	char	seglen;
	char	xoff;
	char	yoff;
	int		filler;
	int		ang[MAXSEGS];
}	shape;

WindowPtr	MyWind;
EventRecord	Event;
char		thechar;
Point		Mouse,where;
int			CurEdit;
int			CurRes;
int			Multiplier=4;
shape		space;

int		Sins[ANGLES+1];
int		Cosins[ANGLES+1];

void	LoadSpace(num)
int		num;
{
	Handle		Spce;
	
	Spce=GetResource('SPCE',num);
	if(Spce)
	{	space=**(shape **)Spce;
		ReleaseResource(Spce);
	}
	else
	{	SysBeep(10);
	}
}
void	SaveSpace(num)
int		num;
{
	Handle		Spce;
	int			zip=0;
	
	Spce=GetResource('SPCE',num);
	if(Spce==0)
	{	Spce=GetResource('SPCE',100);
		DetachResource(Spce);
		AddResource(Spce,'SPCE',num,&zip);
	}
	SetHandleSize(Spce,sizeof(space));
	**(shape **)Spce=space;
	ChangedResource(Spce);
	WriteResource(Spce);
	ReleaseResource(Spce);
}
int		QueryNum(num)
int		num;
{
	char		numstr[256];
	DialogPtr	MyDialog;
	int			hit;
	Handle		item;
	Rect		Garbage;
	long		thenum;
	
	NumToString(num,numstr);
	MyDialog=GetNewDialog(200,0,(WindowPtr)-1);
	GetDItem(MyDialog,3,&Garbage,&item,&Garbage);
	SetIText(item,numstr);
	ShowWindow(MyDialog);
	do	{	ModalDialog(0,&hit);
		}	while(hit!=1);
	GetIText(item,numstr);
	StringToNum(numstr,&thenum);
	DisposDialog(MyDialog);

	return thenum;
}
void	InitSins()
{
	double	a;
	double	b;
	int		i;
	
	a=0.0;
	b=PI/(ANGLES/2);
	for(i=0;i<=ANGLES;i++)
	{	Sins[i]=8192*sin(a);
		Cosins[i]=8192*cos(a);
		a+=b;
	}
}
void	SmallCross()
{
	Move(-3,-3);
	Line(6,6);
	Move(-6,0);
	Line(6,-6);
	Move(-3,3);
}
void	DrawSpace()
{
	int		i;
	double	a,x,y;
	char	num[32];

	EraseRect(&MyWind->portRect);
	MoveTo(20,20);
	NumToString(space.seglen,num);
	DrawString(num);
	DrawString("\p, ");
	NumToString(Multiplier,num);
	DrawString(num);
	DrawChar(' ');
	if(space.wraps)
		DrawChar('W');
	a=0;
	x=where.h;
	y=where.v;
	for(i=0;i<space.numsegs;i++)
	{	MoveTo((int)(x+0.5),(int)(y+0.5));
		a+=space.ang[i];
		if(i==CurEdit)	PenSize(2,2);
		SmallCross();		
		x+=cos(a/ANGLES*PI*2)*space.seglen;
		y+=-sin(a/ANGLES*PI*2)*space.seglen;
		LineTo((int)(x+0.5),(int)(y+0.5));
		if(i==CurEdit)	PenSize(1,1);
	}
	SmallCross();
}
void	DoMouseDown()
{
	int		i;
	double	a,x,y,x2,y2,x3,y3;
	char	num[32];

	EraseRect(&MyWind->portRect);
	MoveTo(20,20);
	NumToString(space.seglen,num);
	DrawString(num);
	a=0;
	x=where.h;
	y=where.v;
	for(i=0;i<space.numsegs;i++)
	{	x2=300.0+(x-300.0)*.2;
		y2=200.0+(y-200.0)*.2;
		MoveTo((int)(x2+0.5),(int)(y2+0.5));
		LineTo((int)(x+0.5),(int)(y+0.5));
		a+=space.ang[i];
		x+=cos(a/ANGLES*PI*2)*space.seglen;
		y+=-sin(a/ANGLES*PI*2)*space.seglen;
		LineTo((int)(x+0.5),(int)(y+0.5));
		x3=300.0+(x-300.0)*.2;
		y3=200.0+(y-200.0)*.2;
		LineTo((int)(x3+0.5),(int)(y3+0.5));
		LineTo((int)(x2+0.5),(int)(y2+0.5));
	}
	while(Button());
}
void	DoKeyHit()
{
	int		amount=Multiplier;
	
	if(Event.modifiers & shiftKey)
	{	amount*=2;
	}
	if(Event.modifiers & alphaLock)
	{	amount*=2;
	}
	switch(thechar)
	{	case '6':
			space.ang[CurEdit]=(space.ang[CurEdit]-amount) % ANGLES;
			break;
		case '8':
			CurEdit+=1;
			if(CurEdit>=space.numsegs)	CurEdit=0;
			break;
		case '4':
			space.ang[CurEdit]=(space.ang[CurEdit]+amount) % ANGLES;
			break;
		case '5':
			CurEdit-=1;
			if(CurEdit<0)	CurEdit=space.numsegs-1;
			break;
		case '1':
			Multiplier=1;
			break;
		case '2':
			Multiplier=2;
			break;
		case '3':
			Multiplier=3;
			break;
		case 3:
			Multiplier=5;
			break;
		case '+':
			space.numsegs++;
			if(space.numsegs>MAXSEGS)		space.numsegs--;
			break;
		case '-':
			space.numsegs--;
			if(space.numsegs<=0)			space.numsegs=1;
			if(CurEdit>=space.numsegs)		CurEdit=space.numsegs-1;
			break;
		case '*':
			space.seglen+=amount;
			break;
		case '/':
			space.seglen-=amount;
			break;
		case '0':	/*	Insert segment	*/
			space.numsegs++;
			if(space.numsegs>MAXSEGS)		space.numsegs--;
			else
			{	for(amount=space.numsegs;amount>CurEdit;amount--)
					space.ang[amount]=space.ang[amount-1];
			}
			break;
		case ',':
		case '.':
			space.numsegs--;
			if(space.numsegs<=0)			space.numsegs=1;
			else
			{	for(amount=CurEdit;amount<=space.numsegs;amount++)
					space.ang[amount]=space.ang[amount+1];
			}
			break;
		case 's':
		case 'S':
			CurRes=QueryNum(CurRes);
			SaveSpace(CurRes);
			break;
		case 'l':
		case 'L':
			CurRes=QueryNum(CurRes);
			LoadSpace(CurRes);
			break;
		case ' ':
			HideCursor();
			break;
		case '@':
			ShowCursor();
			break;
		case '_':
			for(amount=0;amount<space.numsegs;amount++)
			{	space.ang[amount]=(space.ang[amount]*ANGLES)/OLDANGLES;
			}
			break;
		case 9:
			CreateFit();
			while(!EventAvail(everyEvent,&Event));
			break;
		case '7':
			TestFlip();
			break;
		case 'W':
		case 'w':
			space.wraps = !space.wraps;
	}
	DrawSpace();
}
void	main()
{
	CursHandle	cross;
	
	DoInits();
	cross=GetCursor(crossCursor);
	HLock(cross);
	SetCursor(*cross);
	
	MyWind=GetNewWindow(100,0,(WindowPtr)-1);
	SetPort(MyWind);
	InitSins();
	CurRes=100;
	LoadSpace(CurRes);
	GetMouse(&Mouse);
	thechar=' ';
	
	do
	{	if(GetNextEvent(everyEvent,&Event))
		{	switch(Event.what)
			{	case updateEvt:
					BeginUpdate(MyWind);
					DrawSpace();
					EndUpdate(MyWind);
					break;
				case keyDown:
				case autoKey:
					thechar=Event.message;
					DoKeyHit();
					break;
				case mouseDown:
					DoMouseDown();
					break;
				default:
					break;
			}
		}
		if(Event.where.h != Mouse.h || Event.where.v != Mouse.v)
		{	Mouse=Event.where;
			where=Mouse;
			GlobalToLocal(&where);
			DrawSpace();
		}
	} while(thechar!=27);

}