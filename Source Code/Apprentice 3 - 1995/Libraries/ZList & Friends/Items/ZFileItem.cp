
#include "ZFileItem.h"

ZFileItem *ZFileItem::CreateZFileItemFromDragData(FlavorType inType,Ptr inData)
{
	if(inType != flavorTypeHFS) return NULL;
	
	HFSFlavor*	ourFile = (HFSFlavor*)inData;
	ZFileItem * out = NULL;
	
	if(ourFile->fileType == 'TEXT')
		out = new ZFileItem(ourFile->fileSpec,NULL);
	else
		return NULL;
	
	return out;
}

ZFileItem::ZFileItem(FSSpec& ourSpec,ZList *inList)
{
	mFileSpec = ourSpec;
//	BlockMoveData(&ourSpec,&mFileSpec,sizeof(FSSpec));
	// ??? We should convert this to an AliasHandle
		
	if(inList)
		inList->AddItem(inList->GetNumItems()+1,this);
}

void
ZFileItem::Draw(Rect &inRect)
{
	FontInfo	ourInf;
	
	GetFontInfo(&ourInf);
	
	TextMode(srcOr);
	MoveTo(inRect.left+3,inRect.top + ((inRect.bottom - inRect.top)/2) + ((ourInf.ascent /*+ ourInf.descent*/)/2));
	DrawText(mFileSpec.name,1,mFileSpec.name[0]);
}

void
ZFileItem::Export(FlavorType *outOurFlavor,void **outOurData,long *outDataLen)
{
	*outOurFlavor	= flavorTypeHFS;
	*outOurData		= &mFileSpec;
	*outDataLen		= sizeof(mFileSpec);
}

// � Override
void
ZFileItem::Click(EventRecord& /*event*/, Rect & /*r*/)
{
	SysBeep(32);
}