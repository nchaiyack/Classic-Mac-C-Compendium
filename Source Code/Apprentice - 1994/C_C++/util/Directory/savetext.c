//-- SaveText.c

// This writes out a text file containing the information that was contained in
// the directory listing, albeit not in as nice a manner, maybe....

#include <stdio.h>
#include "struct.h"



//--SaveText

// Write the top window as text.

SaveText()
{
	struct DrawWindow *w;
	Point p;
	SFReply reply;
	char buffer[255];
	short refnum;
	long len;
	long x;
	short y;
	short llen;
	struct DirectData *ptr;
	long l;
	CursHandle cursor;

	w = ((struct DrawWindow *)FrontWindow());
	if (w == NULL) return;
	if (w->w.windowKind != WK_PLAN) return;

	p.h = p.v = 75;
	strcpy(buffer,w->vName);
	strcat(buffer,".DIR");
	CtoPstr(buffer);
	SFPutFile(p,"\pSave directory as:",buffer,NULL,&reply);
	if (!reply.good) return;

	cursor = GetCursor(watchCursor);
	HLock(cursor);
	SetCursor(*cursor);
	HUnlock(cursor);
	HPurge(cursor);

	Create(reply.fName,reply.vRefNum,'EDIT','TEXT');
	if (FSOpen(reply.fName,reply.vRefNum,&refnum)) return;
	len = GetHandleSize(w->data) / sizeof(struct DirectData);
	HLock(w->data);
	ptr = *(w->data);
	for (x = 0; x < len; x++) {
		llen = 0;
		for (y = 0; y < ptr[x].indent; y++) {
			l = 2;
			FSWrite(refnum,&l,"| ");
			llen += 2;						/* Tabs are 4 spaces */
		}
		llen++;
		l = 1;
		FSWrite(refnum,&l," ");
		
		PtoCstr(ptr[x].data);
		l = strlen(ptr[x].data);
		llen += l;
		FSWrite(refnum,&l,ptr[x].data);
		CtoPstr(ptr[x].data);
		
		if (ptr[x].auxdata[0] != '\0') {
			while (llen < 60) {
				llen += 4 - llen % 4;
				l = 1;
				FSWrite(refnum,&l,"\t");
			}
			
			PtoCstr(ptr[x].auxdata);
			l = strlen(ptr[x].auxdata);
			llen += l;
			FSWrite(refnum,&l,ptr[x].auxdata);
			CtoPstr(ptr[x].auxdata);
			while (llen < 72) {
				llen += 4 - llen % 4;
				l = 1;
				FSWrite(refnum,&l,"\t");
			}
			
			PtoCstr(ptr[x].auxdata2);
			l = strlen(ptr[x].auxdata2);
			llen += l;
			FSWrite(refnum,&l,ptr[x].auxdata2);
			CtoPstr(ptr[x].auxdata2);
		}
		l = 1;
		FSWrite(refnum,&l,"\r");
	}
	HUnlock(w->data);
	FSClose(refnum);
	InitCursor();
}
