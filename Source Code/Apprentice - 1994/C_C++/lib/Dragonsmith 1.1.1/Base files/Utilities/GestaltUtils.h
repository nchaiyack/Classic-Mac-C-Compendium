/*
	GestaltUtils.h
*/

enum {
	opBitAnd,			// 0	All bits set in the compValue must be set in the Gestalt result
	opEqual,				// 1	Gestalt result must equal the compValue
	opNotEqual,			// 2	Ditto, not equal
	opGreater,			// 3	Gestalt result must be greater than the compValue
	opGreaterOrEqual,		// 4	Ditto, greater than or equal to
	opLess,				// 5	Ditto, less than
	opLessOrEqual,		// 6	Ditto, less than or equal
	opNoErrIsOK			// 7	All is OK as long as noErr was returned � compValue is ignored
};

typedef struct {
	OSType	selector;		// Gestalt selector
	short	compOp;		// How should the compValue and Gestalt result be compared?
	short	failureError;	// Error code if comparison fails
	long		compValue;	// Value to check the Gestalt result against
} GstCheckRec;

typedef struct {
	short			numChecks;
	GstCheckRec		check[1];
} GstCheckList;

OSErr GestaltBatchCheck (GstCheckList **checkList);
Boolean GestaltResultOK (OSType selector, short compOp, long compValue);