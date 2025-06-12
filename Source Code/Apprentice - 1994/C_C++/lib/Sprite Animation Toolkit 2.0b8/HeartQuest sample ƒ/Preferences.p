unit Preferences;

{Preference file handling. If we can write in the app itself, we do, but if not, or if there is already}
{a prefs file, we use a pref file in the system folder.}

{Made with help from the sources of}
{DeHQX v2.0.0 © Peter Lewis, Aug 1991 }
{Many thanks to Peter for sharing his sources with us!}

interface

	uses
		SAT, GameGlobals; {SAT defines ReportStr. GameGlobals defines MyGetIndString}

	var
		applFile, prefFile: integer; {refnummer att göra UseResFile med.}
		sysenv: SysEnvRec;

	function SetPrefFile: boolean;
{SetPrefFile opens the pref file if needed, and sets the variables ApplFile and PrefFile}
{Returns true if a new Preffile was created.}

implementation

{ 1) hitta Preferencefolder}
{ 2) Kolla om prefsfil existerar. I så fall, öppna den och gå till 6. }
{ 3) Hämta resurs och ändra den med ChangeResource. Om det gick, gå till 6.}
{ 4) Skapa prefsfil och öppna den.}
{ 5) Hämta resurser ur programmet och kopiera in i prefsfilen}
{ 6) Hämta resurser och skapa om de inte finns.}

	const
		PrefsFileName = 'HeartQuest Prefs';
		PrefCreator = 'AHjD'; {alt-i följt av 'Ski'}
		PrefType = 'pref';

{ From Folders:}
		kPreferencesFolderType = 'pref';        {preferences for applications go here}

{ From Folders:}
	function FindFolder (vRefNum: INTEGER; folderType: OSType; createFolder: BOOLEAN; var foundVRefNum: INTEGER; var foundDirID: LONGINT): OSErr;
	inline
		$7000, $A823;

	function GetBlessed (vRefNum: INTEGER; var blessed: longint; var volID: integer): OSErr;
		var
			myHPB: HParamBlockRec;
			error: OSErr;
	begin
		blessed := 0;
		with myHPB do
			begin
				ioNamePtr := nil;
				ioVRefNum := vRefNum;	{get for default volume}
				ioVolIndex := 0;		{we’re not making indexed calls}
				error := PBHGetVInfo(@myHPB, FALSE);
				if error = noErr then
					begin
						blessed := ioVFndrInfo[1];
						volID := ioVRefNum;
					end;
			end; {WITH}
		GetBlessed := error;
	end;

	function GetDirID (wdrn: integer; var vrn: integer; var dirID: longInt): OSErr;
		var
			procID: longInt;
			oe: OSErr;
	begin
		oe := GetWDInfo(wdrn, vrn, dirID, procID);
		if oe <> noErr then
			begin
				vrn := wdrn;
				dirID := 0;
			end;
		GetDirID := oe;
	end;

	function GetPrefsFolder (var ovrn: integer): OSErr;
		var
			vrn: integer;
			sDirID: longint;
			oe: OSErr;
			pb: WDPBRec;
			oDirID: longint; {förr var-deklarerad utvariabel}
			oVolID: integer;{förr var-deklarerad utvariabel}
	begin
		oe := SysEnvirons(1, sysenv);
		vrn := sysenv.sysVRefNum;
		if sysenv.systemVersion >= $0700 then {System7}
			begin
				sDirID := 0;
				oe := GetDirID(vrn, vrn, sDirID);
				oe := FindFolder(vrn, kPreferencesFolderType, true, oVolID, oDirID);

{Konvertera FindFolders volref+dirid till WDref}
				pb.ioVRefNum := oVolID;
				pb.ioWDProcID := longint('ERIK');
				pb.ioWDDirID := oDirID;
				pb.ioWDIndex := 0;
{pb.ioWDVRefNum := -1; {???}
				pb.ioNamePtr := nil;
				pb.ioCompletion := nil;
				oe := PBOpenWD(@pb, false);

				ovrn := pb.ioVRefNum; {WDRefNum}
			end
		else
			begin
				ovrn := vrn;
{oe := GetBlessed(ovrn, oDirID, oVolID);}
				oe := noErr;
			end;
		GetPrefsFolder := oe;
	end;

	function SetPrefFile: boolean;
		var
			err: OSErr;
			PrefsFolder{, PrefsVolId}
			: integer;
{PrefsFolderId: longint;}
			h: handle;
			s: str255;
	begin
		applFile := CurResFile; {spara programmets resursfilreferens}
		prefFile := applFile; {if we use no pref, we use the appl}
		SetPrefFile := false;
{ 1) hitta Preferencefolder}
		err := GetPrefsFolder(PrefsFolder);{, PrefsFolderId, PrefsVolId}
		if err <> NoErr then
			ReportStr(MyGetIndString(cantfindsysStrID)); {str 3: Error finding system folder.}

{ 2) Kolla om prefsfil existerar. I så fall, öppna den och gå till 6. }
		prefFile := OpenRFPerm(PrefsFileName, PrefsFolder, FSRdWrPerm);
		if ResError = noErr then
			begin
{Check prefs file version number? That's up to the host.}
			end
		else
			begin {No old preffile. Try to get a resource}
				prefFile := applFile;
{ 3) Hämta resurs och ändra den med ChangeResource. Om det gick, gå till 6.}
				h := Get1Resource(PrefCreator, 0); {Programmets signatur finns alltid om den har en BNDL.}
				if h = nil then
					begin
{Error! Resursen saknas!}
						ReportStr(MyGetIndString(resmissingStrID)); {str 4: Resource missing!}
					end;
				ChangedResource(h);
				if ResError <> noErr then
					begin
{ 4) Skapa prefsfil och öppna den.}
						err := Create(PrefsFileName, PrefsFolder, PrefCreator, PrefType);
						if err = noErr then
							begin
								ReportStr(MyGetIndString(creatingprefStrID)); {str 5: Application locked. Creating prefs file instead.}
								HCreateResFile(PrefsFolder, 0, PrefsFileName);{???}
								if ResError <> noErr then
									ReportStr(MyGetIndString(cantcreateresStrID)); {str 6: Couldn''t create resource fork!}

								prefFile := OpenRFPerm(PrefsFileName, PrefsFolder, FSRdWrPerm);

								if ResError <> noErr then {ResError}
									ReportStr(MyGetIndString(cantopenprefStrID)) {str 7: Failed to open new pref-file!}
								else
									SetPrefFile := true; {new pref file!}
{ 5) Hämta resurser ur programmet och kopiera in i prefsfilen}
							end{Create lyckades}
						else
							ReportStr(MyGetIndString(cantcreateprefStrID)); {str 8: Failed to create prefsfile!}
					end; {Programmet skrivskyddat.}
				ReleaseResource(h); {OK, vi är klara med resursen}
			end; {prefsfil existerade inte}
{ 6) Hämta resurser och skapa om de inte finns.}
	end;

end.