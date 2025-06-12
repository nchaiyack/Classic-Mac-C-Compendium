
typedef EventRecord* EventPtr;

void	TestCommandPeriod(void);

Boolean	AbortInQueue(void);
Boolean	CmdPeriod( EventPtr theEvent );
Boolean	IsFrontProcess(void);
Boolean	IsRunningUnderAUX(void);
Boolean	HasGestaltAttr(OSType itsAttr, short itsBit);
