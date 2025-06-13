/*			sio.h


*/
#ifndef	THINK_C
# include	<serial.h>
#else	THINK_C
# include <serial.h>
#endif THINK_C

#define AIN		"\P.AIN"
#define AOUT	"\P.AOUT"
#define BIN		"\P.BIN"
#define BOUT	"\P.BOUT"
#define	XON		'\021'
#define	XOFF	'\023'

#define DOASYNC

#define BUFFERS	20
#define BUFSIZE	256

struct async_write {
	ParamBlockRec	pb;
	long int        bufSize;
	char			*buf;
};

typedef struct SIOPort {
	int refin, refout;
	unsigned short baud, parity, stopbits, databits;
	SerShk	handshake;
	char * out, * in;
	int insize, outsize;
	char * inbuffer, * outbuffer;
} SIOPort;


extern struct SIOPort PortA, PortB, *Port;

extern int SIOInit(char *whichport, char *speed, char *phone);
extern int SIOSpeed(char *speed);
extern int SIOHandshake(int fInx, int fXOn, int fCTS, int xOn, int xOff);
extern int SIOSetting(char *speed, int parity, int stopbits, int databits);
extern int SIOInBuffer(char *buf, int size);
extern int SIOOutBuffer(char *buf, int size);
extern int SIOClose(int dtr);
extern int sfflushout(void);
extern int SIOSetFlowCtl(int software, int hardware);
extern int SIOAllowInterrupts(int flag);
extern int SIOInterrupt(void);
extern int SIOPurge(void);
extern int SIOPutchar(char ch);
extern int SIOWrite(char *buf, int count);
extern int SIOAvail(void);
extern int SIOWStr(char *st);
extern int SIORead(char *byt, int mincount, int maxcount, long int tenths);
extern int srdchk(void);
extern int ssendbrk(int bnulls);
extern int getn(char *cp, int cmin, int cmax, short refin);
extern int SIOWriteBusy(void);
extern int putu(char *c, short count, short refout);
