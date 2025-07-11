#ifndef __MY_TEXT_LAYER_H__
#define __MY_TEXT_LAYER_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	Boolean AnyTextInScrapQQ(void);
extern	void SetTheText(WindowRef theWindow, Ptr data, long count);
extern	Boolean AnyTextQQ(WindowRef theWindow);
extern	Boolean AnyHighlightedQQ(WindowRef theWindow);
extern	short SelectionStart(WindowRef theWindow);
extern	short SelectionEnd(WindowRef theWindow);
extern	Boolean InsertBeforeStart(WindowRef theWindow, Str255 theStr);
extern	Boolean InsertAfterEnd(WindowRef theWindow, Str255 theStr);
extern	short TotalNumberOfLines(TEHandle hTE);
extern	void GetSelectionString(WindowRef theWindow, Str255 theStr);
extern	void HandleShiftArrow(TEHandle hTE, unsigned char theChar);
extern	void MyTrackThumb(WindowRef theWindow, Point thePoint, ControlHandle theControl);
extern	pascal void ScrollActionProc(ControlHandle theHandle, short partCode);
extern	pascal void HScrollActionProc(ControlHandle theHandle, short partCode);
extern	void MyMoveScrollBox(ControlHandle theControl, short scrollDistance);
extern	void AdjustVScrollBar(ControlHandle theControl, TEHandle hTE);
extern	short CurrentLineNumber(TEHandle hTE);
extern	short LineNumberFromOffset(TEHandle hTE, short offset);
extern	short LineStart(TEHandle hTE, short lineNum);
extern	pascal Boolean MyClikLoop(void);
extern	void ClickLoopAddOn(WindowRef theWindow);
extern	void AdjustForEndScroll(ControlHandle theControl, TEHandle hTE);
extern	void GetTERect(WindowRef window, Rect *teRect, Boolean adjustForScrollBars);
extern	void AdjustViewRect(TEHandle docTE);
extern	void AdjustScrollSizes(WindowRef window, TEHandle hTE, ControlHandle vScrollBar,
	ControlHandle hScrollBar, short destOverload, short headerHeight);
extern	void AdjustTE(TEHandle hTE, ControlHandle vScrollBar, ControlHandle hScrollBar);
extern	void DrawTheShadowBox(Rect theRect, Boolean eraseBackground);
extern	void DrawGrowIconNoLines( WindowRef wp, Boolean showLines);
#if powerc
extern	pascal Boolean PPCClickLoopProc(TEPtr pTE);
#endif
extern	void SetTheDrawingFont(short fontNum, short fontSize, short fontFace, short fontMode);
extern	void UseDefaultDrawingFont(void);
extern	short MyHideTE(TEHandle hTE);
extern	void MyShowTE(TEHandle hTE, short viewBottom);
extern	short CalculateFontHeight(short font, short fontSize);
extern	short MyGetDefaultFont(void);
extern	short MyGetDefaultFontSize(void);

#ifdef __cplusplus
}
#endif

#endif
