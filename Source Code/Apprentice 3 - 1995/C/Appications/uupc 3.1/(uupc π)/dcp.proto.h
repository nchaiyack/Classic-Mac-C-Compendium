
/* dcp.c */
int complain(char *s);
int cant(char *file);
int dcpmain(int argc, char **argv);
int master(void);
int slave(void);
int receive(void);
int send(void);
int dcxqt(void);
int printmsg(int level, char *fmt, ...);
