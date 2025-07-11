unit Resources;

interface
	uses
		Matrix, Transformations, OffscreenCore, GrafSysCore, GrafSysScreen, GrafSysObject, ResourceAccess;

{The resource access procedures are version 1.x compatible }
	function GetNewObject (theObjectID: INTEGER): TSObject3D;		{Create new 3D object and fill with data from res}
	function GetNewNamedObject (theName: Str255): TSObject3D;
	procedure SaveObject (Obj: TSObject3D; theName: Str255; ID: integer);	{save data to resource with number = ID }
	procedure SaveNamedObject (Obj: TSObject3D; theName: Str255; var ID: integer);

implementation

	function GetNewObject (theObjectID: INTEGER): TSObject3D;

		var
			theObject: TSObject3D;

	begin
		New(theObject); (* allocate and initialize *)
		theObject.Init;
		LoadObjRes(theObjectID, theObject);

		GetNewObject := theObject;
	end;

	function GetNewNamedObject (theName: Str255): TSObject3D;

		var
			theObject: TSObject3D;

	begin
		New(theObject); (* allocate and initialize *)
		theObject.Init;

		LoadNamedObjRes(theName, theObject); (* load resource from file *)
		GetNewNamedObject := theObject;
	end;

(* SaveObjRes : Save the 3D structure of the object into a '3Dob' resource. Note that         *)
(*                      resources are not identified by ID but by name, so you better pass a name *)

	procedure SaveNamedObject (Obj: TSObject3D; theName: Str255; var ID: integer);

	begin
		SaveNamedObjRes(ID, theName, Obj);
	end;

	procedure SaveObject (Obj: TSObject3D; theName: Str255; ID: integer);	{save data to resource with number = ID }
	begin
		SaveObjRes(ID, theName, Obj);
	end;
end.