
/* dcpfpkt.c */
int fopenpk(int master);
int fclosepk(void);
int fwrmsg(char *str, boolean nowait);
int frdmsg(register char *str, int *bytes);
int fgetpkt(char *packet, int *bytes);
int fsendpkt(char *ip, int len, int flag);
int ffilepkt(void);
int feofpkt(void);
int fsendresp(int state);
