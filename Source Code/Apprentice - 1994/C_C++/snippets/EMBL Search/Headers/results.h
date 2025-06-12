
/* results.c */
Boolean NewResultWindow(short w, WDPtr wdp, ResultHdl resHdl, StringPtr title);
void DisposeResults(WDPtr wdp);
void DrawResWinAll(WDPtr wdp, short dummy);
void SelectAllResults(WDPtr wdp, Boolean what);
void SelectDraw(WDPtr wdp, short i, short what);
void AdjustResText(WindowPtr wPtr, short oldvalue, short newvalue, scrollBarType which);
void DoResClicks(WDPtr wdp, EventRecord *eventPtr);
Boolean Load1Results(short w, StringPtr fName, short vRefNum);
void LoadResults(void);
void DrawResHeader(WDPtr wdp, ResultHdl resHdl);
void HideShowResSelections(WDPtr wdp, Boolean show);
