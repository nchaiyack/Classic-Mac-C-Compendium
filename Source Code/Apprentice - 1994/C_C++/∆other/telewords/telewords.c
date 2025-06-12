/*
   Telewords takes a telephone number as the single input argument
   and outputs, one per line, all of the equivalent character strings,
   in the hope that one of them will be a real word, or at least
   pronouncable..  The digits 0 and 1, * which don't have letters,
   are printed as themselves.  Command line options allow you to change
   the string of characters into which each digit is mapped.
   This is helpful for selecting a PIN number on automatic teller machines
   that put q and z on the 1 (Why doesn't the phone company do that?).
   The phone number can have a arbitrary number of digits, and non-digits
   are ignored: '(301) 555-1212' is the same as '3015551212', provided the
   space is properly quoted to the shell.
  
   Written by James W. Williams
   williams@cs.umd.edu
   Last hacked on 28 January 1990.
  	This really ought to be ASNIfied at some point...
*/

#include <stdio.h>
#include <ctype.h>

#define USAGE	fprintf(stderr, "\tusage: telewords [-<digit>c...] ...  telephone-number\n")

char	*table[]   = {	"0",	/* mapping table for input digits. */
			"1",	/* The -<digit> options can modify */
			"abc",	/* this table. */
			"def",
			"ghi",
			"jkl",
			"mno",
			"prs",
			"tuv",
			"wxy"
			},
	teleword[128];  	/* buffer to hold output word. */


/*
 * A recursive subroutine to do the dirty work.
 */

void
doLetter(numberString, currentLetter)
char	*numberString,		/* pointer into input phone number string */
	*currentLetter;	/* pointer into the output word string */
{

	register char	ch, *mapstring;

	/* get the string of characters to use for the leading digit
	   in numberString and iterate over each character.  currentLetter
	   points to the position in teleword into which to stick each
	   possible mapping of the current digit.
	 */
	mapstring = table[*numberString-'0'];
	while (ch = *mapstring++) {
		*currentLetter = ch;
		if (*(numberString+1) == '\0') {
			printf("%s\n", teleword);
		}
		else {
			doLetter(numberString+1, currentLetter+1);
		}
	}
}

int
main(argc, argv)
int argc;
char **argv;
{
	int i;
	char *argptr, *cp = teleword, *to, *from;

        if (argc < 2) {
		USAGE;
		exit(1);
        }

	/* process arguments */
	argv++; argc--;
	while (**argv == '-') {
		argptr = *argv;
		switch (argptr[1]) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if (argptr[2] == '\0') {
				/* null string means map this digit 
				 * into itself.
				 */
				table[argptr[1]-'0'][0] = argptr[1];
				table[argptr[1]-'0'][1] = '\0';
			}
			else {
				/* the map for this digit is the string
				 * following the -<digit>
				 */
				table[argptr[1]-'0'] = &argptr[2];
			}
			break;
		default: /* unknown option */
			fprintf(stderr, "Unknown option letter %c.\n", argptr[1]);
		}
		argv++; argc--;
	}

	if (argc > 1) {
		fprintf(stderr, "Too many arguments!\n");
		USAGE;
		exit (1);
	}
	else if (argc < 1) {
		fprintf(stderr, "Missing telephone number.\n");
		USAGE;
		exit (1);
	}

	/* squeeze out all non-digits from argv[0].  This simplifies doLetter.
	 */
	to = from = argv[0];
	while (*from != '\0') {
		if (isdigit(*from)) {
			*to++ = *from++;
		}
		else {
			from++;
		}
	}
	*to = '\0';

	doLetter(argv[0], teleword);
}
