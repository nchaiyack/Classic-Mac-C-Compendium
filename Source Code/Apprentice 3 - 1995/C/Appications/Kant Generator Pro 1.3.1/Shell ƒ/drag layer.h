#ifndef __MY_DRAG_LAYER_H__
#define __MY_DRAG_LAYER_H__

#include <Drag.h>

#ifdef __cplusplus
extern "C" {
#endif

extern	void InitTheDragManager(void);
extern	Boolean DragManagerAvailableQQ(void);
extern	pascal OSErr MyReceiveDropHandler(WindowRef duhWindow, unsigned long handlerRefCon,
								  DragReference theDrag);
extern	pascal OSErr MyTrackingHandler(short message, WindowRef duhWindow,
							   void *handlerRefCon, DragReference theDrag);
extern	Boolean DragInWindow(WindowRef theWindow, EventRecord *theEvent);
extern	void ResetHiliteRgn(WindowRef theWindow);
extern	Boolean CursorInDraggableRgn(Point thePoint, WindowRef theWindow);

#ifdef __cplusplus
}
#endif

#endif
