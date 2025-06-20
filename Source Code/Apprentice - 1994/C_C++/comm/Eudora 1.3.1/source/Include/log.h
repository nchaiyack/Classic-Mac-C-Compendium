/* Copyright (c) 1992 by Qualcomm, Inc */
/**********************************************************************
 * the log file
 **********************************************************************/
UPtr ComposeLogR(short level,UPtr into,short format,...);
UPtr ComposeLogS(short level,UPtr into,UPtr format,...);
UPtr Log(short level,Uptr string);
void MyParamText(PStr p1,PStr p2,PStr p3,PStr p4);
void CloseLog(void);
void LogAlert(short template);
void CarefulLog(short level,short format,UPtr data,short dSize);
#ifdef DEBUG
#define LOGFLOW(x) do{ComposeLogS(LOG_FLOW,nil,"\p{%d:%d}:%d",FILE_NUM,__LINE__,x);}while(0)
#else
#define LOGFLOW(x) 
#endif
#define LOG_SEND	1
#define LOG_RETR	2
#define LOG_NAV		4
#define LOG_ALRT	8
#define LOG_PROG	16
#define LOG_TRANS	32
#define LOG_EVENT	64
#define LOG_MENU	128
#define LOG_FLOW	256
#define	LOG_ALL		-1