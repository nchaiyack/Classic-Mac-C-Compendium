/*

This is an example of how to drag text files to a WASTE instance and
have them imported.  For this example to work, you must be using the 
'hfs ' type WASTE object supplied with this package.

Michael F. Kamprath
kamprath@earthlink.net
19 March 1995
*/

pascal OSErr	MyWEReceiveHandler(	WindowPtr theWindow,
							Ptr handlerRefCon,
							DragReference theDrag)
{
ItemReference	theItem;
Size			itemSize;
Ptr				textP;
StScrpHandle	styleH;
WESoupHandle	soupH;
GrafPtr			savePort;
DocPtr			theDoc;
HFSFlavor		**theHFS;
long			insertionOffset, oldNLines, curEOF;
unsigned short	items, index;
short			fRefNum;	
OSErr			iErr;
Point 			zeroPoint = {0, 0};
Point 			mouse;
LongPt 			dropLocation;
char 			dropEdge;

	if (theWindow != nil)
	{
		GetPort(&savePort);
		SetPort(theWindow);

		theDoc = (DocPtr)GetWRefCon( theWindow ); 	// Or recover your document or
													// WASTE handle how ever you see 
													// fit.
		
		oldNLines = WECountLines( theDoc->theWE );

		CountDragItems(theDrag, &items);

		for (index = 1; index <= items; index++) 	// step through each drag item
													// and check for text files
		{
			GetDragItemReferenceNumber(theDrag, index, &theItem);
				
			if (!(iErr = GetFlavorDataSize(theDrag, theItem, flavorTypeHFS, &itemSize)))
			{
													// Found a text file.  Now import
													// it to the WASTE instance;
													
				theHFS = (HFSFlavor **)NewHandleClear(sizeof(HFSFlavor));
			
				if (theHFS)
				{
					HLockHi( (Handle)theHFS );
					GetFlavorData( theDrag, theItem, flavorTypeHFS, *theHFS, &itemSize, 0L);
					
					if (( (*theHFS)->fileType == 'TEXT' )||( (*theHFS)->fileType == 'ttro' ))
					{
						// Load the text file into memory
						
						iErr = FSpOpenDF( &(*theHFS)->fileSpec, fsCurPerm, &fRefNum );
						iErr = GetEOF(fRefNum,&curEOF);
						
						textP = NewPtrClear( curEOF );
	
						iErr = SetFPos(fRefNum,fsFromStart,0);
						iErr = FSRead( fRefNum, &curEOF, textP);
						iErr = FSClose( fRefNum );
	
						fRefNum = FSpOpenResFile(&(*theHFS)->fileSpec,fsCurPerm);
	
						if ( fRefNum != -1 )
						{
							// has resource fork, so look for style and soup
							
							styleH = (StScrpHandle)Get1Resource( 'styl', 128 );
							if (iErr = ResError())
								styleH = nil;
							else
								DetachResource( (Handle)styleH );

							soupH = (WESoupHandle)Get1Resource( 'soup', 128 );
							if (iErr = ResError())
								soupH = nil;
							else
								DetachResource( (Handle)soupH );

		
							CloseResFile(fRefNum);
						}
						else	
						{
							// does not have resource fork, so no style or soup.
							
							styleH = nil;
							soupH = nil;
						}
	
						// insert the text file into the current position
						
						iErr = GetDragMouse(theDrag, &mouse, &zeroPoint);
						if (iErr != noErr)
							goto cleanup;
						GlobalToLocal(&mouse);

						WEPointToLongPoint(mouse, &dropLocation);
						insertionOffset = WEGetOffset(&dropLocation, &dropEdge, theDoc->theWE);
						WESetSelection( insertionOffset, insertionOffset, theDoc->theWE );
							 
						iErr = WEInsert(textP, curEOF, styleH, soupH, theDoc->theWE);
							
						WESetSelection( insertionOffset, insertionOffset+curEOF, theDoc->theWE );
							
						// dispose of memory
cleanup:						
						HUnlock( (Handle)theHFS );
						DisposHandle( (Handle)theHFS );
						DisposPtr( textP );
						if (styleH) DisposHandle( (Handle)styleH );
						if (soupH) DisposHandle( (Handle)soupH );
						
						// update the WASTE instance if need be
						//
						// your program will probably do ssomething different
						
						if (  WECountLines( theDoc->theWE ) != oldNLines )
							SetVScroll(theDoc);
						theDoc->dirty = true;
						
						SetPort(savePort);
						return (iErr);
					}
					HUnlock( (Handle)theHFS );
					DisposHandle( (Handle)theHFS );
				}
			}
			
			// If the code got to here, that means there was no
			// text file item in the drag.  Let WASTE handle the drag then.
			
			HideDragHilite(theDrag);  // I found I need to do this for cosmetic purposes sometimes
			
			iErr = WEReceiveDrag(theDrag, theDoc->theWE );
			
			// update the WASTE instance if need be
			//
			// your program will probably do ssomething different
			if (  WECountLines( theDoc->theWE ) != oldNLines )
				SetVScroll(theDoc);
			theDoc->dirty = true;
		}
		else
			iErr = dragNotAcceptedErr );

		SetPort(savePort);
	}
	else
		iErr = noErr;
	
	return(noErr);
}
