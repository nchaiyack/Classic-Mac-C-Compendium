
/* dcpgpkt.c */
int gopenpk(int master);
int gclosepk(void);
int ggetpkt(char *data, int *len);
int gsendpkt(char *data, int len);
int gfilepkt(void);
int geofpkt(void);
int gsendresp(int state);
int gwrmsg(char *str, boolean nowait);
int grdmsg(register char *str, int *bytes);
