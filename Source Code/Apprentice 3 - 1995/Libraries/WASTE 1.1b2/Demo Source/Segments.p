unit Segments;

{ Routines for unloading code segments easily }
{ Copyright © 1994 Marco Piovanelli }

{ *** RATIONALE: *** }
{ Many applications are segmented so that some code segments are persistent }
{ (automatically preloaded and locked when the application is launched, }
{ never unloaded) and the others are non-persistent (loaded on demand, }
{ usually unloadable from the main event loop). }
{ Unfortunately, you must explicitly call _UnloadSeg to have a segment unloaded }
{ (i.e., unlocked and made purgeable) and you have to pass _UnloadSeg a pointer }
{ to [the jump table entry for] a routine in the segment to unload. }
{ This may not be always handy.  But wait!  The UnloadNonPersistentSegments }
{ routine automatically finds and unloads all non-persistent segments. }
{ It does this by listing all CODE resources whose locked attribute is clear }
{ (the resource attribute, _not_ the handle state) and whose purgeable attribute }
{ is set.  It then calculates the address of the jump table entry for the first routine }
{ in the segment and calls _UnloadSeg on the address. }

{ *** THIS CODE ASSUMES THAT: *** }
{ The application has a traditional, IM II-style jump table (not a far-code JT) }
{ The current resource file is the application file }
{ The A5 register is set up correctly }
{ UnloadNonPersistentSegments is called from a persistent segment }

interface

	procedure UnloadNonPersistentSegments;

implementation
	uses
		ConditionalMacros, Types, Resources, SegLoad, LowMem;

{$IFC NOT UNDEFINED THINK_PASCAL}

	const

		kTypeCodeSegment = 'CODE';		{ application code segment resource type }

	type

		CodeSegment = record
				firstEntryOffset: Integer;		{ offset of the first routine's entry from the beginning of the Jump Table }
				nEntries: Integer;					{ number of entries for this segment }
{ actual code follows... }
			end;
		CodeSegmentPtr = ^CodeSegment;
		CodeSegmentHandle = ^CodeSegmentPtr;

	function GetA5: LongInt;
	inline
		$2E8D;					{ movea.l a5, (sp) }

	function GetSegmentByIndex (segmentIndex: Integer): Handle;
		var
			saveResLoad: Boolean;
	begin

{ temporarily disable loading of resources }
		saveResLoad := Boolean(LMGetResLoad);
		SetResLoad(false);

{ get a (possibly empty) handle to the specified segment }
		GetSegmentByIndex := Get1IndResource(kTypeCodeSegment, segmentIndex);

{ restore the original ResLoad flag }
		SetResLoad(saveResLoad);

	end;  { GetSegmentByIndex }

	procedure UnloadSegmentHandle (hSegment: Handle);
	begin

{ do nothing if the segment handle is null or empty }
		if (hSegment <> nil) then
			if (hSegment^ <> nil) then

{ calculate the address of the first routine entry in the segment }
{ and call _UnloadSeg on the calculated address }
				UnloadSeg(ProcPtr(GetA5 + LMGetCurJTOffset + CodeSegmentHandle(hSegment)^^.firstEntryOffset + 2));

	end;  { UnloadSegmentHandle }

	procedure UnloadNonPersistentSegments;
		var
			segmentIndex: Integer;
			segmentAttributes: Integer;
			hSegment: Handle;
	begin

{ loop through all code segments }
		for segmentIndex := Count1Resources(kTypeCodeSegment) downto 1 do
			begin

{ get segment handle }
				hSegment := GetSegmentByIndex(segmentIndex);

{ get resource attributes of the segment }
				segmentAttributes := GetResAttrs(hSegment);

{ do nothing if a resource error occurred }
				if (ResError <> noErr) then
					Cycle;

{ check whether 'purgeable' is on and 'locked' is off }
				if (BAND(segmentAttributes, resPurgeable + resLocked) = resPurgeable) then
					UnloadSegmentHandle(hSegment);

			end;  { for }
	end;  { UnloadNonPersistentSegments }

{$ELSEC}

	procedure UnloadNonPersistentSegments;
	begin
	end;  { UnloadNonPersistentSegments }

{$ENDC}

end.