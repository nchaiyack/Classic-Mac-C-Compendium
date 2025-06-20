
/* sio.c */
int SIOInit(char *whichport, char *speed, char *phone);
int SIOSpeed(char *speed);
int SIOHandshake(int fInx, int fXOn, int fCTS, int xOn, int xOff);
int SIOSetting(char *speed, int parity, int stopbits, int databits);
int SIOInBuffer(char *buf, int size);
int SIOOutBuffer(char *buf, int size);
int SIOClose(int dtr);
int sfflushout(void);
int SIOSetFlowCtl(int software, int hardware);
int SIOAllowInterrupts(int flag);
int SIOInterrupt(void);
int SIOBreak(int tenths);
int SIOSetParity(int dataBitsCode, int parityCode);
int SIOPurge(void);
int SIOPutchar(char ch);
int SIOWrite(char *buf, int count);
int SIOAvail(void);
int SIOWStr(char *st);
int SIORead(char *byt, int mincount, int maxcount, long int tenths);
int srdchk(void);
int ssendbrk(int bnulls);
int getn(char *cp, int cmin, int cmax, short refin);
int SIOWriteBusy(void);
int SIOIdle(void);
int SIOEvent(EventRecord *anEvent);
int putu(char *c, short count, short refout);
