pascal Handle GetIHandle(DialogPtr d, short item);
pascal void SetStrItem(DialogPtr d, short item, unsigned char s[]);
pascal void ReadStrItem(DialogPtr d, short item, unsigned char s[]);
pascal void SetDlgCtl(DialogPtr d, short item, Boolean flag);
pascal Boolean GetDlgCtl(DialogPtr d, short item);
pascal Boolean TestDlgCtl(DialogPtr d, short item);
pascal void XAbleDlgCtl(DialogPtr d, short item, Boolean flag);
pascal void SetupUserItem(DialogPtr d, short item, pascal void (*ItemProc)(DialogPtr d, short item));
