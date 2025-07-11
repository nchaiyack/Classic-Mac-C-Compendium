#include	<stdio.h>
#include	<strings.h>
#include	<Appletalk.h>


#define	maxSlaves		64
#define	myBufSize		106*maxSlaves
#define	returnKey		(char) 0x0d
#define	nbpNameBufSize	32+32+32+12
#define	nameSize		32
#define	rtInterval		8
#define	rtCount			3
#define	mandelType		(unsigned) 15

#define helpTEXT		128
#define	helpSTYL		128


#define	infoDLOG		128
#define	inputDLOG		129
#define	textDLOG		130
#define	limsDLOG		131
#define	errorDLOG		134

#define	mainWIND		128
#define	helpWIND		129

#define appleMenu		0
#define	fileMenu		1
#define	editMenu		2
#define modeMenu		3
#define	graphMenu		4
#define	controlMenu		5

#define	appleID			128
#define	fileID			129
#define	editID			130
#define modeID			131
#define	graphID			132
#define	controlID		133

#define aboutMe			1
#define	helpCommand		2

#define undoCommand 	1
#define cutCommand		3
#define copyCommand		4
#define pasteCommand	5
#define clearCommand	6

#define	slaveCommand		1
#define	masterCommand		2

#define	setLimsCommand		1
#define	setIterCommand		2
#define	setPixSizeCommand	3
#define	shadingCommand		4
#define	defaultLimsCommand	6
#define	clearGraphCommand	7
#define	mandelCommand		8

#define	checkSlavesCommand	1
#define	stopSlavesCommand	2
#define	slaveStatusCommand	3
#define	setDelayCommand		5
#define	meTooCommand		6



#define	workOrder		0
#define	stopOrder		1
#define	confirmOrder	2
#define	bitmapRequest	3
#define	bitmapResponse	4
#define	busyResponse	5
#define	queryStatus		6
#define	idleResponse	7


typedef	struct	{	double	lims[4];
					int		firstRow,lastRow,maxIterations,pixelSize;
					char	shading;
				} slaveMessage;

#define xmin 0
#define xmax 1
#define ymin 2
#define ymax 3



extern	ABRecHandle		receiveRecord;
extern	char			myBuffer[myBufSize];
extern	int				errCode,myNode,myNet,errno;
extern	unsigned		mySocket;
extern	int				numSlaves;
extern	char			myName[nameSize];
extern	EntityName		myEntity;
extern	char			nbpRegistered;
extern	int				slaveNet[maxSlaves],slaveNode[maxSlaves],slaveSocket[maxSlaves];
extern	char			slaveStatus[maxSlaves];
extern	slaveMessage	theWorkOrder;
extern	MenuHandle		myMenus[6];
extern	WindowRecord	wRecord;
extern	WindowPtr		theWPtr;
extern	char			alive,slaveMode,shading,coords,meToo;
extern	slaveMessage	theMessage;
extern	char			slaveSig[];
extern	char			masterSig[];
extern	Ptr				nbpName;
extern	char			theDisplayMessages[3][256];
extern	Cursor			waitCursor,xCursor;
extern	Rect			graphRect,infoRect,messageRect,nullClipRect,coordRect;
extern	double			lims[4];
extern	int				maxIterations,pixelSize;
extern	double			timeDelay;
extern	BitMap			graphBits;
extern	Point			oldPoint;



double XToD();
double YToD();