
/**	program SETUP.C  					*
 *	execution will read the four adventure text files	*
 *	files; "advent1.txt", "advent2.txt", "advent3.txt" &	*
 *	"advent4.txt".  it will create the file "advtext.h"	*
 *	which is an Index Sequential Access Method (ISAM)	*
 *	header to be #included into "advent.c" before the	*
 *	header "advdef.h" is #included.				*/


#include	<stdio.h>
#include	<stdlib.h>
#include	"advent.h"

_PROTOTYPE(void wheel, (void));
_PROTOTYPE(void encode, (unsigned char *));

int main(void)
{

    FILE *isam, *src, *dest;
    char itxt[255];
    int cnt;
    long llen;

    isam = fopen("advtext.h", "w");
    if (!isam) {
	printf("Sorry, I can't open advtext.h...\n");
	exit(EXIT_FAILURE);
    }
    fprintf(isam, "\n/");
    fprintf(isam, "*\theader: ADVTEXT.H\t\t\t\t\t*/\n\n\n");


    cnt = -1;
    llen = 0L;
    printf("\nProcessing file advent1.txt  ");
    src = fopen("advent1.txt", "r");
    if (!src) {
	printf("Sorry, I can't open advent1.txt...\n");
	exit(EXIT_FAILURE);
    }
    dest = fopen("advent1.dat", "w");
    if (!src) {
	printf("Sorry, I can't open advent1.dat...\n");
	exit(EXIT_FAILURE);
    }
    fprintf(isam, "long\tidx1[MAXLOC] = {\n\t");
    while (fgets(itxt, 255, src)) {
	wheel();
	encode((unsigned char *) itxt);
	fprintf(dest, "%s\n", itxt);
	if (itxt[0] == '#') {
	    if (llen)
		fprintf(isam, "%ld,", llen);
	    llen = ftell(dest);
	    if (!llen) {
		printf("ftell err in advent1.dat\n");
		exit(EXIT_FAILURE);
	    }				/* if (!llen)	 */
	    if (++cnt == 5) {
		fprintf(isam, "\n\t");
		cnt = 0;
	    }				/* if (cnt)	 */
	}				/* if (itxt[0])	 */
    }					/* while fgets	 */
    fprintf(isam, "%ld\n\t};\n\n", llen);
    fclose(src);
    fclose(dest);

    cnt = -1;
    llen = 0L;
    printf("\b completed\nProcessing file advent2.txt  ");
    src = fopen("advent2.txt", "r");
    if (!src) {
	printf("Sorry, I can't open advent2.txt...\n");
	exit(EXIT_FAILURE);
    }
    dest = fopen("advent2.dat", "w");
    if (!src) {
	printf("Sorry, I can't open advent2.dat...\n");
	exit(EXIT_FAILURE);
    }
    fprintf(isam, "long\tidx2[MAXLOC] = {\n\t");
    while (fgets(itxt, 255, src)) {
	wheel();
	encode((unsigned char *) itxt);
	fprintf(dest, "%s\n", itxt);
	if (itxt[0] == '#') {
	    if (llen)
		fprintf(isam, "%ld,", llen);
	    llen = ftell(dest);
	    if (!llen) {
		printf("ftell err in advent2.dat\n");
		exit(EXIT_FAILURE);
	    }				/* if (!llen)	 */
	    if (++cnt == 5) {
		fprintf(isam, "\n\t");
		cnt = 0;
	    }				/* if (cnt)	 */
	}				/* if (itxt[0])	 */
    }					/* while fgets	 */
    fprintf(isam, "%ld\n\t};\n\n", llen);
    fclose(src);
    fclose(dest);

    cnt = -1;
    llen = 0L;
    printf("\b completed\nProcessing file advent3.txt  ");
    src = fopen("advent3.txt", "r");
    if (!src) {
	printf("Sorry, I can't open advent3.txt...\n");
	exit(EXIT_FAILURE);
    }
    dest = fopen("advent3.dat", "w");
    if (!src) {
	printf("Sorry, I can't open advent3.dat...\n");
	exit(EXIT_FAILURE);
    }
    fprintf(isam, "long\tidx3[MAXOBJ] = {\n\t");
    while (fgets(itxt, 255, src)) {
	wheel();
	encode((unsigned char *) itxt);
	fprintf(dest, "%s\n", itxt);
	if (itxt[0] == '#') {
	    if (llen)
		fprintf(isam, "%ld,", llen);
	    llen = ftell(dest);
	    if (!llen) {
		printf("ftell err in advent3.txt\n");
		exit(EXIT_FAILURE);
	    }				/* if (!llen)	 */
	    if (++cnt == 5) {
		fprintf(isam, "\n\t");
		cnt = 0;
	    }				/* if (cnt)	 */
	}				/* if (itxt[0])	 */
    }					/* while fgets	 */
    fprintf(isam, "%ld\n\t};\n\n", llen);
    fclose(src);
    fclose(dest);

    cnt = -1;
    llen = 0L;
    printf("\b completed\nProcessing file advent4.txt  ");
    src = fopen("advent4.txt", "r");
    if (!src) {
	printf("Sorry, I can't open advent4.txt...\n");
	exit(EXIT_FAILURE);
    }
    dest = fopen("advent4.dat", "w");
    if (!src) {
	printf("Sorry, I can't open advent4.dat...\n");
	exit(EXIT_FAILURE);
    }
    fprintf(isam, "long\tidx4[MAXMSG] = {\n\t");
    while (fgets(itxt, 255, src)) {
	wheel();
	encode((unsigned char *) itxt);
	fprintf(dest, "%s\n", itxt);
	if (itxt[0] == '#') {
	    if (llen)
		fprintf(isam, "%ld,", llen);
	    llen = ftell(dest);
	    if (!llen) {
		printf("ftell err in advent4.dat\n");
		exit(EXIT_FAILURE);
	    }				/* if (!llen)	 */
	    if (++cnt == 5) {
		fprintf(isam, "\n\t");
		cnt = 0;
	    }				/* if (cnt)	 */
	}				/* if (itxt[0])	 */
    }					/* while fgets	 */
    fprintf(isam, "%ld\n\t};\n\n", llen);
    printf("\b completed\n");
    fclose(src);
    fclose(dest);

    fclose(isam);
    return EXIT_SUCCESS;
}					/* main		 */

const unsigned char key[4] = {'c' | 0x80, 'L' | 0x80, 'y' | 0x80, 'D' | 0x80};

void encode(msg)
unsigned char *msg;
{
    register int i;

    for (i = 1; msg[i]; i++)
	msg[i] ^= key[i & 3];
    msg[--i] = '\0';
    return;
}

static char gyliph[5] = "|/-\\";
static char *g_ptr = gyliph;

void wheel()
{
    putchar('\b');
    putchar(*g_ptr++);
    if (g_ptr >= (gyliph + 4))
	g_ptr = gyliph;

    return;
}
