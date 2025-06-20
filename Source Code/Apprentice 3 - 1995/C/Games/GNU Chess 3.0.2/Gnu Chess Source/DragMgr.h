/*
	D'apr�s Scott T. Boyd of the MacHax Group
	Utilisation: Cf MacTutor v3 n�7 [July '87]
	
	Modifications pour l'adaptation � une fen�tre : Airy ANDRE
	
	Interface C : Airy ANDRE
*/

typedef struct ShadowRecord {
  	Boolean visible;
	int 	dx,dy;
	int		CopyMode;
} ShadowRecord;

typedef struct DragRecord {
  	BitMap		ShadowBits, UnderShadowBits,UnderBits,PictureBits;
	RgnHandle	ShadowRegion,ThePictureRgn;
} DragRecord, *DragPtr, **DragHandle;


extern ShadowRecord ShadowStuff;
extern BitMap OffScreenBits;

extern Boolean InitDrag(BitMapPtr, Rect *);
extern Boolean NewDraggable(PicHandle, PicHandle, BitMapPtr, BitMapPtr,
									DragHandle *);
extern void DragItTo(DragHandle, Point, Boolean, Boolean);
extern void DisposeDraggable(DragHandle);
extern void UpdateOffScreen(void);
extern void CloseDrag(Boolean);
extern Rect * GetLastRect(void);
