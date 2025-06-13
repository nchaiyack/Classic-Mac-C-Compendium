/*		mac.h
 *
 *		Macintosh version
 *
 */

/* on */
#define MULTIFINDER	/* turn on support for running under multifinder on mac */

#ifdef MULTIFINDER
extern int Check_Events(long Sleep_Time);		/* call here periodically for multifinder to work */

extern long Last_Check_Event;		/* last tick count of Check_Events call */
#define MF_DELAY (6)				/* ticks between event checks */
#endif

/* on */
#define Upgrade 	/* turn on upgrades to standard uupc for mac
					 * \number (variable timeout) feature for L.sys (systems) file.
					 * fixed BREAK sending.
					 * alen@crash 10/22/89.
					 * garym@crash implemented L.sys blank, comment and continuation
					 * lines 10/27/89.
					 */
/* on */
#define BREAK		/* enable BREAKx send string
					 */
/* on */
#define SLAVELOG	/* so slave mode will put out Username: & Password:
					 * prompts.
					 */

/* on */
#define DEF_FRAME	/* to enable default ring around default button in dialog */

typedef int boolean;

#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#ifdef THINK_C
# include <stdlib.h>
# include <unix.h>
#include <Packages.h>
#include <Connections.h>
#endif THINK_C

#define SAME 	0
#define	FALSE	0
#define	TRUE	1

#define	TFILENAME	"tmpfile.%.7ld"
#define	SFILENAME	"SEQF"

#define SEPCHAR		'/'	/* path separater character		*/
#define DIRCHAR		':' /* Mac directory-separator character */
#define SIGFILE		".signature"	/* file containing signature	*/
#define COPYFILE	"mail.sent"	/* copy of sent mail	*/
#define NEWSDIR		"rnews.%02d%02d%02d.%02d%02d%02d.%03d"

#define HOSTINIT	macinit()	/* name of function used to initialize	*/

#define FILEMODE(f)

#define	CALLFILE	"C."
#define	XQTFILE		"X."
#define STATFILE	"ST."

#include "genv.h"
#define	 index	strchr
#define  rindex	strrchr
extern	char *fgets();
extern	char *index();
extern	char *rindex();

extern 	int	debuglevel;                  /* debugging level */
extern	time_t theTime;

typedef enum {Idle_System=1, Call_Systems, Cancel_Call, Quit_Program,
              Abort_Program, Shutdown_Program, Startup_Program};
extern  int Main_State;

typedef int datalist[61];

typedef struct {
	char min[40];
	char hour[40];
	char day[40];
	char month[40];
	char weekday[40];
	char name[40];
	int retrytime;	/* minutes between retries */
} schedule_record;
	
typedef struct {
	datalist min;
	datalist hour;
	datalist day;
	datalist month;
	datalist weekday;
} schedule_list_record;

#ifdef Upgrade
extern long int systimeout;
#endif Upgrade


#include "uupcproto.h"
#include "unixlibproto.h"
#include "mailproto.h"

