/* GNUUCP prototypes */

/* gio.c */

int writeframe(int, unsigned char *msgo);
int ackmsg(void);
int ctlmsg(char);
int sendpacket(char *, int, int);
int inpkt(void);
int chksum(unsigned char *, int n);
int indata(void);
int gturnon(int);
int gturnoff(void);
int grdmsg(char *, int);
int gwrmsg(char, char *, int);
int gwrdata(FILE *, int);
int grddata(int, FILE *);
extern int segsiz;

/* GNUUCP.c */

int getstring(char *);
int instr(char *, int, int);
int twrite(char *, int);
int main (int, char **);
int debuggit(void);
int sendexpect(char *, char *);
int xlat_str(char *);
struct port *findport(char *, struct port *);
struct port *pickport(char *, long, struct port *);
int dial_nbr(struct port *, char *);
int call_system(char *);
char *get_sysline(char *, int, FILE *);
int call_sysline(char *);
int do_session(int, char *);
int top_level(int);
int do_one_slave(void);
int do_one_master(void);
int yesno(char, int, int);
int host_send_file(char *);
int host_receive_file(char *);
int local_send_file(char *);
int local_receive_file(void);
int receive_file(FILE *, char *, char *, char *);
int send_file(FILE *);

/* locking.c */

int ttylock(char *);
int ttyunlock(void);

/* sysdep.c */

extern long total_data_processed;
int openline(char *, int);
int openout(struct port *);
int openin(struct port *);
void sigint(int);
int xwrite(/* int, */ char *, int);
int xgetc_fill_buff(void);
int xgetc(void);
void send_break (void);
int hangup(struct port *);
char *temp_filename(char *);
char *munge_filename(char *);
char *unmunge_filename(char *);
void work_done(void);
char *index(char *, char);
int work_scan(char *, char *);
int work_look(void);
char *work_next(void);
char *time_and_pid(void);
int chdir(char *);
/* int execlp(char *, char *, int); */
struct DIR *opendir(char *);
int closedir(struct DIR *);
struct dirent *readdir(struct DIR *);
/* char *strtok(char *, char *); */
int strncmpic (char *, char *, int);
void bzero (char *, unsigned);
void bcopy(char *, char *, int);
/* int system(char *); */
int access(char *, int);
int random(void);
int mindex(char *, char);
void gnuucp_cleanup(void);
void HandleEvents (void);
char *currtime(void);
void gnusleep(unsigned);
int parse(char *, char *, char **argv[]);

/* uumisc.c */

int read_params(char *);
char *gimmestring(void);
char *gimmefilestring(void);
int getargs(char *, char **, int);
int logit(char *, char *);
int mlogit(char *, char *);
void cuserid(char *);

/* gnuuxqt.c */
int uuxqt(int);
int invoke (char *, char *, char *, char *);

