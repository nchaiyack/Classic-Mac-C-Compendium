#pragma once

#include "ZList.h"

class ZIconItem : public ZListItem{
	Int16 	mIconID;
	Str31	mIconName;
	
	public:
	static ZIconItem *CreateZIconItemFromData(FlavorType inType,Ptr inData);
	
	ZIconItem(Int16 mIconID, Str31& inIconName = "\p", ZList *inList = NULL);
	virtual void Click(EventRecord&,Rect&);
	virtual void Draw(Rect&);
	
	virtual void Export(FlavorType *outOurFlavor,void **outOurData,long *outDataLen);
};