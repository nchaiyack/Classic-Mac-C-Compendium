unit DemoFiles;

{ WASTE DEMO PROJECT: }
{ File Handling }

{ Copyright © 1993-1994 Merzwaren }
{ All Rights Reserved }

interface
	uses
		DemoIntf;

	function ReadTextFile (pFileSpec: FSSpecPtr;
									hWE: WEHandle): OSErr;
	function WriteTextFile (pFileSpec: FSSpecPtr;
									hWE: WEHandle): OSErr;

implementation

	function ReadTextFile (pFileSpec: FSSpecPtr;
									hWE: WEHandle): OSErr;
		var
			dataForkRefNum, resForkRefNum: Integer;
			hText: Handle;
			hStyleScrap: StScrpHandle;
			fileSize: LongInt;

		procedure CleanUp;
			var
				err: OSErr;
		begin
			if (hText <> nil) then
				begin
					DisposeHandle(hText);
					hText := nil;
				end;
			if (dataForkRefNum <> 0) then
				begin
					err := FSClose(dataForkRefNum);
					dataForkRefNum := 0;
				end;
			if (resForkRefNum <> 0) then
				begin
					CloseResFile(resForkRefNum);
					resForkRefNum := 0;
				end;
		end;  { CleanUp }

		procedure CheckErr (err: OSErr);
		begin
			if (err <> noErr) then
				begin
					ReadTextFile := err;
					CleanUp;
					Exit(ReadTextFile);
				end;
		end;  { CheckErr }

	begin
		ReadTextFile := noErr;
		dataForkRefNum := 0;
		resForkRefNum := 0;
		hText := nil;

{ open the data fork with read-only permission }
		CheckErr(FSpOpenDF(pFileSpec^, fsRdPerm, dataForkRefNum));

{ get data fork size }
		CheckErr(GetEOF(dataForkRefNum, fileSize));

{ try to allocate a handle that large; use temporary memory if available }
		CheckErr(NewHandleTemp(fileSize, hText));

{ read in the text }
		CheckErr(FSRead(dataForkRefNum, fileSize, hText^));

{ install the text in the WE instance }
		CheckErr(WEUseText(hText, hWE));
		hText := nil;

{ see if the file has a resource fork }
		resForkRefNum := FSpOpenResFile(pFileSpec^, fsRdPerm);
		if (resForkRefNum <> -1) then
			begin

{ look for a style scrap resource (get the first one; the resource ID doesn't matter) }
				hStyleScrap := StScrpHandle(Get1IndResource(kTypeStyleScrap, 1));

{ if there's a style scrap, apply it to the text }
				if (hStyleScrap <> nil) then
					begin
						WESetSelection(0, maxLongInt, hWE);
						CheckErr(WEUseStyleScrap(hStyleScrap, hWE));
						WESetSelection(0, 0, hWE);
					end;
			end;

{ clean up and exit }
		CleanUp;

	end;  { ReadTextFile }

	function WriteTextFile (pFileSpec: FSSpecPtr;
									hWE: WEHandle): OSErr;
		var
			dataForkRefNum, resForkRefNum: Integer;
			hText: Handle;
			hStyleScrap: StScrpHandle;
			fileSize: LongInt;
			deleteErr: OSErr;

		procedure CleanUp;
			var
				err: OSErr;
		begin

			if (dataForkRefNum <> 0) then
				begin
					err := FSClose(dataForkRefNum);
					dataForkRefNum := 0;
				end;

			if (hStyleScrap <> nil) then
				begin
					ReleaseResource(Handle(hStyleScrap));
					hStyleScrap := nil;
				end;

			if (resForkRefNum <> 0) then
				begin
					CloseResFile(resForkRefNum);
					resForkRefNum := 0;
				end;

		end;  { CleanUp }

		procedure CheckErr (err: OSErr);
		begin
			if (err <> noErr) then
				begin
					WriteTextFile := err;
					ErrorAlert(err);
					CleanUp;
					Exit(WriteTextFile);
				end;
		end;  { CheckErr }

	begin
		WriteTextFile := noErr;
		dataForkRefNum := 0;
		resForkRefNum := 0;
		hStyleScrap := nil;

{ delete existing file, if any }
		deleteErr := FSpDelete(pFileSpec^);
		if (deleteErr <> fnfErr) then
			CheckErr(deleteErr);

{ create a new TEXT file }
		FSpCreateResFile(pFileSpec^, kAppSignature, kTypeText, 0);
		CheckErr(ResError);

{ open the data fork for writing }
		CheckErr(FSpOpenDF(pFileSpec^, fsRdWrPerm, dataForkRefNum));

{ get the text handle from the WE instance }
{ WEGetText returns the original handle, not a copy, so don't dispose of it! }
		hText := WEGetText(hWE);
		fileSize := GetHandleSize(hText);

{ write the text }
		CheckErr(FSWrite(dataForkRefNum, fileSize, hText^));

{ open the resource file for writing }
		resForkRefNum := FSpOpenResFile(pFileSpec^, fsRdWrPerm);
		CheckErr(ResError);

{ allocate a temporary handle to hold the style scrap }
		CheckErr(NewHandleTemp(0, hStyleScrap));

{ create a style scrap describing the styles of the whole text }
		CheckErr(WECopyRange(0, maxLongInt, nil, hStyleScrap, hWE));

{ make the style scrap handle a resource handle }
		AddResource(Handle(hStyleScrap), kTypeStyleScrap, 1000, '');
		CheckErr(ResError);

{ write the style scrap to the resource file }
		WriteResource(Handle(hStyleScrap));
		CheckErr(ResError);

{ clean up }
		CleanUp;

	end;  { WriteTextFile }

end.