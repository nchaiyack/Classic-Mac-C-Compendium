#pragma once

#include "ZList.h"

class ZTextItem : public ZListItem{
	unsigned char *mText;
	
	public:
	static ZTextItem *CreateZTextItemFromDragData(FlavorType inType,Ptr inData);
	ZTextItem(Str255& inName,ZList *inList = NULL);
	ZTextItem(Ptr inName,ZList *inList = NULL);
	
	virtual void Click(EventRecord&,Rect&);
	virtual void Draw(Rect&);
	
	virtual void Export(FlavorType *outOurFlavor,void **outOurData,long *outDataLen);
};