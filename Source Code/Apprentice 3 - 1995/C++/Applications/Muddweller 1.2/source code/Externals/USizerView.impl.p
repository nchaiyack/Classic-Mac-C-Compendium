CONST
	kMinLogHeight = 56;
	kLineHeight = 12;

{$S AInit}

PROCEDURE InitUSizerView;

	BEGIN
	SetVRect(gVertSBarSetback, 0, 0, kSBarSizeMinus1, 0);
	SetVRect(gHorzSBarSetback, 0, 0, 0, kSBarSizeMinus1);
	SetVRect(gBothSBarSetback, 0, 0, kSBarSizeMinus1, kSBarSizeMinus1);

	gNonPanes := NewIntegerArray(2);
	gNonPanes.AddElement(GetClassIDFromName('TSScrollBar'));
	gNonPanes.AddElement(GetClassIDFromName('TSplitter'));

	RegisterStdType('TSplitter', kStdSplitter);

	IF gDeadStripSuppression THEN BEGIN
		IF Member(TObject(NIL), TSizerView) THEN;
		IF Member(TObject(NIL), THorizontalSizer) THEN;
		IF Member(TObject(NIL), TSplitter) THEN;
		END;
	END;

PROCEDURE ExcludeAsPane(obj: TObject);
{ Make the class of obj ineligible for pane-dom }

	VAR
		itsClassId:		ObjClassId;

	BEGIN
	IF obj <> NIL THEN BEGIN
		itsClassId := GetClassID(obj);
		gNonPanes.AddUnique(itsClassId);
		END;
	END;

{$S ADoCommand}
FUNCTION CloneAView(aView: TView): TView;
{ Clone aView and all its subviews }

	VAR
		newView:		TView;

	PROCEDURE CloneASubview(aSubView: TView);
		VAR	newSubview:		TView;
		BEGIN
		newSubView := CloneAView(aSubView);
		newView.AddSubView(newSubView);
		END;

	PROCEDURE CreateIfScrollBar(aScroller: TScroller; dir: VHSelect);

		BEGIN
		IF aScroller.fScrollBars[dir] <> NIL THEN BEGIN	{ create the dir scroll bar }
			aScroller.fScrollBars[dir] := NIL;
			aScroller.CreateScrollBar(dir);
			END;
		END;

	BEGIN
	newView := TView(aView.Clone);
	newView.fNextHandler := NIL;				{ will be set by AddSubView }
	IF aView.fSubViews <> NIL THEN BEGIN
		newView.fSubViews := NewList;
		aView.EachSubView(CloneASubview);
		END;
	IF Member(aView, TScroller) THEN BEGIN		{ aView is a TScroller }
		CreateIfScrollBar(TScroller(newView), h);
		CreateIfScrollBar(TScroller(newView), v);
		END;
	CloneAView := newView;
	END;

{$S ARes}
FUNCTION OtherDirection(direction: VHSelect): VHSelect;

	BEGIN
	IF direction = v
		THEN OtherDirection := h
		ELSE OtherDirection := v;
	END;

{-------------------------- TVRectList ----------------------}

{$S AOpen}

PROCEDURE TVRectList.IVRectList(initialSize: INTEGER);

	BEGIN
	IDynamicArray(initialSize, SIZEOF(VRect));
	END;

{$S ARes}

FUNCTION TVRectList.At(index: ArrayIndex): VRect;
{ Code stolen from TList.At }

	BEGIN
	IF qRangeCheck & ((index <= kEmptyIndex) | (index > fSize)) THEN BEGIN
		WRITELN('fSize = ', fSize: 1, '  index = ', index: 1);
		ProgramBreak('Range Check in TVRectList.At');
		END;

	At := VRectPtr(ComputeAddress(index))^;
	END;

PROCEDURE TVRectList.AtPut(index: ArrayIndex; newItem: VRect);
{ Code stolen from TList.AtPut }

	BEGIN
	WHILE index > fSize DO			{ add items to list so index is valid }
		InsertElementsBefore(fSize + 1, @gZeroVRect, 1);

	VRectPtr(ComputeAddress(index))^ := newItem;
	END;

PROCEDURE TVrectList.AtCoordPut(index: ArrayIndex; whichCoord: Coordinate;
								newCoord: VCoordinate);
{ Change a single coordinate of the specified VRect to the given value. }

	VAR
		pVR:		VRectPtr;

	BEGIN
	IF qRangeCheck & ((index <= kEmptyIndex) | (index > fSize)) THEN BEGIN
		WRITELN('fSize = ', fSize: 1, '  index = ', index: 1);
		ProgramBreak('Range Check in TVRectList.AtCoordPut');
		END;

	pVR := VRectPtr(ComputeAddress(index));
	CASE whichCoord OF
		kLeftCoord:		pVR^.left := newCoord;
		kRightCoord:	pVR^.right := newCoord;
		kTopCoord:		pVR^.top := newCoord;
		kBottomCoord:	pVR^.bottom := newCoord;
		END;
	END;

{$S ANever}

PROCEDURE TVrectList.AtSetVRect(index: ArrayIndex; left, top, right, bottom: VCoordinate);
{ Change the specified VRect�s coordinates to the given values. }

	VAR
		pVR:		VRectPtr;

	BEGIN
	IF qRangeCheck & ((index <= kEmptyIndex) | (index > fSize)) THEN BEGIN
		WRITELN('fSize = ', fSize: 1, '  index = ', index: 1);
		ProgramBreak('Range Check in TVRectList.AtSetVRect');
		END;

	pVR := VRectPtr(ComputeAddress(index));
	SetVRect(pVR^, left, top, right, bottom);
	END;

{$S APane}

PROCEDURE TVRectList.InsertBefore(index: ArrayIndex; item: VRect);

	BEGIN
	IF qRangeCheck & ((index <= kEmptyIndex) | (index > fSize + 1)) THEN
		BEGIN
		WRITELN('fSize = ', fSize: 1, '  index = ', index: 1);
		ProgramBreak('Range Check in TVRectList.InsertBefore');
		END;

	InsertElementsBefore(index, @item, 1);
	END;

{$S AFields}

PROCEDURE TVRectList.Fields(PROCEDURE DoToField(fieldName: Str255;
												fieldAddr: Ptr;
												fieldType: INTEGER)); OVERRIDE;

	BEGIN
	DoToField('TVRectList', NIL, bClass);
	INHERITED Fields(DoToField);
	END;

PROCEDURE TVRectList.DynamicFields(PROCEDURE DoToField(fieldName: Str255;
													   fieldAddr: Ptr;
													   fieldType: integer)); OVERRIDE;

	FUNCTION DoToElement(theIndex: ArrayIndex): Boolean;

		VAR
			aString:			Str255;

		BEGIN
		DoToElement := FALSE;
		NumToString(theIndex, aString);
		aString := CONCAT('[', aString, ']');
		DoToField(aString, ComputeAddress(theIndex), bVRect);
		END;

	BEGIN
	DoToField('VRect Elements', NIL, bTitle);

	IF EachElementDoTil(DoToElement, kIterateForward) <> kEmptyIndex THEN;
	END;

{-------------------------- TSizerView ----------------------}

{$S AOpen}

PROCEDURE TSizerView.Initialize; OVERRIDE;

	BEGIN
	fPanes := NIL;
	fSetbacks := NIL;
	fSizerRects := NIL;
	fSplitter := NIL;
	fMinPaneSize := kMinSizerPane;
	fSizerThickness := kSizerThickness;
	END;

PROCEDURE TSizerView.IRes(itsDocument: TDocument; itsSuperView: TView; VAR itsParams: Ptr); OVERRIDE;

	BEGIN
	IObject;						(* shouldn�t this be called automagically? *)
	INHERITED IRes(itsDocument, itsSuperView, itsParams);
	InitLists;
	END;

PROCEDURE TSizerView.ISizerView(itsDocument: TDocument;
								itsSuperview: TView;
								itsLocation: VPoint;
								itsSize: VPoint;
								itsHSizeDet, itsVSizeDet: SizeDeterminer);

	BEGIN
	IObject;						(* shouldn�t this be called automagically? *)
	IView(itsDocument, itsSuperview, itsLocation, itsSize, itsHSizeDet, itsVSizeDet);
	InitLists;
	END;

PROCEDURE TSizerView.InitLists;
{ Create and initialize the various lists of subviews and VRects }

	VAR
		aVRectList:		TVRectList;

	BEGIN
	{ Create a list for the subviews }
	fPanes := NewList;
	IF qDebug THEN fPanes.SetEltType('TView');

	{ Initialize list of setback VRects }
	NEW(aVRectList);
	FailNIL(aVRectList);
	aVRectList.IVRectList(2);		{ big enough for two subpanes }
	fSetbacks := aVRectList;

	{ Initialize list of sizer VRects }
	NEW(aVRectList);
	FailNIL(aVRectList);
	aVRectList.IVRectList(1);		{ big enough for one sizer rectangle }
	fSizerRects := aVRectList;
	END;

{$S AOpen}

PROCEDURE TSizerView.FixupPanes(equalSpacing: BOOLEAN);
{ a/k/a �PostRes�; call after IRes (NewTemplateWindow).
  If equalSpacing is TRUE, make the panes of equal size;
  if equalSpacing is FALSE, try to locate the panes using their fSizes. }

	VAR
		panes:				INTEGER;
		sizers:				INTEGER;
		i:					INTEGER;
		thePane:			TView;
		aScroller:			TScroller;
		aVRect:				VRect;
		defaultSizerRects:	TVRectList;

	PROCEDURE GatherPanes(thePane: TView);
	{ Figure out which subviews are panes: all but those whose class id
	  is in the gNonPanes list. }

		BEGIN
		IF NOT gNonPanes.Contains(GetClassId(thePane)) THEN
			fPanes.InsertLast(thePane);
		END;

	PROCEDURE InitSetbacks(thePane: TView);
	{ Initialize the setbacks according to whether each pane has scroll bars or not. }

		BEGIN
		aVRect := gZeroVRect;			{ assume subview fills pane }
		IF Member(thePane, TScroller) THEN
			BEGIN						{ subview is a scroller--check for scroll bars }
			aScroller := TScroller(thePane);
			IF aScroller.fScrollBars[v] <> NIL THEN
				IF aScroller.fScrollBars[h] <> NIL THEN
					aVRect := gBothSBarSetback
				ELSE
					aVRect := gVertSBarSetback
			ELSE
				IF aScroller.fScrollBars[h] <> NIL THEN
					aVRect := gHorzSBarSetback;
			END;
		fSetbacks.AtPut(i, aVRect);
		i := i + 1;
		END;

	FUNCTION PaneCompare(item1, item2: TObject): CompareResult;

		BEGIN
		{ Pane comparison is direction dependent! }
		PaneCompare := SELF.CompareViewLocations(TView(item1), TView(item2));
		END;

	BEGIN								{ TSizerView.FixupPanes }
	EachSubView(GatherPanes);			{ make a list of the panes }
	panes := fPanes.GetSize;
	IF qDebug & (panes = 0) THEN
		ProgramBreak('No panes, no gains!');
	fPanes.SortBy(PaneCompare);			{ order panes by location }

	i := 1;
	fPanes.Each(InitSetbacks);			{ initialize setback VRects }
	sizers := panes - 1;				{ number of sizers needed }

	{ Create a list of sizer rectangles }
	NEW(defaultSizerRects);
	FailNIL(defaultSizerRects);
	defaultSizerRects.IVRectList(sizers);

	IF equalSpacing THEN BEGIN			{ Initialize the sizer rectangles� }
		FOR i := 1 TO sizers DO BEGIN	{ �to be evenly spaced }
			aVRect := GetDefaultSizerRect(i);	{ direction dependent! }
			defaultSizerRects.AtPut(i, aVRect);
			END;
		END
	ELSE BEGIN							{ Initialize the sizer rectangles� }
		FOR i := 1 TO sizers DO BEGIN	{ �from the panes� locations and sizes }
			thePane := TView(fPanes.At(i));
			aVRect := GetNextSizerRect(thePane);
			defaultSizerRects.AtPut(i, aVRect);
			(* make another pass to check the sizer rects for consistency *)
			END;
		END;

	IF sizers > 0 THEN					{ Fit the subviews into the sizer rects }
		SetPanes(defaultSizerRects, kDontInvalidate);
	defaultSizerRects.Free;
	END;

{$S AClose}

PROCEDURE TSizerView.Free; OVERRIDE;

	BEGIN
	FreeIfObject(fSetbacks);
	fSetbacks := NIL;
	FreeIfObject(fSizerRects);
	fSizerRects := NIL;

	INHERITED Free;
	END;

{$S APane}

PROCEDURE TSizerView.AddPane(newPane: TView; itsLocation: VCoordinate; itsSetbacks: VRect);
{ Install thePane at the specified location, shrinking any existing pane accordingly. }

	VAR
		itsPosition:	INTEGER;
		itsSizerRect:	VRect;
		newSizerRects:	TVRectList;

	BEGIN
	IF (fSubViews = NIL) | (fSubViews.GetSameItemNo(newPane) = 0) THEN
		AddSubView(newPane);
	newSizerRects := NIL;

	IF GetNumberOfPanes = 0 THEN
		itsPosition := 0
	ELSE BEGIN		{ need to add a sizer rect }
		itsPosition := FindSizerPosition(itsLocation);
		newSizerRects := TVRectList(fSizerRects.Clone);
		END;

	{ Add the new pane to the list }
	fPanes.InsertBefore(itsPosition+1, newPane);

	{ Add the new pane�s setbacks to the list }
	fSetbacks.InsertBefore(itsPosition+1, itsSetbacks);

	IF newSizerRects <> NIL THEN BEGIN	{ add the new sizer rect and resize panes }
		itsSizerRect := MakeSizerRect(itsLocation);
		newSizerRects.InsertBefore(itsPosition, itsSizerRect);
		SetPanes(newSizerRects, kDontInvalidate);
		newSizerRects.Free;				{ discard temp sizerRects object }
		END;
	END;

PROCEDURE TSizerView.AddEqualPane(newPane: TView; itsPosition: INTEGER; itsSetbacks: VRect);
{ Install thePane at the specified position, resizing all panes equally. }

	VAR
		existingPanes:	INTEGER;
		itsSizerRect:	VRect;
		newSizerRects:	TVRectList;
		s:				INTEGER;

	BEGIN
	IF (fSubViews = NIL) | (fSubViews.GetSameItemNo(newPane) = 0) THEN
		AddSubView(newPane);

	existingPanes := fPanes.GetSize;
	{ constrain value of itsPosition to [1..existingPanes+1] }
	IF itsPosition <= 0 THEN
		itsPosition := 1
	ELSE IF itsPosition > existingPanes+1 THEN
		itsPosition := existingPanes + 1;
	fPanes.InsertBefore(itsPosition, newPane);

	{ Add the new pane�s setbacks to the list }
	fSetbacks.InsertBefore(itsPosition, itsSetbacks);

	IF existingPanes > 0 THEN BEGIN		{ need to add a sizer rect too }
		newSizerRects := TVRectList(fSizerRects.Clone);
		newSizerRects.AtPut(newSizerRects.GetSize+1, gZeroVRect);	{ append a rect }
		FOR s := 1 TO newSizerRects.GetSize DO BEGIN	{ resize each rect }
			itsSizerRect := GetDefaultSizerRect(s);
			newSizerRects.AtPut(s, itsSizerRect);
			END;
		SetPanes(newSizerRects, kDontInvalidate);	{ resize the panes to fit }
		newSizerRects.Free;				{ discard temp sizerRects object }
		END;
	END;

FUNCTION TSizerView.DeletePane(whichPane, whichSizer: INTEGER): TView;

	VAR
		thePane:		TView;
		changedPane:	TView;
		scroller:		TScroller;
		newSize:		VPoint;

	PROCEDURE RemoveIfSubView(sbar: TSScrollBar);

		BEGIN
		IF sbar <> NIL THEN RemoveSubView(sbar);
		END;

	BEGIN
	thePane := TView(fPanes.At(whichPane));
	IF whichPane > whichSizer THEN BEGIN		{ deleting pane after sizer }
		changedPane := TView(fPanes.At(whichPane-1));
		END
	ELSE BEGIN									{ deleting pane before sizer }
		{ changedPane gets re-located and expanded }
		changedPane := TView(fPanes.At(whichPane+1));

		{ If changedPane has become the new first pane, leave room for splitter well }
		IF (whichSizer = 1) & (Member(changedPane, TScroller)) THEN BEGIN
			scroller := TScroller(changedPane);
			IF GetSplitDirection = h
				THEN scroller.fSBarOffsets.top := scroller.fSBarOffsets.top + fSplitter.GetThickness
				ELSE scroller.fSBarOffsets.left := scroller.fSBarOffsets.left + fSplitter.GetThickness;
			(* Locate will call AdjustScrollBars *)
			END;

		changedPane.Locate(thePane.fLocation.h, thePane.fLocation.v, kInvalidate);
		END;

	{ Expand changedPane to include oldPane's area }
	newSize := MergedSize(thePane, changedPane);
	changedPane.Resize(newSize.h, newSize.v, kInvalidate);
	DeletePane := changedPane;					{ return the expanded view }

	RemoveSubView(thePane);
	IF Member(thePane, TScroller) THEN BEGIN	{ remove the TSScrollBars, too }
		scroller := TScroller(thePane);
		RemoveIfSubView(scroller.fScrollBars[h]);
		RemoveIfSubView(scroller.fScrollBars[v]);
		END;
	thePane.Free;								{ frees its TSScrollBars, if any }
	fPanes.AtDelete(whichPane);
	fSetbacks.DeleteElementsAt(whichPane, 1);
	fSizerRects.DeleteElementsAt(whichSizer, 1);
	END;

FUNCTION TSizerView.FindSizerPosition(VAR itsLocation: VCoordinate): INTEGER;
{ Given the desired coordinate for a new sizer rectangle, return its position
  (index) in the list of sizers, modifying itsLocation if necessary. }

	VAR
		sizers:				INTEGER;
		itsPosition:		INTEGER;
		s:					INTEGER;
		firstSizerCoord:	VCoordinate;
		lastSizerCoord:		VCoordinate;

	BEGIN
	sizers := GetNumberOfSizers;
	itsPosition := 0;
	s := 1;
	REPEAT
		firstSizerCoord := GetSizerCoord(s, kGetMinCoord);
		lastSizerCoord := GetSizerCoord(s, kGetMaxCoord);
		IF itsLocation < firstSizerCoord THEN		{ precedes sizer s }
			itsPosition := s
		ELSE IF itsLocation <= lastSizerCoord THEN	{ in sizer s }
			BEGIN
			itsPosition := s+1;
			itsLocation := lastSizerCoord + fMinPaneSize;
			END
		ELSE										{ follows sizer s }
			s := s + 1;
	UNTIL (itsPosition > 0) | (s > sizers);
	IF itsPosition = 0 THEN itsPosition := s;

	FindSizerPosition := itsPosition;
	END;

{$S AOpen}

PROCEDURE TSizerView.InstallSetbacks(whichPane: INTEGER; itsSetbacks: VRect);

	BEGIN
	fSetbacks.AtPut(whichPane, itsSetbacks);
	END;

{$S ASelCommand}

FUNCTION TSizerView.DoMouseCommand(VAR theMouse: Point; VAR info: EventInfo;
								   VAR hysteresis: Point): TCommand; OVERRIDE;

	VAR
		aViewSizer: 		TSizerCommand;
		aViewDeSizer:		TDeSizerCommand;
		whichSizer:			INTEGER;

	BEGIN
	whichSizer := IsPointInSizer(theMouse);
	IF whichSizer = 0 THEN				{ not in a sizer rectangle }
		DoMouseCommand := INHERITED DoMouseCommand(theMouse, info, hysteresis)
	ELSE IF fSplitter = NIL THEN BEGIN	{ ordinary resize }
		NEW(aViewSizer);
		FailNIL(aViewSizer);
		aViewSizer.ISizerCommand(SELF, whichSizer, GetSplitDirection);
		DoMouseCommand := aViewSizer;
		END
	ELSE BEGIN							{ resize or delete pane }
		NEW(aViewDeSizer);
		FailNIL(aViewDeSizer);
		aViewDeSizer.IDeSizerCommand(SELF, whichSizer, GetSplitDirection);
		DoMouseCommand := aViewDeSizer;
		END;
	END;

{$S ARes}

FUNCTION TSizerView.DoSetCursor(localPoint: Point; cursorRgn: RgnHandle): BOOLEAN; OVERRIDE;

	VAR
		sizingCursNum:		Integer;

	BEGIN
	IF IsPointInSizer(localPoint) > 0 THEN BEGIN
		DoSetCursor := TRUE;
		sizingCursNum := GetSizingCursor;
		IF (GetCursor(sizingCursNum) <> NIL)
			THEN SetCursor(GetCursor(sizingCursNum)^^)
			ELSE SetCursor(arrow);
		END
	ELSE
		DoSetCursor := FALSE;
	END;

{$S ARes}

PROCEDURE TSizerView.Draw(area: Rect); OVERRIDE;

	VAR
		bRect:				Rect;
		aRect:				Rect;
		tempRect:			VRect;
		s:					INTEGER;

	BEGIN
	PenNormal;
	FOR s := 1 TO GetNumberOfSizers DO BEGIN			{ draw each sizerRect }
		tempRect := GetSizerRect(s);
		VRectToRect(tempRect, aRect);
		IF SectRect(area, aRect, bRect) THEN
			DrawSizerRect(aRect);
		END;
	END;

{$S ANever}

FUNCTION TSizerView.CompareViewLocations(view1, view2: TView): CompareResult;

	BEGIN
	IF qDebug THEN ProgramBreak('CompareViewLocations must be overridden!');
	END;

PROCEDURE TSizerView.DrawSizerRect(aRect: Rect);

	BEGIN
	IF qDebug THEN ProgramBreak('DrawSizerRect must be overridden!');
	END;

FUNCTION TSizerView.GetNextSizerRect(aPane: TView): VRect;

	BEGIN
	IF qDebug THEN ProgramBreak('GetNextSizerRect must be overridden!');
	END;

FUNCTION TSizerView.GetSizerCoord(whichSizer: INTEGER; min: BOOLEAN): VCoordinate;
{ Return the left/top (min=T) or right/bottom (min=F) coordinate of the specified sizer rect.
  If whichSizer is larger than the number of sizers, return the width/height of the view.
  If whichSizer is 0, return 0. }

	BEGIN
	IF qDebug THEN ProgramBreak('GetSizerCoord must be overridden!');
	END;

FUNCTION TSizerView.GetSizerRect(whichSizer: INTEGER): VRect;

	BEGIN
	IF qDebug THEN ProgramBreak('GetSizerRect must be overridden!');
	END;

FUNCTION TSizerView.GetSizingCursor: INTEGER;

	BEGIN
	IF qDebug THEN ProgramBreak('GetSizingCursor must be overridden!');
	GetSizingCursor := 0;
	END;

FUNCTION TSizerView.IsValidSplitPt(aPoint: VPoint): BOOLEAN;

	BEGIN
	IF qDebug THEN ProgramBreak('IsValidSplitPt must be overridden!');
	END;

FUNCTION TSizerView.MakeSizerRect(itsLocation: VCoordinate): VRect;

	BEGIN
	IF qDebug THEN ProgramBreak('MakeSizerRect must be overridden!');
	END;

PROCEDURE TSizerView.TrackConstrain(anchorPoint, previousPoint: VPoint;
									VAR nextPoint: VPoint); OVERRIDE;
{ Constrain mouse tracking to my interior, allowing for the minimum pane size }

	BEGIN
	IF qDebug THEN ProgramBreak('TrackConstrain must be overridden!');
	END;

{$S ADoCommand}

FUNCTION TSizerView.FindPane(aView: TView): INTEGER;
{ Return the position (index) of aView in the fPanes list }

	BEGIN
	IF aView = NIL
		THEN FindPane := 0
		ELSE FindPane := fPanes.GetSameItemNo(aView);
	END;

FUNCTION TSizerView.FindPaneAt(theCoords: VPoint): TView;
{ Return the pane at the given coordinates }

	FUNCTION ContainsCoords(aView: TView): BOOLEAN;

		VAR	viewFrame:	VRect;

		BEGIN
		aView.GetFrame(viewFrame);
		ContainsCoords := PtInVRect(theCoords, viewFrame);
		END;

	BEGIN
	FindPaneAt := TView(fPanes.FirstThat(ContainsCoords));
	END;

{$S AOpen}
FUNCTION TSizerView.GetDefaultSizerRect(whichSizer: INTEGER): VRect;

	VAR
		paneLength:			LONGINT;
		dir:				VHSelect;

	BEGIN
	dir := OtherDirection(GetSplitDirection);
	paneLength := fSize.vh[dir] DIV GetNumberOfPanes;
	GetDefaultSizerRect := MakeSizerRect(paneLength * whichSizer);
	END;

{$S ADoCommand}

FUNCTION TSizerView.GetMinPaneLength: VCoordinate;
{ Return the minimum width/height of a pane of this view. }

	BEGIN
	GetMinPaneLength := fMinPaneSize;
	END;

PROCEDURE TSizerView.SetMinPaneLength(minLength: VCoordinate);

	BEGIN
	fMinPaneSize := minLength;
	END;

{$S ADoCommand}

FUNCTION  TSizerView.GetNumberOfPanes: INTEGER;

	BEGIN
	GetNumberOfPanes := fPanes.GetSize;
	END;

FUNCTION  TSizerView.GetNumberOfSizers: INTEGER;

	BEGIN
	GetNumberOfSizers := fSizerRects.GetSize;
	END;

{$S ARes}

FUNCTION TSizerView.GetSizerThickness: INTEGER;

	BEGIN
	GetSizerThickness := fSizerThickness;
	END;

PROCEDURE TSizerView.SetSizerThickness(thickness: INTEGER);

	BEGIN
	fSizerThickness := thickness;
	END;

{$S ARes}

FUNCTION TSizerView.GetSplitDirection: VHSelect;

	BEGIN
	GetSplitDirection := h;
	END;

{$S ARes}
PROCEDURE TSizerView.InvalidateFocus; OVERRIDE;

	BEGIN
	gFocusedView := NIL;		{ Eschew traversing all the subviews; just do it! }
	END;

{$S ARes}
FUNCTION TSizerView.IsPointInSizer(localPoint: Point): INTEGER;
{ Return the number of the sizer rect the point is in, or 0 if none. }

	VAR
		aVPt:				VPoint;
		tempRect:			VRect;
		s:					INTEGER;

	BEGIN
	IsPointInSizer := 0;
	PtToVPt(localPoint, aVPt);
	FOR s := 1 TO GetNumberOfSizers DO BEGIN
		tempRect := GetSizerRect(s);
		IF PtInVRect(aVPt, tempRect) THEN BEGIN	{ mouse is in sizer }
			IsPointInSizer := s;
			LEAVE;
			END;
		END;
	END;

{$S APane}
FUNCTION TSizerView.MergedSize(oldPane, changedPane: TView): VPoint;
{ Return the combined size of oldPane and changedPane }

	VAR
		newSize:			VPoint;
		myDir, opDir:		VHSelect;
		sbar:				TSScrollBar;

	BEGIN
	newSize := changedPane.fSize;
	myDir := GetSplitDirection;
	opDir := OtherDirection(myDir);
	newSize.vh[opDir] := newSize.vh[opDir] + oldPane.fSize.vh[opDir] + fSizerThickness;
	IF Member(oldPane, TScroller) THEN BEGIN	{ allow for disappearing scrollbars }
		sbar := TScroller(oldPane).fScrollBars[myDir];
		IF sbar <> NIL THEN
			newSize.vh[opDir] := newSize.vh[opDir] + sbar.fSize.vh[opDir] - 1;
		END;
	MergedSize := newSize;
	END;

{$S ANonRes}

PROCEDURE TSizerView.SetPane(whichSizer: INTEGER; itsSizerRect: VRect);

	BEGIN
	IF qDebug THEN ProgramBreak('SetPane must be overridden!');
	END;

PROCEDURE TSizerView.SetPanes(newSizerRects: TVRectList; invalidate: BOOLEAN);

	BEGIN
	IF qDebug THEN ProgramBreak('SetPanes must be overridden!');
	END;

PROCEDURE TSizerView.SetSizerRect(whichSizer: INTEGER; itsSizerRect: VRect);

	VAR
		aVRect: 			VRect;
		localRect:			VRect;
		r:					Rect;

	BEGIN
	localRect := itsSizerRect;
	aVRect := GetSizerRect(whichSizer);
	IF IsShown & Focus THEN BEGIN						{ code from TView.InvalidVRect }
		ViewToQDRect(localRect, r);
		VisibleRect(r);
		IF NOT EmptyRect(r) THEN
			InvalidRect(r);
		ViewToQDRect(aVRect, r);
		VisibleRect(r);
		IF NOT EmptyRect(r) THEN
			InvalidRect(r);
		END;
	fSizerRects.AtPut(whichSizer, itsSizerRect);
	END;

{$S ANonRes}

PROCEDURE TSizerView.SuperViewChangedSize(delta: VPoint; invalidate: BOOLEAN); OVERRIDE;

	VAR
		lastCommand: TCommand;

	BEGIN
	lastCommand := GetLastCommand;
	IF lastCommand <> NIL THEN
		IF (lastCommand.fCmdNumber = cSizeViews) & (lastCommand.fView = SELF) THEN
			CommitLastCommand;
	INHERITED SuperViewChangedSize(delta, invalidate);
	END;


{$S AFields}

PROCEDURE TSizerView.Fields(PROCEDURE DoToField(fieldName: Str255; fieldAddr: Ptr;
												fieldType: INTEGER)); OVERRIDE;

	BEGIN
	DoToField('TSizerView', NIL, bClass);
	DoToField('fPanes', @fPanes, bObject);
	DoToField('fSetbacks', @fSetbacks, bObject);
	DoToField('fSizerRects', @fSizerRects, bObject);
	DoToField('fMinPaneSize', @fMinPaneSize, bLongint);
	DoToField('fSizerThickness', @fSizerThickness, bInteger);
	DoToField('fSplitter', @fSplitter, bObject);
	INHERITED Fields(DoToField);
	END;

{-------------------------- THorizontalSizer ----------------------}

{$S AOpen}

FUNCTION THorizontalSizer.CompareViewLocations(view1, view2: TView): CompareResult; OVERRIDE;

	VAR
		frame1, frame2:	VRect;

	BEGIN
	view1.GetFrame(frame1);
	view2.GetFrame(frame2);
	IF frame1.top > frame2.top THEN
		CompareViewLocations := kItem1GreaterThanItem2
	ELSE IF frame1.top < frame2.top THEN
		CompareViewLocations := kItem1LessThanItem2
	ELSE
		CompareViewLocations := kItem1EqualItem2;
	END;

{$S ARes}

PROCEDURE THorizontalSizer.DrawSizerRect(aRect: Rect); OVERRIDE;

	BEGIN
	WITH aRect DO BEGIN	{ horizontal splitter }
		MoveTo(left, top);
		LineTo(right, top);
		MoveTo(left, bottom-1);
		LineTo(right, bottom-1);
		END;
	END;

{$S AOpen}
FUNCTION THorizontalSizer.GetNextSizerRect(aPane: TView): VRect; OVERRIDE;

	VAR
		vr:		VRect;

	BEGIN
	aPane.GetFrame(vr);
	GetNextSizerRect := MakeSizerRect(vr.bottom);
	END;

{$S ADoCommand}

FUNCTION THorizontalSizer.GetSizerCoord(whichSizer: INTEGER;
										min: BOOLEAN): VCoordinate; OVERRIDE;

	VAR
		aSizerRect:		VRect;

	BEGIN
	IF whichSizer = 0 THEN
		GetSizerCoord := 0
	ELSE IF whichSizer > fSizerRects.GetSize THEN
		GetSizerCoord := fSize.v
	ELSE BEGIN
		aSizerRect := fSizerRects.At(whichSizer);
		IF min
			THEN GetSizerCoord := aSizerRect.top
			ELSE GetSizerCoord := aSizerRect.bottom;
		END;
	END;

{$S ARes}

FUNCTION THorizontalSizer.GetSizingCursor: INTEGER; OVERRIDE;

	BEGIN
	GetSizingCursor := kHorzSizingCursor;
	END;

FUNCTION THorizontalSizer.GetSizerRect(whichSizer: INTEGER): VRect; OVERRIDE;

	VAR
		vr:		VRect;

	BEGIN
	IF whichSizer = 0 THEN BEGIN
		SetVRect(vr, 0, 0, fSize.h, 0);
		END
	ELSE IF whichSizer > GetNumberOfSizers THEN BEGIN
		SetVRect(vr, 0, fSize.v, fSize.h, fSize.v);
		END
	ELSE
		vr := fSizerRects.At(whichSizer);

	GetSizerRect := vr;
	END;

{$S ADoCommand}
FUNCTION THorizontalSizer.IsValidSplitPt(aPoint: VPoint): BOOLEAN; OVERRIDE;

	BEGIN
	IsValidSplitPt := (aPoint.v >= fMinPaneSize) &
					  (aPoint.v <= fSize.v - fMinPaneSize - fSizerThickness);
	END;

{$S ARes}

FUNCTION THorizontalSizer.MakeSizerRect(itsLocation: VCoordinate): VRect; OVERRIDE;

	VAR
		aVRect:		VRect;

	BEGIN
	SetVRect(aVRect, 0, itsLocation, fSize.h, itsLocation+fSizerThickness);
	MakeSizerRect := aVRect;
	END;

{$S ANonRes}

PROCEDURE THorizontalSizer.Resize(width, height: VCoordinate; invalidate: BOOLEAN); OVERRIDE;

	VAR
		aSizerRect: 		VRect;
		oldHeight:			VCoordinate;
		paneHeight:			VCoordinate;
		previous: 			VCoordinate;
		panes:				INTEGER;
		sizers:				INTEGER;
		checkMinimum:		BOOLEAN;
		newSizerRects:		TVRectList;
		s:					INTEGER;
		delta:				VPoint;

	PROCEDURE NotifyNonPanes(theSubView: TView);

		BEGIN
		IF fPanes.GetSameItemNo(theSubView) = 0 THEN	{ not a pane }
			theSubView.SuperViewChangedSize(delta, invalidate);
		END;

	BEGIN
	sizers := GetNumberOfSizers;
	panes := sizers + 1;
	checkMinimum := (((panes*fMinPaneSize) + (sizers*fSizerThickness)) <= height);
	{$IFC qDebug}
		IF NOT checkMinimum THEN
			ProgramBreak('THorizontalSizer.Resize: height too small for panes!!');
	{$ENDC}

	IF sizers = 0 THEN
		INHERITED Resize(width, height, invalidate)
	ELSE BEGIN
		oldHeight := fSize.v;
		IF (fSize.h <> width) OR (oldHeight <> height) THEN BEGIN
		(*	INHERITED Resize(width, height, FALSE);	{ we�ll notify the subviews ourself } *)
			SetVPt(delta, width - fSize.h, height - fSize.v);
			fSize.h := width;
			fSize.v := height;
			InvalidateFocus;								{Must re-focus because size changed}
			gApplication.InvalidateCursorRgn;				{Must re-calc cursor rgn.}
	
			IF sizers > 0 THEN BEGIN						{ subviews have been installed }
				newSizerRects := TVRectList(fSizerRects.Clone);
				IF oldHeight = height THEN BEGIN			{ adjust width only }
					FOR s := 1 TO sizers DO
						newSizerRects.AtCoordPut(s, kRightCoord, width);
					END
				ELSE BEGIN									{ need to adjust height as well }
					previous := 0;
					FOR s := 1 TO sizers DO BEGIN			{ adjust each sizer rect }
						aSizerRect := GetSizerRect(s);
						paneHeight := aSizerRect.top + delta.v - previous;
						IF checkMinimum & (paneHeight < fMinPaneSize) THEN
							paneHeight := fMinPaneSize;
						IF checkMinimum & (s = 1) & (paneHeight < kMinLogHeight) THEN
							paneHeight := kMinLogHeight;
						SetVRect(aSizerRect, 0, previous + paneHeight, width,
							previous + paneHeight + fSizerThickness);
						newSizerRects.AtPut(s, aSizerRect);
						previous := aSizerRect.bottom;
						END;
					IF checkMinimum THEN BEGIN				{ check last pane }
						previous := height;
						s := sizers;
						REPEAT
							aSizerRect := newSizerRects.At(s);
							paneHeight := previous - aSizerRect.bottom;
(*							paneHeight := fMinPaneSize + ((paneHeight - fMinPaneSize) DIV
								kLineHeight) * kLineHeight; *)
							IF paneHeight < fMinPaneSize THEN paneHeight := fMinPaneSize;
							SetVRect(aSizerRect, 0, previous - paneHeight - fSizerThickness, width,
								previous - paneHeight);
							newSizerRects.AtPut(s, aSizerRect);
							previous := aSizerRect.top;
							s := s - 1;
						UNTIL s = 0;
						END;
					END;
				SetPanes(newSizerRects, invalidate);
				newSizerRects.Free;							{ discard temp sizerRects object }
				END;
	
			EachSubView(NotifyNonPanes);
			END;
		END;
	END;

{$S ADoCommand}

PROCEDURE THorizontalSizer.SetPane(whichSizer: INTEGER; itsSizerRect: VRect); OVERRIDE;
{ A single sizer rectangle has moved, so adjust the panes above and below it }

	VAR
		oldSizerRect:		VRect;
		vp:					VPoint;
		setbacks: 			VRect;
		deltaV:				VCoordinate;
		firstView:			TView;
		secondView:			TView;

	BEGIN
	oldSizerRect := GetSizerRect(whichSizer);
	deltaV := itsSizerRect.top - oldSizerRect.top;
	SetSizerRect(whichSizer, itsSizerRect);

	firstView := TView(fPanes.At(whichSizer));
	vp := firstView.fSize;	(* can�t use GetExtent �cause TScrollers add in translation *)
	firstView.Resize(vp.h, vp.v + deltaV, kInvalidate);

	secondView := TView(fPanes.At(whichSizer+1));
	vp := secondView.fSize;
	secondView.Resize(vp.h, vp.v - deltaV, kInvalidate);
	setbacks := fSetbacks.At(whichSizer+1);
	vp := secondView.fLocation;
	secondView.Locate(vp.h, itsSizerRect.bottom + setbacks.top, kInvalidate);
	END;

{$S ANonRes}

PROCEDURE THorizontalSizer.SetPanes(newSizerRects: TVRectList; invalidate: BOOLEAN); OVERRIDE;

	VAR
		itsSizerRect:		VRect;
		vr:					VRect;
		setbacks: 			VRect;
		theView:			TView;
		s:					INTEGER;
		sizers:				INTEGER;
		pTop:				VCoordinate;
		pBottom:			VCoordinate;

	BEGIN
	sizers := newSizerRects.GetSize;
	pTop := 0;
	FOR s := 1 TO sizers+1 DO BEGIN
		IF s > sizers THEN
			pBottom := fSize.v						{ last pane }
		ELSE BEGIN									{ not the last pane }
			IF invalidate THEN BEGIN				{ invalidate old sizer rectangle }
				itsSizerRect := GetSizerRect(s);
				InvalidVRect(itsSizerRect);
				END;
			itsSizerRect := newSizerRects.At(s);
			IF invalidate THEN						{ invalidate new sizer rectangle }
				InvalidVRect(itsSizerRect);
			pBottom := itsSizerRect.top;
			fSizerRects.AtPut(s, itsSizerRect);
			END;					
		theView := TView(fPanes.At(s));
		setbacks := fSetbacks.At(s);
		theView.Locate(itsSizerRect.left + setbacks.left, pTop + setbacks.top, invalidate);
		theView.Resize(itsSizerRect.right - setbacks.left - setbacks.right,
					   pBottom - pTop - setbacks.top - setbacks.bottom, invalidate);
		pTop := itsSizerRect.bottom;
		END;
(*	IF invalidate THEN ForceRedraw; *)
	END;

{$S ADoCommand}

PROCEDURE THorizontalSizer.TrackConstrain(anchorPoint, previousPoint: VPoint;
										  VAR nextPoint: VPoint); OVERRIDE;
{ Constrain mouse tracking to my interior, allowing for the minimum pane size }

	VAR
		theMin:				LONGINT;
		theMax:				LONGINT;

	BEGIN
	theMin := fMinPaneSize;
	theMax := fSize.v - theMin;
	IF nextPoint.v < theMin
		THEN nextPoint.v := theMin
		ELSE nextPoint.v := Min(nextPoint.v, theMax);
	END;

{-------------------------- TSizerCommand ----------------------}

{$S ASelCommand}

PROCEDURE TSizerCommand.ISizerCommand(itsSizerView: TSizerView; whichSizer: INTEGER;
									  whichWay: VHSelect);

	VAR
		sizerRect: 			VRect;

	BEGIN
	ICommand(cSizeViews, itsSizerView.fDocument, itsSizerView, NIL);
	fCausesChange := FALSE;
	fConstrainsMouse := TRUE;
	fSizerView := itsSizerView;
	fSplitDir := whichWay;
	fWhichSizer := whichSizer;

	sizerRect := fSizerView.GetSizerRect(whichSizer);
	fOldSizerRect := sizerRect;
	IF fSplitDir = kSplitVertically
		THEN fNewEdge := sizerRect.left
		ELSE fNewEdge := sizerRect.top;
	END;										{ TSizerCommand.ISizerCommand }

{$S ADoCommand}

FUNCTION TSizerCommand.TrackMouse(aTrackPhase: TrackPhase; VAR anchorPoint, previousPoint,
								  nextPoint: VPoint; mouseDidMove: BOOLEAN): TCommand; OVERRIDE;
{ Arguments are in fSizerView coordinates }

	VAR
		aVRect: 			VRect;

	BEGIN
	TrackMouse := SELF;							{ keep on trackin� }
	IF fSplitDir = kSplitVertically
		THEN fNewEdge := nextPoint.h
		ELSE fNewEdge := nextPoint.v;

	IF aTrackPhase = trackRelease THEN BEGIN
		aVRect := fOldSizerRect;
		IF PtInVRect(nextPoint, aVRect) THEN
			TrackMouse := NIL;
		END;
	END;										{ TSizerCommand.TrackMouse }

{$S ADoCommand}

PROCEDURE TSizerCommand.TrackFeedback(anchorPoint, nextPoint: VPoint; turnItOn,
									  mouseDidMove: BOOLEAN); OVERRIDE;

	VAR
		viewedRect: 		Rect;
		pState: 			PenState;
		aQDPt:				Point;

	BEGIN
	IF mouseDidMove THEN
		BEGIN
		GetPenState(pState);
		SetPenForFeedback(nextPoint);

		fSizerView.GetQDExtent(viewedRect);

		IF fSplitDir = kSplitVertically THEN
			BEGIN
			MoveTo(nextPoint.h, viewedRect.top);
			Line(0, viewedRect.bottom - viewedRect.top);
			END
		ELSE
			BEGIN
			MoveTo(viewedRect.left, nextPoint.v);
			Line(viewedRect.right - viewedRect.left, 0);
			END;
		SetPenState(pState);
		END;
	END;												{ TSizerCommand.TrackFeedback }

{$S ADoCommand}

PROCEDURE TSizerCommand.TrackConstrain(anchorPoint, previousPoint: VPoint;
									   VAR nextPoint: VPoint); OVERRIDE;

	VAR
		theMin:				LONGINT;
		theMax:				LONGINT;
		temp:				LONGINT;
		minSizerPane:		VCoordinate;

	BEGIN
	minSizerPane := fSizerView.GetMinPaneLength + fSizerView.GetSizerThickness;
	theMin := (fWhichSizer * minSizerPane) + ((fWhichSizer-1) * fSizerView.GetSizerThickness);
	{ don�t let sizer rect overlap preceding ones }
	temp := fSizerView.GetSizerCoord(fWhichSizer-1, kGetMaxCoord);
	theMin := Max(theMin, temp+minSizerPane);
	theMin := Max(theMin, kMinLogHeight);

	IF fSplitDir = kSplitVertically
		THEN temp := fSizerView.fSize.h
		ELSE temp := fSizerView.fSize.v;

	theMax := temp - ((fSizerView.GetNumberOfPanes-fWhichSizer) * minSizerPane);
	{ don�t let sizer rect overlap following ones }
	temp := fSizerView.GetSizerCoord(fWhichSizer+1, kGetMinCoord);
	theMax := Min(theMax, temp-minSizerPane);

	IF fSplitDir = kSplitVertically THEN BEGIN
		IF nextPoint.h < theMin
			THEN nextPoint.h := theMin
			ELSE nextPoint.h := MIN(nextPoint.h, theMax);
		END
	ELSE BEGIN
		IF nextPoint.v < theMin THEN
			nextPoint.v := theMin
		ELSE
			nextPoint.v := MIN(nextPoint.v, theMax);
		END;
(*		nextPoint.v := theMax - ((theMax - nextPoint.v + (kLineHeight DIV 2)) DIV kLineHeight) *
			kLineHeight;
		IF nextPoint.v < theMin THEN nextPoint.v := nextPoint.v + kLineHeight; *)
	END;												{ TSizerCommand.TrackConstrain }

{$S ADoCommand}

PROCEDURE TSizerCommand.DoIt; OVERRIDE;

	VAR
		tempRect:			VRect;

	BEGIN
	tempRect := fSizerView.MakeSizerRect(fNewEdge);
	fSizerView.SetPane(fWhichSizer, tempRect);
	END;												{ TSizerCommand.DoIt }

{$S ADoCommand}

PROCEDURE TSizerCommand.UndoIt; OVERRIDE;

	VAR
		tempRect:			VRect;
		
	BEGIN
	tempRect := fOldSizerRect;
	fSizerView.SetPane(fWhichSizer, tempRect);
	END;												{ TSizerCommand.UndoIt }

{$S ADoCommand}

PROCEDURE TSizerCommand.RedoIt; OVERRIDE;

	BEGIN
	DoIt;
	END;												{ TSizerCommand.RedoIt }

PROCEDURE TSizerCommand.SetPenForFeedback(aPoint: VPoint);
{ Set the pen pattern and size for showing feedback when tracking at the given point }

	BEGIN
	PenPat(black);
	PenSize(1, 1);
	END;

{$S AFields}

PROCEDURE TSizerCommand.Fields(PROCEDURE DoToField(fieldName: Str255; fieldAddr: Ptr;
												   fieldType: INTEGER)); OVERRIDE;

	BEGIN
	DoToField('TSizerCommand', NIL, bClass);
	DoToField('fSizerView', @fSizerView, bObject);
	DoToField('fNewEdge', @fNewEdge, bLongint);
	DoToField('fOldSizerRect', @fOldSizerRect, bVRect);
	DoToField('fSplitDir', @fSplitDir, bByte);
	DoToField('fWhichSizer', @fWhichSizer, bInteger);
	INHERITED Fields(DoToField);
	END;


{---------------------- TSplitterCommand ----------------------}

{$S ASelCommand}

PROCEDURE TSplitterCommand.ISplitterCommand(itsSizerView: TSizerView);
{ Track over the entire SizerView }

	BEGIN
	ISizerCommand(itsSizerView, 0, itsSizerView.GetSplitDirection);
	fCanUndo := FALSE;		{ Sorry, folks! }
	END;

{$S ADoCommand}

PROCEDURE TSplitterCommand.TrackConstrain(anchorPoint, previousPoint: VPoint;
										  VAR nextPoint: VPoint); OVERRIDE;

	BEGIN
	{ do nothing: tracking will be constrained to fSizerView. }
	END;

PROCEDURE TSplitterCommand.DoIt; OVERRIDE;

	VAR
		tempRect:		VRect;
		itsSetbacks:	VRect;
		paneToSplit:	TView;
		newPane:		TView;
		splitter:		TSplitter;
		scroller:		TScroller;
		whichPane:		INTEGER;
		temp:			LONGINT;

	BEGIN
	{ figure out which pane the tracker stopped in }
	tempRect := fSizerView.MakeSizerRect(fNewEdge);
	{$Push} {$H-}
	paneToSplit := fSizerView.FindPaneAt(tempRect.topLeft);
	{$Pop}

	IF paneToSplit <> NIL THEN BEGIN	{ found a pane to split }
		whichPane := fSizerView.FindPane(paneToSplit);
		newPane := CloneAView(paneToSplit);
		IF (Member(newPane, TScroller)) & (whichPane = 1) THEN
			BEGIN	{ remove scrollbar offset for splitter control from new pane }
			splitter := fSizerView.fSplitter;
			scroller := TScroller(newPane);
			temp := splitter.GetThickness;
			WITH scroller.fSBarOffsets DO
				IF fSplitDir = h
					THEN top := top - temp
					ELSE left := left - temp;
			END;
		itsSetbacks := fSizerView.fSetbacks.At(whichPane); { just copy split pane's setbacks }
		fSizerView.AddPane(newPane, fNewEdge, itsSetbacks);
		fSizerView.ForceRedraw;
		END;
	END;

{$S ADoCommand}

PROCEDURE TSplitterCommand.SetPenForFeedback(aPoint: VPoint); OVERRIDE;

	BEGIN
	IF fSizerView.IsValidSplitPt(aPoint)
		THEN PenPat(black)
		ELSE PenPat(ltGray);
	PenSize(1, 1);
	END;

FUNCTION TSplitterCommand.TrackMouse(aTrackPhase: TrackPhase;
									 VAR anchorPoint, previousPoint, nextPoint: VPoint;
									 mouseDidMove: BOOLEAN): TCommand; OVERRIDE;
{ Arguments are in fSizerView coordinates }

	BEGIN
	TrackMouse := SELF;							{ keep on trackin� }
	IF fSplitDir = kSplitVertically
		THEN fNewEdge := nextPoint.h
		ELSE fNewEdge := nextPoint.v;

	IF aTrackPhase = trackRelease THEN
		IF NOT fSizerView.IsValidSplitPt(nextPoint) THEN	{ can�t split here }
			TrackMouse := NIL;					{ �so just forget it }
	END;										{ TSplitterCommand.TrackMouse }

{$S AFields}

PROCEDURE TSplitterCommand.Fields(PROCEDURE DoToField(fieldName: Str255; fieldAddr: Ptr;
											fieldType: INTEGER)); OVERRIDE;

	BEGIN
	DoToField('TSplitterCommand', NIL, bClass);
	INHERITED Fields(DoToField);
	END;

{-------------------------- TSplitter ----------------------}

{$S AOpen}

PROCEDURE TSplitter.IRes(itsDocument: TDocument; itsSuperView: TView;
						 VAR itsParams: Ptr); OVERRIDE;

	BEGIN
	IF qDebug & (NOT Member(itsSuperView, TSizerView)) THEN
		ProgramBreak('Splitter�s superview must be a TSizerView!');
	fSizerView := NIL;

	INHERITED IRes(itsDocument, itsSuperView, itsParams);

	IFinish(itsSuperView);
	END;

PROCEDURE TSplitter.ISplitter(itsDocument: TDocument; itsSuperview: TView;
							  itsLocation: VPoint; itsSize: VPoint);

	BEGIN
	IF qDebug & (NOT Member(itsSuperView, TSizerView)) THEN
		ProgramBreak('Splitter�s superview must be a TSizerView!');

	fSizerView := NIL;
	IControl(itsSuperView, itsLocation, itsSize, sizeRelSuperView, sizeRelSuperView);

	IFinish(itsSuperView);
	ViewEnable(TRUE, kDontRedraw);
	END;

PROCEDURE TSplitter.IFinish(itsSuperView: TView);
{ Finish up initialization by setting the fSizerView and fLocDeterminer fields }

	BEGIN
	fSizerView := TSizerView(itsSuperView);
	IF fSizerView.GetSplitDirection = v THEN BEGIN
		fLocDeterminer[v] := kLocationVaries;
		fLocDeterminer[h] := kLocationFixed;
		END
	ELSE BEGIN	{ h }
		fLocDeterminer[h] := kLocationVaries;
		fLocDeterminer[v] := kLocationFixed;
		END;
	fSizerView.fSplitter := SELF;
	END;

{$S ASelCommand}

FUNCTION TSplitter.DoMouseCommand(VAR theMouse: Point; VAR info: EventInfo;
								  VAR hysteresis: Point): TCommand; OVERRIDE;
{ Override to return a TSplitterCommand. theMouse is in local view coordinates }

	VAR
		aSplitterCommand:	TSplitterCommand;

	BEGIN
	IF fSizerView <> NIL THEN BEGIN
		NEW(aSplitterCommand);
		FailNIL(aSplitterCommand);
		aSplitterCommand.ISplitterCommand(fSizerView);
		DoMouseCommand := aSplitterCommand;
		END
	ELSE
		DoMouseCommand := INHERITED DoMouseCommand(theMouse, info, hysteresis);
	END;

{$S ARes}
PROCEDURE TSplitter.Draw(area: Rect); OVERRIDE;
{ Default is just a black rectangle, like a splitter well }

	BEGIN
	ControlArea(area);
	PenPat(black);
	PenMode(patCopy);
	PaintRect(area);

	INHERITED Draw(area);
	END;

{$S ADoCommand}
FUNCTION TSplitter.GetThickness: LONGINT;

	BEGIN
	{$IFC qDebug}
	IF fSizerView = NIL THEN ProgramBreak('Splitter has no fSizerView!')
	ELSE
	{$ENDC}

	IF fSizerView.GetSplitDirection = h
		THEN GetThickness := fSize.v
		ELSE GetThickness := fSize.h;
	END;

{$S ANonRes}

PROCEDURE TSplitter.SuperViewChangedSize(delta: VPoint; invalidate: BOOLEAN); OVERRIDE;

	VAR
		loc:	VPoint;

	BEGIN
	IF fLocDeterminer[h] | fLocDeterminer[v] THEN BEGIN
		loc := fLocation;
		IF fLocDeterminer[h] THEN
			loc.h := loc.h + delta.h;		{ compute its new horizontal position }
		IF fLocDeterminer[v] THEN
			loc.v := loc.v + delta.v;		{ compute its new vertical position }
		Locate(loc.h, loc.v, invalidate);	{ move it }
		END;
	END;

{$S AFields}

PROCEDURE TSplitter.Fields(PROCEDURE DoToField(fieldName: Str255; fieldAddr: Ptr;
											   fieldType: INTEGER)); OVERRIDE;
	CONST
		kFixedString = 'Fixed';
		kVariesString = 'Varies';

	VAR
		str:	Str255;

	BEGIN
	DoToField('TSplitter', NIL, bClass);
	DoToField('fSizerView', @fSizerView, bObject);
	IF fLocDeterminer[h]
		THEN str := kVariesString
		ELSE str := kFixedString;
	DoToField('fLocDeterminer[h]', @str, bString);
	IF fLocDeterminer[v]
		THEN str := kVariesString
		ELSE str := kFixedString;
	DoToField('fLocDeterminer[v]', @str, bString);
	INHERITED Fields(DoToField);
	END;

{---------------------- TDeSizerCommand ----------------------}

{$S ASelCommand}

PROCEDURE TDeSizerCommand.IDeSizerCommand(itsSizerView: TSizerView; whichSizer: INTEGER;
										  whichWay: VHSelect);
{ Track over the entire SizerView, but show gray when too close to neighboring panes,
  and show white when close to the SizerView ends. }

	VAR
		vr1, vr2:		VRect;
		minPaneSize:	VCoordinate;

	BEGIN
	ISizerCommand(itsSizerView, whichSizer, whichWay);
	fCanUndo := FALSE;					{ Sorry, folks! }

	fResizeOK := TRUE;
	{ build the rectangle in which a valid resize can occur }
	vr1 := itsSizerView.GetSizerRect(whichSizer-1);
	vr2 := itsSizerView.GetSizerRect(whichSizer+1);
	minPaneSize := itsSizerView.GetMinPaneLength;
	IF whichWay = h THEN BEGIN
		fResizeRect.top := vr1.bottom + minPaneSize;
		fResizeRect.left := vr1.left;
		fResizeRect.bottom := vr2.top - minPaneSize;
		fResizeRect.right := vr2.right;
		END
	ELSE BEGIN { v }
		fResizeRect.left := vr1.right + minPaneSize;
		fResizeRect.top := vr1.top;
		fResizeRect.right := vr2.left - minPaneSize;
		fResizeRect.bottom := vr2.bottom;
		END;
	END;

{$S ADoCommand}

PROCEDURE TDeSizerCommand.TrackConstrain(anchorPoint, previousPoint: VPoint;
									   VAR nextPoint: VPoint); OVERRIDE;

	BEGIN
	{ Don�t change nextPoint, so tracking is constrained to fSizerView }

	{ Determine whether nextPoint is a valid place to which to move the sizer bars }
	{$Push} {$H-}
	fResizeOK := PtInVRect(nextPoint, fResizeRect);	{$Pop}
	END;

PROCEDURE TDeSizerCommand.SetPenForFeedback(aPoint: VPoint); OVERRIDE;

	BEGIN
	{$Push} {$H-}
	IF PtInVRect(aPoint, fResizeRect) THEN	{$Pop}
		PenPat(black)
	ELSE IF fSizerView.IsValidSplitPt(aPoint) THEN
		PenPat(ltGray)
	ELSE
		PenPat(white);
	PenSize(1, 1);
	END;

PROCEDURE TDeSizerCommand.DoIt; OVERRIDE;
{ If final mouse position was at either end of the SizerView, delete the sizer bars. }

	VAR
		tempRect:			VRect;
		paneToDelete:		INTEGER;

	FUNCTION IsVpt1LessThan2(vpt1, vpt2: VPoint): BOOLEAN;

		BEGIN
		IF fSplitDir = h
			THEN IsVpt1LessThan2 := (vpt1.v < vpt2.v)
			ELSE IsVpt1LessThan2 := (vpt1.h < vpt2.h);
		END;

	BEGIN
	tempRect := fSizerView.MakeSizerRect(fNewEdge);
	IF NOT fSizerView.IsValidSplitPt(tempRect.topLeft) THEN	{ sizer dragged to end }
		BEGIN									{ delete sizer rect and pane }
		{$Push} {$H-}
		IF IsVpt1LessThan2(tempRect.topLeft, fOldSizerRect.topLeft)	{$Pop}
			THEN paneToDelete := fWhichSizer		{ delete pane before sizer }
			ELSE paneToDelete := fWhichSizer + 1;	{ delete pane after sizer }
		IF fSizerView.DeletePane(paneToDelete, fWhichSizer) = NIL THEN ;
		END
	ELSE										{ just move sizer rect }
		fSizerView.SetPane(fWhichSizer, tempRect);
	END;										{ TDeSizerCommand.DoIt }

FUNCTION TDeSizerCommand.TrackMouse(aTrackPhase: TrackPhase; VAR anchorPoint, previousPoint,
									nextPoint: VPoint; mouseDidMove: BOOLEAN): TCommand; OVERRIDE;
{ Arguments are in fSizerView coordinates }

	VAR
		cmd: 			TCommand;

	BEGIN
	cmd := INHERITED TrackMouse(aTrackPhase, anchorPoint, previousPoint, nextPoint, mouseDidMove);
	IF aTrackPhase = trackRelease THEN
		IF (NOT fResizeOK) &						{ not a valid resizing area }
		   (fSizerView.IsValidSplitPt(nextPoint))	{ not a sizer deletion area }
			THEN cmd := NIL;
	TrackMouse := cmd;
	END;										{ TDeSizerCommand.TrackMouse }

{$S AFields}

PROCEDURE TDeSizerCommand.Fields(PROCEDURE DoToField(fieldName: Str255; fieldAddr: Ptr;
										fieldType: INTEGER)); OVERRIDE;

	BEGIN
	DoToField('TDeSizerCommand', NIL, bClass);
	DoToField('fResizeRect', @fResizeRect, bVRect);
	DoToField('fResizeOK', @fResizeOK, bBoolean);
	INHERITED Fields(DoToField);
	END;

