#ifndef	_STATUS_
#define	_STATUS_





typedef struct StatusInfo
{
	short	cancel;
	short	remaining;
	long	start;
	long	total;
	long	lastKPerSec;
	
	long	current;
	char	action[32];
	char	file[64];
} StatusInfo, *StatusInfoPtr;


void InitStatusWindow(void);
WindowPtr NewStatusWindow(char *title,StatusInfoPtr *info);
void DisposeStatusWindow(WindowPtr window);
void DoStatusClick(WindowPtr window,StatusInfoPtr info,Point where);
void DoStatusKey(WindowPtr window,StatusInfoPtr info,char key,long modifiers);
void DoStatusHit(WindowPtr window,StatusInfoPtr info,short item);
void DoStatusUpdate(WindowPtr window,StatusInfoPtr info);
void UpdateStatusItem(WindowPtr window,short item);
pascal void DrawRemaingItemsItem(WindowPtr window,short item);
pascal void DrawActionItem(WindowPtr window,short item);
pascal void DrawKPerSecondItem(WindowPtr window,short item);
pascal void DrawStatusBarItem(WindowPtr window,short item);
void DrawStringClipped(char *s,Rect *box);
StringPtr ConvertCtoPStr(StringPtr pstr,char *cstr);


#endif	_STATUS_