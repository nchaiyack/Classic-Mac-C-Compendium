/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
void GetDisplayRect(Rect *rectPtr);
void RedrawAllWindows(void);
void SetDIText(DialogPtr dialog,int item,UPtr text);
void GetDIText(DialogPtr dialog,int item,UPtr text);
short SetDItemState(DialogPtr pd,short dItem,short hilite);
short GetDItemState(DialogPtr pd,short dItem);
pascal Boolean DlgFilter(DialogPtr dgPtr,EventRecord *event,short *item);
void CenterDialog(int template);
void TopCenterDialog(int template);
void BottomCenterDialog(int template);
void ThirdCenterDialog(int template);
void GlobalizeRgn(RgnHandle rgn);
void LocalizeRgn(RgnHandle rgn);
void HiliteButtonOne(DialogPtr dgPtr);
typedef short SICN[16];
typedef SICN **SICNHand;
void PlotSICN(Rect *theRect, SICNHand theSICN, long theIndex);
void SavePosPrefs(UPtr name,Rect *r, Boolean zoomed);
void SavePosFork(short vRef,long dirId,UPtr name,Rect *r, Boolean zoomed);
Boolean RestorePosPrefs(UPtr name,Rect *r, Boolean *zoomed);
Boolean RestorePosFork(short vRef,long dirId,UPtr name,Rect *r, Boolean *zoomed);
Boolean PositionPrefsTitle(Boolean save, MyWindowPtr win);
void ZoomPosition(MyWindowPtr win);
void DefPosition(MyWindowPtr win,Rect *r);
void GreyOutRoundRect(Rect *r,short r1,short r2);
UPtr SetDIPopup(DialogPtr pd,short item,UPtr toName);
UPtr GetDIPopup(DialogPtr pd,short item,UPtr whatName);
void DrawMyControls(WindowPeek win);
void SanitizeSize(Rect *r);
void HotRect(Rect *r,Boolean False);
Boolean MyWinHasSelection(MyWindowPtr win);
Boolean CursorInRect(Point pt,Rect r,RgnHandle mouseRgn);
void MaxSizeZoom(MyWindowPtr win,Rect *zoom);
#define StdState(win) (*(WStateData**)(win)->qWindow.dataHandle)->stdState
#define UserState(win) (*(WStateData**)(win)->qWindow.dataHandle)->userState
Rect CurState(MyWindowPtr win);
Boolean AboutSameRect(Rect *r1,Rect *r2);
void OutlineControl(ControlHandle cntl,Boolean blackOrWhite);
#ifdef DEBUG
void ShowGlobalRgn(RgnHandle globalRgn,UPtr string);
void ShowLocalRgn(RgnHandle localRgn,UPtr string);
#endif