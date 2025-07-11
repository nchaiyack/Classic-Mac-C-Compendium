/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/**********************************************************************
 * functions of which I am not proud
 **********************************************************************/
#define DoAnAlert(level,string) AlertStr(OK_ALRT,level,string)
#define DoATimedAlert(level,string) 																		\
	AlertTicks=GetRLong(ALERT_TIMEOUT)*60+TickCount(),										\
	M_T1 = DoAnAlert(level,string), 																			\
	AlertTicks = 0, 																											\
	M_T2=M_T1
#define DoABigAlert(level,string) AlertStr(BIG_OK_ALRT,level,string)
#define DoNumAlert(level,errNum) \
	do {Str255 _s_;DoAnAlert(level,GetRString(_s_,errNum));}while(0)
#define DoNumBigAlert(level,errNum) \
	do {Str255 _s_;DoABigAlert(level,GetRString(_s_,errNum));}while(0)
int ReallyDoAnAlert(int template,int which);
int AlertStr(int template, int which, UPtr str);
Boolean IsSwitch(EventRecord *event);
Boolean Switch(void);
void SetAlertBeep(Boolean onOrOff);
Boolean MommyMommy(short sId,UPtr string);
int Aprintf(short template,short which,short rFormat,...);
#ifdef DEBUG
Handle NuHandle(long size);
UPtr NuPtr(long size);
void SetHandleBig(Handle h,long size);
#else
#define NuHandle NewHandle
#define NuPtr NewPtr
#define SetHandleBig SetHandleSize
#endif
