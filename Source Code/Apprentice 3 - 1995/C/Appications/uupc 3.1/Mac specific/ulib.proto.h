
/* ulib.c */
int login(int waitforever, int callup_done);
int swrite(char *data, int num);
void makenull(int refno);
int sread(char *buf, int num, long int timeout);
int savail(void);
int openline(char *name, char *baud, char *phone);
int closeline(void);
int flowcontrol(int software, int hardware);
int nodot(int string);
int notimp(int argc, char *argv[]);
int shell(char *command, char *inname, char *outname);
int macbin(int argc, char *argv[]);
