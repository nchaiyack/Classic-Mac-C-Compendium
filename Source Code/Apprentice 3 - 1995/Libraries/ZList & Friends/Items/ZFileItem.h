#pragma once

#include "ZList.h"

class ZFileItem : public ZListItem{
	public:
	FSSpec mFileSpec;
	
	static ZFileItem *CreateZFileItemFromDragData(FlavorType inType,Ptr inData);
	ZFileItem(FSSpec& ourSpec,ZList *inList = NULL);
	virtual void Click(EventRecord&,Rect&);
	virtual void Draw(Rect&);
	
	virtual void Export(FlavorType *outOurFlavor,void **outOurData,long *outDataLen);
};