pascal OSErr	HandleNewHFS(Point *defaultObjectSize,WEObjectReference objectRef);
pascal OSErr	HandleDisposeHFS(WEObjectReference objectRef );
pascal OSErr	HandleDrawHFS(Rect *destRect, WEObjectReference objectRef );
pascal Boolean	HandleClickHFS(	Point hitPt, 
									short modifiers, 
									long clickTime, 
									WEObjectReference objectRef);

