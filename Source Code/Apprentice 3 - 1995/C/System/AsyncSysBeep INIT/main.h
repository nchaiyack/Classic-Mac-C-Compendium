#ifndef	_MAIN_
#define	_MAIN_





#define	GetTrapType(x)		(((x & 0x0800) > 0) ? ToolTrap : OSTrap)
#define	sdVolume			0x0260
#define	WWExist				0x08F2
#define	ExpandMem			0x02B6





void main(void);
pascal void MySysBeep(short duration);
void DoSysBeep(short beepRsrcID);
void LoadNewSysBeep(short beepRsrcID);
void JunkOldSysBeep(void);
Byte GetResLoad(void) = {0x1038,0x0A5E};
pascal void SndPlayCompletion(SndChannelPtr chan,SndCommand cmd);
pascal void MySystemTask(void);


#endif