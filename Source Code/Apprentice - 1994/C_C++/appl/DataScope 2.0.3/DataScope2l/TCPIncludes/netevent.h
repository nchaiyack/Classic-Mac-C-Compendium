#define USERCLASS	1
#define ICMPCLASS	2
#define ERRCLASS    4
#define SCLASS		8
#define CONCLASS    0x10

#define ERR1	1		/* an error message is waiting, ERRCLASS */

#define IREDIR	1		/* ICMP redirect, ICMPCLASS */

#define CONOPEN 1		/* connection has opened, CONCLASS */
#define CONDATA 2       /* there is data available on this connection */
#define CONCLOSE 3		/* the other side has closed its side of the connection */
#define CONFAIL 4		/* connection open attempt has failed */

#define UDPDATA 1		/* UDP data has arrived on listening port, USERCLASS */
#define DOMOK	2		/* domain name ready */
#define DOMFAIL 3		/* domain name lookup failed */
#define FTPCOPEN 20     /* FTP command connection has opened */
#define FTPCLOSE 21     /* FTP command connection has closed */
#define FTPBEGIN 22     /* FTP transfer beginning, dat =1 for get, 0 for put */
#define FTPEND   23     /* FTP transfer ending */
#define FTPLIST  24     /* FTP file listing taking place */
#define FTPUSER  25     /* FTP user name has been entered */
#define FTPPWOK	26		/* FTP password verified */
#define FTPPWNO 27		/* FTP password failed */
#define RCPBEGIN 30		/* RCP beginning */
#define RCPEND 31		/* RCP ending */

#define UDPTO 1			/* UDP request from DOMAIN timed out, SCLASS */
#define FTPACT 2		/* FTP transfer is active, keep sending */
#define TCPTO  3		/* TCP for DOMAIN timed out */
#define RCPACT 4		/* rcp is active, needs CPU time */
#define RETRYCON 5		/* retry connection packet, might be lost */
#define CLOSEDONE 6		/* Close completion routine has been called (for Mac Drivers only) */