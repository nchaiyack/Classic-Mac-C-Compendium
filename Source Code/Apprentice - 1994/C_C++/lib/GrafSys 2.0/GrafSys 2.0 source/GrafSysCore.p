unit GrafSysCore;

interface

	uses
		Matrix, Transformations;

	const
	{Misc Constants }
		Res3D = '3Dob'; (* This is the 3D object data resource name *)
		Res3DColor = 'lClr'; (* line color resource for 3Dob *)

	{Error Codes defined by the GrafSys }
		cNoFFallocated = -1;
		cOutOfMem = -2;
		cBadMethodCall = -3; {this method should not be called. Instance it yourself!}
		cNothingToInherit = -4; {this error occures when you try to pass on as first thing in the FF chain }
		cTooManyPoints = -5; {model database is full. Maximum # of points exceeded }
		cIllegalPointIndex = -6; {point index specified has no corresponding point in model }
		cTooManyLines = -7; {trying to add a line to a model that is full }
		cIllegalLineIndex = -8; {trying to access a line that does not exist }
		cCantDeletePoint = -9; {the point is still referenced by at least one line}
		cNotOwner = -10; {FF matrix doesn't belong to this object}
		cBadFF = -11;	{FF matrix was NIL}
		cBadFFType = -12; {FF matrix type cant be made current }
		cCantLoadRes = -13; {error loading resource }
		cNo3DWindow = -14; {Window passed is no 3D window }
		cCantCreateOffscreen = -15; {Error occured while trying to allocate Off-Screen PixMap}
		cCantChangeOffscreen = -16; {Error occured while trying to resize or recolor Off-Screen PixMap}
		cNoOSAttached = -17; {the 3D Window passed has no Off-Screen PixMap attached}
		cCantUseWindowCLUT = -18; {Current active window does not use indexed (1-8 bit/pixel) colors}
		cNoActiveOSPixMap = -19; {User did not call BeginOSDraw. no active pix map}
	{other constants}
		cErrorAlertID = 32700; {standard error-handler alert }

	type
		TGenericObject = object
				ErrorCode: integer;
				function Clone: TGenericObject;
				procedure Kill; (* deallocate myself *)
				procedure HandleError;
				procedure ResetError;
				function Test (opcode: integer): integer; (* does anything to check integrity of object 		*)
														(* This incarnation just pops up the Error Dialog	*)
														(* and returns Error Code. Opcode is ignored 		*)
				procedure Init;								(* just so it is defined for every object. Does nothing     *)
														(* but initialization of ErrorCode. Make sure you do this *)
														(* first, then do your own init if no error reported        *)
				procedure Reset;							(* reset this object. doesn't really do anything but provided *)
														(* so all objects support this method *)
			end;


		TMatrixList = object(TGenericObject)
				M: Matrix4;
				next: TMatrixList;
				owner: TGenericObject;							{who owns this matrix? }
				procedure Init; (* set matrix to identity *)
				override;
				procedure Reset;
				override;
				procedure TMRotate (dx, dy, dz: real);		{rotate this matrix further}
				procedure TMScale (dx, dy, dz: real);		{scale this matrix further }
				procedure TMTranslate (dx, dy, dz: real);	{Translate this matrix further }
				procedure TMRotArbAchsis (p, x: Vector4; phi: real); {rotate around achsis defined by p and x}
			end;

		TMatrixInherit = object(TMatrixList) 	{This links a string of inherited matrixes to a 'father' }
				upLink: TMatrixList;						{M contains the result of all MxM multiplie of prior }
													{Matrices. M is updated via the downlink whenever  }
													{father object gets updated. Update from father}
				meTheSon: Tabstract3DObject;			{link to myself. Used for killing son when father gets}
													{kill message, father accesses this field via downlink}
				procedure Init;
				override;
			end;

		TMatrixPass = object(TMatrixList)		{this links a FFmatrix-string (up to here) to a son}
				downLink: TMatrixInherit;				{whenever Transform is called, it will place the current }
													{result of the FF transformations in the downlinks M field }
				meTheFather: Tabstract3DObject;		{link to myself}
				procedure Init;
				override;
			end;


		Tabstract3DObject = object(TGenericObject)
				xTrans, yTrans, zTrans: real;	{translation for origin}
				xScale, yScale, zScale: Real;	{scale factors for object}
				xrot, yrot, zrot: real; 			{ rotation in radiants }
				xForm: Matrix4;					{ result of all xforms including freeform }
				arbRot: Matrix4;					{ arbitrary rotation is stored here }
				currentFF: TMatrixList;			{current FF matrix}
				FFMatrix: TMatrixList;			{ listhead of freeform xform matrices }
				objChanged: Boolean; 			{true if object description changed. A call to calcTransform will reset it}
				versionsID: longint;				{used for sync with eye. If ID <> eyes ID a recalc is required }
				hasChanged: boolean;				{true after calctransform call that changed data. Should be reset by Draw etc. }
				procedure Init;										{initialize object}
				override;
				procedure Reset;									{reset all rot, trans, scale to default }
				override;
				function Clone: TGenericObject;					{must also clone all TMatrix }
				override;												{ATTN: what about inheritances??}
				procedure Translate (dx, dy, dz: real); 			{ xlate object }
				procedure SetTranslation (x, y, z: real);	 	{ set xlation to fixed amount }
				procedure Rotate (dx, dy, dz: real); 			 	{ rotate further (just changes x-,y- and zrot) }
				procedure SetRotation (x, y, z: real); 		 	{ set rot to x,y and z }
				procedure Scale (dx, dy, dz: real); 			 	{scale factor for x, y and z }
				procedure SetScale (x, y, z: real); 			 	{set absolute scale factor }
				procedure RotArb (p, x: Vector4; phi: real);	{rotate around arbitrary axis}
				procedure ResetArb;								{reset arb operator to identity}
				procedure FFTranslate (dx, dy, dz: real); 	 	{xlate current FFMatrix further}
				procedure FFRotate (dx, dy, dz: real); 		 	{rotate current FFMatrix further }
				procedure FFScale (dx, dy, dz: real); 		 	{scale current FFMatrix further }
				procedure FFRotArbAchsis (p, x: Vector4; phi: real); {rotate around achsis defined by p and x}
				procedure FFReset;								{resets *current* FFMatrix to Identity (i.e. not all) }
				function FFNewPostConcat: TMatrixList;		 	{allocate new FF matrix and postconcat it, put it into currentFF}
				function FFNewPreConcat: TMatrixList;			{allocate new FF matrix and preconcat it, put it into currentFF }
				function FFActivate (theFF: TMatrixList): boolean;
																{puts theFF into currentFF. Returns true if successful}
				function FFPassOn: TMatrixPass;					{generates a link for inheritance to 'son' object. I'm Father}
																{it appends a link field to the FFMatrix list}
																{does not update currentFF field}
				procedure FFInherit (var FatherList: TMatrixPass);	{Preconcatenate Fathers list to current FF List }
				procedure CalcTransform;						{calculate xForm from rot,koord,scale and FF }
				function ForeignPoint (p: Vector4): Vector4;	{convert p using current object's xForm. Call Transform first!}
				function WorldToModel (wc: Vector4): Vector4; {xform world coordinates to model coordinates}
				procedure Draw;									{not supported at this level }
				procedure Kill;										{deallocate memory for this object. it will call kill for }
				override;												{all associated FF matrices. If it passes on, it will deallocate }
																{all sons and their FF matrices as well}
			end;

		TPoint3D = object(Tabstract3DObject)
				Koord: Vector4;
				procedure Init;
				override;
				procedure Reset;
				override;
				procedure SetKoords (Koordinates: Vector4);
				function GetKoords: Vector4;
			end;

		TLine3D = object(Tabstract3DObject)
				FromLoc: Vector4;
				ToLoc: Vector4;
				procedure Init;
				override;
				procedure Reset;
				override;
				procedure SetKoords (K1, K2: Vector4);
				procedure GetKoords (var K1, K2: Vector4);
			end;

(* return the error string that belongs to an error code *)

	function InterpretError (theErr: integer): Str255;

implementation

(* return the error string that belongs to an error code *)

	function InterpretError (theErr: integer): Str255;
		var
			theString: Str255;
			numStr: Str255;

	begin
		theString := 'User Error (unknown to the GrafSys)';
		case theErr of
			cNoFFallocated: 
				theString := 'No FreeForm Matrix allocated';
			cOutOfMem: 
				theString := 'Memory Manager returned a NIL handle. Out of Memory';
			cBadMethodCall: 
				theString := 'Method not implemented. You should not call it ';
			cNothingToInherit: 
				theString := 'There are no FF matrices to inherit. You must allocate at least one before inheriting';
			cTooManyPoints: 
				theString := 'You exceeded the maximum number of points per object';
			cIllegalPointIndex: 
				theString := 'The point number you specified does not exist';
			cTooManyLines: 
				theString := 'You exceeded the maximum number of points per object';
			cIllegalLineIndex: 
				theString := 'The line number you specified does not exist';
			cCantDeletePoint: 
				theString := 'The point you want to delete is part of at least one Line. Cannot delete point';
			cNotOwner: 
				theString := 'The matrix is not owned by the object';
			cBadFF: 
				theString := 'The matrix you passed is bad (nil?)';
			cBadFFType: 
				theString := 'The matrix you passed is of a bad type (inherit or passOn)';
			cNo3DWindow: 
				theString := 'Window you passed is no 3D window';
			cCantCreateOffscreen: 
				theString := 'Error occured while trying to allocate Off-Screen PixMap. Probably not enough Memory';
			cCantChangeOffscreen: 
				theString := 'Error occured while trying to resize or recolor Off-Screen PixMap';
			cNoOSAttached: 
				theString := 'The 3D Window passed has no Off-Screen PixMap attached';
			cCantUseWindowCLUT: 
				theString := 'Current active window does not use indexed (1-8 bit/pixel) colors';
			cNoActiveOSPixMap: 
				theString := 'User did not call BeginOSDraw. no active pix map ';

			otherwise
				begin
					NumToString(theErr, numStr);
					theString := Concat('ID = ', numStr, ': ??? (InterpretError does not know this one. Fascinating.)');
				end;
		end; (* case *)
		InterpretError := theString;
	end;

{Clone allocates an exact copy of the object }
{this means that the object points to the same FF matrices if an instance of TabstractObject etc}

	function TGenericObject.Clone: TGenericObject;
		var
			theHandle: Handle;
	begin
		theHandle := Handle(self);
		ErrorCode := HandToHand(theHandle);
		Clone := TGEnericObject(theHandle);
	end;

	procedure TGenericObject.HandleError;
		var
			theString: Str255;
			theItem: integer;

	begin
		NumToString(ErrorCode, theString);
		ParamText('I am sorry, GrafSys reports an error:', Concat('Error #', theString, ' = ', InterpretError(ErrorCode)), 'TGenericObject', '');
		theItem := StopAlert(cErrorAlertID, nil);
	end;

	procedure TGenericObject.ResetError;
	begin
		ErrorCode := NoErr;
	end;

	function TGenericObject.Test (opcode: integer): integer; 	(* does anything to check integrity of object 		*)
																		(* This incarnation just pops up the Error Dialog	*)
																		(* and returns Error Code. Opcode is ignored 		*)
		var
			theString: Str255;
			theItem: integer;

	begin
		if errorCode = noErr then
			begin
				ParamText('GrafSys reports the result of an object-test', 'Test of object succesful.', 'No Error detected', 'TGenericObject.Test');
			end
		else
			begin
				NumToString(ErrorCode, theString);
				ParamText('Test of object failed.', Concat('Error #', theString, ' = ', InterpretError(ErrorCode)), 'TGenericObject.Test', '');
			end;
		theItem := NoteAlert(cErrorAlertID, nil);
		Test := ErrorCode;
	end;

	procedure TGenericObject.Kill;
	begin
		DisposHandle(Handle(self));
	end;

	procedure TGenericObject.Init;
	begin
		ErrorCode := 0;
	end;

	procedure TGenericObject.Reset;
	begin
		ErrorCode := 0;
	end;

	procedure TMatrixList.Init;
	begin
		M := Identity;
		next := nil;
	end;

	procedure TMatrixList.Reset;
	begin
		inherited Reset;
		M := Identity;
	end;

{rotate this matrix further }
	procedure TMatrixList.TMRotate (dx, dy, dz: real);	{rotate this matrix further}
		var
			theMatrix: Matrix4;
			change: boolean;
	begin
		theMatrix := Identity;
		if dx <> 0 then
			begin
				RotX(theMatrix, dx);
				change := TRUE;
			end;
		if dy <> 0 then
			begin
				RotY(theMatrix, dy);
				change := TRUE;
			end;
		if dz <> 0 then
			begin
				RotZ(theMatrix, dz);
				change := TRUE;
			end;
		if change then
			begin
				Self.M := MMult(Self.M, theMatrix); (* postconcatenate operation*)
			end;
	end;

	procedure TMatrixList.TMScale (dx, dy, dz: real);		{scale this matrix further }
	begin
		MScale(self.M, dx, dy, dz);
	end;

	procedure TMatrixList.TMTranslate (dx, dy, dz: real);	{Translate this matrix further }
	begin
		MTranslate(self.M, dx, dy, dz);
	end;

	procedure TMatrixList.TMRotArbAchsis (p, x: Vector4; phi: real); {rotate around achsis defined by p and x}
	begin
		RotArbitraryAchsis(self.M, p, x, phi);
	end;

	procedure TMatrixInherit.Init;
		override;
	begin
		inherited Init;
		upLink := nil;
	end;
	procedure TMatrixPass.Init;
		override;
	begin
		inherited Init;
		downLink := nil;
	end;

	procedure Tabstract3DObject.Init;
		override;
	begin
		inherited Init;
		if ErrorCode <> noErr then
			Exit(init);
		currentFF := nil;
		FFMatrix := nil; {no FF matrix allocated so far }
		xForm := Identity;
		arbRot := Identity;
		xTrans := 0;
		yTrans := 0;
		zTrans := 0;
		xScale := 1;
		yScale := 1;
		zScale := 1;
		xRot := 0;
		yRot := 0;
		zRot := 0;
		ErrorCode := 0;
		objChanged := FALSE;
		hasChanged := FALSE;
		versionsID := -1; (* minimum eye setting is zero *)
	end;

{the clone method will also clone all FFmatrices. If we run into a TMInherit we insert a new passon in father }
{to this clone of son. If we encounter a passon, it is skipped and not cloned									        }

	function Tabstract3DObject.Clone: TGenericObject;					{must also clone all TMatrix }
		override;																	{ATTN: what about inheritances??}

		var
			theClone: Tabstract3DObject;
			walker: TMatrixList; (* with this we walk down the original *)
			CloneWalker: TMatrixList; (* with this we walk down the clone's list *)
			MatrixClone: TMatrixList; (* this is the clone of the matrix *)
			passClone: TMatrixPass; (* this is the clone of father's pass on if we hit a TMatrixInherit *)

	begin
		theClone := Tabstract3DObject(inherited Clone);
		walker := self.FFMatrix; (* get the first ffMatrix *)
		CloneWalker := theClone.FFMatrix;
		while walker <> nil do (* clone if not of type TMatrixPass *)
			begin
				if Member(walker, TMatrixPass) then (* do nothing ! *)
				else
					begin
						MatrixClone := TMatrixList(walker.Clone);
						MatrixClone.owner := theClone;
						TMatrixInherit(MatrixClone).MeTheSon := theClone;
						CloneWalker.next := MatrixClone;
						if Member(MatrixClone, TMatrixInherit) then (* we must insert a inherit block into fathers FF list *)
							begin
								passClone := TMatrixPass(TMatrixInherit(MatrixClone).upLink.Clone); (* owner is already set correctly *)
								passClone.downLink := TMatrixInherit(MatrixClone);
								TMatrixInherit(MatrixClone).upLink.next := passClone; 	(* insert into fathers list. Don't need to update 	*)
																				(*passClone.next  since it already points to the 	*)
																				(*correct object                   						*)
								TMatrixInherit(MatrixClone).upLink := passClone; (* destroy last reference to original *)
							end;
						CloneWalker := CloneWalker.next; (* advance clonewalker *)
						MatrixClone.next := nil; (* failsafe in case a skipped follows last *)
					end;
				walker := walker.next; (* advance *)
			end; (* while *)
		Clone := theClone;
	end;

	procedure Tabstract3DObject.Reset;
		override;
		var
			walker: TMatrixList;

	begin
		inherited Reset;
		xForm := Identity;
		arbRot := Identity;
		xTrans := 0;
		yTrans := 0;
		zTrans := 0;
		xScale := 1;
		yScale := 1;
		zScale := 1;
		xRot := 0;
		yRot := 0;
		zRot := 0;
		ErrorCode := 0;
		objChanged := FALSE;
		hasChanged := TRUE;
(* now reset all FF matrices that are in the chain *)
		walker := FFMatrix;
		while walker <> nil do
			begin
				walker.Reset;
				walker := walker.next;
			end;
	end;

{ xlate object : we only update the _Trans coordinates (objects origin). from this we }
{                       generate the real translation matrix later if we call Transform          }

	procedure Tabstract3DObject.Translate (dx, dy, dz: real);
	begin
		xTrans := xTrans + dx;
		yTrans := yTrans + dy;
		zTrans := zTrans + dz;
		objChanged := TRUE;
	end;

{ xlate object : we only update the _Trans coordinates (objects origin). from this we }
{                       generate the real translation matrix later if we call Transform          }

	procedure Tabstract3DObject.SetTranslation (x, y, z: real);
	begin
		xTrans := x;
		yTrans := y;
		zTrans := z;
		objChanged := TRUE;
	end;

	procedure Tabstract3DObject.Rotate (dx, dy, dz: real); 			 	{ rotate further (just changes x-,y- and zrot) }
	begin
		xRot := xRot + dx;
		yRot := yRot + dy;
		zRot := zRot + dz;
		objChanged := TRUE;
	end;

	procedure Tabstract3DObject.SetRotation (x, y, z: real); 		 	{ set rot to x,y and z }
	begin
		xRot := x;
		yRot := y;
		zRot := z;
		objChanged := TRUE;
	end;

	procedure Tabstract3DObject.Scale (dx, dy, dz: real); 			 	{scale factor for x, y and z }
	begin
		xScale := xScale + dx;
		yScale := yScale + dy;
		zScale := zScale + dz;
		objChanged := TRUE;
	end;


	procedure Tabstract3DObject.SetScale (x, y, z: real); 			 	{set absolute scale factor }
	begin
		xScale := x;
		yScale := y;
		zScale := z;
		objChanged := TRUE;
	end;

	procedure Tabstract3DObject.RotArb (p, x: Vector4; phi: real);
	begin
		RotArbitraryAchsis(arbRot, p, x, phi);
	end;

	procedure Tabstract3DObject.ResetArb;
	begin
		arbRot := Identity;
	end;

{translate currentFF for dx,dy and dz *)
	procedure Tabstract3DObject.FFTranslate (dx, dy, dz: real); 	 	{xlate current FFMatrix further }
	begin
		if currentFF = nil then
			ErrorCode := cNoFFallocated
		else
			begin
				MTranslate(currentFF.M, dx, dy, dz);
				objChanged := TRUE;
			end;
	end;


	procedure Tabstract3DObject.FFRotate (dx, dy, dz: real); 		 	{rotate current FFMatrix further }
		var
			theMatrix: Matrix4;
			change: Boolean;

	begin
		if currentFF = nil then
			ErrorCode := cNoFFallocated
		else
			begin
				theMatrix := Identity;
				if dx <> 0 then
					begin
						RotX(theMatrix, dx);
						change := TRUE;
					end;
				if dy <> 0 then
					begin
						RotY(theMatrix, dy);
						change := TRUE;
					end;
				if dz <> 0 then
					begin
						RotZ(theMatrix, dz);
						change := TRUE;
					end;
				if change then
					begin
						currentFF.M := MMult(currentFF.M, theMatrix); (* postconcatenate operation*)
						objChanged := TRUE;
					end;
			end;
	end;

	procedure Tabstract3DObject.FFScale (dx, dy, dz: real); 		 	{scale current FFMatrix further }

	begin
		if currentFF = nil then
			ErrorCode := cNoFFallocated
		else
			begin
				MScale(currentFF.M, dx, dy, dz);
				objChanged := TRUE;
			end;
	end;

	procedure Tabstract3DObject.FFRotArbAchsis (p, x: Vector4; phi: real); {rotate around achsis defined by p and x}

	begin
		if currentFF = nil then
			ErrorCode := cNoFFallocated
		else
			begin
				RotArbitraryAchsis(currentFF.M, p, x, phi);
				objChanged := TRUE;
			end;
	end;


	procedure Tabstract3DObject.FFReset;								{resets *current* FFMatrix to Identity (i.e. not all) }
	begin
		if currentFF = nil then
			ErrorCode := cNoFFallocated
		else
			begin
				currentFF.M := Identity;
				objChanged := TRUE;
			end;
	end;

	procedure Tabstract3DObject.FFInherit (var FatherList: TMatrixPass);	{concatenate Fathers list to current FF List }
		var
			listTemp: TMatrixInherit;
			walker: TMatrixList;

	begin

		if FatherList = nil then (* nothing to concat *)
			begin
				ErrorCode := cBadFF;
				exit(FFInherit);
			end;

		New(ListTemp); (* allocate memory for matrix and link fields *)
		if ListTemp = nil then
			begin
				ErrorCode := cOutOfMem;
				Exit(FFInherit);
			end;

		listTemp.Init; {init to identity, will be updated when father is Transformed. next is set to nil }
		listTemp.owner := self;

(* now look for end to list and append myself to list *)
		walker := self.FFMatrix; (* load first matrix or nil if none allocated *)
		if walker = nil then
			self.FFMatrix := listTemp
		else
			begin
				while walker.next <> nil do (* look for end of list *)
					walker := walker.next;
				walker.next := ListTemp; (* put at end *)
			end;

(* now link the pass and inherit matrices *)
		ListTemp.next := nil; (* failsafe, postconcat *)
		ListTemp.upLink := fatherList;
		FatherList.downLink := ListTemp;
		listTemp.meTheSon := self; (* required so father can kill son if he himself receives kill msg *)
		objChanged := TRUE;
	end;


	function Tabstract3DObject.FFNewPostConcat: TMatrixList;			{allocate new FF matrix and postconcat it, put it into currentFF}
		var
			temp: TMatrixList;

	begin
		NEW(currentFF);
		if currentFF = nil then
			begin
				ErrorCode := cOutOfMem;
				Exit(FFNewPostConcat);
			end;
		currentFF.Init; (* identity and next = nil *)
		currentFF.owner := self;
		if FFmatrix = nil then
			FFMatrix := currentFF
		else
			begin
				temp := FFMatrix;
				while temp.next <> nil do
					temp := temp.next;
				temp.next := currentFF;
			end;
		objChanged := TRUE;
		FFNewPostConcat := currentFF;
	end;



	function Tabstract3DObject.FFNewPreConcat: TMatrixList;		{puts currentFF first in list }
	begin
		NEW(currentFF);
		if currentFF = nil then
			begin
				ErrorCode := cOutOfMem;
				Exit(FFNewPreConcat);
			end;
		currentFF.Init;
		currentFF.owner := self;
		currentFF.next := self.FFMatrix;
		self.FFMatrix := currentFF;
		objChanged := TRUE;
		FFNewPreConcat := currentFF;
	end;

	function Tabstract3DObject.FFActivate (theFF: TMatrixList): boolean;
																{puts theFF into currentFF. Returns true if successful}

	begin
		if theFF = nil then
			begin
				ErrorCode := cBadFF;
				FFActivate := FALSE;
				Exit(FFActivate);
			end;
		if theFF.owner <> self then
			begin
				ErrorCode := cNotOwner;
				FFActivate := FALSE;
				Exit(FFActivate);
			end;
		if Member(theFF, TMatrixInherit) or Member(theFF, TMatrixPass) then
			begin
				ErrorCode := cBadFFType;
				FFActivate := FALSE;
				Exit(FFActivate);
			end;
		currentFF := theFF;
	end;

{generates a link for inheritance to 'son' object. I'm Father}
{it appends a link field to the FFMatrix list}
{the normal link call would look like this: object1.Inherit(object2.PassOn) }

	function Tabstract3DObject.FFPassOn: TMatrixPass;
		var
			theLink: TMatrixPass;
			temp: TMatrixList;

	begin
		NEW(theLink); (* allocate mem *)
		if theLink = nil then
			begin
				ErrorCode := cOutOfMem;
				Exit(FFPassOn);
			end;

		theLink.Init; (* init it *)
		theLink.owner := self;
		temp := self.FFMatrix;

		if temp = nil then
			FFMatrix := theLink
		else
			begin
				while temp.next <> nil do
					temp := temp.next; (* search for end of transform list *)
				temp.next := theLink; (* append it *)
			end;

		FFPassOn := theLink;
		theLink.meTheFather := self;

		objChanged := TRUE;
	end;

{CalcTransform is the heart of the 3D Package. It calculates the final transformation }
{matrix for this object. It does this in the following steps: }
{ 1. Calculate the Rotation from xRot,yRot and zRot }
{ 2. Add scaling and Translation }
{ 3. walk down the FFMatrix list and multiply these matrices one after }
{     another. If the Type is TMatrixPass, pass the current result to the }
{     son's uplink matrix field                                                                 }
{ 4. the final result is TxFF and is stored in xForm }

	procedure Tabstract3DObject.CalcTransform;								{calculate xForm from rot,koord,scale and FF-List }

		var
		{FreeFormMatrix: Matrix4;}
			tFFM: TMatrixList;

	begin
		xForm := Identity; (* reset to neutral *)
		if xRot <> 0 then
			begin
				RotX(xForm, xRot);
			end;
		if yRot <> 0 then
			begin
				RotY(xForm, yRot);
			end;
		if zRot <> 0 then
			begin
				RotZ(xForm, zRot);
			end;
		MTranslate(xForm, xTrans, yTrans, zTrans);
		MScale(xForm, xScale, yScale, zScale);
		xForm := MMult(xForm, arbRot); (* include the freeform rotations *)

	(* now walk down the FF List and multiply this if neccessary *)
		if FFMatrix <> nil then
			begin
				tFFM := FFMatrix;
				while tFFM <> nil do
					begin
						if Member(tFFM, TMatrixPass) then (* pass down current FreeFormMatrix to linked son *)
							begin
								TMatrixPass(tFFM).downLink.M := xForm; (* changed to pass default as well *)
								if TMatrixPass(tFFM).downLink.meTheSon <> nil then (* check if son's still alive *)
									Tabstract3DObject(TMatrixPass(tFFM).downLink.owner).objChanged := TRUE; (* tell son we changed *)
							end
						else
							begin (* in this case we are either normal or inherited *)
								xForm := MMult(xForm, tFFM.M)
							end;
						tFFM := tFFM.next; (* go to next *)
					end;
			end;

		objChanged := FALSE; (* for auto-detecting a change in the matrix *)
		hasChanged := TRUE;
	end;

{convert p using current object's xForm. Call calcTransform first!}
	function Tabstract3DObject.ForeignPoint (p: Vector4): Vector4;
	begin
		ForeignPoint := VMult(p, xForm)
	end;

	function Tabstract3DObject.WorldToModel (wc: Vector4): Vector4; {xform world coordinates to model coordinates}

		var
			wcOrigin: Vector4;
			Origin: vector4;

	begin
		SetVector4(Origin, 0, 0, 0); (* model coordinate origin *)
		wcOrigin := ForeignPoint(Origin); (* get the origin in wc *)
		WorldToModel := VSub(wc, wcOrigin); (* subtract global origin from global point to get local point *)
	end;

	procedure Tabstract3DObject.Draw;									{not supported at this level }
	begin
		ErrorCode := cBadMethodCall;
	end;

	procedure Tabstract3DObject.Kill;					{deallocate memory for this object. it will call kill for }
		override;												{all associated FF matrices. If it passes on, it will deallocate }
															{all sons and their FF matrices as well}
		var
			currentMatrix: TMatrixList;
			nextMatrix: TMatrixList;
			sonsLink: TMatrixInherit;
			son: Tabstract3DObject;

	begin
	{begin by deallocating all FFMatrices}
		currentMatrix := FFmatrix;
		while currentMatrix <> nil do
			begin
				nextMatrix := currentMatrix.next;
				if Member(currentMatrix, TMatrixPass) then {kill son if alive. this is done in four steps: }
					begin
						sonsLink := TMatrixPass(currentMatrix).downLink; (* get to his link-up matrix (Step 1) *)
						son := sonsLink.meTheSon; (* get himself (Step 2) *)
						if son <> nil then (* son still alive? *)
							son.kill; (* tell son to kill himself and all his FF matrices except his link-up field(Step 3) *)
						sonsLink.kill; (* deallocate mem for sons uplink (Step 4) *)
						currentMatrix.kill; (* finally, deallocate the downlink too *)
					end
				else if Member(currentMatrix, TMatrixInherit) then 	{just eliminate the link to this object and 			}
					begin														{do not deallocate. this is done when father dies.	}
						TMatrixInherit(currentMatrix).meTheSon := nil; 	(* just remove reference *)
					end
				else {the matrix is normal calculation type. deallocate it }
					begin
						currentMatrix.kill;
					end;

				currentMatrix := nextMatrix;
			end; (* while *)
		inherited kill; (* suicide *)
	end;


end. {implementation }