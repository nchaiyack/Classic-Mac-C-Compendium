unit ICAPI;

interface

	uses
{$ifc undefined THINK_Pascal}
		Types, Files, QuickDraw, AppleTalk, Aliases, 
{$endc}
		ICTypes, ICKeys;

	function ICStart (var inst: ICInstance; creator: OSType): ICError;
	(* call at application initialisation *)

	function ICStop (inst: ICInstance): ICError;
	(* call at application termination *)

	function ICFindConfigFile (inst: ICInstance; count: integer; folders: ICDirSpecArrayPtr): ICError;
	(* count is the number of ICDirSpecs that are valid in folders *)
	(* searches the specified folders first, then backs out to preferences folder *)
	(* don't you worry about how it finds the file (; *)
	(* you can pass nil to folders if count is 0 *)

	function ICFindUserConfigFile (inst: ICInstance; where: ICDirSpec): ICError;
	(* requires IC 1.1 *)
	(* similar to ICFindConfigFile except *)
	(* For use with double clickable preference files, this searches only the where directory *)

	function ICSpecifyConfigFile (inst: ICInstance; config: FSSpec): ICError;
	(* for use *only* by Internet Configuration application *)

	function ICGetSeed (inst: ICInstance; var seed: longint): ICError;
	(* returns current seed for prefs file *)
	(* this seed changes every time a preference is modified *)
	(* poll this to detect preference changes by other applications *)

	function ICGetPerm (inst: ICInstance; var perm: ICPerm): ICError;
	(* returns the permissions currently associated with this file *)
	(* mainly used by overriding components, applications normally *)
	(* know what permissions they have *)

	function ICBegin (inst: ICInstance; perm: ICPerm): ICError;
	(* start reading/writing the preferences *)
	(* must be balanaced by a ICEnd *)
	(* do not call WaitNextEvent between this pair *)
	(* specify either icReadOnlyPerm or icReadWritePerm *)
	(* note that this may open resource files and leave them open until ICEnd *)

	function ICGetPref (inst: ICInstance; key: Str255; var attr: ICAttr; buf: Ptr; var size: longint): ICError;
	(* this routine may be called without a ICBegin/ICEnd pair, in which case *)
	(* it implicitly calls ICBegin(inst, icReadOnlyPerm *)
	(* given a key string, returns the attributes and the (optionally) the data for a preference *)
	(* key must not be the empty string *)
	(* if buf is nil then no data fetched and incoming size is ignored*)
	(* size must be non-negative, is size of allocated space for data at buf *)
	(* attr and size and always set on return *)
	(* size is actual size of data (if key present) *)
	(* attr is pref attributes *)
 	(* if icTruncatedErr then everything is valid, except you lost some data, size is size of real data*)
	(* on other errors, attr is ICattr_no_change and size is 0 *)

	function ICSetPref (inst: ICInstance; key: Str255; attr: ICAttr; buf: Ptr; size: longint): ICError;
	(* this routine may be called without a ICBegin/ICEnd pair, in which case *)
	(* it implicitly calls ICBegin(inst, icReadWritePerm *)
	(* given a key string, sets the attributes and the data for a preference (either is optional) *)
	(* key must not be the empty string *)
	(* if buf is nil then no data stored and size is ignored, used for setting attr *)
	(* size must be non-negative, is size of the data at buf to store *)
	(* icPermErr if ICBegin was given icReadOnlyPerm *)
	(* icPermErr if current attr is locked, new attr is locked and buf <> nil *)

	function ICGetPrefHandle (inst: ICInstance; key: Str255; var attr: ICAttr; var prefh: Handle): ICError;
	(* same as ICGetPref except that it returns the result in a newly created handle *)
	(* prefh is set to a newly created handle *)

	function ICSetPrefHandle (inst: ICInstance; key: Str255; attr: ICAttr; prefh: Handle): ICError;
	(* same as ICSetPref except that it sets the preference based on a handle *)
	(* prefh must not be nil *)

	function ICCountPref (inst: ICInstance; var count: longint): ICError;
	(* count total number of preferences *)
	(* if error then count is 0 *)

	function ICGetIndPref (inst: ICInstance; n: longint; var key: Str255): ICError;
	(* return the key of the Nth preference *)
	(* n must be positive *)
	(* icPrefNotFoundErr if n is beyond the last preference *)

	function ICDeletePref (inst: ICInstance; key: Str255): ICError;
	(* delete the preference specified by key *)
	(* key must not be the empty string *)
	(* preference specified by key must be present *)
	(* icPrefNotFoundErr if it isn't *)

	function ICEnd (inst: ICInstance): ICError;
	(* stop reading/writing the preferences *)

	function ICDefaultFileName (inst: ICInstance; var name: Str63): ICError;
	(* return the default file name *)
	(* the component calls this routine to set up the default internet configuration file name*)
	(* this allows this operation to be intercepted by a component that has captured us *)
	(* it currently gets it from the component resource file *)
	(* the glue version is hardwired *)

	function ICGetComponentInstance (inst: ICInstance; var component_inst: univ Ptr): ICError;
	(* returns noErr and the component instance that we're talking to, if we're using the component *)
	(* returns an error and nil if we're doing it with glue *)
	(* univ Ptr rather than ComponentInstance so that you don't need Components.p *)
	(* in order to use this file *)

	function ICEditPreferences (inst: ICInstance; key: Str255): ICError;
	(* requires IC 1.1 *)
	(* this routine may be called without a ICBegin/ICEnd pair *)
	(* open the IC application and edits the preference *)
	(* you must have found a config file before calling this *)
	(* if key is empty then it just opens the application with the found config file *)
	(* otherwise it displays the preference with the specified key *)
	(* may have an implementation that is radically different in the future *)

	function ICParseURL (inst: ICInstance; hint: Str255; data: Ptr; len: longint; var selStart, selEnd: longint; url: Handle): ICError;
	(* requires IC 1.1 *)
	(* this routine may be called without a ICBegin/ICEnd pair *)
	(* parses a URL out of a chunk of text and returns it in a canonical form *)
	(* hint is an indication of how you want slack URLs to be parsed *)
	(* data points to the beginning of the text *)
	(* len is the length of the text *)
	(* selStart is the start of the current selection *)
	(* selEnd is the end of the current selection *)
	(* both of these are interpreted as per TextEdit *)
	(* both of these are adjusted to indicate the bounds of the text that was interpreted as a URL *)
	(* the URL is copied into the url handle, which is resized to accomodate the URL *)

	function ICLaunchURL (inst: ICInstance; hint: Str255; data: Ptr; len: longint; var selStart, selEnd: longint): ICError;
	(* requires IC 1.1 *)
	(* this routine may be called without a ICBegin/ICEnd pair *)
	(* identical to ParseURL but it launches the appropriate application (as specified in the 'Helper�' preference *)
	(* and sends it a GURL AppleEvent *)
	(* your code must be running in the context of a high level event aware application for this to work *)

	(* ----- Mappings Routines ----- *)

	(* Routines for interrogating mappings database *)
	(* All of these routines require IC 1.1 *)

	(* High Level Routines *)

	(* you do not need to call begin/end around any of these high level routines *)
	(* but if you're going to call them repeatedly, it will help *)

	function ICMapFilename (inst: ICInstance; filename: Str255; var entry: ICMapEntry): ICError;
	(* high level interface to the mappings database *)
	(* takes a filename and returns the incoming entry associated with that file's extension *)
	(* or errors with icPrefNotFoundErr *)

	function ICMapTypeCreator (inst: ICInstance; fType, fCreator: OSType; filename: Str255; var entry: ICMapEntry): ICError;
	(* high level interface to the mappings database *)
	(* takes a filename and returns the outgoing entry associated with that type, creator and filename *)
	(* or errors with icPrefNotFoundErr *)

	(* Mid Level Routines *)

	(* you do not need to call begin/end around any of these mid level routines *)

	function ICMapEntriesFilename (inst: ICInstance; entries: Handle; filename: Str255; var entry: ICMapEntry): ICError;
	(* mid level interface to the mappings database *)
	(* entries must be a valid IC mappings database handle *)
	(* takes a filename and returns the incoming entry associated with that file's extension *)
	(* or errors with icPrefNotFoundErr *)

	function ICMapEntriesTypeCreator (inst: ICInstance; entries: Handle; fType, fCreator: OSType; filename: Str255; var entry: ICMapEntry): ICError;
	(* mid level interface to the mappings database *)
	(* entries must be a valid IC mappings database handle *)
	(* takes a filename and returns the outgoing entry associated with that type, creator and filename *)
	(* or errors with icPrefNotFoundErr *)

	(* Low Level Routines *)

	(* you do not need to call begin/end around any of these low level routines *)

	function ICCountMapEntries (inst: ICInstance; entries: Handle; var count: longint): ICError;
	(* entries must be a valid IC mappings database handle *)
	(* count the number of entries in the Mappings database *)

	function ICGetIndMapEntry (inst: ICInstance; entries: Handle; ndx: longint; var pos: longint; var entry: ICMapEntry): ICError;
	(* entries must be a valid IC mappings database handle *)
	(* return the ndx'th entry in the mappings database *)
	(* ndx must be from 1 to CountMapEntries *)
	(* pos is ignored on input and comes back as the position of the entry in the database *)
	(* entry is ignored on input and comes back as the database entry at the ndx position *)

	function ICGetMapEntry (inst: ICInstance; entries: Handle; pos: longInt; var entry: ICMapEntry): ICError;
	(* entries must be a valid IC mappings database handle *)
	(* return the entry at position pos *)
	(* pos should be a valid entry start position *)
	(* set pos to 0 for the first entry *)
	(* set pos to pos + entry.total_size for subsequent entries *)
	(* entry is ignored on input and comes back as the database entry at position pos *)

	function ICSetMapEntry (inst: ICInstance; entries: Handle; pos: longInt; var entry: ICMapEntry): ICError;
	(* entries must be a valid IC mappings database handle *)
	(* set the entry at position pos *)
	(* pos should be a valid entry start position *)
	(* any user data at the end of the entry is preserved *)
	(* entry is not modified, it's a var parameter simply to preserve stack space *)

	function ICDeleteMapEntry (inst: ICInstance; entries: Handle; pos: longint): ICError;
	(* entries must be a valid IC mappings database handle *)
	(* delete the entry at position pos *)
	(* pos should be a valid entry start position *)
	(* also deletes the user data associated with the entry *)

	function ICAddMapEntry (inst: ICInstance; entries: Handle; var entry: ICMapEntry): ICError;
	(* entries must be a valid IC mappings database handle *)
	(* add entry to the mappings database *)
	(* entry is not modified, it's a var parameter simply to preserve stack space *)

implementation
end. (* ICAPI *)
