/**     Adventure translated from Fortran to "C"
  and ported to Minix by:
  Robert R. Hall
  San Diego,  Calif  92115
  hall@crash.cts.com
 */

/**	program ADVENT.C					*
 *	WARNING: "advent.c" allocates GLOBAL storage space by	*
 *		including "advdef.h".				*
 *		All other modules use "advdec.h"		*/


#include        <string.h>
#include	<ctype.h>
#include	<stdlib.h>
#include	<time.h>
#include        <stdio.h>
#include	"advent.h"		/* #define preprocessor equates	 */
#include	"advword.h"		/* definition of "word" array	 */
#include	"advtext.h"		/* definition of "text" arrays	 */
#include	"advdef.h"
#ifdef _MINIX
#include	<unistd.h>
#endif

char *textdir = "/usr/src/data";/* directory where text files live. */

_PROTOTYPE(int main, (void));
_PROTOTYPE(static void opentxt, (void));

int main(void)
{
#ifdef _MINIX
    int n;			/* user restore request option	 */

    n = chdir(textdir);		/* all the goodies are kept there. */
    if (n < 0) {
  	printf("Unable to chdir(%s) where text files must be kept\n", textdir);
  	exit(EXIT_FAILURE);
    }
#endif

    opentxt();
    initialize();
    rspeak(325);
    g.hinted[3] = yes(65, 1, 0);
    if (g.hinted[3])
	g.limit = 900;
    else
	g.limit = 650;
    gaveup = FALSE;
    srand((unsigned) time(NULL));	/* seed random	 */
    while (!gaveup)
	turn();
    fclose(fd1);
    fclose(fd2);
    fclose(fd3);
    fclose(fd4);
    return (EXIT_SUCCESS);		/* exit = ok	 */
}					/* main		 */

/*
  Open advent?.txt files
*/
static void opentxt()
{
    fd1 = fopen("advent1.dat", "r");
    if (!fd1) {
	printf("Sorry, I can't open advent1.dat... \n");
	exit(EXIT_FAILURE);
    }
    fd2 = fopen("advent2.dat", "r");
    if (!fd2) {
	printf("Sorry, I can't open advent2.dat... \n");
	exit(EXIT_FAILURE);
    }
    fd3 = fopen("advent3.dat", "r");
    if (!fd3) {
	printf("Sorry, I can't open advent3.dat... \n");
	exit(EXIT_FAILURE);
    }
    fd4 = fopen("advent4.dat", "r");
    if (!fd4) {
	printf("Sorry, I can't open advent4.dat... \n");
	exit(EXIT_FAILURE);
    }
    return;
}

/*
  save adventure game
*/
void saveadv()
{
    unsigned char *sptr;
    FILE *savefd;
    char *username = "advent.sav";

    savefd = fopen(username, "wb");
    if (savefd == NULL) {
	printf("Sorry, I can't create the file... %s\n",
	       username);
	exit(EXIT_FAILURE);
    }
    for (sptr = (unsigned char *) &g.turns;
	 sptr < (unsigned char *) &g.lastglob; sptr++) {
	if (fputc(*sptr, savefd) == EOF) {
	    printf("Write error on save file... %s\n",
		   username);
	    printf("ferror is: 0x%.4x\n", ferror(savefd));
	    exit(EXIT_FAILURE);
	}
    }
    if (fclose(savefd) == -1) {
	printf("Sorry, I can't close the file... %s\n",
	       username);
	exit(EXIT_FAILURE);
    }
    printf("Accomplished\n");
    return;
}

/*
  restore saved game handler
*/
void restore()
{
    char *username = "advent.sav";
    int c;
    FILE *restfd;
    unsigned char *sptr;

    restfd = fopen(username, "rb");
    if (restfd == NULL) {
	printf("Sorry, I can't open the file... %s\n",
	       username);
	exit(EXIT_FAILURE);
    }
    for (sptr = (unsigned char *) &g.turns;
	 sptr < (unsigned char *) &g.lastglob; sptr++) {
	if ((c = fgetc(restfd)) == -1) {
	    printf("Read error on save file... %s\n",
		   username);
	    printf("ferror is: 0x%.4x\n", ferror(restfd));
	    exit(EXIT_FAILURE);
	}
	*sptr = (unsigned char) c;
    }
    if (fclose(restfd) == -1) {
	printf("Warning -- can't close save file... %s\n",
	       username);
    }
    printf("Done\n");
    return;
}
