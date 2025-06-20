
#include "ZTextItem.h"

ZTextItem *ZTextItem::CreateZTextItemFromDragData(FlavorType inType,Ptr inData)
{
	if(inType != 'TEXT') return NULL;
	
	long len = GetPtrSize(inData);
	
	if(MemError()) return NULL;
	
	Str255	theStr;
	
	if (len>255) len = 255;
	
	theStr[0] = len;
	
	BlockMoveData(inData,&theStr[1],len);
	ZTextItem * out = new ZTextItem(theStr,NULL);
	
	return out;
}

ZTextItem::ZTextItem(Str255& inName,ZList *inList)
{
	mText = new unsigned char[inName[0]];
	if(mText)
		BlockMoveData(inName,mText,inName[0]+1);
	else
		;//throw exception
	
	if(inList)
		inList->AddItem(inList->GetNumItems()+1,this);
}

void
ZTextItem::Draw(Rect &inRect)
{
	FontInfo	ourInf;
	
	GetFontInfo(&ourInf);
	
	TextMode(srcOr);
	MoveTo(inRect.left+3,inRect.top + ((inRect.bottom - inRect.top)/2) + ((ourInf.ascent /*+ ourInf.descent*/)/2));
	DrawText(mText,1,mText[0]);
}

void
ZTextItem::Export(FlavorType *outOurFlavor,void **outOurData,long *outDataLen)
{
	*outOurFlavor	= 'TEXT';
	*outOurData		= &mText[1];
	*outDataLen		= mText[0];
}

// � Override
void
ZTextItem::Click(EventRecord& /*event*/, Rect & /*r*/)
{
	SysBeep(32);
}