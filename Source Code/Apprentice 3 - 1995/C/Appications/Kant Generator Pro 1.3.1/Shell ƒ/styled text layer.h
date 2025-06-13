#ifndef __MY_STYLED_TEXT_LAYER_H__
#define __MY_STYLED_TEXT_LAYER_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	void StyledInsertStr255(TEHandle hTE, Str255 theStr, short font, short size, Style face);
extern	pascal void StyledScrollActionProc(ControlHandle theHandle, short partCode);
extern	void StyledAdjustVScrollBar(ControlHandle theControl, TEHandle hTE);
extern	void StyledAdjustForEndScroll(ControlHandle theControl, TEHandle hTE);
extern	void StyledTrackThumb(WindowPtr theWindow, Point thePoint, ControlHandle theControl);

#ifdef __cplusplus
}
#endif

#endif
