// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// ViewTools.h

#define __VIEWTOOLS__

#ifndef __UGRIDVIEW__
#include <UGridView.h>
#endif

void DirectDeltaRows(TGridView *gv, 
		short beforeRow, short numOfRows, short aHeight);
// does not Update() after TGridView have been touched


const long kKeyTimeout = 40; // for typing names in lists

// Header from TechNote 306: Drawing Icons the System 7 Way
const short ttNone = 0;
const short ttSelected = 0x4000;
pascal OSErr PlotIconID(const CRect &aRect, short align, short transForm, short id) = {0x303C, 0x0500, 0xABC9};


IDType MyPoseModally(TWindow *&window); // as TWindow::PoseModally frees the window if it fails
Boolean DoGridViewKey(TGridView *gv, char ch);

typedef void (*PlainDoEvent)(void *useritem, TView *view, EventNumber eventNumber, TEventHandler* source, TEvent* event);
void AddActionBehaviour(TView *view, PlainDoEvent func, void *useritem);
