unit WEDemoScripting;

{ WASTE DEMO PROJECT: }
{ Minimal Scripting Support: }
{ service "get data" and "set data" events }
{ in which the object descriptor is "contents of selection" }

{ The code in this unit is partly based on public domain code written by: }
{ Ed Lai, Apple Computer Inc. }

{ Copyright © 1993-1995 Marco Piovanelli }
{ All Rights Reserved }

interface
	uses
		WEDemoIntf;

	function InstallCoreHandlers: OSErr;

implementation
	uses
		AEObjects, AERegistry;

	const

		kMaxPropLevel = 2;

	type

		PropArray = array[1..kMaxPropLevel] of DescType;

	procedure InitDesc (var desc: AEDesc);
	begin
		desc.descriptorType := typeNull;
		desc.dataHandle := nil;
	end;  { InitDesc }

	procedure ForgetDesc (var desc: AEDesc);
	begin
		if (desc.dataHandle <> nil) then
			if (AEDisposeDesc(desc) <> noErr) then
				;
		desc.descriptorType := typeNull;
		desc.dataHandle := nil;
	end;  { ForgetDesc }

	function PropertyOf (var spec: AERecord;
									var propLevel: Integer;
									var properties: PropArray): Boolean;
		var
			objSpec: AERecord;
			theType: DescType;
			actualType: DescType;
			actualSize: LongInt;
			key: AEKeyword;
	begin
		PropertyOf := false;			{ we don't know this is a property yet }
		InitDesc(objSpec);

{ if spec is an Apple event (propLevel = 0), use its direct object }
{ otherwise use 'from' parameter of object specifier }
		if (propLevel = 0) then
			key := keyDirectObject
		else
			key := keyAEContainer;

{ extract object specifier }
		if (AEGetParamDesc(spec, key, typeAERecord, objSpec) = noErr) then

{ is this a property? -- get desired class }
			if (AEGetParamPtr(objSpec, keyAEDesiredClass, typeType, actualType, @theType, SizeOf(theType), actualSize) = noErr) then
				if (theType = cProperty) then

{ this is redundunt, but won't hurt to make sure }
					if (AEGetParamPtr(objSpec, keyAEKeyForm, typeEnumerated, actualType, @theType, SizeOf(theType), actualSize) = noErr) then
						if (theType = formPropertyID) then

{ which property? }
							if (AEGetParamPtr(objSpec, keyAEKeyData, typeType, actualType, @theType, SizeOf(theType), actualSize) = noErr) then
								begin

{ we now know what property }
									propLevel := propLevel + 1;
									properties[propLevel] := theType;

{ property of what? }
									if (AESizeOfParam(objSpec, keyAEContainer, actualType, actualSize) = noErr) then
										if (actualType = typeNull) then

{ property of application, we are done }
											PropertyOf := true

										else if (actualType = typeObjectSpecifier) and (propLevel < kMaxPropLevel) then

{ property of another object specifier, do a recursive call if we haven't reached max level }
											PropertyOf := PropertyOf(objSpec, propLevel, properties);
								end;

		ForgetDesc(objSpec);

	end;  { PropertyOf }

	function HandleGetData ({const} var ae, reply: AppleEvent;
									refCon: LongInt): OSErr;
		var
			propLevel: Integer;
			properties: PropArray;
			window: WindowPtr;
			selStart, selEnd: LongInt;
			textDesc, stylesDesc, recordDesc, styledTextDesc: AEDesc;
			requestedType, actualType: DescType;
			actualSize: Size;

		procedure CleanUp;
		begin
			ForgetDesc(textDesc);
			ForgetDesc(stylesDesc);
			ForgetDesc(recordDesc);
			ForgetDesc(styledTextDesc);
		end;  { CleanUp }

		procedure CheckErr (err: OSErr);
		begin
			if (err <> noErr) then
				begin
					HandleGetData := err;
					CleanUp;
					Exit(HandleGetData);
				end;
		end;  { CheckErr }

	begin
		HandleGetData := noErr;
		InitDesc(textDesc);
		InitDesc(stylesDesc);
		InitDesc(recordDesc);
		InitDesc(styledTextDesc);

{ the only Get Data phrase we support is "Get the Contents of the Selection" }
		propLevel := 0; 					{ 0 means we are passing in an Apple Event }
		if PropertyOf(ae, propLevel, properties) & ((propLevel = 2) & (properties[1] = pContents) & (properties[2] = pSelection)) then
			begin

{ make sure there is a window in front }
				window := FrontWindow;
				if (window = nil) then
					CheckErr(errAENoSuchObject);

{ allocate a handle to hold a temporary copy of the selection text }
				CheckErr(NewHandleTemp(0, textDesc.dataHandle));
				textDesc.descriptorType := typeChar;

{ extract the keyAERequestedType parameter, if present }
				if (AEGetParamPtr(ae, keyAERequestedType, typeType, actualType, @requestedType, SizeOf(requestedType), actualSize) = noErr) then
					if (requestedType = typeStyledText) then
						begin

{ our client wants styled text: }
{ allocate a handle to hold a temporary copy of the selection text }
							CheckErr(NewHandleTemp(0, stylesDesc.dataHandle));
							stylesDesc.descriptorType := typeScrapStyles;
						end;

{ make a copy of the selection in the frontmost window }
				WEGetSelection(selStart, selEnd, DocumentPeek(window)^.hWE);
				CheckErr(WECopyRange(selStart, selEnd, textDesc.dataHandle, StScrpHandle(stylesDesc.dataHandle), nil, DocumentPeek(window)^.hWE));

{ fill in the reply Apple event }
				if (stylesDesc.dataHandle = nil) then

{ UNSTYLED TEXT }
{ put the text descriptor into the reply Apple event }
					CheckErr(AEPutParamDesc(reply, keyDirectObject, textDesc))

				else
					begin

{ STYLED TEXT }
{ create an Apple event record to hold text + styles }
						CheckErr(AECreateList(nil, 0, true, recordDesc));

{ add the text descriptor to the record }
						CheckErr(AEPutParamDesc(recordDesc, keyAEText, textDesc));

{ add the styles descriptor to the record }
						CheckErr(AEPutParamDesc(recordDesc, keyAEStyles, stylesDesc));

{ coerce the record into a styled text descriptor }
						CheckErr(AECoerceDesc(recordDesc, typeStyledText, styledTextDesc));

{ put the styled text descriptor into the reply Apple event }
						CheckErr(AEPutParamDesc(reply, keyDirectObject, styledTextDesc));

					end;
			end
		else
			CheckErr(errAENoSuchObject);

{ clean up }
		CleanUp;

	end;  { HandleGetData }

	function HandleSetData ({const} var ae, reply: AppleEvent;
									refCon: LongInt): OSErr;
		var
			propLevel: Integer;
			properties: PropArray;
			window: WindowPtr;
			textDesc, stylesDesc, recordDesc, styledTextDesc: AEDesc;
			dataType: DescType;
			dataSize: Size;

		procedure CleanUp;
		begin
			ForgetDesc(textDesc);
			ForgetDesc(stylesDesc);
			ForgetDesc(recordDesc);
			ForgetDesc(styledTextDesc);
		end;  { CleanUp }

		procedure CheckErr (err: OSErr);
		begin
			if (err <> noErr) then
				begin
					HandleSetData := err;
					CleanUp;
					Exit(HandleSetData);
				end;
		end;  { CheckErr }

	begin
		HandleSetData := noErr;
		InitDesc(textDesc);
		InitDesc(stylesDesc);
		InitDesc(recordDesc);
		InitDesc(styledTextDesc);

{ the only Set Data phrase we support is "Set the Contents of the Selection" }
		propLevel := 0; 				{ 0 means we are passing in an Apple Event }
		if PropertyOf(ae, propLevel, properties) & ((propLevel = 2) & (properties[1] = pContents) & (properties[2] = pSelection)) then
			begin

{ make sure there is a window in front }
				window := FrontWindow;
				if (window = nil) then
					CheckErr(errAENoSuchObject);

{ we expect the data parameter to be either TEXT or STXT }
				CheckErr(AESizeOfParam(ae, keyAEData, dataType, dataSize));
				if (dataType = typeStyledText) then
					begin

{ STYLED TEXT }
{ extract styled text data from the Apple event }
						CheckErr(AEGetParamDesc(ae, keyAEData, typeStyledText, styledTextDesc));

{ coerce the styled text descriptor to an Apple event record }
						CheckErr(AECoerceDesc(styledTextDesc, typeAERecord, recordDesc));

{ extract text + styles from the record descriptor }
						CheckErr(AEGetParamDesc(recordDesc, keyAEText, typeChar, textDesc));
						CheckErr(AEGetParamDesc(recordDesc, keyAEStyles, typeScrapStyles, stylesDesc));
					end
				else

{ UNSTYLED TEXT }
{ extract text data from the Apple event }
					CheckErr(AEGetParamDesc(ae, keyAEData, typeChar, textDesc));

{ insert the text }
				HLock(textDesc.dataHandle);
				CheckErr(WEInsert(textDesc.dataHandle^, GetHandleSize(textDesc.dataHandle), StScrpHandle(stylesDesc.dataHandle), nil, DocumentPeek(window)^.hWE));
				HUnlock(textDesc.dataHandle);

			end
		else
			CheckErr(errAENoSuchObject);

{ clean up }
		CleanUp;

	end;  { HandleSetData }

{ THINK Pascal compiler directive: put the following code in the "Init" segment }
{$S Init}

	function InstallCoreHandlers: OSErr;

		procedure CheckErr (err: OSErr);
		begin
			if (err <> noErr) then
				begin
					InstallCoreHandlers := err;
					Exit(InstallCoreHandlers);
				end;
		end;  { CheckErr }

	begin
		InstallCoreHandlers := noErr;

{ install Apple event handlers for the Required Suite }
		CheckErr(AEInstallEventHandler(kAECoreSuite, kAEGetData, NewAEEventHandlerProc(@HandleGetData), 0, false));
		CheckErr(AEInstallEventHandler(kAECoreSuite, kAESetData, NewAEEventHandlerProc(@HandleSetData), 0, false));

	end;  { InstallCoreHandlers }

end.