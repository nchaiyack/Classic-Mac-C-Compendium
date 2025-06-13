pascal OSErr	HandleNewPicture(Point *defaultObjectSize,WEObjectReference objectRef);
pascal OSErr	HandleDisposePicture(WEObjectReference objectRef );
pascal OSErr	HandleDrawPicture (Rect *destRect, WEObjectReference objectRef );


#define		kTypePicture		'PICT'