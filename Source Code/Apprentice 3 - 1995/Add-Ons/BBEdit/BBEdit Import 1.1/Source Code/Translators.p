unit Translators;

{ BBIMPORT PROJECT: }
{ Utility routines for handling lists of translator descriptors }

{ ANTI© 1993-1994 Merzwaren }

interface

	const

		kUseDefault = -1;					{ use default translator }

	function BuildFormatList (includeFlags: LongInt;
									var hList: Handle): OSErr;

	function BuildFormatMenu (menuID: Integer;
									hList: Handle;
									var hMenu: MenuHandle): OSErr;

	function CountTranslators (hList: Handle): Integer;

	procedure GetDescriptor (hList: Handle;
									index: Integer;
									descriptor: Ptr);

	procedure GetTranslatorAttributes (hList: Handle;
									index: Integer;
									var creator, fileType: OSType);

	function IsMatch (creator, fileType: OSType;
									hList: Handle;
									index: Integer): Boolean;

	function CanTranslate (spec: FSSpec;
									hList: Handle;
									preferredTranslator: Integer;
									var translator: Ptr): Boolean;

implementation
	uses
		Utilities, XTNDInterface;

	type

		TransDescrArray = array[1..1] of TransDescribe;
		TransDescrArrayPtr = ^TransDescrArray;
		TransDescrArrayHandle = ^TransDescrArrayPtr;

	function CountTranslators (hList: Handle): Integer;
	begin
		CountTranslators := GetHandleSize(hList) div SizeOf(TransDescribe);
	end;  { CountTranslators }

	procedure GetDescriptor (hList: Handle;
									index: Integer;
									descriptor: Ptr);
	begin
		with TransDescrArrayHandle(hList)^^[index] do
			BlockMove(@version, descriptor, SizeOf(TransDescribe));
	end;  { GetDescriptor }

	procedure GetTranslatorAttributes (hList: Handle;
									index: Integer;
									var creator, fileType: OSType);
	begin
		with TransDescrArrayHandle(hList)^^[index] do
			begin
				creator := matches[0].docCreator;
				fileType := matches[0].docType;
			end;  { with }
	end;  { GetTranslatorAttributes }

	function IsMatch (creator, fileType: OSType;
									hList: Handle;
									index: Integer): Boolean;
		var
			j: Integer;
	begin
		IsMatch := false;

		with TransDescrArrayHandle(hList)^^[index] do
			begin

{ if numMatches is zero for the translator, then match all files }
				if (numMatches = 0) then
					IsMatch := true
				else

{ loop through list of matches for this translator }
					for j := numMatches - 1 downto 0 do
						with matches[j] do
							begin
								if (docType = fileType) & ((not exactMatch) | (docCreator = creator)) then
									begin
										IsMatch := true;
										Leave;
									end;
							end;  { for j }

			end;  { with }
	end;  { IsMatch }

	function BuildFormatMenu (menuID: Integer;
									hList: Handle;
									var hMenu: MenuHandle): OSErr;
		var
			theMenu: MenuHandle;
			i, defaultItems, nFormats: Integer;
			saveLock: Boolean;
	begin
		BuildFormatMenu := noErr;
		hMenu := nil;

{ get a handle to the a minimal menu resource }
		theMenu := GetMenu(menuID);
		if (theMenu = nil) then
			begin
				BuildFormatMenu := ResError;
				Exit(BuildFormatMenu);
			end;

{ count the default items in the minimal menu }
		defaultItems := CountMItems(theMenu);

{ count the translators in the list }
		nFormats := CountTranslators(hList);

{ temporarily lock the list }
		saveLock := SetHandleLock(hList, true);

{ loop through list of formats }
		for i := 1 to nFormats do
			with TransDescrArrayHandle(hList)^^[i] do
				begin

{ append the name of this translator to the menu }
					AppendMenu(theMenu, 'x');
					SetItem(theMenu, defaultItems + i, name);
				end;  { for }

{ unlock the list }
		saveLock := SetHandleLock(hList, saveLock);

{ insert the pop-up menu into the hierarchical portion of the menu list }
		InsertMenu(theMenu, -1);

{ return a handle to the menu we have built }
		hMenu := theMenu;

	end;  { BuildFormatMenu }

	function CanTranslate (spec: FSSpec;
									hList: Handle;
									preferredTranslator: Integer;
									var translator: Ptr): Boolean;
		var
			pb: MatchTranslatorBlock;
			pList: TransDescrArrayPtr;
			saveLock: Boolean;
	begin
		CanTranslate := false;
		translator := nil;

{ temporarily lock the list of import translators }
		saveLock := SetHandleLock(hList, true);
		pList := TransDescrArrayHandle(hList)^;

{ prepare the parameter block for XTNDMatchFile }
		BlockClear(@pb, SizeOf(pb));
		pb.allowFlags := allowText + allowImport;
		pb.fileName := @spec.name;
		pb.vRefNum := spec.vRefNum;
		pb.ioDirID := spec.parID;

		if (preferredTranslator = kUseDefault) then
			begin
				pb.chosenTranslator := CountTranslators(hList);
				pb.theChosenTranslator := @pList^[1];
				pb.useTransList := true
			end
		else
			pb.oneTrans := @pList^[preferredTranslator];

{ ask the XTND System to find a match }
{ (but make sure the chosen descriptor is not really one of our native type) }
		CanTranslate := XTNDMatchFile(@pb);
		translator := Ptr(pb.theChosenTranslator);

{ unlock the list }
		saveLock := SetHandleLock(hList, saveLock);

	end;  { CanTranslate }

	function BuildFormatList (includeFlags: LongInt;
									var hList: Handle): OSErr;
		label
			1;
		var
			theList: Handle;
			selectPB: SelectParamBlock;
			err: OSErr;
	begin
		hList := nil;
		theList := nil;

{ allocate a handle that will hold the list of translators we can use }
		err := %_NewHandleClear(0, theList);
		if (err <> noErr) then
			goto 1;

{ gather information about the available XTND text translators }
		BlockClear(@selectPB, SizeOf(selectPB));
		selectPB.pbVersion := 1;
		selectPB.includeFlags := includeFlags;
		selectPB.menuSortFlag := true;
		err := XTNDSelectTranslators(@selectPB, TransDescrHandle(theList));
		if (err <> noErr) then
			goto 1;

{ return a handle to the list we have built }
		hList := theList;
		theList := nil;

{ clear result code }
		err := noErr;

1:
{ clean up }
		ForgetHandle(theList);

{ copy result }
		BuildFormatList := err;

	end;  { BuildFormatList }

end.