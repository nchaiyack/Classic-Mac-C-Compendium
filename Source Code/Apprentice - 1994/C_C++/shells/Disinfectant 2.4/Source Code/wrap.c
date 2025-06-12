/*______________________________________________________________________

	wrap.c - Tool to word wrap paragraphs.
	
	Author: John Norstad
	
	Copyright © 1988, 1989, 1990 Northwestern University.  Permission is granted
	to use this code in your own projects, provided you give credit to both
	John Norstad and Northwestern University in your about box or document.
	
	This tool reads standard input and writes standard output.  It
	word wraps each paragraph from the input file.
	
	In Disinfectant I use this tool to help build the online document.  I
	maintain the source using Microsoft word on a text-only file, without line 
	breaks.  This saves each paragraph as one big long line.  My makefile 
	runs the wrap tool to word wrap each of these paragraphs to the exact 
	width of my help window rectangle.  The output of wrap is then fed through 
	the cvrt tool to generate the sequence of STR# resources used by the program.
	
	wrap -r xxxx [-p]
	
	xxxx = right margin in pixels.
	
	-p = if specified, add a special end-of-paragraph byte 31 to the
		end of each line.  This option should be specified when preparing
		type 1 reports for the rep.c module.  Rep.c uses the special eop
		markers to identify paragraphs in the printing code.
	
_____________________________________________________________________*/

#pragma load "precompile"

#include "doc.h"


short main(short argc, char *argv[])

{
	char		line[10000];
	char		*first;
	char		*last;
	char		*prev;
	char		*next;
	char		save;
	short		len;
	GrafPort	myPort;
	Boolean	rSpecified;
	Boolean	pSpecified;
	short		margin;
	short		i;
	
	/* Crack and check parameters. */

	i = 1;
	rSpecified = pSpecified = false;
	while (i < argc) {
		if (*argv[i] == '-') {
			if (tolower(*(argv[i]+1)) == 'r') {
				rSpecified = true;
				margin = atoi(argv[i+1]);
				i += 2;
			} else if (tolower(*(argv[i]+1)) == 'p') {
				pSpecified = true;
				i += 2;
			} else {
				fprintf(stderr, "### %s - Usage: %s -r rightmargin [-p].\n", 
					argv[0], argv[0]);
				return 1;
			};
		} else {
			fprintf(stderr, "### %s - Usage: %s -r rightmargin [-p].\n", 
				argv[0], argv[0]);
			return 1;
		};
	};
	if (!rSpecified) {
		fprintf(stderr, "### %s - right margin not specified.\n", argv[0]);
		return 1;
	};

	/* Process file. */
	
	InitGraf(&qd.thePort);
	OpenPort(&myPort);
	TextFont(applFont);
	TextSize(9);
	while (gets(line)) {
		first = last = prev = line;
		len = strlen(line);
		*(line+len) = ' ';
		*(line+len+1) = 0;
		while (true) {
			while (true) {
				next = strchr(prev, ' ');
				if (!next || TextWidth(first, 0, next-first) > margin ) break;
				last = next;
				prev = last + strspn(last, " ");
			};
			save = *last;
			if (next) {
				*last = 0;
				puts(first);
				*last = save;
				first = prev;
				last = next;
			} else {
				if (pSpecified) {
					*last = docEop;
					*(last+1) = 0;
					puts(first);
					break;
				} else {
					*last = 0;
					puts(first);
					break;
				};
			};
		};
	};
	
	return 0;
}