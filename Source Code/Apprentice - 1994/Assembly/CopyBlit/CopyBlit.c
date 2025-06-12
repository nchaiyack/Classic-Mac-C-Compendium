void CopyBlit( PixMapHandle *pixMap, Rect *rect1, Rect *rect2 )
{
	Point where = {0, 0};
	register Byte *objectPtr, *screenPtr;
	register Byte *objectRow, *screenRow;
	register short longs, bytes, offset;
	short screenRowSize, objectRowSize, numRows;
	
	LocalToGlobal( &where );
	screenRowSize = screenBits.rowBytes * 8;
	screenPtr = screenRow = (Byte *)
		screenBits.baseAddr + (screenRowSize * (long)( rect2->top + where.v ) ) 
							+ (rect2->left + where.h);
	
	objectRowSize = (***pixMap).rowBytes & 0x3FFF;
	objectPtr = objectRow = (Byte *)
		GetPixBaseAddr(*pixMap) + (objectRowSize * (long)rect1->top) + rect1->left;
	
	offset = (long)screenPtr & 3 ;
	rect1->left += offset;
	offset *= 2;
	
	longs = 2 * ( (rect1->right - rect1->left) / 4 );
	bytes = 2 * ( (rect1->right - rect1->left) & 3 ); 
 
	numRows = rect1->bottom - rect1->top;
	
	do
	{
		asm
		{
			lea		@offsetOver, a0
			sub.w	offset, a0
			jmp		(a0)
			
			move.b	(objectPtr)+, (screenPtr)+
			move.b	(objectPtr)+, (screenPtr)+
			move.b	(objectPtr)+, (screenPtr)+
@offsetOver:
		}
		
		asm
		{
			lea		@longOver, a0
			sub.w	longs, a0
			jmp		(a0)
			
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+			
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
			move.l	(objectPtr)+, (screenPtr)+
@longOver:
		}
		
		asm
		{
			lea		@byteOver, a0
			sub.w	bytes, a0
			jmp		(a0)
			
			move.b	(objectPtr)+, (screenPtr)+
			move.b	(objectPtr)+, (screenPtr)+
			move.b	(objectPtr)+, (screenPtr)+
@byteOver:
		}
		
		screenRow += screenRowSize;
		objectRow += objectRowSize;
		screenPtr = screenRow;
		objectPtr = objectRow;
	}
	while( --numRows );
}

// don't use this to copy over 640 bytes...	*Stiles
