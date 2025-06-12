unit DialogExtensions;

{ Pascal Interface to the Macintosh Toolbox: }
{ System 7.0 _DialogMgrDispatch extensions }
{ and dialog utilities dispatched through _CommToolboxDispatch }

{ Copyright © 1991 Apple Computer, Inc. }
{ ANTI© 1993 Merzwaren }

interface

	const

{ DITLMethod }
		overlayDITL = 0;
		appendDITLRight = 1;
		appendDITLBottom = 2;

	type

		DITLMethod = INTEGER;

{ Returns a pointer to the Dialog Manager’s standard dialog filter }
	function GetStdFilterProc (var theProc: ProcPtr): OSErr;
	inline
		$303C, $0203, $AA68;

{ Indicates to the Dialog Manager which item is default.  Will then alias the return & }
{ enter key }
{ to this item, and also bold border it for you (yaaaaa!) }
	function SetDialogDefaultItem (theDialog: DialogPtr;
									newItem: INTEGER): OSErr;
	inline
		$303C, $0304, $AA68;

{ Indicates which item should be aliased to escape or Command - . }
	function SetDialogCancelItem (theDialog: DialogPtr;
									newItem: INTEGER): OSErr;
	inline
		$303C, $0305, $AA68;

{ Tells the Dialog Manager that there is an edit line in this dialog, and }
{ it should track and change to an I-Beam cursor when over the edit line }

	function SetDialogTracksCursor (theDialog: DialogPtr;
									tracks: Boolean): OSErr;
	inline
		$303C, $0306, $AA68;

	function CountDITL (theDialog: DialogPtr): Integer;
	inline
		$3F3C, $0403, $204F, $A08B, $5C4F, $3E80;

	procedure AppendDITL (theDialog: DialogPtr;
									theDITL: Handle;
									method: DITLMethod);
	inline
		$3F3C, $0402, $204F, $A08B, $4FEF, $000C;

	procedure ShortenDITL (theDialog: DialogPtr;
									numberItems: INTEGER);
	inline
		$3F3C, $0404, $204F, $A08B, $504F;

implementation
end.