typedef struct {
	Rect		bounds;
	PicHandle	pict;
} pict_info;

typedef struct {	// storage for deferred highlights
	short		array_size;		// how many rectangles we can store
	short		high_waiting;	// how many rectangles are stored
	Rect		high_rect[];
} high_info;

typedef struct { // piggyback some other info on the dialog record
	DialogRecord	dialog;
	ControlHandle	scrollbar;
	RgnHandle		save_clip;
	CursHandle		ibeam_cursor;
	pascal void 	(*Handle_update)(WindowPtr);
#if USE_PICTS
	high_info		**high;
	short			pict_count;	/* how many pictures */
	pict_info		*pict_data;	/* pointer to an array */
	Boolean			high_defer_flag;
#endif
}	help_record, *help_ptr;

#if COMPRESSION
typedef struct {
	ResType	srcType;
	Handle	srcHandle;
	ResType	dstType;
	Handle	dstHandle;
} ParmInfo;

/* We don't actually use the first parameter passed to the CNVT code,
	but it is a pointer to a structure as below.
*/
typedef struct {
	ProcPtr		entryPoint;
	short		resID;
	short		parmCount;
	Boolean		useDefault; /* 2 bytes? */
} RoutineInfo;

typedef pascal OSErr (*CNVT_routine)(RoutineInfo *, ParmInfo *);
#endif COMPRESSION

typedef struct {	// format of 'Hlp?' resource
	short			DLOG_ID;
	short			first_PICT_ID;
	short			TEXT_ID;
	short			styl_ID;
	unsigned char	strings[];	// default file name, menu text
} Show_help_info;
