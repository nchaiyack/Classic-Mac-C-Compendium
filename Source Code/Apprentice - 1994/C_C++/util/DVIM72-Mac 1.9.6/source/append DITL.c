short
AppendDITL( DialogPtr the_dialog, short the_DITL_ID );

/* This code is translated from the Pascal in Tech Note 95. */
short
AppendDITL( DialogPtr the_dialog, short the_DITL_ID )
/* 
	This routine appends all of the items of a specified DITL onto the
	end of a specified DLOG --- we don't even need to know the format of
	the DLOG.
*/
{
	typedef struct {
		Handle	itmHndl; /* handle or procedure pointer for this item */
		Rect	itmRect; /* display rectangle for this item */
		char	itmType;	/* item type for this item - 1-byte number */
		unsigned char	itmData[];  /* length byte, data */
	} DITL_item, *pDITL_item, **hDITL_item;
	
	typedef struct {
		short			dlg_max_index;	/* number of items minus 1 */
		DITL_item	DITL_items[];	/* array of DITL_items */
	} item_list, *p_item_list, **h_item_list;
	
	typedef short *IntPtr;
	
	Point	offset;	/* Used to offset rectangles of items being appended. */
	Rect	maxRect; /* Used to track increase in window size. */
	h_item_list	hDITL;  /* Handle to DITL being appended. */
	pDITL_item	pItem;	/* Pointer to current item being appended. */
	h_item_list	hItems;	/* Handle to DLOG's item list. */
	short			firstItem;	/* Number of where first item is to be appended */
	short			newItems;	/* Count of new items */
	short			dataSize;	/* Size of data for current item */
	short			i, j;			/* Working index */
	short			USB;
	OSErr		err;
	char		*cp1, *cp2; /* working character pointers */
	
/* 
	We use 3 steps:
	1. append the items of the specified DITL onto the existing DLOG
	2. expand the original dialog window as required
	3. return the adjusted number of the first new user item
*/
/* 
	Using the original DLOG
	
	1. Remember the original window size
	2. Set the offset Point to be the bottom of the original window
	3. Subtract 5 pixels from the bottom and right, to be added
	   back later after we have possibly expanded window.
	4. Get working Handle to original item list
	5. Calculate our first item number to be returned to caller
	6. Get locked Handle to DITL to be appended
	7. Calculate count of new items.
*/

	maxRect = the_dialog->portRect;
	offset.v = maxRect.bottom;
	offset.h = 0;
	maxRect.bottom -= 5;
	maxRect.right -= 5;
	hItems = (h_item_list) ((DialogPeek)the_dialog)->items;
	firstItem = (**hItems).dlg_max_index + 2;
	hDITL = (h_item_list) GetResource('DITL', the_DITL_ID );
	HLock( (Handle)hDITL );
	newItems = (**hDITL).dlg_max_index + 1;
/* 
	For each item,
		1. Offset the rectangle to follow the original window.
		2. Make the original window larger if necessary.
		3. Fill in item Handle according to type.
*/
	pItem = &(**hDITL).DITL_items[0];
	for (i = 1; i <= newItems; i++)
	{
		OffsetRect( &pItem->itmRect, offset.h, offset.v );
		UnionRect( &pItem->itmRect, &maxRect, &maxRect );
		
		USB = (unsigned char) pItem->itmData[0];
		/* USB = USB << 8; */
		
		switch ( pItem->itmType & 0x7F )
		{
			case userItem:	/* Can't do anything meaningful with user items. */
				pItem->itmHndl = NIL;
				break;
			case ctrlItem + btnCtrl:
			case ctrlItem + chkCtrl:
			case ctrlItem + radCtrl:
				pItem->itmHndl = (Handle) NewControl( the_dialog, 
					&pItem->itmRect,
					&pItem->itmData[0],	/* title */
					1, /* visible */
					0, 0, 1, /* value, min, max */
					(pItem->itmType & 0x03),	/* procID */
					0 ); /*  refcon */
					break;
			case ctrlItem + resCtrl:
				pItem->itmHndl = (Handle) GetNewControl(
					*((short *) &pItem->itmData[1]),	/* control ID */
					the_dialog );
				(**( (ControlHandle) pItem->itmHndl )).contrlRect =
					pItem->itmRect;	/* give it the right rectangle */
				/* an actionproc for a control should be installed here */
				break;
			case statText:
			case editText: /* Both need Handle to a copy of their text */
				err = PtrToHand( &pItem->itmData[1], /* Start of data */
					&pItem->itmHndl,	/* Address of new Handle */
					(long)USB );	/* length of text */
				break;
			case iconItem:	/* icon needs resource Handle */
				pItem->itmHndl = GetIcon( *((short *) &pItem->itmData[1]) );
				break;
			case picItem:
				pItem->itmHndl = (Handle) GetPicture(
					*((short *) &pItem->itmData[1]) );
				break;
			default:
				pItem->itmHndl = NIL;
				break;
		} /* end of switch */
		dataSize = (USB + 1) & 0xFFFE; /* round up to even number */
		/* now advance to next item */
		pItem = (pDITL_item) ((char *)pItem + dataSize + sizeof(DITL_item));
	} /* endfor */
	err = PtrAndHand( &(**hDITL).DITL_items,
		(Handle)hItems,
		GetHandleSize( (Handle)hDITL) );
	(**hItems).dlg_max_index += newItems;
	HUnlock( (Handle)hDITL );
	ReleaseResource( (Handle)hDITL );
	maxRect.bottom += 5;
	maxRect.right += 5;
	SizeWindow( the_dialog, maxRect.right, maxRect.bottom, 1 );
	return( firstItem );
}