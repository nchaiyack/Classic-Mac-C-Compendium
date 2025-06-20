#include "mandel.h"


ABRecHandle		receiveRecord;
char			myBuffer[myBufSize];
int				errCode,myNode,myNet,errno;
unsigned		mySocket;
int				numSlaves;
char			myName[nameSize];
EntityName		myEntity;
char			nbpRegistered;
int				slaveNet[maxSlaves],slaveNode[maxSlaves],slaveSocket[maxSlaves];
char			slaveStatus[maxSlaves];
slaveMessage	theWorkOrder;
MenuHandle		myMenus[6];
WindowRecord	wRecord;
WindowPtr		theWPtr;
char			alive,slaveMode,shading,coords,meToo;
slaveMessage	theMessage;
char			slaveSig[] = "MandelSlave";
char			masterSig[] = "MandelMaster";
Ptr				nbpName;
char			theDisplayMessages[3][256];
Cursor			waitCursor,xCursor;
Rect			graphRect,infoRect,messageRect,nullClipRect,coordRect;
double			lims[4];
int				maxIterations,pixelSize;
double			timeDelay;
BitMap			graphBits;
Point			oldPoint;