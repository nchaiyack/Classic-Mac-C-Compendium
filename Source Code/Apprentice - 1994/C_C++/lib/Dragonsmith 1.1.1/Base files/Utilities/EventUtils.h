/*
	EventUtils.h
*/

#pragma once

#include	<Events.h>

void YieldCPUTime (unsigned short ticks);
Boolean IsCancelEvent (EventRecord *theEvent);