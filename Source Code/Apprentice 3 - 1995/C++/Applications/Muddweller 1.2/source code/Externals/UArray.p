{ UArray.p }
{ Copyright � 1989-90 by Apple Computer, Inc. All rights reserved.}

UNIT UArray;

{	This unit implements a dynamically-sized object, called TIntegerArray, which is
	an array of integers.  Each TIntegerArray can hold up to 32767 integers.

	Methods are provided for creating and initializing, as well as adding and
	accessing elements of IntegerArrays.  No methods have been written for
	deleting elements.
}

INTERFACE

	USES
		UObject, UFailure,
		{Types,} Packages;				{ PackIntf }

	CONST
		kElementSize		= 2;		{ size of one element (integer) }
		kIncElements		= 2;		{ number of elements to grow by }

		{ Constants for NewInitializedArray�s zero parameter }
		kZeroValue			= TRUE;		{ initialize all elements to 0 }
		kIntegerValue		= FALSE;	{ initialize elements to 1, 2, 3, ... }

		{ Constant for TIntegerArray.SortBy�s CompareItems function }
		kFirstGreaterThanSecond	=  1;

	TYPE
		TemplateArray		= ARRAY[1..MAXINT] OF INTEGER;
		TemplateArrayPtr	= ^TemplateArray;

		TIntegerArray	= OBJECT (TObject)
			fSize:					INTEGER;		{ the number of valid elements }
			(* fIntegers:	dynamic ARRAY[1..n] of INTEGER *)

			PROCEDURE TIntegerArray.IArray(elements: INTEGER);
			{ Initialize the object to hold the specified number of elements,
			  and set fSize to 0. }

			PROCEDURE TIntegerArray.AddElement(value: INTEGER);
			{ Add an element to the array with the given value. }

			PROCEDURE TIntegerArray.AddUnique(value: INTEGER);
			{ Add the specified value only if it is not yet in the array }

			FUNCTION  TIntegerArray.At(index: INTEGER): INTEGER;
			{ Return the element at the given index. }

			PROCEDURE TIntegerArray.AtPut(index, value: INTEGER);
			{ Change the element at the given index to value. }

			FUNCTION  TIntegerArray.Contains(value: INTEGER): BOOLEAN;
			{ Return TRUE if the array contains the given value. }

			FUNCTION  TIntegerArray.GetSize: INTEGER;
			{ Return the number of valid elements. }

			PROCEDURE TIntegerArray.SortBy(
				FUNCTION CompareItems(index1, index2: INTEGER): INTEGER);
			{ Sort the elements using the given compare routine. }

			PROCEDURE TIntegerArray.Fields(PROCEDURE DoToField(fieldName: Str255;
										fieldAddr: Ptr; fieldType: INTEGER)); OVERRIDE;
			END;


	FUNCTION NewIntegerArray(initialSize: INTEGER): TIntegerArray;
	{ Create a TIntegerArray big enough to hold initialSize elements }

	FUNCTION NewInitializedArray(elements: INTEGER; zero: BOOLEAN): TIntegerArray;
	{ Create a TIntegerArray with the specified number of elements, whose values are
	  initialized to either 0 (zero=T) or consecutive integers 1,2,3,... (zero=F). }

IMPLEMENTATION

{$S AInit}

FUNCTION NewIntegerArray(initialSize: INTEGER): TIntegerArray;

	VAR
		intArray:		TIntegerArray;

	BEGIN
	NEW(intArray);
	FailNIL(intArray);
	intArray.IArray(initialSize);
	NewIntegerArray := intArray;
	END;

FUNCTION NewInitializedArray(elements: INTEGER; zero: BOOLEAN): TIntegerArray;

	VAR
		intArray:		TIntegerArray;
		i:				INTEGER;

	BEGIN
	intArray := NewIntegerArray(elements);
	FOR i := 1 TO elements DO
		IF zero
			THEN intArray.AddElement(0)
			ELSE intArray.AddElement(i);
	NewInitializedArray := intArray;
	END;


PROCEDURE TIntegerArray.IArray(elements: INTEGER);

	VAR
		newSize:		LONGINT;

	BEGIN
	newSize := GetClassSize + (elements * kElementSize);
	SetInstanceSize(newSize);
	fSize := 0;
	END;

{$S Array}

PROCEDURE TIntegerArray.AddElement(value: INTEGER);

	VAR
		extraElements:	INTEGER;
		mySize:			LONGINT;

	BEGIN
	mySize := GetClassSize + (fSize * kElementSize);
	extraElements := (GetInstanceSize - mySize) DIV kElementSize;
	IF extraElements = 0 THEN BEGIN		{ need to grow object to make room }
		mySize := GetInstanceSize + (kIncElements * kElementSize);
		SetInstanceSize(mySize);
		END;

	fSize := fSize + 1;
	AtPut(fSize, value);
	END;

PROCEDURE TIntegerArray.AddUnique(value: INTEGER);
{ Add the specified value only if it is not yet in the array }

	BEGIN
	IF NOT Contains(value) THEN
		AddElement(value);
	END;

FUNCTION TIntegerArray.At(index: INTEGER): INTEGER;

	VAR
		pTemplateArray: TemplateArrayPtr;

	BEGIN
	IF (index <= 0) | (index > fSize) THEN BEGIN
		{$IFC qDebug}
		ProgramBreak('TIntegerArray.At: index exceeds fSize');
		{$ENDC}
		At := 0;
		END
	ELSE BEGIN
		pTemplateArray := TemplateArrayPtr(StripLong(@fSize)+SIZEOF(fSize));
		At := pTemplateArray^[index];
		END;
	END;

PROCEDURE TIntegerArray.AtPut(index, value: INTEGER);

	VAR
		pTemplateArray: TemplateArrayPtr;

	BEGIN
	IF (index <= 0) | (index > fSize) THEN BEGIN
		{$IFC qDebug}
		ProgramBreak('TIntegerArray.AtPut: index exceeds fSize');
		{$ENDC}
		END
	ELSE BEGIN
		pTemplateArray := TemplateArrayPtr(StripLong(@fSize)+SIZEOF(fSize));
		pTemplateArray^[index] := value;
		END;
	END;

{$S Array}

FUNCTION TIntegerArray.Contains(value: INTEGER): BOOLEAN;

	VAR
		p:		TemplateArrayPtr;
		i:		INTEGER;

	BEGIN
	Contains := FALSE;
	p := TemplateArrayPtr(StripLong(@fSize)+SIZEOF(fSize));
	FOR i := 1 TO fSize DO
		IF p^[i] = value THEN BEGIN
			Contains := TRUE;
			LEAVE;
			END;
	END;

FUNCTION TIntegerArray.GetSize: INTEGER;

	BEGIN
	GetSize := fSize;
	END;

{$S Array}

PROCEDURE TIntegerArray.SortBy(FUNCTION CompareItems(index1, index2: INTEGER): INTEGER);
{ Adapted from TList.SortBy; thanks, Steve! Nice variable names, too! }
{ NOTE: This doesn't work with a CompareItems Function that inserts or deletes elements. }

	VAR
		i, j, h:			INTEGER;
		v, item:			INTEGER;

	BEGIN
	{ Do a nice shell sort.  �For _really_ big lists this isn't fast enough }
	{Initialize}
	h := 1;
	REPEAT
		h := 3 * h + 1
	UNTIL h > fSize;

	{Sort}
	REPEAT
		h := h DIV 3;
		FOR i := h + 1 TO fSize DO BEGIN
			v := At(i);
			j := i;
			item := At(j - h);
			WHILE CompareItems(item, v) >= kFirstGreaterThanSecond DO BEGIN
				AtPut(j, item);
				j := j - h;
				IF j <= h THEN LEAVE;
				item := At(j - h);
				END;
			AtPut(j, v);
			END;
	UNTIL h = 1;
	END;
	
{$S AFields}
PROCEDURE TIntegerArray.Fields(PROCEDURE DoToField(fieldName: Str255;
									fieldAddr: Ptr; fieldType: INTEGER)); OVERRIDE;

	VAR
		val, i:			INTEGER;
		aString:		Str255;

	BEGIN
	DoToField('TIntegerArray', NIL, bClass);
	DoToField('fSize', @fSize, bInteger);

	FOR i := 1 to fSize DO BEGIN
		NumToString(i, aString);
		aString := Concat('.At[', aString, ']');
		val := At(i);
		DoToField(aString, @val, bInteger);
		END;

	INHERITED Fields(DoToField);
	END;

END.