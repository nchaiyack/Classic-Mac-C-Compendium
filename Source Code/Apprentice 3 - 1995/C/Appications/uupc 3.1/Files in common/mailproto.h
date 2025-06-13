/* from pmail.c
 */
int sendit(int, char **, char *, char *);

/* Protoypes grabbed from file
** "lmail.c"
** 1989 Oct 15 (Sun) 16:05:09
*/

int lmail(int, char **);
int rmail(int, char **);

/* Protoypes grabbed from file
** "mail.c"
** 1989 Jul 30 (Sun) 23:21:01
*/



char *retfgets(char *, int, FILE *);

int mailmain(
	int, 	/* WARNING: argument was not declared! */char **);

int finis(void);

int sendmail(
	int, 	/* WARNING: argument was not declared! */
	char *);

int linvert(int);

int showmail(
	int, 	/* WARNING: argument was not declared! */char **);

int readaline(long, char *);

int printsub(
	int);	/* WARNING: argument was not declared! */

int copyback(void);

int copymsg(
	int, 	/* WARNING: argument was not declared! */FILE *,
	int);	/* WARNING: argument was not declared! */

int pager(
	int);	/* WARNING: argument was not declared! */

int pagereset(void);

int pageline(char *);

char *getnext(char **, char *);

/* Protoypes grabbed from file
** "mailhost.c"
** 1989 Jul 30 (Sun) 23:55:18
*/

/* Protoypes grabbed from file
** "mlib.c"
** 1989 Oct 22 (Sun) 18:49:37
*/

int get_one(void);

