unit XTNDTextTranslator;

{ Claris XTND Architecture: Header File for Text Translators }

{ Copyright � 1989-1991 Claris Corp. }
{ All Rights Reserved }

{ Adapted for use with THINK Pascal by Merzwaren }
{ 3/24/93: corrected an error in the definition of the ImportParmBlock record type: }
{ redefined decimalChar to SignedByte }

interface
	uses
		XTNDInterface, PrintTraps;

	const

{ Directives to be passed to import translators }
		importAcknowledge = -1;	{ Value set by a translator to acknowledge an action }

		importGetResources = 0;
		importInitAll = 1;

		importInitRightHeader = 2;
		importInitLeftHeader = 3;
		importInitHeader = 4;

		importInitRightFooter = 5;
		importInitLeftFooter = 6;
		importInitFooter = 7;

		importInitMain = 8;

		importInitFootnote = 9;

		importGetText = 10;

		importCloseRightHeader = 11;
		importCloseLeftHeader = 12;
		importCloseHeader = 13;

		importCloseRightFooter = 14;
		importCloseLeftFooter = 15;
		importCloseFooter = 16;

		importCloseMain = 17;

		importCloseFootnote = 18;

		importCloseAll = 19;

{ Directives to be passed to export translators }
		exportAcknowledge = -1;	{ Set by translators to acknowledge certain directives }

		exportInitAll = 0;

		exportOpenRightHeader = 1;
		exportOpenLeftHeader = 2;
		exportOpenHeader = 3;

		exportOpenRightFooter = 4;
		exportOpenLeftFooter = 5;
		exportOpenFooter = 6;

		exportOpenFootnote = 7;

		exportOpenMain = 8;

		exportWriteText = 9;

		exportCloseRightHeader = 10;
		exportCloseLeftHeader = 11;
		exportCloseHeader = 12;

		exportCloseRightFooter = 13;
		exportCloseLeftFooter = 14;
		exportCloseFooter = 15;

		exportCloseFootnote = 16;

		exportCloseMain = 17;

		exportCloseAll = 18;

		exportWriteResources = 19;


{ Defines for CurrentStory }
		rightHeaderStory = 1;
		leftHeaderStory = 2;
		headerStory = 3;
		rightFooterStory = 4;
		leftFooterStory = 5;
		footerStory = 6;
		footnoteStory = 7;
		mainStory = 8;

{ types of pages for headers/footers }
		everyPage = 1;
		leftPage = 2;
		rightPage = 4;

{ XTND 1.x reserved characters }
		pageNumber = $02;	{ Current page number character }
		footnoteChar = $03;	{ Footnote character in the footnote }
		floatingPict = $04;	{ Floating picture marker }
		footnoteMark = $05;	{ Footnote character in the text }
		mergeBreak = $06;	{ Mail merge document break }
		hardReturn = $07;	{ Hard return (does not create new paragraph) }
		tabChar = $09;	{ Tab character }
		newColumn = $0B;	{ Column break character }
		newPage = $0C;	{ Page break character }
		returnChar = $0D;	{ Paragraph break character }
		shortDateChar = $15;	{ Assorted date characters }
		abbrDateChar = $16;
		longDateChar = $17;
		dayAbbrDateChar = $18;	{ Date characters with day of week included }
		dayLongDateChar = $19;
		timeChar = $1A;	{ Time character }
		softHyphen = $1F;	{ Discretionary hyphen character }

	{ Other character values }
		enterChar = $03;

	{ XTND 1.x limits }
		numParaFmts = 9;			{ Number of fields in a paragraph specification }
		maxCols = 10;			{ Maximum number of columns allowed }
		maxTabs = 20;			{ Maximum number of tabs allowed }
		minGutter = $00030000;	{ (minimum gutter) 3 points as a Fixed number }
		maxGutter = $01200000;	{ (maximum gutter) 288 points as a Fixed number }
		dfltGutter = $00090000;	{ (default gutter) 9 points as a Fixed number }

{ XTND 1.x text styles }
		textPlain = $0000;
		textSuperScript = $0020;
		textSubScript = $0040;
		textNonbreaking = $0080;
		textSuperior = $0100;
		textStrikeThru = $0200;
		textAllCaps = $0400;
		textSmallCaps = $0800;
		textWordUnderline = $1000;
		textDoubleUnderline = $2000;
		textLowerCase = $4000;

{ XTND 1.x justification/tab alignment values }
		textLeft = 0;
		textCenter = 1;
		textRight = 2;
		textJustified = 3;		{ For text }
		textDecimal = 3;		{ For tabs }

{ XTND 1.x text color values }
		textWhite = 0;
		textBlack = 1;
		textRed = 2;
		textGreen = 3;
		textBlue = 4;
		textCyan = 5;
		textMagenta = 6;
		textYellow = 7;
		textOtherColor = 8;

	type
	{ Tab specifications record.  The tab array contains 20 of these.}
		TabSpec = record
				tabJust: SignedByte;	{ Tab justification (left, right, center, aligned) }
				tabLead: SignedByte;	{ Tab leader character (default is space) }
				tabIndent: Fixed;		{ Tab indent in points from left page margin }
				decAlign: SignedByte;	{ Tab alignment character (usually period) }
			end;
		TabSpecArray = array[0..19] of TabSpec;


	{ The pictMisc data structure is used to import a quickdraw picture }
		pictMiscHdl = ^pictMiscPtr;
		pictMiscPtr = ^pictMisc;
		pictMisc = record
				thePicture: PicHandle;		{ handle to the quickdraw picture }
				pictSize: LongInt;		{ Size of associated picture }
				destRect: Rect;			{ Used for scaling the picture, OrigRect will be scaled to this rect }
				origRect: Rect;			{ Used for cropping the picture, pictRect will be cropped to this rect }
				reserved: packed array[1..38] of Byte;	{ Reserved, fill with zeros }
			end;

	{ Paragraph format record.  The ParaFormats array contains 9 Fixed values }
		ParaFormat = Fixed;
		ParaFormats = array[0..8] of ParaFormat;

	{ This is the parameter block for passing information to an import translator }
		ImportParmBlkPtr = ^ImportParmBlock;
		ImportParmBlock = record
				textBuffer: Ptr;		{ Pointer to 256 bytes of data }
				directive: INTEGER;	{ Indicates the action to be performed }
				result: OSErr;		{ Return code, 0 if successful, error code otherwise }
				textLength: LONGINT;	{ Number of characters of information being returned }
				translatorState: INTEGER;	{ Available for use by the translator }
				refNum: INTEGER;	{ Reference number of the fork to be read by the translator }
				txtFace: INTEGER;	{ Current text face }
				txtSize: INTEGER;	{ Current text font size }
				txtFont: INTEGER;	{ Current text font family number }
				txtColor: INTEGER;	{ Current text color (MacWrite II color value) }
				txtJust: INTEGER;	{ Justification of this text }
				unused1: INTEGER;	{ Must be zero }
				paraFmts: ^ParaFormats;{ Pointer to this paragraph's format array }
				tabs: ^TabSpecArray;{ Pointer to this paragraph's tab array }
				unused2: BOOLEAN;	{ Currently unused }
				numCols: SignedByte;	{ Number of columns expected }
				currentStory: INTEGER;	{ Header,Footer,Main Body, etc. }
				miscData: LONGINT;	{ For importing pictures, etc. }
				storyHeight: INTEGER;	{ Height of header,footer, etc. }
				decimalChar: SignedByte;		{ Default char to align decimal tab on }
				autoHyphenate: BOOLEAN;	{ If TRUE, auto hyphenation is on }
				printRecord: THPrint;	{ Print record, if one is used, otherwise NIL }
				topMargin: Fixed;		{ Document top margin }
				bottomMargin: Fixed;		{ Document bottom margin }
				leftMargin: Fixed;		{ Document left margin }
				rightMargin: Fixed;		{ Document right margin }
				gutter: Fixed;		{ Space between columns }
				startPageNum: INTEGER;	{ Starting page number }
				startFootnoteNum: INTEGER;	{ starting footnote number }
				footnoteText: Ptr;		{ If not empty, text associated with footnote, else auto }
				rulerShowing: BOOLEAN;	{ If TRUE, ruler is showing }
				doubleSided: BOOLEAN;	{ If TRUE, document has Left/Right pages }
				titlePage: BOOLEAN;	{ If TRUE, document has a title page (with no headers/footers) }
				endnotes: BOOLEAN;	{ If TRUE, footnotes are displayed as endnotes }
				showInvisibles: BOOLEAN;	{ If TRUE, invisible characters will be displayed }
				showPageGuides: BOOLEAN;	{ If TRUE, page guides will be displayed }
				showPictures: BOOLEAN;	{ If TRUE, pictures will be displayed }
				autoFootnotes: BOOLEAN;	{ If TRUE, footnotes will be numbered automatically }
				pagePoint: Point;		{ Position for page number within header/footer }
				datePoint: Point;		{ Position for date character within header/footer }
				timePoint: Point;		{ Position for time character within header/footer }
				globalHandle: Handle;		{ Handle to translator globals, if needed }
				smartQuotes: BOOLEAN;	{ If TRUE, smart quotes will be used }
				fractCharWidths: BOOLEAN;	{ If TRUE, fractional character widths will be used }
				hRes: INTEGER;	{ The horizontal resolution of this document (default is 72) }
				vRes: INTEGER;	{ The vertical resolution of this document (default is 72) }
				windowRect: Rect;		{ May be used to specify document window placement and size }
				theReply: SFReply;	{ Standard reply record describing the file being translated }
				thisTranslator: TransDescribe;{ TransDescribe record describing this translator }
			end;


	{ This is the parameter block for passing information to an export translator.}
		ExportParmBlkPtr = ^ExportParmBlock;
		ExportParmBlock = record
				directive: SignedByte;	{ Indicates the action to be performed }
				filler1: SignedByte;	{ explicit padding }
				result: ^OSErr;		{ Pointer to return code, 0 if successful, error code otherwise }
				refNum: ^INTEGER;	{ Pointer to reference number of the file being written by the translator }
				textLength: ^LONGINT;	{ Pointer to number of characters being exported }
				globalHandle: Handle;		{ Do not change this value }
				reserved1: LONGINT;	{ Do not change this value }
				textBuffer: Handle;		{ Handle to the text being exported }
				txtFace: ^INTEGER;	{ Pointer to the current text face }
				txtSize: ^INTEGER;	{ Pointer to the current text font size }
				txtFont: ^INTEGER;	{ Pointer to the current text font family number }
				txtColor: ^SignedByte;{ Pointer to the current text color (MacWrite II color value) }
				txtJust: ^INTEGER;	{ Pointer to the justification of this text }
				paraFmts: ^ParaFormats;{ Pointer to this paragraph's format array }
				tabs: ^TabSpecArray;{ Pointer to this paragraph's tab array }
				thePicture: PicHandle;	{ Handle to a quickdraw picture }
				pictRect: Rect;		{ Rectangle describing the display rect of the above picture }
				headerStatus: INTEGER;	{ Shows if header is on left/right/every page }
				footerStatus: INTEGER;	{ Shows if footer is on left/right/every page }
				currentStory: INTEGER;	{ Header, Footer, Main Body, etc. }
				numCols: INTEGER;	{ Number of columns in document }
				topMargin: Fixed;		{ Document top margin }
				bottomMargin: Fixed;		{ Document bottom margin }
				leftMargin: Fixed;		{ Document left margin }
				rightMargin: Fixed;		{ Document right margin }
				gutter: Fixed;		{ Space between columns }
				totalCharCount: LONGINT;	{ Total number of chars in document (for Word 3.0) }
				footnoteOffset: LONGINT;	{ If current story is a footnote, its offset in doc }
				footnoteText: StringPtr;	{ If !autoFootnotes, text for footnote (Pascal String) }
				startPageNum: INTEGER;	{ Starting page number }
				startFootnoteNum: INTEGER;	{ Starting footnote number }
				rulerShowing: BOOLEAN;	{ if TRUE, ruler is showing }
				doubleSided: BOOLEAN;	{ if TRUE, document has Left/Right pages }
				titlePage: BOOLEAN;	{ If TRUE, document has a title page (with no headers/footers) }
				endnotes: BOOLEAN;	{ If TRUE, footnotes are displayed as endnotes }
				showInvisibles: BOOLEAN;	{ If TRUE, invisible characters are displayed }
				showPageGuides: BOOLEAN;	{ If TRUE, page guides are displayed }
				showPictures: BOOLEAN;	{ If TRUE, pictures are displayed }
				autoFootnotes: BOOLEAN;	{ If TRUE, footnotes are numbered automatically }
				footnotesExist: BOOLEAN;	{ If TRUE, footnotes are being exported }
				printRecord: THPrint;	{ Print record for this document }
				pagePoint: Point;		{ Point where page number is displayed in header or footer }
				datePoint: Point;		{ Point where date character is displayed in header or footer }
				timePoint: Point;		{ Point where time character is displayed in header or footer }
				smartQuotes: BOOLEAN;	{ If TRUE, SmartQuotes are turned on }
				fractCharWidths: BOOLEAN;	{ If TRUE, fractional character widths are turned on }
				hRes: INTEGER;	{ The horizontal resolution of this document }
				vRes: INTEGER;	{ The vertical resolution of this document }
				windowRect: Rect;		{ Rectangle specifying document window placement and size }
				theReply: SFReply;	{ Standard reply record describing the file being written }
				thisTranslator: TransDescribe;{ TransDescribe record describing this translator }
			end;

    { UsingXTNDTextTranslator }


implementation
end.