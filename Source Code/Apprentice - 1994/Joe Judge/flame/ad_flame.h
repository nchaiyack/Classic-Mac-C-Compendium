


void initflame( Rect *bounds, char depth);
static short halfrandom(int mv);
void drawflame(void);
void XClearWindow( Rect *theRect);
void XDrawPoints(Point	points[], short how_many);

// defines ...
#define MAXSCREENS	3						// 3 screens max (for now) - joe

// this is now a slider
//#define MAXTOTAL	10000

#define MAXBATCH	1000						// was 10 ... added control over this
#define MAXLEV		4						// was 4

// from the Xlock.ad file ... (made into defines)
//XLock.flame.delay: 10000
//XLock.flame.batchcount: 20
//XLock.flame.saturation: 1
#define BATCHCOUNT	20	

