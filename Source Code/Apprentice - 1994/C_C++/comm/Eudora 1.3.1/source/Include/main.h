/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
void main(void);
Boolean DoDependentMenu(WindowPtr,int,int,short);
#define DieWithError(cmbid,num) DWE(cmbid,num,FILE_NUM,__LINE__)
void DWE(int,int,int,int);
#define WarnUser(cmbid,num) WU(cmbid,num,FILE_NUM,__LINE__)
void MonitorGrow(void);
void FlushHIQ(void);
int WU(int,int,int,int);
void DumpData(UPtr description, UPtr data,int length);
pascal long GrowZone(unsigned long needed);
void MakeGrow(long howMuch);
void Trace(UPtr message,...);
void EnableMenus(WindowPtr qdWin);
void EnableMenuItems(void);
#define FileSystemError(ctext,name,err)\
	FSE(ctext,name,err,FILE_NUM,__LINE__)
int FSE(int context, UPtr name, int err, int file,int line);
Boolean MiniMainLoop(EventRecord *event);
void CheckForMail(Boolean setTicks);
Boolean HandleControl(Point pt, MyWindowPtr win);
long CurrentSize(void);
long DefaultSize(void);
long EstimatePartitionSize(void);
MemoryWarning(void);
pascal void Hook(void);
