/* rmail prototypes */

int rmail(int, char **);
int do_rmail_forever(void);
int do_rmail_queue(void);
int do_rmail_interactive(int, char **);
int local_internet_address (char *);
char *strip_local_host (char *);
int known_host (char *);
int do_one_rmail(char *, char *, FILE *);
int validate_site(char *, int);
int get_seq(char *, char *);
int deliver(char *, char *);
int aliasp (char *);
int deliver_alias(char *, char *);
int forward(char *,char *, char *, char *, int);
int mail(char *, char *, char *);
int do_rmail(char *, char *, char *);
int string_contains ( char *, char *);


