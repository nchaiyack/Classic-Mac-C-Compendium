unit LongCoords;

{ WASTE PROJECT: }
{ Long Coordinates: data types and function prototypes }

{ Copyright © 1993-1994 Marco Piovanelli }
{ All Rights Reserved }

interface

	type

		LongPoint = record
				case Integer of
					0: (
							vh: array[VHSelect] of LongInt;
					);
					1: (
							v, h: LongInt;
					);
			end;  { LongPoint }

		LongRect = record
				case Integer of
					0: (
							topLeft, botRight: LongPoint;
					);
					1: (
							top, left, bottom, right: LongInt;
					);
			end;  { LongRect }

	function _WEPinInRange (value: LongInt;
									rangeStart, rangeEnd: LongInt): LongInt;
	procedure WELongPointToPoint (lp: LongPoint;
									var p: Point);
	procedure WEPointToLongPoint (p: Point;
									var lp: LongPoint);
	procedure WESetLongRect (var lr: LongRect;
									left, top, right, bottom: LongInt);
	procedure WELongRectToRect (lr: LongRect;
									var r: Rect);
	procedure WERectToLongRect (r: Rect;
									var lr: LongRect);
	procedure WEOffsetLongRect (var lr: LongRect;
									hOffset, vOffset: LongInt);
	function WELongPointInLongRect (lp: LongPoint;
									lr: LongRect): Boolean;

implementation
end.