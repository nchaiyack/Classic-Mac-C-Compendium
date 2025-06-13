
/* dcpgpkt.c */
int aopenpk(int master);
int aclosepk(void);
int agetpkt(char *data, int *len);
int asendpkt(char *data, int len);
int afilepkt(void);
int aeofpkt(void);
int asendresp(int state);
int awrmsg(char *str, boolean nowait);
int ardmsg(register char *str, int *bytes);
