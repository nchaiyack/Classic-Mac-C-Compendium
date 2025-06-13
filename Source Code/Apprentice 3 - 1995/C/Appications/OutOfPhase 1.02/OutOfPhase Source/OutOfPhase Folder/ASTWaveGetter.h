/* ASTWaveGetter.h */

#ifndef Included_ASTWaveGetter_h
#define Included_ASTWaveGetter_h

/* ASTWaveGetter module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* PcodeObject */
/* CompilerRoot */
/* Memory */
/* TrashTracker */
/* ASTWaveGetter */

#include "PcodeObject.h"
#include "CompilerRoot.h"

struct ASTWaveGetterRec;
typedef struct ASTWaveGetterRec ASTWaveGetterRec;

/* all memory allocated in this module is from TrashTracker */

/* forwards */
struct TrashTrackRec;

typedef enum
	{
		eWaveGetterSampleLeft EXECUTE(= -21652),
		eWaveGetterSampleRight,
		eWaveGetterSampleMono,
		eWaveGetterWaveFrames,
		eWaveGetterWaveTables,
		eWaveGetterWaveArray
	} WaveGetterOp;

/* create a new AST wave getter form */
ASTWaveGetterRec*		NewWaveGetter(char* SampleName, WaveGetterOp TheOperation,
											struct TrashTrackRec* TrashTracker, long LineNumber);

/* type check the wave getter node.  this returns eCompileNoError if */
/* everything is ok, and the appropriate type in *ResultingDataType. */
CompileErrors				TypeCheckWaveGetter(DataTypes* ResultingDataType,
											ASTWaveGetterRec* WaveGetter, long* ErrorLineNumber,
											struct TrashTrackRec* TrashTracker);

/* generate code for a wave getter.  returns True if successful, or False if it fails. */
MyBoolean						CodeGenWaveGetter(struct PcodeRec* FuncCode,
											long* StackDepthParam, ASTWaveGetterRec* WaveGetter);

#endif
