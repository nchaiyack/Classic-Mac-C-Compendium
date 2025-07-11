
/**	program DATABASE.C					*
 *	WARNING: "advent.c" allocates GLOBAL storage space by	*
 *		including "advdef.h".				*
 *		All other modules use "advdec.h".		*/


#include        <string.h>
#include	<ctype.h>
#include	<stdio.h>
#include	"advent.h"
#include	"advdec.h"
#include	"advcave.h"

static char oline[256];

_PROTOTYPE(void rdupto, (FILE *, int, int, char *));
_PROTOTYPE(void rdskip, (FILE *, int, int));
_PROTOTYPE(int binary, (char *, struct wac *, int));


/*
  Routine to fill travel array for a given location
*/
void gettrav(loc, travel)
int loc;
struct trav *travel;
{
    int i;
    long t, *lptr;

    lptr = cave[loc - 1];
    for (i = 0; i < MAXTRAV; i++) {
	t = *lptr++;
	if (!(t)) {
	    travel->tdest = -1;		/* end of array	 */
	    return;			/* terminate for loop	 */
	}
	travel->tverb = (int) (t % 1000);
	t /= 1000;
	travel->tdest = (int) (t % 1000);
	t /= 1000;
	travel->tcond = (int) (t % 1000);
	travel++;
    }
    bug(25);
    return;
}

/*
  Function to scan a file up to a specified
  point and either print or return a string.
*/
void rdupto(fdi, uptoc, print, string)
FILE *fdi;
int uptoc, print;
char *string;
{
    int c, i;
    static const unsigned char key[4] = {'c' | 0x80, 'L' | 0x80,
					 'y' | 0x80, 'D' | 0x80};

    i = 1;
    while ((c = fgetc(fdi)) != uptoc) {
	if (c == EOF)
	    return;
	if (c == '\n')
	    i = 1;
	if (c >= 0x80)
	    c ^= key[i++ & 3];
	if (c == '\r')
	    continue;
	if (print)
	    fputc(c, stdout);
	else
	    *string++ = (char) c;
    }
    if (!print)
	*string = '\0';
    return;
}

/*
  Function to read a file skipping
  a given character a specified number
  of times, with or without repositioning
  the file.
*/
void rdskip(fdi, skipc, n)
FILE *fdi;
int skipc, n;
{
    int c;

    while (n--)
	while ((c = fgetc(fdi)) != skipc)
	    if (c == EOF)
		bug(32);
    return;
}

/*
  Routine to request a yes or no answer to a question.
*/
boolean
yes(msg1, msg2, msg3)
int msg1, msg2, msg3;
{
    char answer[80];

    if (msg1)
	rspeak(msg1);
    fprintf(stdout, "\n> ");
    fgets(answer, 80, stdin);
    if (tolower(answer[0]) == 'n') {
	if (msg3)
	    rspeak(msg3);
	return (FALSE);
    }
    if (msg2)
	rspeak(msg2);
    return (TRUE);
}

/*
  Print a location description from "advent4.txt"
*/
void rspeak(msg)
int msg;
{
    if (msg == 54)
	printf("ok.\n");
    else {
	fseek(fd4, idx4[msg - 1], 0);
	rdupto(fd4, '#', 1, 0);
    }
    return;
}

/*
  Print an item message for a given state from "advent3.txt"
*/
void pspeak(item, state)
int item, state;
{
    fseek(fd3, idx3[item - 1], 0);
    rdskip(fd3, '/', state + 2);
    rdupto(fd3, '/', FALSE, oline);
    if (strncmp(oline, "<$$<", 4) != 0)
	printf("%s", oline);
    return;
}

/*
  Print a long location description from "advent1.txt"
*/
void desclg(loc)
int loc;
{
    fseek(fd1, idx1[loc - 1], 0);
    rdupto(fd1, '#', 1, 0);
    return;
}

/*
  Print a short location description from "advent2.txt"
*/
void descsh(loc)
int loc;
{
    fseek(fd2, idx2[loc - 1], 0);
    rdupto(fd2, '#', 1, 0);
    return;
}

/*
  look-up vocabulary word in lex-ordered table.  words may have
  two entries with different codes. if minimum acceptable type
  = 0, then return minimum of different codes.  last word CANNOT
  have two entries(due to binary sort).
  word is the word to look up.
  type  is the minimum acceptable value,
  if != 0 return %1000
*/
int vocab(word, type)
char *word;
int type;
{
    int v1, v2, temp;

    if ((v1 = binary(word, wc, MAXWC)) >= 0) {
	if (v1 > 1 && strcmp(word, wc[v1 - 1].aword) == 0)
	    v2 = v1 - 1;
	else if (v1 < (MAXWC - 1) && strcmp(word, wc[v1 + 1].aword) == 0)
	    v2 = v1 + 1;
	else
	    v2 = v1;
	if (wc[v1].acode > wc[v2].acode) {
	    temp = v1;
	    v1 = v2;
	    v2 = temp;
	}
	if (type <= CLASS(wc[v1].acode))
	    return (wc[v1].acode);
	else if (type <= CLASS(wc[v2].acode))
	    return (wc[v2].acode);
	else
	    return (-1);
    } else
	return (-1);
}

int binary(w, wctable, maxwc)
char *w;
struct wac wctable[];
int maxwc;
{
    int lo, mid, hi, check;

    lo = 0;
    hi = maxwc - 1;
    while (lo <= hi) {
	mid = (lo + hi) / 2;
	if ((check = strcmp(w, wctable[mid].aword)) < 0)
	    hi = mid - 1;
	else if (check > 0)
	    lo = mid + 1;
	else
	    return (mid);
    }
    return (-1);
}
