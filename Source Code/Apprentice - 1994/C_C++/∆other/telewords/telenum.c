/*
   Telenum changes each of its string arguments into the corresponding
   telephone number.  There is no restriction on the length of the string
   and input letters can be of either case.
   The mapping used is the standard American mapping, unless the -<digit>
   options are used to change it.  Letters which do not map to any digit
   are printed as themselves.  This is the default for q and z.
  
   Written by James W. Williams
   williams@cs.umd.edu
   Last hacked on 28 January 1990.
  	This really ought to be ASNIfied at some point...
*/

#include <stdio.h>
#include <ctype.h>

#define USAGE	fprintf(stderr, "\tusage: telenum [-<digit>c...] ...  word ...\n")

/* table is used to map an input letter to the corresponding digit. */
unsigned char	table[256];

initTable()
{
	int i;

	/* first make each character map to itself, then set up
	   default mapping
	*/
	for (i=0; i < 256; i++) table[i] = i;

	table['A'] = table['a'] = '2';
	table['B'] = table['b'] = '2';
	table['C'] = table['c'] = '2';
	table['D'] = table['d'] = '3';
	table['E'] = table['e'] = '3';
	table['F'] = table['f'] = '3';
	table['G'] = table['g'] = '4';
	table['H'] = table['h'] = '4';
	table['I'] = table['i'] = '4';
	table['J'] = table['j'] = '5';
	table['K'] = table['k'] = '5';
	table['L'] = table['l'] = '5';
	table['M'] = table['m'] = '6';
	table['N'] = table['n'] = '6';
	table['O'] = table['o'] = '6';
	table['P'] = table['p'] = '7';
	table['R'] = table['r'] = '7';
	table['S'] = table['s'] = '7';
	table['T'] = table['t'] = '8';
	table['U'] = table['u'] = '8';
	table['V'] = table['v'] = '8';
	table['W'] = table['w'] = '9';
	table['X'] = table['x'] = '9';
	table['Y'] = table['y'] = '9';
}

int
main(argc, argv)
int argc;
char **argv;
{
	int i;
	char *argptr, *cp, optChar, *currentStr;

        if (argc < 2) {
		USAGE;
		exit(1);
        }

	initTable();

	/* process arguments. */
	argv++; argc--;
	while (**argv == '-') {
		argptr = *argv;
		optChar = argptr[1];
		switch (optChar) {
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
			cp = &argptr[2];
			while (*cp != '\0') {
				table[*cp] = optChar;
				if (isupper(*cp))
					table[tolower(*cp)] = optChar;
				else if (islower(*cp))
					table[toupper(*cp)] = optChar;
				cp++;
			}
			break;
		default: /* unknown option */
			fprintf(stderr, "Unknown option letter %c.\n", argptr[1]);
		}
		argv++; argc--;
	}

	if (argc < 1) {
		fprintf(stderr, "Missing argument.\n");
		USAGE;
		exit (1);
	}

	while (*argv != NULL) {
		currentStr = *argv++;
		while (*currentStr) putchar(table[*currentStr++]);
		putchar('\n');
	}
}
