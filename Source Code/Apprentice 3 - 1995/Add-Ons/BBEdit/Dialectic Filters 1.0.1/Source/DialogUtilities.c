#include "DialogUtilities.h"

pascal Handle GetIHandle(DialogPtr d, short item)
{
	Rect r;
	Handle h;
	short iType;
	
	GetDItem(d, item, &iType, &h, &r);
	return h;
}

pascal void SetStrItem(DialogPtr d, short item, unsigned char s[])
{
	Handle h = GetIHandle(d, item);
	SetIText(h, s);
}

pascal void ReadStrItem(DialogPtr d, short item, unsigned char s[])
{
	Handle h = GetIHandle(d, item);
	GetIText(h, s);
}

pascal void SetDlgCtl(DialogPtr d, short item, Boolean flag)
{
	ControlHandle c = (ControlHandle)GetIHandle(d, item);
	
	SetCtlValue(c, flag);
}

pascal Boolean GetDlgCtl(DialogPtr d, short item)
{
	ControlHandle c = (ControlHandle)GetIHandle(d, item);
	
	return GetCtlValue(c);
}

pascal Boolean TestDlgCtl(DialogPtr d, short item)
{
	ControlHandle c = (ControlHandle)GetIHandle(d, item);
	
	return ((**c).contrlHilite != 0xFF);
}

pascal void XAbleDlgCtl(DialogPtr d, short item, Boolean flag)
{
	ControlHandle c = (ControlHandle)GetIHandle(d, item);
	
	HiliteControl(c, flag ? 0 : 0xFF);
}

pascal void SetupUserItem(DialogPtr d, short item, pascal void (*ItemProc)(DialogPtr d, short item))
{
	short type;
	Handle handle;
	Rect rect;
	
	GetDItem(d, item, &type, &handle, &rect);
	SetDItem(d, item, type, (Handle)ItemProc, &rect);
}
