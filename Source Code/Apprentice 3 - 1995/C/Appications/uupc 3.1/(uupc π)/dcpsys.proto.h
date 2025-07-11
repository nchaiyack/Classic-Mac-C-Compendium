
/* dcpsys.c */
void hangup(void);
void zzz(int nsecs);
int getsystem(char *rmthost);
int checkname(char name[]);
int Time_Less_Than_Or_Equal(struct tm *Left, struct tm *Right);
void Schedule_To_List(char *Schedule, datalist Schedule_List, int Min, int Max);
int Validate_Value(int Value, datalist Valid_List);
int Add_Field(int Last, int *Next, datalist Valid_List, int *Carry);
int Day_Of_Week(int Month, int Day, int Year);
/* void Compute_Next_Call_Time(status_record Last, schedule_record Schedule, struct tm *Next); */
int GetStatus(void);
int checkcron(void);
void Update_Status(int New_Status);
int checktime(char xtime[]);
int paceit(void);
int sysend(void);
int wmsg(char msg[], int syn);
int rmsg(char msg[], int syn);
void cleanmsg(char *msg);
int startup(void);
int setproto(char pr);
int prefix(char *sh, char *lg);
int notin(char *sh, char *lg);
int expectstr(char *str, long int timeout);
int writestr(char *s);
void sendstr(char *str);
int sendexpect(char *s, char *e, long int timeout);
int dial(void);
int callup(void);
int scandir(void);
int dscandir(void);
