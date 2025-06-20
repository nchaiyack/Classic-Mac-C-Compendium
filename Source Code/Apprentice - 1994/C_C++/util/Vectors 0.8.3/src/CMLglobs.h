/*	2a. variables */

extern char *s;
extern short LineOrder;
extern FILE *Tf,*TokFile;
extern FILE *OutFile;		/* may be defined to be NULL by MDL.h */
extern FILE *ErrFile;
extern Handle CurrentRecon;
extern long NChars;
extern char *InBuf,Scratch[],Scratch2[];
extern struct tm *NewTime;
extern long LineCnt;
extern long ContinuationLines;
extern short ListBeginSize;
extern unsigned short State;
extern long NrTempWrites;
extern long NrTempReads;
extern long CurrentAge;
extern short TheOpenField;
extern Desig TheOpenDesig ;
extern short BackgroundField;
extern short ClosureSteps;
extern short MaxErrors;

extern c_float DtoR ;
extern c_float RtoD ;
extern c_float MtoFt;	/* The American standard: 3937/1200 */
extern c_float ClosureTolerence;
extern c_float AngleError;
extern c_float TotalDist;
extern c_float TotalHDist;
extern c_float PSScale;					/* 1 inch is 1000 feet */
extern short DaysToMonth[] ;

#ifdef DOS
extern char *Month[] ;
extern char *WeekDay[] ;
#endif

extern struct parseRec pd[];
extern struct locnRec TempLocn;
extern struct dimData TempDims;

/*	2b. parameters (part of SnapShot) */
extern Str31 *IFileList;
extern short FileLines[];
extern short FileListPosn;
extern short NFiles;
extern long MinField,MaxField;

extern unsigned short *FieldList;		/* not an sList, as it is not paged */
extern short *FixArray;
extern long *NameHashArray;
extern short *FieldsToIgnore;
extern short *LogList;
extern indexType SegmentWithFirstLocn;
extern short FieldInitialURS;
extern short ResolvedSegments;
extern short NumStrJct;
extern short NumLoopStr;
extern long Literals;
extern short Sep;			/* Exchange Format separator */
extern short RotateDimensions;	/* 2 if #cflr */
extern c_float InclCor;
extern c_float CompCor;
extern c_float Convergence;
extern c_float EOffset;
extern c_float NOffset;
extern c_float AOffset;
extern c_float SWorstAngle;
extern c_float Heavy;
extern c_float Light;
extern c_float SError;
extern c_float DVertical;
extern long Length;
extern struct stationRec gTopMost,gBottomMost,gWestMost,gEastMost,gNorthMost,gSouthMost;
extern struct cartData UNE,LSW;
extern struct masterRec *MastersArray;

/* Flags */
extern short SEFlag;
extern short MeterFlag;
extern short ProcessHeaders;
extern short NoFSBFlag;
extern short DoP2 ;
extern short Super3D ;
extern short GraphHow ;
extern short WarningFlag;
extern short VdFlag;
extern short ShotMode;
extern short ShotFlag;			/* usually Line_to_rel */
extern short ListingFlag;		/* flag for listing */
extern short FullListing;
extern short ExchangeFormat;
extern short ParseAll;
extern short EEFlag;
extern short DumpDBFlag;
extern short CloseFlag;
extern short OutputFlag;
extern short LogFlag;	/* 1: Index, 2: Log, 4: Chains */
extern short ExcludeFlag;
extern short Silent;
extern short VirtualFlag;
extern short SnapShot;
extern short EllipseFlag;
extern short UsingFids;
extern short InchFlag;
extern short AziCorrected;
extern short InclCorrected;
extern short SchematicFlag;
extern short HorizontalDistanceFlag;
extern short StreamIsFile;
extern short CompassUnits;
extern short InclinometerUnits;
extern short MetersOut;

/* Lists */
extern struct stationRec FirstAged ;
extern struct context *ThisContext;
extern struct sList *P1LList,*P1LocnList,*MapList;
/* Principle lists */
extern struct lList *DataList,*LinkList,*LocnList,*StringList;
extern struct lList *SubSegList,*SegList,*UnResSegList;
/* Conditionally used */
extern struct lList *ColList,*DimList,*LineList,*LongTraList,*NameList,*DbList;
extern struct lList *RowList,*FieldLList,*CellList,*HeaderList,*EeRowList,*EeColList;

/* Directive jump table stuff */

extern dirTableEntry ThisDir;

extern dirTableEntry DirTable[] ;

extern int DirEntryWidth,	NumDirs;

extern char *ErrorTable[] ;

extern int NumErrors ;

extern char *LListNames[] ;

extern struct listRec JobTable[] ;

extern int NumJobs ;