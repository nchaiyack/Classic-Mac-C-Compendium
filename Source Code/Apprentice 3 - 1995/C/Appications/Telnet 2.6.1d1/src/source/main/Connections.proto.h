
/* Connections.c */
void OpenPortSpecial(MenuHandle menuh, short item);
void PresentOpenConnectionDialog(void);
Boolean CreateConnectionFromParams(ConnInitParams **Params);
void CompleteConnectionOpening(short dat, ip_addr the_IP, OSErr DNRerror, char *cname);
void destroyport(short wind);
void removeport(short n);
void setSessStates(DialogPtr dptr);
