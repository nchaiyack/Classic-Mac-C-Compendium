void	ChangeQueueLength(short delta);
void Stask(void);

short	netputuev
  (
	short	class,
	short	what,
	short	data1,
	long	data2
  );

short	netputevent
  (
	short	class,
	short	what,
	short	data1,
	long	data2
  );

short	netgetevent(
	short	*class,
	short	*data1,
	long	*data2
  );

void DoNetEvents(void);
short setblocksize(short tosize);
short	WindByPort(short port);
void pasteText(short scrn);
void FlushNetwork(short scrn);
