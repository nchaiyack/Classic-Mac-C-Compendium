
/* window.c */
Boolean IsAppWindow(WindowPtr wPtr);
Boolean IsDAWindow(WindowPtr wPtr);
void InvalBars(WindowPtr wPtr);
void ClearWindowRec(WDPtr wdp);
void PrepWindows(void);
void AdjustPosition(short w, Rect *rect);
WDPtr FindMyWindow(WindowPtr wPtr);
short GetFreeWindow(void);
void DoGrow(WindowPtr wPtr, Point where);
void GrowScroll(WDPtr wdp);
void DoZoom(WindowPtr wPtr, short partCode);
Boolean CloseMyWindow(WDPtr wdp, Boolean shift);
Boolean CloseAllWindows(Boolean shift);
void SetVScroll(WDPtr wdp);
void SetHScroll(WDPtr wdp);
void AdjustWSize(short wKind, Rect *r, short height, short width);
void DoDrawGrowIcon(WindowPtr wPtr);
void GetViewRect(WindowPtr wPtr, Rect *r);
void AddWindowToMenu(StringPtr name);
void DelWindowFromMenu(StringPtr name);
void CenterDA(ResType what, short id, short offset);
