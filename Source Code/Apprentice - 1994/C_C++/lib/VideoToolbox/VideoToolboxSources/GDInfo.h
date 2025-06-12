/*
GDInfo.h

This structure is used by GDInfo.c, GDTestClut.c, and TimeVideo.c. The intent is
to allow the test routines to accumulate all the test information in one
convenient VideoInfo record that TimeVideo can then intelligently summarize in
a useful report. All the information in the structures is stable, except the
depth index "d". Call GDInfo() to update d after each call to SetDepth. 

*/

typedef struct {
	// filled in by GDTestClut.c
	struct{
		short doTest;
		short tested;
		short errors;				// error exceeded tolerance
		short errorsAtOnce;			// ", when we loaded whole clut at once
		double rgbGain[3][3];		// linear model for clut write-read transformation 
		double rgbError[3];			// rms error of that linear model
		double rgbErrorAtOnce[3];	// ", when we loaded whole clut at once
		double tolerance;			// criterion used for calling a clut bad
		Boolean identity;			// gains correspond to identity transform, i.e. color mode
	}read;
	// filled in by GDTestClutHash in GDTestClut.c
	struct{
		short doTest;
		short tested;
		short errors;
	}hash;						// visible artifacts during clut loading
	// filled in by GDTestClutVisually in GDTestClut.c
	struct{
		short doTest;
		short tested;
		short errors;
		short errorsAtOnce;
	}visual;					// vs standard flags==0 cond.
}VideoCardClutTest;

typedef struct {
	GDHandle device;
	Boolean basicTested,timeTested,clutTested;	// What's been tested.
	// basicTest: these are filled in by GDInfo.c
	Boolean setEntriesQuickly,gdGetEntries;		// compatibility
	char cardName[100],driverName[100];
	short slot,width,height,dacSize,dacMask;
	struct{
		short pixelSize;						// zero if mode is invalid
		short mode,clutSize,pages;
		// timeTest: these are filled in by GDInfoTime in GDInfo.c
		short timeTested;
		double frameRate,vblPerFrame;
		double movieRate,movieRateQuickly;
		double framesPerClutUpdate,framesPerClutUpdateHighPriority
			,framesPerClutUpdateQuickly;
		double missingFramesPerClutUpdate,missingFramesPerClutUpdateHighPriority
			,missingFramesPerClutUpdateQuickly;
		// These are filled in by GDTestClut.c
		VideoCardClutTest clut[2][2];			// [quickly][isGray]
	} depth[6];									// Indexed by d.
	short d;									// current depth index
} VideoInfo;

// GDInfo.c
OSErr GDInfo(VideoInfo *card);
OSErr GDInfoTime(VideoInfo *card);

// GDTestClut.c
OSErr GDTestClut(FILE *o[2],short flags,VideoInfo *card);
OSErr GDTestClutVisually(short flags,VideoInfo *card);
OSErr SetEntriesQuicklyVsGDSetEntries(VideoInfo *card);
OSErr GDTestClutHash(short flags,VideoInfo *card);

enum{testClutQuicklyFlag=1,testClutSeriallyFlag=2
	,testClutNegativeFlag=4,testClutLinearFlag=8
	,testClutVisually=16,testClutGains=32};
