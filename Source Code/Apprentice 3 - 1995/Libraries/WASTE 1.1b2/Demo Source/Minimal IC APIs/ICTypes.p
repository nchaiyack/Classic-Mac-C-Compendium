unit ICTypes;

interface

{$ifc undefined THINK_Pascal}
	uses
		Types;
{$endc}

	const
		icPrefNotFoundErr = -666;			(* preference not found (duh!) *)
		icPermErr = -667;						(* cannot set preference *)
		icPrefDataErr = -668;					(* problem with preference data *)
		icInternalErr = -669;					(* hmm, this is not good *)
		icTruncatedErr = -670;					(* more data was present than was returned *)
		icNoMoreWritersErr = -671;			(* you cannot begin a write session because someone else is already doing it *)
		icNothingToOverrideErr = -672;	(* no component for the override component to capture *)
		icNoURLErr = -673;						(* no URL found *)

	const
		ICattr_no_change = -1;	(* supply this to ICSetPref to tell it not to change the attributes *)

		ICattr_locked_bit = 0;										(* bits in the preference attributes *)
		ICattr_locked_mask = $00000001;					(* masks for the above *)
		ICattr_volatile_bit = 1;
		ICattr_volatile_mask = $00000002;

		ICfiletype = 'ICAp';
		ICcreator = 'ICAp';
		ICdefault_file_name = 'Internet Preferences';	(* default file name, for internal use, overridden by a component resource *)
		ICdefault_file_name_ID = 1024;						(* ID of resource in component file *)
	type
		ICDirSpec = record											(* a record that specifies a folder *)
				vRefNum: integer;
				dirID: longint;
			end;
		ICDirSpecArray = array[0..3] of ICDirSpec;	(* an array of the above *)
		ICDirSpecArrayPtr = ^ICDirSpecArray;				(* a pointer to that array *)
		ICAttr = longint;												(* type for preference attributes *)
		ICError = longint;												(* type for error codes *)
		ICInstance = Ptr;												(* opaque type for preference reference *)
		ICPerm = (icNoPerm, icReadOnlyPerm, icReadWritePerm);

implementation

end. (* ICTypes *)