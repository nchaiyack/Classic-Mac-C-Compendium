#include "key layer.h"

static	short			gTheModifiers;

void SaveEventModifiers(EventRecord* theEvent)
{
	gTheModifiers=theEvent->modifiers;
}

short GetTheModifiers(void)
{
	return gTheModifiers;
}

Boolean CommandKeyWasDown(void)
{
	return (gTheModifiers&cmdKey) ? TRUE : FALSE;
}

Boolean OptionKeyWasDown(void)
{
	return (gTheModifiers&optionKey) ? TRUE : FALSE;
}

Boolean ShiftKeyWasDown(void)
{
	return (gTheModifiers&shiftKey) ? TRUE : FALSE;
}
