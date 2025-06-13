#ifndef	_MAIN_
#define	_MAIN_





void main(void);
pascal short AECoreHandler(AppleEvent *apple,AppleEvent *reply,long refCon);
short AEGotRequiredParams(AppleEvent *aevent);
void ProcessFile(FSSpec *file);


#endif