void PushButton(DialogPtr theDialog, short theItem);
void GetDItemRect(DialogPtr theDialog, short theItem, Rect *theRect);
void SetDItemText(DialogPtr theDialog, short theItem, Str255 iText);
void SetUserProc(DialogPtr theDialog, short theItem, ProcPtr theProc);
pascal void FrameGrayRect(DialogPtr theDialog, short theItem);
pascal void FrameItemRect(DialogPtr theDialaog, short theItem);
Handle GetDItemHdl(DialogPtr theDialog, short theItem);
void OutlineDefaultButton(DialogPtr theDialog, short theItem);