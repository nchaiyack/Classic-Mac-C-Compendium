#pragma once

#include "ZList.h"
#include <LDynamicArray.h>	//requires PowerPlant

class ZTwistItem : public ZListItem{
	unsigned char	*mText;
	LDynamicArray	*mChildren;
	
	public:
	static ZTwistItem *CreateZTwistItemFromDragData(FlavorType inType,Ptr inData);
	ZTwistItem(Str255& inName,ZList *inParentList = NULL);
	
	virtual void Expand();
	
	virtual void Click(EventRecord&,Rect&);
	virtual void Draw(Rect&);
	
//	virtual Int32 GetNumExportTypes();
	virtual void Export(/*inTypeIndex*/FlavorType *outOurFlavor,void **outOurData,long *outDataLen);
};