/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
typedef struct
{
	short vRef;
	Str31 fileName;
} TextDesc, *TextDPtr, **TextDHandle;
short OpenText(short vRef,UPtr name,MyWindowPtr win,Boolean showIt,UPtr alias);
Boolean TextScroll(MyWindowPtr win,int h,int v);
void TextDidResize(MyWindowPtr win,Rect *oldContR);

