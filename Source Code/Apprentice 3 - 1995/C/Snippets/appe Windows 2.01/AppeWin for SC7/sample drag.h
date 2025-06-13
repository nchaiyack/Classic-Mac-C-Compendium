// File "sample drag.h" - 

#ifndef ____SAMPLE_DRAG_HEADER____
#define ____SAMPLE_DRAG_HEADER____

#ifndef __DRAG__
#include <Drag.h>
#endif  __DRAG__

// * ****************************************************************************** *
// * ****************************************************************************** *
// Function Prototypes

void SetupDragHandlers(WindowPtr win);
short SampleWindowDrag(EventRecord *theEvent, WindowPtr win);

pascal OSErr SampleTrackingHandler(DragTrackingMessage message, WindowPtr win,
		void *refcon, DragReference dragRef);
pascal OSErr SampleReceiveHandler(WindowPtr win, void *refcon, DragReference dragRef);

RgnHandle NewDragRgn(WindowPtr win);

#endif  ____SAMPLE_DRAG_HEADER____

