/*
**	Helpsystem.c
**	HPYTerm
**	Copyright �Juri Munkki 1987
*/

static	int			hpage;
static	DialogPtr	hdialog;

DrawHelp(page)
int	page;
{
	RgnHandle
			savedclip;
	Handle	hpict;
	Rect	destrect;
	Rect	itemrect;
	long	Garbage;
	int		x,y;

	savedclip=NewRgn();
	GetClip(savedclip);

	GetDItem(hdialog,4,&Garbage,&Garbage,&itemrect);
	ClipRect(&itemrect);
	EraseRect(&itemrect);
	x=(itemrect.right+itemrect.left)>>1;
	y=(itemrect.top+itemrect.bottom)>>1;

	hpict=GetResource('PICT',page);
	HLock(hpict);
	destrect=*(Rect *)(*hpict+2);
	OffsetRect(&destrect,
		x-((destrect.right-destrect.left)>>1)-destrect.left,
		y-((destrect.bottom-destrect.top)>>1)-destrect.top);
	
	DrawPicture(hpict,&destrect);

	SetClip(savedclip);
	DisposeRgn(savedclip);
	ReleaseResource(hpict);
}
void	SetButtons()
{
	Handle	picth;
	Rect	grect;
	ControlHandle
			controlh;

	SetResLoad(0);
	GetDItem(hdialog,3,&grect,&controlh,&grect);
	picth=GetResource('PICT',hpage-1);
	if(picth)
		HiliteControl(controlh,0);
	else
		HiliteControl(controlh,255);

	GetDItem(hdialog,2,&grect,&controlh,&grect);
	picth=GetResource('PICT',hpage+1);
	if(picth)
		HiliteControl(controlh,0);
	else
		HiliteControl(controlh,255);
	SetResLoad(-1);
}
void	MoreInfo()
{
	int		hit;
	GrafPtr	saved;
	
	GetPort(&saved);
	hpage=1000;
	hdialog=GetNewDialog(1001,0,(WindowPtr)-1);
	SetPort((GrafPtr)hdialog);
	DrawHelp(hpage);
	SetButtons();
	do
	{	ModalDialog(0L,&hit);
		switch(hit)
		{	case 2:
				hpage+=1;
				SetButtons();
				DrawHelp(hpage);
				break;
			case 3:
				hpage-=1;
				SetButtons();
				DrawHelp(hpage);
				break;
		}
	}	while(hit!=1);
	DisposDialog(hdialog);
	SetPort(saved);
}
