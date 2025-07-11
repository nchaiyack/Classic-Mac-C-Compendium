/* getopt.c */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "getopt.h"
#include "MacMemory.h"

int	optind = 1;
char *optarg = NULL;


int getopt(int argc, char *argv[], char *options)
{
int i, optLen, argLen;
int found = EOF;

	if (optarg) {
		mfree(optarg);
		optarg = NULL;
	}

	optLen = strlen(options);
	argLen = strlen(argv[optind]);
	if ((optind < argc) && (argv[optind][0] == '-') && (argLen > 1)) {

		/* if first character is a '-', and the string has
		 * more than one character, then ...
		 */
		for (i = 0; i < optLen; i++) {

			/* if first character after the '-' is in the options */
			if (argv[optind][1] == options[i]) {

				found = options[i];			/* store to return it */

				/* if this option requires an argument */
				if (options[i+1] == ':') {
					optind++;				/* increase to get to argument */
					optarg = (char *) mmalloc(strlen(argv[optind])+1);
					strcpy(optarg, argv[optind]);

					/* Leave optind ready for next time */
					optind++;
				}

				else {
					/* This option does not require an argument */
					if (optarg) {
						mfree(optarg);
						optarg = NULL;
					}

					/* if there is more than one option, then copy all the
					 * other options up one character
					 */
					if (argLen > 2) {
						strcpy(&argv[optind][1], &argv[optind][2]);
						argLen = strlen(argv[optind]);

						/* do not update optind, so that we check this option
						 * again.
						 */
					}
					else {
						/* Leave optind ready for next time */
						optind++;
					}
				}
				break;
			}
		}

	}
	
	return found;
}
