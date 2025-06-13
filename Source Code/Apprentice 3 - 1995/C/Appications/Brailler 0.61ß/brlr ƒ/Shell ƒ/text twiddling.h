Boolean AnyTextInScrapQQ(void);
void SetTheText(WindowPtr theWindow, Ptr data, long count);
Boolean AnyTextQQ(WindowPtr theWindow);
Boolean AnyHighlightedQQ(WindowPtr theWindow);
short SelectionStart(WindowPtr theWindow);
short SelectionEnd(WindowPtr theWindow);
Boolean InsertBeforeStart(WindowPtr theWindow, Str255 theStr);
Boolean InsertAfterEnd(WindowPtr theWindow, Str255 theStr);
short TotalNumberOfLines(TEHandle hTE);
void GetSelectionString(WindowPtr theWindow, Str255 theStr);
void HandleShiftArrow(TEHandle hTE, unsigned char theChar);
void MyTrackThumb(WindowPtr theWindow, Point thePoint, ControlHandle theControl);
pascal void ScrollActionProc(ControlHandle theHandle, short partCode);
pascal void HScrollActionProc(ControlHandle theHandle, short partCode);
void MyMoveScrollBox(ControlHandle theControl, short scrollDistance);
void AdjustVScrollBar(ControlHandle theControl, TEHandle hTE);
short CurrentLineNumber(TEHandle hTE);
short LineNumberFromOffset(TEHandle hTE, short offset);
short LineStart(TEHandle hTE, short lineNum);
pascal Boolean MyClikLoop(void);
void ClickLoopAddOn(WindowPtr theWindow);
void AdjustForEndScroll(ControlHandle theControl, TEHandle hTE);
void GetTERect(WindowPtr window, Rect *teRect, Boolean adjustForScrollBars);
void AdjustViewRect(TEHandle docTE);
void AdjustScrollSizes(WindowPtr window, TEHandle hTE, ControlHandle vScrollBar,
	ControlHandle hScrollBar, short destOverload, short headerHeight);
void AdjustTE(TEHandle hTE, ControlHandle vScrollBar, ControlHandle hScrollBar);
void DrawTheShadowBox(Rect theRect, Boolean eraseBackground);
void DrawGrowIconNoLines( WindowPtr wp, Boolean showLines);
#if powerc
pascal Boolean PPCClickLoopProc(TEPtr pTE);
#endif
void SetTheDrawingFont(short fontNum, short fontSize, short fontFace);
void UseDefaultDrawingFont(void);
