#include <Script.h>
#include <stdio.h>
#include <string.h>
#include "Event.h"
#include "Status.h"





UserItemUPP		gDrawRemaingItemsItemUPP;
UserItemUPP		gDrawActionItemUPP;
UserItemUPP		gDrawKPerSecondItemUPP;
UserItemUPP		gDrawStatusBarItemUPP;





void InitStatusWindow(void)
{
	gDrawRemaingItemsItemUPP = NewUserItemProc(DrawRemaingItemsItem);
	gDrawActionItemUPP = NewUserItemProc(DrawActionItem);
	gDrawKPerSecondItemUPP = NewUserItemProc(DrawKPerSecondItem);
	gDrawStatusBarItemUPP = NewUserItemProc(DrawStatusBarItem);
}





WindowPtr NewStatusWindow(char *title,StatusInfoPtr *info)
{
	WindowProcsPtr	procs;
	DialogPtr		dialog;
	Str255			ptitle;
	short				itemType;
	Rect				itemRect;
	Handle			itemHandle;
	
	
	dialog = GetNewDialog(128,NULL,(WindowPtr)-1L);
	SetWTitle(dialog,ConvertCtoPStr(ptitle,title));
	
	procs = (WindowProcsPtr)NewPtrClear(sizeof(WindowProcs));
	procs->click = (ClickProc)DoStatusClick;
	procs->key = (KeyProc)DoStatusKey;
	procs->hit = (HitProc)DoStatusHit;
	procs->update = (UpdateProc)DoStatusUpdate;
	procs->param = NewPtrClear(sizeof(StatusInfo));
	SetWRefCon(dialog,(long)procs);
	
	*info = (StatusInfoPtr)procs->param;
	
	GetDItem(dialog,2,&itemType,&itemHandle,&itemRect);
	SetDItem(dialog,2,itemType,(Handle)gDrawRemaingItemsItemUPP,&itemRect);
	GetDItem(dialog,3,&itemType,&itemHandle,&itemRect);
	SetDItem(dialog,3,itemType,(Handle)gDrawActionItemUPP,&itemRect);
	GetDItem(dialog,4,&itemType,&itemHandle,&itemRect);
	SetDItem(dialog,4,itemType,(Handle)gDrawKPerSecondItemUPP,&itemRect);
	GetDItem(dialog,5,&itemType,&itemHandle,&itemRect);
	SetDItem(dialog,5,itemType,(Handle)gDrawStatusBarItemUPP,&itemRect);
	
	return dialog;
}





void DisposeStatusWindow(WindowPtr window)
{
	WindowProcsPtr	procs;
	
	
	procs = (WindowProcsPtr)GetWRefCon(window);
	if (procs)
	{
		if (procs->param)
			DisposePtr((Ptr)procs->param);
		DisposePtr((Ptr)procs);
	}
	
	DisposeDialog(window);
}





void DoStatusClick(WindowPtr window,StatusInfoPtr info,Point where)
{
	
}





void DoStatusKey(WindowPtr window,StatusInfoPtr info,char key,long modifiers)
{
	short		itemType;
	Rect		itemRect;
	Handle	itemHandle;
	long		finalTick;
	
	
	if ((modifiers & cmdKey) && (key == '.'))
	{
		GetDItem(window,1,&itemType,&itemHandle,&itemRect);
		HiliteControl((ControlHandle)itemHandle,10);
		Delay(5,&finalTick);
		HiliteControl((ControlHandle)itemHandle,0);
		info->cancel = 1;
	}
}





void DoStatusHit(WindowPtr window,StatusInfoPtr info,short item)
{
	if (item == 1)
		info->cancel = 1;
}




void DoStatusUpdate(WindowPtr window,StatusInfoPtr info)
{
	
}





void UpdateStatusItem(WindowPtr window,short item)
{
	GrafPtr	savedPort;
	
	
	GetPort(&savedPort);
	SetPort(window);
	
	switch(item)
	{
		case 0:
			DrawRemaingItemsItem(window,2);
			DrawActionItem(window,3);
			DrawKPerSecondItem(window,4);
			DrawStatusBarItem(window,5);
			break;
		case 2:
			DrawRemaingItemsItem(window,item);
			break;
		case 3:
			DrawActionItem(window,item);
			break;
		case 4:
			DrawKPerSecondItem(window,item);
			break;
		case 5:
			DrawStatusBarItem(window,item);
			break;
	}
	
	SetPort(savedPort);
}





pascal void DrawRemaingItemsItem(WindowPtr window,short item)
{
	WindowProcsPtr	procs;
	StatusInfoPtr	info;
	short				itemType;
	Rect				itemRect;
	Handle			itemHandle;
	char				s[100];
	
	
	procs = (WindowProcsPtr)GetWRefCon(window);
	info = procs->param;
	
	GetDItem(window,item,&itemType,&itemHandle,&itemRect);
	sprintf(s,"Items Remaining:  %ld",(int)info->remaining);
	DrawStringClipped(s,&itemRect);
}





pascal void DrawActionItem(WindowPtr window,short item)
{
	WindowProcsPtr	procs;
	StatusInfoPtr	info;
	short				itemType;
	Rect				itemRect;
	Handle			itemHandle;
	char				s[100];
	
	
	procs = (WindowProcsPtr)GetWRefCon(window);
	info = procs->param;
	
	GetDItem(window,item,&itemType,&itemHandle,&itemRect);
	sprintf(s,"%s %s",info->action,info->file);
	DrawStringClipped(s,&itemRect);
}





pascal void DrawKPerSecondItem(WindowPtr window,short item)
{
	WindowProcsPtr	procs;
	StatusInfoPtr	info;
	RgnHandle		savedClipRgn;
	short				itemType;
	Rect				itemRect;
	Handle			itemHandle;
	Str255			text;
	char				s[100];
	long				cur;
	
	
	GetDItem(window,item,&itemType,&itemHandle,&itemRect);
	procs = (WindowProcsPtr)GetWRefCon(window);
	info = procs->param;
	
	if ((TickCount() - info->lastKPerSec) < 10)
		return;
	info->lastKPerSec = TickCount();
	
	savedClipRgn = NewRgn();
	GetClip(savedClipRgn);
	ClipRect(&itemRect);
	
	cur = TickCount() - info->start;
	cur = (info->total-info->current)*60/cur;
	cur /= 1024;
	
	sprintf(s,"      %ld K/sec",(int)cur);
	ConvertCtoPStr(text,s);
	MoveTo(itemRect.right-StringWidth(text),itemRect.bottom-4);
	TextMode(srcCopy);
	DrawString(text);
	
	SetClip(savedClipRgn);
	DisposeRgn(savedClipRgn);
}





pascal void DrawStatusBarItem(WindowPtr window,short item)
{
	WindowProcsPtr	procs;
	StatusInfoPtr	info;
	RgnHandle		savedClipRgn;
	RGBColor			savedColor,color;
	short				itemType;
	Rect				itemRect,box;
	Handle			itemHandle;
	float				temp;
	GDHandle			gd;
	short				blacknwhite = 0;
	Point				pt;
	
	
	pt.h = window->portRect.left;
	pt.v = window->portRect.top;
	LocalToGlobal(&pt);
	itemRect.left = pt.h;
	itemRect.top = pt.v;
	pt.h = window->portRect.right;
	pt.v = window->portRect.bottom;
	LocalToGlobal(&pt);
	itemRect.right = pt.h;
	itemRect.bottom = pt.v;
	
	gd = GetDeviceList();
	while(gd)
	{
		if (SectRect(&itemRect,&(**gd).gdRect,&box))
		{
			if ((**(**gd).gdPMap).pixelSize == 1)
				blacknwhite++;
		}
		
		gd = GetNextDevice(gd);
	}
	
	
	procs = (WindowProcsPtr)GetWRefCon(window);
	info = procs->param;
	
	GetForeColor(&savedColor);
	GetDItem(window,item,&itemType,&itemHandle,&itemRect);
	FrameRect(&itemRect);
	
	InsetRect(&itemRect,1,1);
	savedClipRgn = NewRgn();
	GetClip(savedClipRgn);
	ClipRect(&itemRect);
	
	box = itemRect;
	temp = info->total ? (info->current)/(float)info->total : 1;
	temp = (box.right-box.left)*temp;
	box.right -= temp-1;
	
	color.red = 0x4400;
	color.green = 0x4400;
	color.blue = 0x4400;
	RGBForeColor(&color);
	PaintRect(&box);
	
	box.left = box.right;
	box.right = itemRect.right;
	
	if (blacknwhite)
	{
		color.red = 0xFFFF;
		color.green = 0xFFFF;
		color.blue = 0xFFFF;
	}
	else
	{
		color.red = 0xCC00;
		color.green = 0xCC00;
		color.blue = 0xFFFF;
	}
	RGBForeColor(&color);
	PaintRect(&box);
	
	RGBForeColor(&savedColor);
	SetClip(savedClipRgn);
	DisposeRgn(savedClipRgn);
}





void DrawStringClipped(char *s,Rect *box)
{
	RgnHandle	savedClipRgn;
	Str255		text;
	
	
	savedClipRgn = NewRgn();
	GetClip(savedClipRgn);
	ClipRect(box);
	
	ConvertCtoPStr(text,s);
	TruncString(box->right-box->left-4,text,smTruncEnd);
	memset(&text[text[0]+1],' ',100);
	text[0] += 100;
	
	MoveTo(box->left+2,box->bottom-4);
	TextMode(srcCopy);
	DrawString(text);
	
	SetClip(savedClipRgn);
	DisposeRgn(savedClipRgn);
}





StringPtr ConvertCtoPStr(StringPtr pstr,char *cstr)
{
	pstr[0] = strlen(cstr);
	BlockMove(cstr,pstr+1,pstr[0]);
	return pstr;
}