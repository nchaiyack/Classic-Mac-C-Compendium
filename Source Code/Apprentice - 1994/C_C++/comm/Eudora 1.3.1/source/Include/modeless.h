/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/************************************************************************
 * dialogs, with a little extra added
 ************************************************************************/
MyWindowPtr GetNewMyDialog(short template,UPtr wStorage,WindowPtr behind,
	void (*centerFunction)());
Boolean DoModelessEvent(EventRecord *event);
Boolean DoModelessEdit(MyWindowPtr win,short item);
