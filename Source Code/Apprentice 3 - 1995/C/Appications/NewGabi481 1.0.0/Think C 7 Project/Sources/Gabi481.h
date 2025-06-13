//	Includes
//	========

// Sind alle im precompiled Header "DruckenIncludes!!
// Beide werden im Sourcefile included!


//	Macros
//	======

//#define		TEST
#define		RL				0								//Din A4 Rect
#define		RT				0								//...
#define		RR				530								//...
#define		RB				780								//Din A4 Rect
#define		QRL				0								//Din A4 Rect QUER
#define		QRT				0								//...
#define		QRR				780								//...
#define		QRB				530								//Din A4 Rect QUER
#define		G1RL			115								//Graph1: Oben X
#define		G1RT			73								//Graph1: Oben Y
#define		G1X				351								//Graph1: Distanz X
#define		G1Y				301								//Graph1: Distanz Y
#define		G1RR			(G1RL + G1X)					//Graph1: Unten X
#define		G1RB			(G1RT + G1Y)					//Graph1: Unten Y
#define		G12Y			70								//Y-Abstand der Graphen
#define		G2RL			G1RL							//Graph2: Oben X
#define		G2RT			(G1RB + G12Y)					//Graph2: Oben Y
#define		G2X				G1X								//Graph2: Distanz X
#define		G2Y				G1Y								//Graph2: Distanz Y
#define		G2RR			(G2RL + G2X)					//Graph2: Unten X
#define		G2RB			(G2RT + G2Y)					//Graph2: Unten Y
#define		G1XSUC			5								//Skalen Abstand G1/X
#define		G2XSUC			5								//Skalen Abstand G2/X
#define		G1YSUC			10								//Skalen Abstand G1/Y
#define		G2YSUC			10								//Skalen Abstand G2/Y
#define		G1XSU			(short)((G1X-1) / G1XSUC)		//Skalen Abstand G1/X
#define		G2XSU			(short)((G2X-1) / G2XSUC)		//Skalen Abstand G2/X
#define		G1YSU			(short)((G1Y-1) / G1YSUC)		//Skalen Abstand G1/Y
#define		G2YSU			(short)((G2Y-1) / G2YSUC)		//Skalen Abstand G2/Y
#define		GSL				3								//Teilungsstrichlänge
#define 	MaxValY1		1								// = 100 Prozent
#define 	MaxValY2		1								//Zahl Zwei
#define 	MaxValX1		300								//Sekunden bei 5 min
#define 	MaxValX2		300								//Sekunden bei 5 min
#define		CircRad			1								//Radius der Kreise des Graphen
#define 	MaxStrLen		500
#define		GreetStrX		(G1RL - 20)
#define		GreetStrY		(G1RT - 40)
#define		RelStrX			G1RL
#define		RelStrY			(G1RT - 15)
#define		AStrX			G2RL
#define		AStrY			(G2RT - 15)
#define		YASSOX			20
#define		YASSOY			5
#define		XASSOX			1
#define		XASSOY			15

#define 	rNoODocAlert	128
#define 	rMemError		129
#define 	rInError		130
#define 	rFileNameError	131		
#define 	rMiscError		132		
#define 	dOKButton		1


//	Globals
//	=======

Boolean 	gDone=FALSE;
char		*MemStr=NULL,
			IOStr[MaxStrLen],
			GreetStr[]="Output Program NEWGABI481 for file ",
			RelStr[]="Relative Plot",
			AStr[]="Atom Enrichment in 48/49",
			M45Str[]="45",
			M47Str[]="47",
			M49Str[]="49";
short 		fRefNum=0;
long 		n=0L,
			OutFType='PICT',
		 	//OutFCreat='SABI',
			OutFCreat='ttxt',
			curEOF=0L,
			FSSListLength=0L;
double		t=0,
			ts=0,
			A1=0, 
			A2=0,
			A3=0,
			A4=0,
			*fivex=NULL,
			*sevex=NULL,
			*ninex=NULL,									//Eingelesene x-Werte der 3 Kurven in mm
			*fivey=NULL,
			*sevey=NULL,
			*niney=NULL,									//Eingelesene y-Werte der 3 Kurven in mm
			*ergqy=NULL,									//Summe der eingelesenen y-Werte in mm
			*fivery=NULL,			
			*severy=NULL,
			*ninery=NULL,									//Relative y-Werte, bezogen auf Summe für Graph 1
			*ergx=NULL,										//gemittelte x-Werte in Sekunden
			*ergy=NULL;										//auf Gesamtes bezogene und logarithmierte y-Werte in Einheiten
Point		*Dlog=NULL,
			*Dfive=NULL,
			*Dseve=NULL,
			*Dnine=NULL;									//Malkoordinaten in Pixel
Rect		PageRect={RT, RL, RB, RR},						//Beim Malen prüfen ob in PageRect!
			ArcRect={0, 0, 0, 0},
			G1Rect={G1RT, G1RL, G1RB, G1RR},
			G2Rect={G2RT, G2RL, G2RB, G2RR};
			// Rects werden {T, L, B, R} definiert!
			// SetRect frißt aber {L, T, R, B}
			//PageRect={RL, RT, RR, RB},
			//G1Rect={G1RL, G1RT, G1RR, G1RB},
			//G2Rect={G2RL, G2RT, G2RR, G2RB};
RGBColor	RGBRed={65517, 6521, 65535},
			RGBGreen={28221, 65535, 7238},
			RGBBlue={1257, 63034, 65535};			
GrafPtr		SavePort=NULL;
WindowPtr	Port=NULL;
PicHandle	myPicture=NULL;
FSSpec		InFSS,
			OutFSS, 
			*FSSList=NULL,
			*FSSListPtr=NULL;


//	Structs
//	=======



//	Protos
//	======

void	Debug(Str255);
void	SetPrinterLineWidth(short, short);
short	RoundFiltConv(short, double);
OSErr	Read_In(void);
OSErr	Compute_Out(void);
OSErr	Create_Out(FSSpec);
OSErr	Create_Out_Port(void);
OSErr	Draw_Axes(void);
void	Draw_Strings(void);
OSErr 	Draw_Chart(void);
OSErr	Delete_Out_Port(void);
OSErr	Write_Out_Pict(void);
OSErr	Reinit(void);
void	ErrorHandler(short);

void	InitToolbox(void);
Boolean AppleEventsInstalled (void);
pascal OSErr  MyHandleODoc (AppleEvent *theAppleEvent, AppleEvent* reply, long handlerRefCon);
pascal OSErr  MyHandlePDoc (AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefCon);
pascal OSErr  MyHandleOApp (AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefCon);
OSErr MyGotRequiredParams (AppleEvent *theAppleEvent);


