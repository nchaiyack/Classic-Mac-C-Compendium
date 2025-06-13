unit XTNDInterface;

{ Claris XTND Architecture: Application Header File }

{ Copyright й 1989-1991 Claris Corp. }
{ All Rights Reserved }

{ Adapted for use with THINK Pascal by Merzwaren }

interface

	const
	{ Defines for XTND Version }

		xtndVersion = 2;

	{ Defines for file types }
		xtndTranslatorFileType = 'Fltr';

	{ Defines for resource types }
		translatorDescription = 'FTYP';
		textImportType = 'FLTI';
		textExportType = 'FLTE';
		pictImportType = 'PFLT';

	{ Flag values for translator characteristics which are set in TransDescribe and FTYP }
		ftypIsSpecial = $0001;
		ftypHasPreferences = $0002;	{ Reserved for future use }
		ftypNeedsResources = $0004;
		ftypWritesResources = $0008;	{ Reserved for future use }
		ftypOnlyPreferences = $0010;	{ Reserved for future use }
		ftypIsLocalized = $0020;

	{ Type of translators to be included in list }
		allowTypeMask = $00FF;
		allowText = $0001;
		allowGraphics = $0002;	{ Reserved for future use }
		allowDataBase = $0004;	{ Reserved for future use }
		allowPict = $0008;
		allowOtherTypes = $0080;	{ Reserved for future use }
		allowAllTypes = $00FF;

	{ Use of translators to be included in list }
		allowUseMask = $FF00;
		allowImport = $0100;
		allowExport = $0200;
		allowOtherUses = $8000;	{ Reserved for future use }
		allowAllUses = $FF00;

	{ Error codes which the XTND System can return }

		badXTNDVersionErr = -9991;
		noTransMatchErr = -9999;

	{ Error codes which a translator can return }

		badImportFileErr = 12;
		translationCanceledErr = 32767;

	type

		XTNDDlgHookProcPtr = ProcPtr; { XTNDDlgHook(item: INTEGER; theDialog: DialogPtr;}
{				mySFParamPtr: Ptr; VAR changedFlag: BOOLEAN; unusedParm: LONGINT);}
		TransProcPtr = ProcPtr; { Translator(pb: ParamBlk); C;    (Note: this uses C calling conventions) }

	{ The MatchInfo data stucture defines a single file type (and possibly creator type)}
	{	  that a translator can recognize.}
		MatchInfo = record
				docCreator: ResType;		{ Document creator, eg 'MACA' - MacWrite }
				docType: ResType;		{ Document type, eg 'WORD' - MacWrite	}
				exactMatch: BOOLEAN;		{ If TRUE, Creator MUST match }
				creatorAndTypeMask: SignedByte;		{ Flags telling which bytes of file & creator types matter }
			end;


	{ The TransDescribe data stucture defines an import/export translator.  There is one }
	{ of these (corresponds to the 'FTYP' resource) for every import/export translator. }
		TransDescrHandle = ^TransDescrPtr;
		TransDescrPtr = ^TransDescribe;
		TransDescribe = record
				version: INTEGER;		{ Version of this FTYP (currently 2) }
				translatorType: ResType;		{ Type of translator (ie, 'PFLT' for pictures) }
				codeResID: INTEGER;		{ Resource ID of translator code resource }
				FDIFResID: INTEGER;		{ Resource ID of 'FDIF' (or -1 if none) }
				numVersBytes: INTEGER;		{ Number of bytes for version check (zero if no check) }
				versBytesOffSet: LONGINT;		{ Offset into file for version check }
				versBytes: packed array[0..15] of SignedByte; { Byte values we are looking for (max 16) }
				appWDRefNum: INTEGER;		{ Internal XTND use only - WDRefNum of the directory containing the application folder }
				unused1: INTEGER;		{ Reserved for future use - must be zero for this version }
				pathLength: INTEGER;		{ Internal XTND use only - Number of times this file has been translated }
				flags: INTEGER;		{ Flags for filter characteristics }
												{ bit 0 - set if this filter won't work with All Available }
												{ bit 1 - reserved for future use - must be zero for this version }
												{ bit 2 - set if this filter must be able to read its resources }
												{ bit 3 - 4 reserved for future use - must be zero for this version }
												{ bit 5 - set if this filter is localized to a particular language }
												{ bit 6 - 14 reserved for future use - must be zero for this version }
												{ bit 15 - Used internally - must be zero for this version }
				transIndex: INTEGER;		{ Internal XTND use only - Index of this translator in the list stored by the FINI }
				resRefNum: INTEGER;		{ RefNum of resource fork of the translator file when open }
				directoryID: LONGINT;		{ Directory ID of the folder containing the filter }
				vRefNum: INTEGER;		{ Volume RefNum of volume which contains filter directory }
				fileName: Str31;			{ Name of file if this is an external filter }
				numMatches: INTEGER;		{ Number of Matches for this translator (0 means all files) }
				matches: array[0..9] of MatchInfo; { An ARRAY of all file matches for this translator }
				name: Str31;			{ Description of translator - appears in pop-up menus }
			end;

	{ The XTND file parameter block.		}
		SFParamPtr = ^SFParamBlock;
		SFParamBlock = record
	{ The following fields must be the same in this structure and MatchTranslatorBlock }
				allowFlags: LONGINT;				{ --> Flags which indicate Translators to be used }
				numStandard: LONGINT;				{ --> How many "standard" file types }
				standard: TransDescrPtr;		{ --> Native file format information }
				ioResult: OSErr;				{ <-- Errors reported through this variable }
				chosenTranslator: INTEGER;				{ <-> On input, number of translators supplied in theChosenTranslator; on output, chosen Translator number }
				theChosenTranslator: TransDescrPtr;	{ <-> On input, Pointer to list of translators to build popup menu from (UseMyTransList must be true if you supply this list); on output, pointer to the Chosen Translator }
		{ End of common fields }
				fileReply: ^SFReply;				{ <-- File information is returned through this pointer }
				applicNativeType: ResType;				{ --> Application native document file type, for use when library cannot be found }
				XTNDDlogHook: XTNDDlgHookProcPtr;	{ --> Pointer to your SF dialog hook }
				currentMenuItem: INTEGER;				{ <-- chosen menu item (GetFile) }
				currentSaveItem: INTEGER;				{ <-- chosen menu item (PutFile) }
				where: Point;				{ --> Top left corner of dialog, if zero, center on screen }
				prompt: StringPtr;			{ --> Pascal string which is displayed on dialog }
				buttonTitle: StringPtr;			{ --> Pascal string which replaces the "Open" or "Save" button }
				origName: StringPtr;			{ --> Pascal string which is the original name in a PutFile }
				dialogID: INTEGER;				{ --> Resource number of your dialog template }
				SFFilterProc: ModalFilterProcPtr;	{ --> Pointer to your modal dialog filter proc }
				showAllFiles: BOOLEAN;				{ --> TRUE if you want all files to be shown }
				useMyTransList: BOOLEAN;				{ --> TRUE if you are using chosenTranslator and theChosenTranslator to input a list of translators to be used in creating the popup menu }
				Unused: array[0..1] of LONGINT;	{ Reserved for future use, must be zero }
			end;

	{ This parameter block is used to find a translator to read a file.}
	{	}
		MatchTranslatorPtr = ^MatchTranslatorBlock;
		MatchTranslatorBlock = record
		{ The following fields must be the same in this structure and in XTNDParamBlock }
				allowFlags: LONGINT;			{ --> Flags which indicate Translators to be used }
				numStandard: LONGINT;			{ --> How many "standard" file types }
				standard: TransDescrPtr;	{ --> Native file format information }
				ioResult: OSErr;			{ <-- IO Result from XTND library }
				chosenTranslator: INTEGER;			{ <-> On input, number of translators supplied in theChosenTranslator; on output, chosen Translator number }
				theChosenTranslator: TransDescrPtr;{ <-> On input, Pointer to list of translators to build popup menu from (UseMyTransList must be true if you supply this list); on output, pointer to the Chosen Translator }
	{ End of common fields }
				fileName: StringPtr;		{ --> File name }
				vRefNum: INTEGER;			{ --> Volume Reference Number of file }
				ioDirID: LONGINT;			{ --> Directory id of file }
				initFlag: BOOLEAN;			{ --> TRUE if the list of translators should be rebuilt }
				useTransList: BOOLEAN;			{ --> TRUE if you are using chosenTranslator and theChosenTranslator to input a list of translators to be used in creating the popup menu }
				oneTrans: TransDescrPtr;	{ --> If this is supplied, it will be tried instead of searching all available translators }
				unused: array[0..3] of LONGINT;	{ Reserved for future use, must be zero }
			end;

		SelectParamPtr = ^SelectParamBlock;
		SelectParamBlock = record
				translatorVersion: INTEGER;		{ To select by translator type }
				translatorType: ResType;		{ and version }
				includeFlags: LONGINT;		{ Include any of these translator types }
				excludeFlags: LONGINT;		{ Exclude all of these translator types }
				includeTrans: INTEGER;		{ Include translators with any of these flags set }
				excludeTrans: INTEGER;		{ Excluded translator with any of these flags set }
				transName: StringPtr;	{ Include only translators with this name }
				PBVersion: LONGINT;		{ еее MUST be ONE (1) for this version еее }
				menuSortFlag: BOOLEAN;		{ TRUE if you want the list sorted and duplicates removed as when the XTND System creates a menu in XTNDGetFile or XTNDPutFile for you }
				unused1: BOOLEAN;		{ Reserved for future use, must be zero }
				unused2: INTEGER;		{ Reserved for future use, must be zero }
				unused: array[0..3] of LONGINT; { Reserved for future use, must be zero }
			end;

	{ 	Routines available in XTNDLibrary	}
	function XTNDInitTranslators (transVersion: INTEGER;
									xtndSystemName, clarisFolderName: Str255): OSErr;

	function XTNDGetFile (paramPtr: SFParamPtr): BOOLEAN;

	function XTNDPutFile (paramPtr: SFParamPtr): BOOLEAN;

	function XTNDLoadTranslator (theTranslator: TransDescrPtr;
									var transResource: TransProcPtr): OSErr;

	function XTNDReleaseTranslator (theTranslator: TransDescrPtr): OSErr;

	procedure XTNDCloseTranslators;

	function XTNDRebuildTransList: OSErr;

	function XTNDSelectTranslators (selectPtr: SelectParamPtr;
									transHandle: TransDescrHandle): OSErr;

	function XTNDMatchFile (matchPtr: MatchTranslatorPtr): BOOLEAN;

	procedure XTNDCallTranslator (theParamBlock: univ Ptr;
									theProc: TransProcPtr);
	inline
		$205F,		 	{ 	MOVE.L	(SP)+,A0	;	get translator address off stack }
		$4E90,			{	JSR		(A0)		;	Call it }
		$584F;			{	ADDQ	#4,A7		;	Adjust the stack (C calling conventions) }



    { UsingXTNDInterface }


implementation
end.