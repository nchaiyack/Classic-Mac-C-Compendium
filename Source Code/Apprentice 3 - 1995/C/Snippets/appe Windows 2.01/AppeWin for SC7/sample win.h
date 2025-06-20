// File "sample win.h" - 

#ifndef ____SAMPLE_WIN_HEADER____
#define ____SAMPLE_WIN_HEADER____

#ifndef __DRAG__
#include <Drag.h>
#endif  __DRAG__

// * ****************************************************************************** *
// * ****************************************************************************** *

pascal unsigned char GetHiliteMode(void) = { 0x1EB8, 0x0938 }; /* MOVE.B 0x0938,(A7) */
pascal void SetHiliteMode(unsigned char) = { 0x11DF, 0x0938 }; /* MOVE.B (A7)+,0x0938 */

// * ****************************************************************************** *
// * ****************************************************************************** *
// Function Prototypes

WindowPtr NewSampleWindow(void);
void DisposeSampleWindow(WindowPtr win);
void SampleWindowEventHandler(EventRecord *theEvent, WindowPtr win);

void SetSampleWindowText(WindowPtr win, StringPtr winText);
StringPtr GetSampleWindowText(WindowPtr win);

#endif  ____SAMPLE_WIN_HEADER____

