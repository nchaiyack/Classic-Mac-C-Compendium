
/* mail.c */
int mailmain(int argc, char **argv);
int finis(void);
int sendmail(int n, char *line);
int linvert(int i);
int showmail(int argc, char **argv);
int readaline(long adr, char *line);
int printsub(int K);
int copyback(void);
int copymsg(int n, FILE *f, int noheaders);
int pager(int n);
int pagereset(void);
int pageline(char *s);
char *getnext(char **s, char *p);
int Check_Events(long Sleep_Time);
