/*______________________________________________________________________

	cvrt.c - Tool to Convert a Text File to a Sequence of STR# Resources.
	
	Copyright � 1988, 1989, 1990 Northwestern University.  Permission is granted
	to use this code in your own projects, provided you give credit to both
	John Norstad and Northwestern University in your about box or document.
	
	This tool is used to prepare text for the report module rep.c
	and the help module hlp.c.
	
	When preparing type 1 reports for the report module rep.c, the wrap tool
	should be run first (with the -p option), then the output of wrap should 
	be processed by cvrt.
	
	cvrt -o file1 -i nnn [-t aaa] [-g bbb] [-c ccc]
		[-p file2 -h xxx -w yyy] [file3]
	
	file1 = output resource file.
	nnn = starting STR# resource id.
	aaa = TCON resource id.
	bbb = TAG resource id.
	ccc = CELL resource id.
	file2 = pict resource file.
	xxx = height of report cells in pixels.
	yyy = width of report cells in pixels.
	file3 = input text file.
	
	The text file is converted to a consecutive sequence of STR# 
	resources starting with id nnn.
	
	The source text is intermixed with special cvrt directives.  Any line 
	starting with the character "\" is treated as a directive.  Directives
	must be all lower case.  The syntax checking by this tool is minimal.
	
	\str#
	
		Starts a new STR# resource.
	
	\tcon title
	
		Table of contents entry.  The title and the current line number
		(zero based) are saved in the TCON resource.
		
	\tag nnn
	
		Tag resource entry.  The number nnn and the current line number
		(zero based) are saved in the TAG resource.
		
	\style xxx xxx xxx

		The next text line is drawn in the specified style or styles.  xxx may
		be any of the following:
		
		normal	(default)
		bold
		italic
		underline
		outline
		shadow
		condense
		extend
		
	\just xxx
	
		The next text line or picture is drawn with the specified justification.
		xxx may be any of the following:
		
		left		(default for text lines)
		center	(default for pictures)
		right
		
	\size xxx
	
		Print the following line of text xxx% larger than normal.  Printing
		only.  The normal font size is multiplied by xxx%, then truncated.  
		For non-laserwriters it is then rounded down to the nearest font
		size which can be printed without Font Manager scaling. 
		
	\only xxx xxx
	
		The next text line or picture or table of contents entry should only 
		be output as specified.  xxx may be any of the following:
		
		screen	Only in the report on the screen.
		print		Only in the printed document.
		save		Only in the saved document.
		
		The default for text lines is: screen print save
		The default for pictures is: screen print
		The default for tcon entries is: screen print
	
	\pict id
	
		A picture from the pict resource file should be inserted at this point.
		"id" is the resource id of the pict.
		
	\page noheader
	
		Start a new page.  Printing only.  The "noheader" parameter is 
		optional.  If the parameter is present, header printing is disabled
		until the next \page directive is encountered.
		
	\keep
	
		Keep the following block of text on the same page.  Force a page
		break if necessary.  Printing only.  Paragraphs are always kept
		on a page, never split across page boundaries.
		
	\endkeep
	
		End a keep block.  Printing only.
		
	\itcon nnn
	
		Insert table of contents.  Printing only.
		
		nnn = total number of printing tcon entries.  This number must be
		adjusted by hand whenever you add or remove tcon entries (sorry). It
		is verified at the end of the cvrt run, and if it is incorrect the
		correct value is printed in an error message (this helps a bit).
		
	Some of the  directives above cause escape sequences to be generated at 
	the beginnings of lines in the STR# resources.  These escape sequences 
	have the following common format:
	
		byte 0 = escape code = a small number less than 31.
		byte 1 = number of bytes in the escape sequence.
		remaining bytes = parameters.
		
	The following escape sequences are used:
	
	Style:
	
		byte 0 = docStyle = 0
		byte 1 = 3
		byte 2 = style
		
	Justification:
	
		byte 0 = docJust = 1
		byte 1 = 3
		byte 2 = justification:
			0 = Left
			1 = Center
			2 = Right
			
	Size:
	
		byte 0 = docSize = 2
		byte 1 = 4
		bytes 2,3 = percent size change
			
	Only:
	
		byte 0 = docOnly = 3
		byte 1 = 3
		byte 2 = flags
			bit 0 = Screen
			bit 1 = Print
			bit 2 = Save
			
	Pict:
	
		byte 0 = docPict = 4
		byte 1 = 6
		bytes 2,3 = PICT resource id
		bytes 4,5 = band number
		
		For pictures multiple lines are output to the STR# resource, each
		in the format above.  Enough lines are output to completely cover
		the picture in hands of of the same height as the cell height.
		Pictures wider than the (cell width - 8) are scaled so that their
		width is exactly (cell width - 8). The band numbers in the escape 
		sequences increment from 0 to number of bands - 1.

	The remaining escape sequences are only used for printing, and they always
	occur on lines by themselves, preceded by on "Only" escape sequence that 
	says the line only applies to printing.
		
	Page:
	
		byte 0 = docPage = 5
		byte 1 = 3
		byte 2 = 1 if noheader param specified, else 0.
		
	Keep:
	
		byte 0 = docKeep = 6
		byte 1 = 2
		
	EndKeep:
	
		byte 0 = docEndKeep = 7
		byte 1 = 2
		
	ITcon:
	
		byte 0 = docITcon = 8
		byte 1 = 3
		byte 2 = tcon line number
		
		For the table of contents multiple lines are output to the STR#
		resource, each in the format above.  A separate line is output for
		each line in the printed table of contents, with the line numbers
		incrementing from 0 to the numbr of entries - 1.
		
	There is one other special escape code, the end-of-paragraph escape
	code 31.  This escape code is not inserted by cvrt, but by the wrap
	tool.  It marks ends of paragraphs.  The printing code in rep.c uses
	these codes when rewrapping paragraphs to new margins.
	
	The TCON resource, if one is generated, has the following format
	(using Rez syntax):
	
	type 'TCON' {
		integer = $$CountOf(list);
		array list {
			integer;				{ line number for online doc }
			byte;					{ only code - docScreen, docPrint, or 
										docScreen | docPrint }
			pstring;				{ table of contents entry }
			align word;
		};
	}
	
	The TAG resource, if one is generated, has the following format
	(using Rez syntax):
	
	type 'TAG ' {
		integer = $$CountOf(list);
		array list {
			integer;				{ line number }
			integer;				{ tag }
		};
	}
	
	The CELL resource, if one is generated, contains one entry per
	screen line, in the following format (using Rez syntax):
	
	type 'CELL' {
		array list {
			byte;					{ ordinal of STR# resource containing line }
			integer;				{ offset in STR# of line }
		};
	}
	
	CELL resources must be generated for type 1 reports.  The CELL resource
	becomes the "cell array" in the list manager data structure for the 
	report (see rep.c).  Prebuilding this array here in cvrt speeds up the
	initialization of large type 1 reports.
_____________________________________________________________________*/

#pragma load "precompile"
#include "doc.h"

unsigned char	*p;				/* pointer to cur pos in STR# rsrc */

void putOnlyPrint (void)

{
	*p++ = docOnly;
	*p++ = 3;
	*p++ = docPrint;
};

short main(int argc, char *argv[])

{
	short				i;						/* loop index */
	Boolean			inFileSpecified = false;	
	Boolean			outFileSpecified = false; 
	Boolean			idSpecified = false;		
	Boolean			tconSpecified = false;		
	Boolean			tagSpecified = false;
	Boolean			cellSpecified = false;
	Boolean			pictFileSpecified = false;
	Boolean			cellWidthSpecified = false;
	Boolean			cellHeightSpecified = false;
	char				*ofile;				/* output file name */
	char				*ifile;				/* input file name */
	short				id;					/* STR# resource id */
	short				firstID;				/* first STR# resource id */
	short				tconID;				/* TCON resource id */
	short				tagID;				/* TAG resource id */
	short				cellID;				/* CELL resource id */
	char 				*pfile;				/* pict file name */
	short				cellWidth;			/* cell width */
	short				cellHeight;			/* cell height */
	Handle			theRez;				/* handle to STR# resource */
	Handle			tconRez;				/* handle to TCON resource */
	Handle			tagRez;				/* handle to TAG resource */
	Handle			cellRez;				/* handle to CELL resource */
	unsigned char	*pp;					/* pointer to beginning of cur line
													in STR# rsrc */
	unsigned char	*q;					/* pointer to cur pos in TCON rsrc */
	unsigned char	*r;					/* pointer to cur pos in TAG rsrc */
	unsigned char	*t;					/* pointer to cur pos in CELL rsrc */
	unsigned short	offset;				/* offset of line if STR# resource */ 
	short				strLen;				/* length of STR# line */
	FILE				*input;				/* input file */
	OSErr				rCode;				/* result code */
	short				ofileNum;			/* output file ref num */
	short				pfileNum;			/* pict file ref num */
	short				nlines;				/* number of lines in cur STR# rsrc */
	short				totLines = 0;		/* total number of screen-visible lines */
	short				ntcon;				/* number of tcon entries */
	short				nptcon;				/* number of printed tcon entries */
	short				ntag;					/* number of tag entries */
	char				str[256];			/* line buffer */
	short				len;					/* length of line */
	unsigned char	*s;					/* pointer to cur pos in str */
	short				size;					/* size of resource */
	Boolean			newRez;				/* true if new STR# should be started */
	Boolean			nextStyle = false;	/* true if next line has style */
	Boolean			nextJust = false;	/* true if next line or pict has just */
	Boolean			nextSize = false;	/* true if next line has size multiplier */
	Boolean			nextOnly = false;	/* true if next line or pict has only */
	unsigned char	styleCode;			/* style code for next line */
	unsigned char	justCode;			/* justification code for next line or
													pict */
	short				sizePercent;		/* size percentage for next line */													
	unsigned char	onlyCode;			/* only code for next line or pict */													
	short				picID;				/* pict resource id */
	PicHandle		picHandle;			/* handle to pict */
	short				picWidth;			/* pict width */
	short				picHeight;			/* pict height */
	short				maxPicWidth;		/* max pict width */
	short				maxPicHeight;		/* max pict height */
	short				picScaledHeight;	/* pict height after scaling */
	short				picBands;			/* number of bands in scaled pict */
	short				ktcon;				/* number of tcon entries from \itcon
													directive */

	/* Crack and check parameters. */

	i = 1;
	while (i < argc) {
		if (*argv[i] == '-') {
			if (tolower(*(argv[i]+1)) == 'o') {
				outFileSpecified = true;
				ofile = argv[i+1];
				i += 2;
			} else if (tolower(*(argv[i]+1)) == 'i') {
				idSpecified = true;
				firstID = atoi(argv[i+1]);
				i += 2;
			} else if (tolower(*(argv[i]+1)) == 't') {
				tconSpecified = true;
				tconID = atoi(argv[i+1]);
				i += 2;
			} else if (tolower(*(argv[i]+1)) == 'g') {
				tagSpecified = true;
				tagID = atoi(argv[i+1]);
				i += 2;
			} else if (tolower(*(argv[i]+1)) == 'c') {
				cellSpecified = true;
				cellID = atoi(argv[i+1]);
				i += 2;
			} else if (tolower(*(argv[i]+1)) == 'p') {
				pictFileSpecified = true;
				pfile = argv[i+1];
				i += 2;
			} else if (tolower(*(argv[i]+1)) == 'w') {
				cellWidthSpecified = true;
				cellWidth = atoi(argv[i+1]);
				i += 2;
			} else if (tolower(*(argv[i]+1)) == 'h') {
				cellHeightSpecified = true;
				cellHeight = atoi(argv[i+1]);
				i += 2;
			} else {
				fprintf(stderr, "### %s - \"%s\" is not an option.\n",
					argv[0], argv[i]);
				return 1;
			};
		} else {
			inFileSpecified = true;
			ifile = argv[i];
			i++;
		};
	};
	if (!outFileSpecified) {
		fprintf(stderr, "### %s - output file not specified.\n", argv[0]);
		return 1;
	};
	if (!idSpecified) {
		fprintf(stderr, "### %s - resource id not specified.\n", argv[0]);
		return 1;
	};
	if (pictFileSpecified && (!cellWidthSpecified || !cellHeightSpecified)) {
		fprintf(stderr, "### %s - when the p option is used the w and h options\n", 
			argv[0]);
		fprintf(stderr, "### must also be specified.\n");
		return 1;
	};

	/* Open the input file.  Create it if it doesn't already exist. */
	
	if (!inFileSpecified) {
		input = stdin;
	} else if ((input = fopen(ifile, "r")) == nil) {
		fprintf(stderr, "### %s - unable to open file %s.\n",
			argv[0], ifile);
		return 2;
	};
	
	/* Open the pict file. */
	
	if (pictFileSpecified) {
		pfileNum = openresfile(pfile);
		if (pfileNum == -1) {
			fprintf(stderr, "### %s - unable to open pict file %s.\n",
				argv[0], pfile);
			return 2;
		};
	};
	
	/* Open the output file. */
	
	SetResLoad(false);
	ofileNum = openresfile(ofile);
	if (ofileNum == -1) {
		createresfile(ofile);
		ofileNum = openresfile(ofile);
		if (ofileNum == -1) {
			fprintf(stderr, "### %s - unable to open or create file %s.\n",
				argv[0], ofile);
			return 2;
		};
	};
	SetResLoad(true);
	
	/* Get the old TCON resource and increase its size to 32K.  If it
		doesn't exist create a 32K new one. */
		
	if (tconSpecified) {
		if ((tconRez = GetResource('TCON', tconID))  && 
			HomeResFile(tconRez) == ofileNum) {
			SetHandleSize(tconRez, 0x7fff);
			if (rCode = MemError()) {			
				fprintf(stderr, "### %s - error %d on SetHandleSize.\n",
					argv[0], rCode);
				return 2;
			};
		} else {
			tconRez = NewHandle(0x7fff);
			if (rCode = MemError()) {			
				fprintf(stderr, "### %s - error %d on NewHandle.\n",
					argv[0], rCode);
				return 2;
			};
			addresource(tconRez, 'TCON', tconID, "");
			if (rCode = ResError()) {			
				fprintf(stderr, "### %s - error %d on TCON addresource.\n",
					argv[0], rCode);
				return 2;
			};
		};
		HLock(tconRez);
		ntcon = nptcon = 0;
		q = *tconRez + 2;
	};
	
	/* Get the old TAG resource and increase its size to 32K.  If it
		doesn't exist create a 32K new one. */
		
	if (tagSpecified) {
		if ((tagRez = GetResource('TAG ', tagID))  && 
			HomeResFile(tagRez) == ofileNum) {
			SetHandleSize(tagRez, 0x7fff);
			if (rCode = MemError()) {			
				fprintf(stderr, "### %s - error %d on SetHandleSize.\n",
					argv[0], rCode);
				return 2;
			};
		} else {
			tagRez = NewHandle(0x7fff);
			if (rCode = MemError()) {			
				fprintf(stderr, "### %s - error %d on NewHandle.\n",
					argv[0], rCode);
				return 2;
			};
			addresource(tagRez, 'TAG ', tagID, "");
			if (rCode = ResError()) {			
				fprintf(stderr, "### %s - error %d on TAG addresource.\n",
					argv[0], rCode);
				return 2;
			};
		};
		HLock(tagRez);
		ntag = 0;
		r = *tagRez + 2;
	};
	
	/* Get the old CELL resource and increase its size to 32K.  If it
		doesn't exist create a 32K new one. */
		
	if (cellSpecified) {
		if ((cellRez = GetResource('CELL', cellID))  && 
			HomeResFile(cellRez) == ofileNum) {
			SetHandleSize(cellRez, 0x7fff);
			if (rCode = MemError()) {			
				fprintf(stderr, "### %s - error %d on SetHandleSize.\n",
					argv[0], rCode);
				return 2;
			};
		} else {
			cellRez = NewHandle(0x7fff);
			if (rCode = MemError()) {			
				fprintf(stderr, "### %s - error %d on NewHandle.\n",
					argv[0], rCode);
				return 2;
			};
			addresource(cellRez, 'CELL', cellID, "");
			if (rCode = ResError()) {			
				fprintf(stderr, "### %s - error %d on CELL addresource.\n",
					argv[0], rCode);
				return 2;
			};
		};
		HLock(cellRez);
		t = *cellRez;
	};
	
	/* Main loop.  Create each new STR# resource one at a time. */
	
	id = firstID;
	maxPicWidth = maxPicHeight = 0;
	while (true) {
		
		/*	Get the old STR# resource and increase its size to 32K.  If it
			doesn't exist create a 32K new one. */
		
		if (id-firstID >= 64) {
			fprintf(stderr, "### %s - more than 64 STR# resources.\n",
				argv[0], rCode);
			return 2;
		};
		if ((theRez = GetResource('STR#', id))  && 
			HomeResFile(theRez) == ofileNum) {
			SetHandleSize(theRez, 0x7fff);
			if (rCode = MemError()) {			
				fprintf(stderr, "### %s - error %d on SetHandleSize.\n",
					argv[0], rCode);
				return 2;
			};
		} else {
			theRez = NewHandle(0x7fff);
			if (rCode = MemError()) {			
				fprintf(stderr, "### %s - error %d on NewHandle.\n",
					argv[0], rCode);
				return 2;
			};
			addresource(theRez, 'STR#', id, "");
			if (rCode = ResError()) {			
				fprintf(stderr, "### %s - error %d on STR# id=%d addresource.\n",
					argv[0], rCode, id);
				return 2;
			};
		};
		
		/* Read lines from the input file, convert them to pascal format,
			and append them to the STR# buffer. */
			
		HLock(theRez);
		p = *theRez + 2;
		nlines = 0;
		while (fgets(str, 255, input)) {
			len = strlen(str);
			if (len && *(str+len-1) == '\n') {
				*(str+len-1) = 0;
				len--;
			};
			newRez = false;
			if (len && *str == '\\') {
				if (*(str+len-1) == docEop) {
					*(str+len-1) = 0;
					len--;
				};
				if (newRez = len >= 5 && !strncmp(str, "\\str#", 5)) break;
				if (len >= 5 && !strncmp(str, "\\tcon", 5)) {
					if (!tconSpecified) {
						fprintf(stderr, "### %s - tcon id not specified.\n",
							argv[0]);
						return 2;
					};
					*((short*)q) = totLines;
					q += 2;
					*q = nextOnly ? onlyCode : docScreen | docPrint;
					nextOnly = false;
					if (*q++ & docPrint) nptcon++;
					*q++ = len-6;
					memcpy(q, str+6, len-6);
					q += len-6;
					if ((long)q & 1) q++;
					ntcon++;
				} else if (len >= 4 && !strncmp(str, "\\tag", 4)) {
					if (!tagSpecified) {
						fprintf(stderr, "### %s - tag id not specified.\n",
							argv[0]);
						return 2;
					};
					*((short*)r) = totLines;
					r += 2;
					*((short*)r) = atoi(str+5);
					r += 2;
					ntag++;
				} else if (len >= 8 && !strncmp(str, "\\style", 6)) {
					nextStyle = true;
					styleCode = 0;
					s = str+6;
					while (s < str+len) {
						s += strspn(s, " \t");
						if (!strncmp(s, "normal", 6)) {
							styleCode = normal;
						} else if (!strncmp(s, "bold", 4)) {
							styleCode |= bold;
						} else if (!strncmp(s, "italic", 6)) {
							styleCode |= italic;
						} else if (!strncmp(s, "underline", 9)) {
							styleCode |= underline;
						} else if (!strncmp(s, "outline", 7)) {
							styleCode |= outline;
						} else if (!strncmp(s, "shadow", 6)) {
							styleCode |= shadow;
						} else if (!strncmp(s, "condense", 8)) {
							styleCode |= condense;
						} else if (!strncmp(s, "extend", 6)) {
							styleCode |= extend;
						} else {
							fprintf(stderr, 
								"### %s - illegal style option %s.\n",
								argv[0], s);
							return 2;
						};
						s += strcspn(s, " \t");
					};
				} else if (len >= 7 && !strncmp(str, "\\just", 5)) {
					nextJust = true;
					s = str+6;
					s += strspn(s, " \t");
					if (!strncmp(s, "left", 4)) {
						justCode = docLeft;
					} else if (!strncmp(s, "center", 6)) {
						justCode = docCenter;
					} else if (!strncmp(s, "right", 5)) {
						justCode = docRight;
					} else {
						fprintf(stderr, 
							"### %s - illegal justification option %s.\n",
							argv[0], s);
						return 2;
					};
				} else if (len >= 7 && !strncmp(str, "\\size", 5)) {
					nextSize = true;
					s = str+6;
					s += strspn(s, " \t");
					sizePercent = atoi(s);
				} else if (len >= 7 && !strncmp(str, "\\only", 5)) {
					nextOnly = true;
					onlyCode = 0;
					s = str+6;
					while (s < str+len) {
						s += strspn(s, " \t");
						if (!strncmp(s, "screen", 6)) {
							onlyCode |= docScreen;
						} else if (!strncmp(s, "print", 5)) {
							onlyCode |= docPrint;
						} else if (!strncmp(s, "save", 4)) {
							onlyCode |= docSave;
						} else {
							fprintf(stderr, 
								"### %s - illegal only option %s.\n",
								argv[0], s);
							return 2;
						};
						s += strcspn(s, " \t");
					};
				} else if (len >= 7 && !strncmp(str, "\\pict", 5)) {
					if (!pictFileSpecified) {
						fprintf(stderr, "### %s - picture file not specified.\n",
							argv[0]);
						return 2;
					};
					s = str+6;
					s += strspn(s, " \t");
					picID = atoi(s);
					picHandle = GetPicture(picID);
					if (!picHandle) {
						fprintf(stderr, "### %s - picture id %d not found.\n",
							argv[0], picID);
						return 2;
					};
					picWidth = (**picHandle).picFrame.right -
						(**picHandle).picFrame.left;
					picHeight = (**picHandle).picFrame.bottom -
						(**picHandle).picFrame.top;
					if (picWidth > maxPicWidth) maxPicWidth = picWidth;
					if (picHeight > maxPicHeight) maxPicHeight = picHeight;
					ReleaseResource((Handle)picHandle);
					if (picWidth <= cellWidth-8) {
						picScaledHeight = picHeight;
					} else {
						picScaledHeight = picHeight * (cellWidth-8) / picWidth;
					};
					picBands = (picScaledHeight + cellHeight - 1) / cellHeight;
					for (i = 0; i < picBands; i++) {
						if (p+13 - *theRez > 0x7f00) {
							fprintf(stderr, 
								"### %s - too much text in STR# resource (>32K).\n", 
								argv[0]);
							return 2;
						};
						pp = p;
						p++;
						strLen = 6;
						if (nextJust && justCode != docCenter) {
							*p++ = docJust;
							*p++ = 3;
							*p++ = justCode;
							strLen += 3;
						};
						if (nextOnly && onlyCode != (docScreen + docPrint)) {
							*p++ = docOnly;
							*p++ = 3;
							*p++ = onlyCode;
							strLen += 3;
						};
						*p++ = docPict;
						*p++ = 6;
						*p++ = picID >> 8;
						*p++ = picID & 0xff;
						*p++ = i >> 8;
						*p++ = i & 0xff;
						*pp = strLen;
						nlines++;
						if (!nextOnly || (onlyCode & docScreen)) {
							totLines++;
							if (cellSpecified) {
								*t++ = id-firstID;
								offset = pp - *theRez;
								*t++ = offset >> 8;
								*t++ = offset & 0xff;
							};
						};
					};
					nextStyle = nextJust = nextSize = nextOnly = false;
				} else if (len >= 5 && !strncmp(str, "\\page", 5)) {
					*p++ = 6;
					putOnlyPrint();
					s = str+5;
					s += strspn(s, " \t");
					*p++ = docPage;
					*p++ = 3;
					*p++ = (*s) ? 1 : 0; 
					nlines++;
				} else if (len >= 5 && !strncmp(str, "\\keep", 5)) {
					*p++ = 5;
					putOnlyPrint();
					*p++ = docKeep;
					*p++ = 2;
					nlines++;
				} else if (len >= 8 && !strncmp(str, "\\endkeep", 8)) {
					*p++ = 5;
					putOnlyPrint();
					*p++ = docEndKeep;
					*p++ = 2;
					nlines++;
				} else if (len >= 8 && !strncmp(str, "\\itcon", 6)) {
					s = str+6;
					s += strspn(s, " \t");
					ktcon = atoi(s);
					for (i = 0; i < ktcon; i++) {
						if (p+6 - *theRez > 0x7f00) {
							fprintf(stderr,
								"#### %s - too much text in STR# resource (>32K).\n",
								argv[0]);
							return 2;
						};
						*p++ = 6;
						putOnlyPrint();
						*p++ = docITcon;
						*p++ = 3;
						*p++ = i;
						nlines++;
					};
				};
			} else {
				if (p+len+1 - *theRez > 0x7f00) {
					fprintf(stderr, "### %s - too much text (>32K).\n", 
						argv[0]);
					return 2;
				};
				pp = p;
				p++;
				strLen = len;
				if (nextStyle && styleCode != normal) {
					*p++ = docStyle;
					*p++ = 3;
					*p++ = styleCode;
					strLen += 3;
				};
				if (nextJust && justCode != docLeft) {
					*p++ = docJust;
					*p++ = 3;
					*p++ = justCode;
					strLen += 3;
				};
				if (nextSize && sizePercent != 100) {
					*p++ = docSize;
					*p++ = 4;
					*p++ = sizePercent >> 8;
					*p++ = sizePercent & 0xff;
					strLen += 4;
				};
				if (nextOnly && onlyCode != 7) {
					*p++ = docOnly;
					*p++ = 3;
					*p++ = onlyCode;
					strLen += 3;
				};
				memcpy(p, str, len);
				p += len;
				*pp = strLen;
				nlines++;
				if (!nextOnly || (onlyCode & docScreen)) {
					totLines++;
					if (cellSpecified) {
						*t++ = id-firstID;
						offset = pp - *theRez;
						*t++ = offset >> 8;
						*t++ = offset & 0xff;
					};
				};
				nextStyle = nextJust = nextSize = nextOnly = false;
			};
		};
		
		/* Store the number of lines. */
		
		**(short**)theRez = nlines;
		HUnlock(theRez);
		
		/* Adjust STR# resource size and mark it purgable and changed. */
		
		size = p - *theRez;
		SetHandleSize(theRez, size);
		if (rCode = MemError()) {			
			fprintf(stderr, "### %s - error %d on SetHandleSize.\n",
				argv[0], rCode);
			return 2;
		};
		SetResAttrs(theRez, 0x20);
		ChangedResource(theRez);
		if (rCode = ResError()) {			
			fprintf(stderr, "### %s - error %d on ChangedResource.\n",
				argv[0], rCode);
			return 2;
		};
		
		/* Increment STR# resource id and loop. */
		
		if (!newRez) break;
		id++;
	};
	
	/* Store the number of tcon entries, adjust the TCON resource size,
		and mark it purgable and changed. */
	
	if (tconSpecified) {
		if (nptcon != ktcon) {
			fprintf(stderr, 
				"### %s - number of entries in \itcon directive should be %d.\n",
				argv[0], nptcon);
				return 2;
		};
		**((short**)tconRez) = ntcon;
		size = q - *tconRez;
		SetHandleSize(tconRez, size);
		if (rCode = MemError()) {			
			fprintf(stderr, "### %s - error %d on SetHandleSize.\n",
				argv[0], rCode);
			return 2;
		};
		SetResAttrs(tconRez, 0x20);
		ChangedResource(tconRez);
		if (rCode = ResError()) {			
			fprintf(stderr, "### %s - error %d on ChangedResource.\n",
				argv[0], rCode);
			return 2;
		};
	};
	
	/* Store the number of tag entries, adjust the TAG resource size,
		and mark it purgable and changed. */
	
	if (tagSpecified) {
		**((short**)tagRez) = ntag;
		size = r - *tagRez;
		SetHandleSize(tagRez, size);
		if (rCode = MemError()) {			
			fprintf(stderr, "### %s - error %d on SetHandleSize.\n",
				argv[0], rCode);
			return 2;
		};
		SetResAttrs(tagRez, 0x20);
		ChangedResource(tagRez);
		if (rCode = ResError()) {			
			fprintf(stderr, "### %s - error %d on ChangedResource.\n",
				argv[0], rCode);
			return 2;
		};
	};
	
	/* Adjust the CELL resource size, and mark it purgable and changed. */
	
	if (cellSpecified) {
		size = t - *cellRez;
		SetHandleSize(cellRez, size);
		if (rCode = MemError()) {			
			fprintf(stderr, "### %s - error %d on SetHandleSize.\n",
				argv[0], rCode);
			return 2;
		};
		SetResAttrs(cellRez, 0x20);
		ChangedResource(cellRez);
		if (rCode = ResError()) {			
			fprintf(stderr, "### %s - error %d on ChangedResource.\n",
				argv[0], rCode);
			return 2;
		};
	};
	
	/* Print the max pic width and height - commented out for now.
		Uncomment out if you want this info.
	
	if (maxPicWidth) {
		printf("### %s - Max picture width = %d.\n",
			argv[0], maxPicWidth);
		printf("### %s - Max picture height = %d.\n",
			argv[0], maxPicHeight);
	}; 
	
	*/
	
	/* Close the output and input files. */
		
	CloseResFile(ofileNum);
	if (pictFileSpecified) CloseResFile(pfileNum);
	if (rCode = ResError()) {			
		fprintf(stderr, "### %s - error %d on CloseResFile.\n",
			argv[0], rCode);
		return 2;
	};
	fclose(input);
	return 0;
};

			