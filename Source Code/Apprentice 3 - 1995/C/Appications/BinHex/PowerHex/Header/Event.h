#ifndef	_EVENT_
#define	_EVENT_





typedef short (*WorkProc)(void *param);
typedef void (*KillProc)(void *param);
typedef void (*ClickProc)(WindowPtr window,void *param,Point where);
typedef void (*KeyProc)(WindowPtr window,void *param,char key,long modifiers);
typedef void (*HitProc)(WindowPtr window,void *param,short item);
typedef void (*UpdateProc)(WindowPtr window,void *param);


typedef struct WorkerInfo
{
	struct WorkerInfo	*next;
	void				*param;
	WorkProc			proc;
	KillProc			kill;
} WorkerInfo, *WorkerInfoPtr;

typedef struct WindowProcs
{
	ClickProc	click;
	KeyProc		key;
	HitProc		hit;
	UpdateProc	update;
	void		*param;
} WindowProcs, *WindowProcsPtr;


void InitEvents(void);
short doEventLoop(long sleepTime);
void doMouseDown(Point where);
void doKeyDown(long message,short modifiers);
void doWork(void);
void NewWorker(WorkProc worker,KillProc kill,void *param);
void KillWorkers(void);


#endif	_EVENT_