/* DCP a uucp clone. Copyright Richard H. Lamb 1985,1986,1987 */
#include <stdio.h>		/* Standard UNIX  definitions */

#include "host.h"		/* Host specific definitions */
#include "pcmail.h"		/* Differentiates pcmail from uupc */


/* FILE * FOPEN(); */
/* int CREAT(); */

#define SYSTEMS		"systems"
#define LOGFILE		"LOGFILE"
#define SYSLOG		"SYSLOG"
#define SCHEDULE	"schedule"

#define MSGTIME         20
#define MAXPACK         4096
#define MAXPACKCODE     8
#define DFLPACK			64

#define PORTBUFSIZ		5120
#define CMDBUFSIZE		4096

#define NFLDS			60

#define MAX_XFER_RETRIES 3

#define ERROR	100
#define EMPTY	101

#define	SLAVE	0
#define	MASTER	1

#ifndef TRUE
#define TRUE   (-1)
#define FALSE   0
#endif

/*#define SAME	0*/
#define RETRY	-2
#define FAILED	-1
#define OK		0
#define EQUAL	0


/* L.sys field defines */

#define	FLD_REMOTE	0
#define	FLD_CCTIME	1
#define	FLD_DEVICE	2
#define	FLD_TYPE	3
#define	FLD_SPEED	4
#define	FLD_PHONE	5
#define	FLD_PROTO	6
#define	FLD_EXPECT	7
#define FLD_SEND	8

/**/
typedef int	(*procref)();

typedef struct {
	char	type;
	procref a;
	procref b;
	procref c;
	procref d;
	procref e;
	procref f;
	procref g;
	procref h;
	procref i;
} Proto;

/* the various protocols available. Add here for others */
extern procref          getpkt, sendpkt, openpk, closepk, filepkt, eofpkt,
						getmsg, sendmsg, sendresp;

extern int	ggetpkt(), gsendpkt(), gopenpk(), gclosepk(), gfilepkt(), geofpkt(),
						gwrmsg(), grdmsg(), gsendresp();
extern int	fgetpkt(), fsendpkt(), fopenpk(), fclosepk(), ffilepkt(), feofpkt(),
						fwrmsg(), frdmsg(), fsendresp();
/*
extern int	kgetpkt(), ksendpkt(), kopenpk(), kclosepk();
extern int	rgetpkt(), rsendpkt(), ropenpk(), rclosepk();
extern int	tgetpkt(), tsendpkt(), topenpk(), tclosepk();
*/

typedef enum
                { phantom,          /* Entry not fully initialized      */
                  localhost,        /* This entry is for ourselves      */
                  routed,           /* This entry is actually a path    */
                  gatewayed,        /* This entry is delivered to via   */
                                    /* an external program on local sys */
                  aliasof,          /* This entry is alias of VIA system*/
                  nocall,           /* real host, never called          */
                  inprogress,       /* Call now active                  */
                  callback_req,     /* System must call us back         */
                  dial_failed,      /* Hardcoded auto-dial failed       */
                  script_failed,    /* script in L.SYS failed           */
                  max_retry,        /* Have given up calling this sys   */
                  too_soon,         /* In retry mode, too soon to call  */
                  succeeded,        /* self-explanatory                 */
                  wrong_host,       /* Call out failed, wrong system    */
                  unknown_host,     /* Call in cailed, unknown system   */
                  wrong_time,       /* Unable to call because of time   */
                  last_status }
                        hostatus;

struct HostStats {
      time_t ltime;              /* Last time this host was called      */
      time_t lconnect;           /* Last time we actually connected     */
      unsigned long calls;       /* Total number of calls to host       */
      unsigned long connect;     /* Total length of connections to host */
      unsigned long fsent;       /* Total files sent to this host       */
      unsigned long freceived;   /* Total files received from this host */
      unsigned long bsent;       /* Total bytes sent to this host       */
      unsigned long breceived;   /* Total bytes received from this host */
      unsigned long errors;      /* Total transmission errors noted     */
      unsigned long packets;     /* Total packets exchanged             */
      unsigned long bstart;      /* Bytes sent/rcvd at beginning of file*/
      hostatus save_hstatus;     /* host status, as defined by hostatus */
   };

typedef struct {
	time_t time_secs;
	char   name[40];
	char   status[40];
	int    failures;
} status_record;

typedef struct {
	int (*Init) (char * whichport, char * speed, char * phone);
	int (*InBuffer) (char * buf, int size);
	int (*OutBuffer) (char * buf, int size);
	int (*Speed) (char *speed);
	int (*Handshake) (int fInx, int fXOn, int fCTS, int xOn, int xOff);
	int (*Setting) (char *speed, int parity, int stopbits, int databits);
	int (*Close) (int dtr);
	int (*SetFlowCtl) (int software, int hardware);
	int (*AllowInterrupts) (int flag);
	int (*Interrupt) (void);
	int (*Purge) (void);
	int (*PutChar) (char ch);
	int (*Write) (char *buf, int count);
	int (*Avail) (void);
	int (*WStr) (char *st);
	int (*Read) (char *byt, int mincount, int maxcount, long int tenths);
	int (*Idle) (void);
	int (*Event) (EventRecord *anEvent);
	int (*Break) (int tenths);
	int (*SetParity) (int dataBits, int parityCode);
} ConnectionHandler;

/**/
extern int	pktsize;                /* packet size for this pro*/
extern FILE	*logfile;            	/* system log file */
extern FILE	*syslog;            	/* system log file */
extern FILE	*fw;           		/* cfile pointer */
extern char	cfile[80];              /* work file pointer */
extern int	remote;                 /* -1 means we're remote*/
extern int maxhops;
extern int sync_timeout;
extern int xfer_retries_left;
extern int PacketTimeout;
extern int MaxErr;
extern int msgtime;
extern int pktsize;         /* packet size for this protocol*/
extern int xfer_bufsize;
extern int	findwork;
extern int xfer_problems;
extern int	del_file_flag;
extern char	fromfile[132];
extern char	rmtfname[132];
extern char	hostfile[132];		/* host version of fromfile */
extern char	tofile[132];
extern char	state;                  /* present state */
extern int	fp;                     /* current disk file ptr */
extern int	size;                   /* nbytes in buff */
extern int  useHardwareFlowControl;
extern FILE	*fsys;
extern char	Rmtname[20];
extern char	rmtname[20];
extern char gotname[20];
extern char gotitfrom[BUFSIZ];
extern char gotfromuser[BUFSIZ];
extern char callcause[64];
extern char	*cctime;
extern char	proto[5];
extern time_t runStartSecs;
extern int		settingsFileRefNum;
extern int	failureCount;
extern ConnectionHandler *currentConnection;
extern long int connectionManagerGestalt;


extern char sysline[BUFSIZ*2];
extern char s_systems[64];
extern char s_logfile[64];
extern char s_syslog[64];
extern char s_schedule[64];
extern char s_statfile[64];
extern char *flds[NFLDS];
extern int  kflds;

extern int	debuglevel;		/* debugging flag */
extern unsigned int	checksum();
extern void zzz(int secs);
extern char *index();
extern char *rindex();

extern	char *curdir;

#ifdef Upgrade
extern long int systimeout;
#endif Upgrade

extern struct HostStats remote_stats;
extern status_record Last;

