unit LongControls;

{ WASTE DEMO PROJECT: }
{ Macintosh Controls with Long Values }

{ Copyright © 1993-1995 Marco Piovanelli }
{ All Rights Reserved }

interface
	uses
		Controls;

{ creation and destruction }

	function LCAttach (control: ControlRef): OSErr;
	procedure LCDetach (control: ControlRef);

{ setting variables }

	procedure LCSetValue (control: ControlRef;
									value: LongInt);
	procedure LCSetMin (control: ControlRef;
									min: LongInt);
	procedure LCSetMax (control: ControlRef;
									max: LongInt);

{ getting variables }

	function LCGetValue (control: ControlRef): LongInt;
	function LCGetMin (control: ControlRef): LongInt;
	function LCGetMax (control: ControlRef): LongInt;

{ synchronizing long settings with control (short) settings }

	procedure LCSynch (control: ControlRef);

implementation
	uses
		FixMath, ToolUtils;

{ LongControls private constants and data types }

	const

		kMaxShort = $7FFF;			{ maximum signed short integer }
		kMinShort = $8000;			{ minimum signed short integer }

	type

{ long control auxiliary record used for keeping long settings }
{ a handle to this record is stored in the reference field of the control record }

		LCAuxRec = record
				value: LongInt;				{ long value }
				min: LongInt;				{ long min }
				max: LongInt;				{ long max }
			end;  { LCAuxRec }
		LCAuxPtr = ^LCAuxRec;
		LCAuxHandle = ^LCAuxPtr;

	function LCAttach (control: ControlRef): OSErr;
		var
			aux: Handle;
			pAux: LCAuxPtr;
	begin
		LCAttach := noErr;

{ allocate the auxiliary record that will hold long settings }
		aux := NewHandleClear(SizeOf(LCAuxRec));
		if (aux = nil) then
			begin
				LCAttach := MemError;
				Exit(LCAttach);
			end;

{ store a handle to the auxiliary record in the reference field }
		SetControlReference(control, LongInt(aux));

{ copy current control settings into the auxiliary record }
		pAux := LCAuxHandle(aux)^;
		pAux^.value := GetControlValue(control);
		pAux^.min := GetControlMinimum(control);
		pAux^.max := GetControlMaximum(control);

	end;  { LCAttach }

	procedure LCDetach (control: ControlRef);
		var
			aux: Handle;
	begin
		aux := Handle(GetControlReference(control));
		if (aux <> nil) then
			begin
				SetControlReference(control, 0);
				DisposeHandle(aux);
			end
	end;  { LCDispose }

	procedure LCSetValue (control: ControlRef;
									value: LongInt);
		var
			pAux: LCAuxPtr;
			controlMin, controlMax, newControlValue: Integer;
	begin
		pAux := LCAuxHandle(GetControlReference(control))^;

{ make sure value is in the range min..max }
		if (value < pAux^.min) then
			value := pAux^.min;
		if (value > pAux^.max) then
			value := pAux^.max;

{ save value in auxiliary record }
		pAux^.value := value;

{ calculate new thumb position }
		controlMin := GetControlMinimum(control);
		controlMax := GetControlMaximum(control);
		newControlValue := controlMin + FixRound(FixMul(FixDiv(value - pAux^.min, pAux^.max - pAux^.min), BSL(controlMax - controlMin, 16)));

{ do nothing if the thumb position hasn't changed }
		if (newControlValue <> GetControlValue(control)) then
			SetControlValue(control, newControlValue);

	end;  { LCSetValue }

	procedure LCSetMin (control: ControlRef;
									min: LongInt);
		var
			pAux: LCAuxPtr;
	begin
		pAux := LCAuxHandle(GetControlReference(control))^;

{ make sure min is less than or equal to max }
		if (min > pAux^.max) then
			min := pAux^.max;

{ save min in auxiliary record }
		pAux^.min := min;

{ set contrlMin field to min or kMinShort, whichever is greater }
		if (min < kMinShort) then
			min := kMinShort;
		SetControlMinimum(control, min);

{ reset value }
		LCSetValue(control, pAux^.value);

	end;  { LCSetMin }

	procedure LCSetMax (control: ControlRef;
									max: LongInt);
		var
			pAux: LCAuxPtr;
	begin
		pAux := LCAuxHandle(GetControlReference(control))^;

{ make sure max is greater than or equal to min }
		if (max < pAux^.min) then
			max := pAux^.min;

{ save max in auxiliary record }
		pAux^.max := max;

{ set contrlMax field to max or kMaxShort, whichever is less }
		if (max > kMaxShort) then
			max := kMaxShort;
		SetControlMaximum(control, max);

{ reset value }
		LCSetValue(control, pAux^.value);

	end;  { LCSetMax }

	function LCGetValue (control: ControlRef): LongInt;
	begin
		LCGetValue := LCAuxHandle(GetControlReference(control))^^.value;
	end;  { LCGetValue }

	function LCGetMin (control: ControlRef): LongInt;
	begin
		LCGetMin := LCAuxHandle(GetControlReference(control))^^.min;
	end;  { LCGetMin }

	function LCGetMax (control: ControlRef): LongInt;
	begin
		LCGetMax := LCAuxHandle(GetControlReference(control))^^.max;
	end;  { LCGetMax }

	procedure LCSynch (control: ControlRef);
		var
			controlMin, controlMax, controlValue: Integer;
			pAux: LCAuxPtr;
	begin
		controlMin := GetControlMinimum(control);
		controlMax := GetControlMaximum(control);
		controlValue := GetControlValue(control);
		pAux := LCAuxHandle(GetControlReference(control))^;

{ calculate new long value }
		pAux^.value := pAux^.min + FixMul(FixRatio(controlValue - controlMin, controlMax - controlMin), pAux^.max - pAux^.min);

	end;  { LCSynch }

end.