/*
 * mail.c
 */

#include <stdio.h>
#ifdef THINK_C
# include <console.h>
#endif THINK_C
#include "host.h"

#define exit return

FILE *freopen();

#define LSIZE    256

#include "mail.proto.h"

#include "mail.proto.h"

static char linebuf[LSIZE];
static char resp[LSIZE];

static char mfilename[132];

struct ldesc {
   int    delete;  /* status of this message */
   long  adr;  /* address of From line */
   long  Date;  /* address of Date: line */
   long  subject; /* address of Subject: line */
   long  from;  /* address of From: line */
   long  size;  /* number of lines */
};

struct ldesc letters[300];

int    letternum   = 0;
char    thefilename[50];

char    tmailbox[64];
char  tmailbag[64];


FILE    *fmailbox;
FILE  *rmailbox;
FILE *fmailbag;

int error = 0;
int modified;

extern SysEnvRec SysEnv;

int printonly;

int PageCount = 0;

mailmain(int argc, char **argv)
{

 /* HOSTINIT;
 loadenv(); */

 mkfilename( tmailbox, tempdir, "mailbox");
 mkfilename( tmailbag, tempdir, "mailbag");

 unlink( tmailbox );

 if ( mailbox == (char *)NULL )
  strcpy(mailbox,"???");

 if ( argc == 1 ||
  ( argv[1][0] == '-' ) && ( argv[1][1] != 's' ))
  showmail (argc, argv );
 else
  lmail (argc, argv );

 finis();

}

finis(void)
{
 if ( fmailbag != (FILE *)NULL) fclose( fmailbag );
 unlink( tmailbox );
 unlink( tmailbag );
 exit( error );
}

/* sendmail
 send message n to "line"
*/
sendmail(int n, char *line) {
 char  *argv[50];
 int  argc;
 char buf[BUFSIZ];
 long  bytes;

 /* open a temporary file */
 fmailbag = FOPEN( tmailbag, "w", 'b' );

 /* copy nth message to mailbag file */
 fseek( fmailbox, letters[ n ].adr, 0 );
 bytes = letters[ n+1 ].adr - letters[ n ].adr;

 while( bytes > 0 && fgets( buf, BUFSIZ, fmailbox) ) {
  fputs( buf, fmailbag  );
  bytes -= strlen( buf );
 }

 fclose( fmailbag );

 /* use mailbag as stdin to mail delivery */
 fclose( stdin );
 FILEMODE( 'b' );
 if(freopen(tmailbag, "r", stdin) != (FILE *)NULL) {
  argc = getargs(line, argv);
  lmail(argc, argv);
  fclose(stdin);
 }
 
#ifdef MSDOS
 FILEMODE( 't' );
 freopen( CONSOLE, "r", stdin );
#else
# ifndef THINK_C
 /* I don't know how this line following could ever
  * produce the correct results - device is "modem" here
  * not even /dev/tty for UNIX A. Shapiro 11/21/89
  */
 freopen( device, "r", stdin );
# else THINK_C
 /* opening a new console is NOT what is required here since
  * the old stdout is still attached to the old console.
  * Still if we get this far we'd better have something
  * from which to issue the 'q'. (hope this code is never reached
  * since the 'f' option was turned off.)
  * A. Shapiro 11/21/89
  */
 freopenc((FILE *)NULL, stdin);
 csetmode(C_ECHO, stdin);
# endif THINK_C
#endif

 /* get rid of mailbag */
 unlink( tmailbag );
}


linvert(int i)
{
 return( letternum - i - 1 );
}

showmail(int argc, char **argv) {
 int flg, i, j, k, print;
 char *p, *cp, *getnext();
 long nextadr;
 struct ldesc *letter;

 /* get mailbox file name */
 mkfilename( mfilename, maildir, mailbox );

 /* parse arg's */
 for (; argc > 1; argv++, argc--) {
  if ( argv[1][0] == '-' ) {
   if ( argv[1][1] == 'f' ) {
    if ( argc >= 3) {
     if ( argv[2][0] == '=' )
      mkfilename( mfilename, maildir, &argv[2][1] );
     else
      strcpy( mfilename, argv[2] );
     argv++;
     argc--;
    }

   }
   else if(argv[1][1 ]== 'p')
    printonly++;
   else {
    fprintf(stderr, "mail: unknown option %c\n", argv[1][1]);
    finis();
   }
  }
  else
   break;
 }

 /* open real mailbox file */
#ifdef Upgrade
 if((rmailbox=FOPEN(mfilename, "r", 't')) == (FILE *)NULL)
#else Upgrade
 if((rmailbox=FOPEN(mfilename, "r", 'b')) == (FILE *)NULL)
#endif Upgrade
 {
  fprintf(stdout, "No mail in %s.\n", mfilename);
  return;
 }

 /* open up tmp mailbox file */
   if (( fmailbox = FOPEN( tmailbox, "w", 'b' )) == (FILE *)NULL ) {
  fprintf( stderr, "mail: cannot open %s for writing\n", tmailbox );
  finis();
 }

 /* copy real mailbox file to tmp mailbox file */
 letternum = 0 ;
 nextadr = 0;
 while(fgets(linebuf, LSIZE, rmailbox) != (char *)NULL) {
  fputs(linebuf, fmailbox);

  if ( strncmp( linebuf, "From ", 5 ) == SAME ) {
   letter = &letters[letternum++];
   letter->from =
   letter->subject =
   letter->Date = -1L;
   letter->adr = nextadr;
   letter->delete = FALSE;
   letter->size = 0L;
  }else if ( strncmp( linebuf, "Date: ", 6 ) == SAME )
   letter->Date = nextadr;
  else if ( strncmp( linebuf, "From: ", 6 ) == SAME )
   letter->from = nextadr;
  else if ( strncmp( linebuf, "Subject: ", 9 ) == SAME )
   letter->subject = nextadr;

  letter->size += 1L;
  nextadr += strlen( linebuf );
 }

 letters[letternum].adr = nextadr;   /* last plus 1 */

 /* close mailbox files */
 fclose( rmailbox );
 fclose( fmailbox );

 fmailbox = FOPEN( tmailbox, "r", 'b' );

 modified = 0;

 if ( printonly ) {
  j = letternum;
  while ( j > 0 ) {
   pager( j );
   j--;
  }

  return;
     }


 printsub( -1 );

 i = letternum - 1;
 while ( TRUE ) {

  if ( i < 0 )
   i = 0;
  if ( i >= letternum)
   i = letternum - 1;
  j = linvert( i );

  fprintf( stdout, "%d ? ", linvert( i ) );
  
  if ( fgets( resp, LSIZE, stdin) == (char *)NULL )
   break;

  switch ( resp[0] ) {
  default:
   fprintf(stderr, "usage\n");
  case '?':
   print = 0;
   fprintf( stderr, "q\tquit\n" );
   fprintf( stderr, "x\texit\tmailbox restored\n" );
   fprintf( stderr, "p\tprint message\n" );
   fprintf( stderr, "s[file]\tsave message to a file (default =mbox)\n" );
   fprintf( stderr, "w[file]\tsave message to a file (default =mbox) without headers\n" );
   fprintf( stderr, "-\tprint last message\n" );
   fprintf( stderr, "d\tdelete current message\n" );
   fprintf( stderr, "+\tnext message (no delete)\n" );
   fprintf( stderr, "f user\tforward current message to user\n" );
   fprintf( stderr, "m user\tmail a message to user\n" );
   fprintf( stderr, "g num\t set current message\n" );
   break;
  case '+':
  case 'n':
   i-- ;
   break;
  case 'g':
   i = linvert( atoi( resp+2 ) );
   break;
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
   i = linvert( atoi( resp ) );
   break;

  case 'x':
   modified = 0;
  case 'q':
   goto donep;
  case '\n':
  case 'p':
   pager( j );
   break;
  case '^':
  case '-':
   i++;
   break;
  case 'y':
  case 'w':
  case 's':
   flg = 0;
   if (resp[1] != '\n' && resp[1] != ' ') {
    printf("illegal\n");
    flg++;
    print = 0;
    continue;
   }
   if (strlen( resp ) == 1)
    strcat( resp, " =mbox" );
   for (p = resp+1; (p = getnext( &cp, p )) != (char *)NULL; ) {
      if ( *cp == '=' )
                       mkfilename( thefilename, maildir, ++cp );
      else
         strcpy( thefilename, cp );
      fprintf( stderr, "%s to %s\n",
        (resp[0] == 's') ? "Appending":"Writing", thefilename );
      if (( fmailbag=FOPEN( thefilename, "a", 't' )) == (FILE *)NULL ) {
         fprintf( stdout, "mail: cannot append to %s\n",
                  thefilename );
      flg++;
      continue;
      }
      copymsg( j, fmailbag, resp[0]=='w' );
      fclose( fmailbag );
   }
   if (flg)
    print = 0;
   else {
    letters[j].delete = TRUE;
    modified++;
    i--;
   }
   break;
  case 'm':
   fprintf( stderr, "mail command not implemented yet!\n" );
   break;
  case 'f':
   fprintf( stderr, "forward command not implemented yet!\n" );
   break;
   
   /* NOTREACHED */
   /* the problem with this code seems to be with closure
    * of stdin to redirect input from mailbag. I would suggest
    * recoding this program to allow an extra parameter to
    * lmail, namely <stream> so that stdin is not futzed
    * with. This will (of course) mess with the beautifully
    * convoluted nature of this program and make it intelligible
    * but I guess we can't have everything
    * 8-). A. Shapiro 11/21/89
    */
   flg = 0;
   if (resp[1] == '\n' || resp[1] == '\0') {
    i++;
    continue;
   }
   if (resp[1] != ' ') {
    printf( "invalid command\n" );
    flg++;
    print = 0;
    continue;
   }

   sendmail(j, p);

   if (flg)
    print = 0;
   else {
    letters[j].delete = TRUE;
    modified++;
    i++;
   }
   break;

  case 'd':
   letters[j].delete = TRUE;
   modified++;
   i--;
   if (resp[1] == 'q')
    goto donep;
   break;

  case 'h':
   printsub( -1 );
   break;

  case 'H':
   printsub( i );
   break;

  }

 }
 donep:
 if ( modified )
  copyback();
}

readaline(long adr, char *line)
{
 char  buf[132];
 char *cp;

 *line = '\0';
 if ( adr != -1L )
  if ( fseek( fmailbox, adr, 0 ) == 0 )
   if ( fgets( buf, 132, fmailbox ) != (char *)NULL )
    strcpy( line, buf );

 if ( (cp = index( line, '\n' ) ) != (char *) NULL)
  *cp = '\0';
}


/*
returnaddress()
{
 readaline( lp->from, linebuf );
 if (
  ( i = index( linebuf, '<' )) != 0 &&
  ( k = index( linebuf, '>')) != 0 &&
  ( k > i )
  )
 {
  k -= i;
  strncpy( from, linebuf[i], k );
  from[k] = '\0';
 }
 else {


 }
}
*/

printsub(int K)
{
 struct ldesc *ld;
 int  j;
 char from[132], subject[132], Date[132], line[132];
 register char *sp, *dp, *lp, *tp;
 int k, mink, maxk;

 pagereset();

 if ( K == -1 ) {
  maxk = letternum -1;
  mink = 0;
 }
 else
  maxk = mink = K;

 for ( k = maxk ; k >= mink ; k-- ) {

  j = linvert( k );
  ld = & letters[ j /*k*/];

   (void)strcpy(from, "unknown"); /* default to "unknown" */
  readaline( ld->from, line );
  if ( strlen( line ) > 0 ) {
   /* Assume From: mailbox@site.domain */
   /* get first usable stuff after From: */
   sp = index( line, ':' ) + 1;

   /* do we have From: mailbox@site.domain ( Proper Name ) */
   if ( ( ( tp = index( line, '(' )) != (char *) NULL &&
     ( lp = index( line, ')')) != (char *) NULL  ) )
    sp = ++tp;
   /* or we may have From: Proper Name < mailbox@site.domain > */
   else
    lp = index( line, '<');

   if ( *lp == '<' || *lp == ')' )
    --lp;

   /* if we didn't get either of the last two, just grab line */
   if ( lp < sp || lp == (char *) NULL )
    lp = line + strlen( line ) - 1;

   /*debugMsg("\PCheck isspace *sp");*/
   while ( isspace( *sp ) ) sp++;

   /*debugMsg("\PCheck isspace *lp");*/
   while ( isspace( *lp ) ) lp--;

   dp = from;
   while ( sp <= lp ) *dp++ = *sp++;
   *dp = '\0';
  }

  /* Date: Wed May 13 23:59:53 1987 */
   *Date = '\0'; /* default Date to null */
  readaline( ld->Date, Date );
  if ( strlen( Date ) > 0 ) {
   sscanf( Date, "%*s %*s %s %s", line, subject );
   sprintf( Date, "%s %s", line, subject );
  }

  strcpy( subject, "--- no subject ---" );
  readaline( ld->subject, line );
  if ( strlen( line ) > 0 ) {
   sp = line;
   while ( !isspace( *sp ) ) sp++;
   while ( isspace( *sp ) ) sp++;
   strcpy( subject, sp );
   }

  from[20] = '\0';
  Date[6] = '\0';
  subject[35] = '\0';

  sprintf( line, "%3d  %6s  %20s %35s (%6d)\n",
      j, Date, from, subject, ld->size );
  if ( pageline( line ) == TRUE )
   break;
 }
}


copyback(void)
{
 register i, n, c;
 int new = 0;

 if (( fmailbag = FOPEN( mfilename, "w", 'b' )) == (FILE *)NULL ) {
  fprintf (stderr, "mail: can't rewrite %s\n", mfilename );
  finis();
 }
 n = 0;
 for ( i = 0; i < letternum; i++ )
  if ( letters[i].delete == FALSE ) {
   copymsg( i, fmailbag, FALSE );
   n++;
  }
 fclose (fmailbag );
}


/* copy a message
   if noheaders true, don't copy header lines
*/
copymsg(int n, FILE *f, int noheaders)
{
 long  bytes;
 char buf[BUFSIZ];

 fseek( fmailbox, letters[n].adr, 0 );
 bytes = letters[n+1].adr - letters[n].adr;

 while ( bytes > 0 && fgets( buf, BUFSIZ, fmailbox ) != (char *)NULL ) {
  bytes -= strlen( buf );

  /* write line */
  if ( !noheaders  )
   fputs( buf, f );

  /* reset header */
  if ( noheaders  && strcmp( buf, "\n") == SAME )
   noheaders  = FALSE;

 }
}

pager(int n)
{
 long  bytes;
 char buf[BUFSIZ];

 fseek( fmailbox, letters[n].adr, 0 );
 bytes = letters[n+1].adr - letters[n].adr;

 pagereset();
 while ( bytes > 0 && fgets( buf, BUFSIZ, fmailbox ) != (char *)NULL ) {
  bytes -= strlen( buf );
  if ( pageline( buf ) == TRUE )
   break;
 }

 pageline( "\n" );
}

pagereset(void)
{
 PageCount = 0;
}

pageline(char *s)
{
 char c;

 fputs( s, stdout );
 if ( ++PageCount > 23 ) {
  PageCount = 0;
  c = get_one();
  switch( c ) {
  case 'q':
  case 'Q':
  case '\003':
  case 'x':
  case 'X':
   return( TRUE );
  }
 }
 return( FALSE );
}

char *getnext(char **s, char *p)
{
 *s = (char *)NULL;
 while ( *p == ' ' || *p == '\t' )
  p++;
 if ( *p == '\n' || *p == '\0' )
  return( (char *)NULL );
 *s = p;
 while ( *p != ' ' && *p != '\t' && *p != '\n' && *p != '\0' )
  *p++;
 if ( *p != '\0' )
  *p++ = '\0';
 return(p);
}

#ifdef MULTIFINDER

int Check_Events(long Sleep_Time)
{
	 EventRecord Cur_Event;
	(void) WaitNextEvent(updateMask, &Cur_Event, 0, (RgnHandle) NULL);
	SystemTask();
}

#endif
