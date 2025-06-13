// File "sample win.h" - 

#ifndef ____SAMPLE_WIN_HEADER____
#define ____SAMPLE_WIN_HEADER____

#ifndef __DRAG__
#include <Drag.h>
#endif  __DRAG__

// * ****************************************************************************** *
// * ****************************************************************************** *
// Function Prototypes

WindowPtr NewSampleWindow(void);
void DisposeSampleWindow(WindowPtr win);
void SampleWindowEventHandler(EventRecord *theEvent, WindowPtr win);

void SetSampleWindowText(WindowPtr win, StringPtr winText);
StringPtr GetSampleWindowText(WindowPtr win);

#endif  ____SAMPLE_WIN_HEADER____

