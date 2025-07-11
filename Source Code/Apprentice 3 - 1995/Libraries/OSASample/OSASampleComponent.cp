#include <OSAComp.h>

#include <string.h>

extern "C" ComponentResult Main(ComponentParameters * params, Handle storage);

#if GENERATINGPOWERPC
#ifdef powerc
#define CallComponentFunctionWithStorageUniv(storage, params, funcName) \
   CallComponentFunctionWithStorage(storage, params, &funcName##RD)
#define CallComponentFunctionUniv(params, funcName) \
   CallComponentFunction(params, &funcName##RD)
#define INSTANTIATE_ROUTINE_DESCRIPTOR(funcName) RoutineDescriptor funcName##RD = \
   BUILD_ROUTINE_DESCRIPTOR (upp##funcName##ProcInfo, funcName)
#else
#define CallComponentFunctionWithStorageUniv(storage, params, funcName) \
   CallComponentFunctionWithStorage(storage, params, (ComponentFunctionUPP)funcName)
#define CallComponentFunctionUniv(params, funcName) \
   CallComponentFunction(params, (ComponentFunctionUPP)funcName)
#endif


const int 		MaxIDSlot	= 100;
const OSType	MySignature	=	'SMPL';

typedef struct {
	ComponentInstance	gSelf;
	OSErr					errorNumber;
	AEDesc				errorDesc;
	AEDesc				scriptIDSlot[MaxIDSlot];
} GlobalsRecord, ** GlobalsHandle;

ComponentResult	MyClose(GlobalsHandle globals, ComponentInstance self);

ComponentResult	MyCanDo(GlobalsHandle globals, short selector);

ComponentResult	DoOSALoad(
							GlobalsHandle	globals, 
							AEDesc *			scriptData, 
							long 				modeFlags,
							OSAID *			resultingScriptID);

ComponentResult	DoOSAStore(
							GlobalsHandle	globals, 
							OSAID				scriptID, 
							DescType			desiredType,
							long 				modeFlags,
							AEDesc *			resultingScriptData);

ComponentResult	DoOSADispose(GlobalsHandle globals, OSAID scriptID);

ComponentResult	DoOSAScriptError(
							GlobalsHandle	globals, 
							OSType			selector, 
							DescType			desiredType,
							AEDesc *			resultingErrorDescription);

ComponentResult	DoOSAExecute(
							GlobalsHandle	globals, 
							OSAID				compiledScriptID, 
							OSAID				contextID, 
							long 				modeFlags,
							OSAID *			resultingScriptValueID);

ComponentResult	DoOSADisplay(
							GlobalsHandle	globals,
							OSAID				scriptValueID,
							DescType			desiredType,
							long				modeFlags,
							AEDesc *			resultingText);

ComponentResult	DoOSAScriptingComponentName(
							GlobalsHandle 	globals, 
							AEDesc *			resultingScriptingComponentName);

ComponentResult	DoOSACompile(
							GlobalsHandle	globals, 
							AEDesc *			sourceData, 
							long 				modeFlags,
							OSAID *			resultingCompiledScriptID);

ComponentResult	DoOSAGetSource(
							GlobalsHandle	globals, 
							OSAID				scriptID, 
							DescType			desiredType,
							AEDesc *			resultingSourceData);

ComponentResult	DoOSACoerceFromDesc(
							GlobalsHandle	globals, 
							AEDesc *			scriptData, 
							long 				modeFlags,
							OSAID *			resultingScriptValueID);

ComponentResult	DoOSACoerceToDesc(
							GlobalsHandle	globals, 
							OSAID				scriptValueID, 
							DescType			desiredType,
							long 				modeFlags,
							AEDesc *			result);

ComponentResult	DoOSALoadExecute(
							GlobalsHandle	globals, 
							AEDesc *			scriptData,
							OSAID				contextID, 
							long 				modeFlags,
							OSAID *			resultingScriptValueID);

ComponentResult	DoOSACompileExecute(
							GlobalsHandle	globals, 
							AEDesc *			sourceData,
							OSAID				contextID, 
							long 				modeFlags,
							OSAID *			resultingScriptValueID);

ComponentResult	DoOSADoScript(
							GlobalsHandle	globals, 
							AEDesc *			sourceData,
							OSAID				contextID, 
							DescType			desiredType,
							long 				modeFlags,
							AEDesc *			resultingText);

ComponentResult	DoOSAMakeContext(
							StringPtr		contextName, 
							OSAID				parentContext, 
							OSAID *			resultingContextID);

enum {
	uppMyCloseProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(ComponentInstance))),
	uppMyCanDoProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short))),
	uppDoOSALoadProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(AEDesc *)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(OSAID *))),
	uppDoOSAStoreProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(OSAID)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(DescType)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(AEDesc *))),
	uppDoOSADisposeProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(OSAID))),
	uppDoOSAScriptErrorProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(OSType)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(DescType)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(AEDesc *))),
	uppDoOSAExecuteProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(OSAID)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(OSAID)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(OSAID *))),
	uppDoOSADisplayProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(OSAID)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(DescType)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(AEDesc *))),
	uppDoOSAScriptingComponentNameProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(AEDesc *))),
	uppDoOSACompileProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(AEDesc *)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(OSAID *))),
	uppDoOSAGetSourceProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(OSAID)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(DescType)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(AEDesc *))),
	uppDoOSACoerceFromDescProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(AEDesc *)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(OSAID *))),
	uppDoOSACoerceToDescProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(OSAID)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(DescType)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(AEDesc *))),
	uppDoOSALoadExecuteProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(AEDesc *)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(OSAID)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(OSAID *))),
	uppDoOSACompileExecuteProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(AEDesc *)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(OSAID)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(OSAID *))),
	uppDoOSADoScriptProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(AEDesc *)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(OSAID)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(DescType)))
		 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(6, SIZE_CODE(sizeof(AEDesc *))),
	uppDoOSAMakeContextProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(StringPtr)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(OSAID)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(OSAID *)))
};

INSTANTIATE_ROUTINE_DESCRIPTOR(MyClose);
INSTANTIATE_ROUTINE_DESCRIPTOR(MyCanDo);
INSTANTIATE_ROUTINE_DESCRIPTOR(DoOSALoad);
INSTANTIATE_ROUTINE_DESCRIPTOR(DoOSAStore);
INSTANTIATE_ROUTINE_DESCRIPTOR(DoOSADispose);
INSTANTIATE_ROUTINE_DESCRIPTOR(DoOSAScriptError);
INSTANTIATE_ROUTINE_DESCRIPTOR(DoOSAExecute);
INSTANTIATE_ROUTINE_DESCRIPTOR(DoOSADisplay);
INSTANTIATE_ROUTINE_DESCRIPTOR(DoOSAScriptingComponentName);
INSTANTIATE_ROUTINE_DESCRIPTOR(DoOSACompile);
INSTANTIATE_ROUTINE_DESCRIPTOR(DoOSAGetSource);
INSTANTIATE_ROUTINE_DESCRIPTOR(DoOSACoerceFromDesc);
INSTANTIATE_ROUTINE_DESCRIPTOR(DoOSACoerceToDesc);
INSTANTIATE_ROUTINE_DESCRIPTOR(DoOSALoadExecute);
INSTANTIATE_ROUTINE_DESCRIPTOR(DoOSACompileExecute);
INSTANTIATE_ROUTINE_DESCRIPTOR(DoOSADoScript);
INSTANTIATE_ROUTINE_DESCRIPTOR(DoOSAMakeContext);

RoutineDescriptor MainRD = BUILD_ROUTINE_DESCRIPTOR(uppComponentRoutineProcInfo, Main);
#endif

#ifdef __MWERKS__
ProcInfoType __procinfo = uppComponentRoutineProcInfo;
#endif

ComponentResult Main(ComponentParameters * params, Handle storage)
{
	ComponentResult	err;
	AEDesc				myErrDesc;
#if 0
	unsigned char		hex[] 	= "0123456789ABCDEF";
	unsigned char		diag[] 	= "\pEntering Sample, Request 00000000";
	unsigned char *   digit  	= diag+diag[0];
	long					req		=	params->what;
	
	while (*digit != ' ') {
		*digit = hex[req & 15];
		req    >>= 4;
		--digit;
	}
	
	DebugStr(diag);
#endif
	
	if (params->what == kComponentOpenSelect) {
		ComponentInstance	self 		= ComponentInstance(params->params[0]);
		GlobalsHandle		globals	= GlobalsHandle(NewHandleClear(sizeof(GlobalsRecord)));
		if (!globals)
			err = MemError();
		else
			if (!(err = AECreateDesc(typeChar, nil, 0, &myErrDesc))) {
				SetComponentInstanceStorage(self, Handle(globals));
				
				(*globals)->gSelf 		= self;
				(*globals)->errorDesc	= myErrDesc;
			} else
				DisposeHandle(Handle(globals));
		
		return err;
	} 
	
	if (params->what < 0)	// Negative selectors used for component manager calls
		switch (params->what) {
		case kComponentCloseSelect: 
			return CallComponentFunctionWithStorageUniv(storage, params, MyClose);
		case kComponentCanDoSelect: 
			return CallComponentFunctionWithStorageUniv(storage, params, MyCanDo);
		case kComponentVersionSelect: 
			return 0;
		default:
			return badComponentSelector;
		}
	else	// Specific component routines called through the component manager
		switch (params->what) {
		case kOSASelectLoad:
			return CallComponentFunctionWithStorageUniv(storage, params, DoOSALoad); 
		case kOSASelectStore:
			return CallComponentFunctionWithStorageUniv(storage, params, DoOSAStore); 
		case kOSASelectDispose:
			return CallComponentFunctionWithStorageUniv(storage, params, DoOSADispose); 
		case kOSASelectScriptError:
			return CallComponentFunctionWithStorageUniv(storage, params, DoOSAScriptError); 
		case kOSASelectExecute:
			return CallComponentFunctionWithStorageUniv(storage, params, DoOSAExecute); 
		case kOSASelectDisplay:
			return CallComponentFunctionWithStorageUniv(storage, params, DoOSADisplay); 
		case kOSASelectCompile:
			return CallComponentFunctionWithStorageUniv(storage, params, DoOSACompile); 
		case kOSASelectScriptingComponentName:
			return CallComponentFunctionWithStorageUniv(storage, params, DoOSAScriptingComponentName); 
		case kOSASelectGetSource:
			return CallComponentFunctionWithStorageUniv(storage, params, DoOSAGetSource); 
		case kOSASelectCoerceFromDesc:
			return CallComponentFunctionWithStorageUniv(storage, params, DoOSACoerceFromDesc); 
		case kOSASelectCoerceToDesc:
			return CallComponentFunctionWithStorageUniv(storage, params, DoOSACoerceToDesc); 
		case kOSASelectLoadExecute:
			return CallComponentFunctionWithStorageUniv(storage, params, DoOSALoadExecute); 
		case kOSASelectCompileExecute:
			return CallComponentFunctionWithStorageUniv(storage, params, DoOSACompileExecute); 
		case kOSASelectDoScript:
			return CallComponentFunctionWithStorageUniv(storage, params, DoOSADoScript); 
		case kOSASelectMakeContext:
			return CallComponentFunctionUniv(params, DoOSAMakeContext);
		default:
			return badComponentSelector;
		}
}

ComponentResult MyClose(GlobalsHandle globals, ComponentInstance self)
{
	int	i;
	
	AEDisposeDesc(&(*globals)->errorDesc);
	
	HLock((Handle) globals);
	for (i = 0; i<MaxIDSlot; ++i)
		AEDisposeDesc((*globals)->scriptIDSlot+i);
	HUnlock((Handle) globals);

	return noErr;
}

ComponentResult MyCanDo(GlobalsHandle globals, short selector)
{
	return 1;	// A lie. So sue me.
}

void ClearErrorDesc(GlobalsHandle globals)
{
	(*globals)->errorNumber = noErr;
	SetHandleSize((*globals)->errorDesc.dataHandle, 0);
}

ComponentResult TryDoScript(GlobalsHandle globals, AEDesc * scriptData, short * num)
{
	// try to compile the text and return the result
	// we just try to translate one, two, three etc into number in this simple example
	
	OSErr				err;
	Str255			aStr;
	const char * 	huh = "Cannot understand ";
	
	*num = 0;
	
	if (scriptData->descriptorType == MySignature) {
      GetIText(scriptData->dataHandle, aStr);
		UprString(aStr, false);
		if (!PLstrcmp(aStr, "\pONE"))
			*num = 1;
		else if (!PLstrcmp(aStr, "\pTWO"))
			*num = 2;
		else if (!PLstrcmp(aStr, "\pTHREE"))
			*num = 3;
		else if (!PLstrcmp(aStr, "\pFOUR"))
			*num = 4;
		else if (!PLstrcmp(aStr, "\pFIVE"))
			*num = 5;
		else if (!PLstrcmp(aStr, "\pSIX"))
			*num = 6;
		else if (!PLstrcmp(aStr, "\pSEVEN"))
			*num = 7;
		else if (!PLstrcmp(aStr, "\pEIGHT"))
			*num = 8;
		else if (!PLstrcmp(aStr, "\pNINE"))
			*num = 9;
		else if (!PLstrcmp(aStr, "\pTEN"))
			*num = 10;
			
      ClearErrorDesc(globals);
      if (!*num) {
			err = errOSAScriptError;
         (*globals)->errorNumber = err;
         PtrToXHand(huh, (*globals)->errorDesc.dataHandle, strlen(huh));
         HLock(scriptData->dataHandle);
         HandAndHand(scriptData->dataHandle, (*globals)->errorDesc.dataHandle);
         HUnlock(scriptData->dataHandle);
		} else 
			err = 0;
	} else
		err = errOSABadStorageType;
		
   return err;
}

ComponentResult ActualDoScript(
						GlobalsHandle globals, 
						AEDesc * scriptData, 
						AEDesc * result)
{
	short	num;
	OSErr	err;
	
	err = TryDoScript(globals, scriptData, &num);
	
	if (!err)
		err = AECreateDesc(typeShortInteger, &num, sizeof(short), result);
		
	return err;
}

void NukeIt(AEDesc * desc)
{
	desc->descriptorType = typeNull;
	desc->dataHandle		= nil;
}

long FindScriptSlotID(GlobalsHandle globals)
{
	long	i;
	
	for (i=0; i<MaxIDSlot; ++i)
		if (!(*globals)->scriptIDSlot[i].dataHandle)
			return i+1;
	
	return 0;
}

ComponentResult FindSlotAndPutIt(
						GlobalsHandle	globals,
						AEDesc *			desc,
						OSAID *			slotID)
// put a desc into a slot,
// if slotID starts as 0 find a new slot, otherwise reuse the slot ID
{
	OSErr		err;
	AEDesc	oldDesc;
	
	if (!desc->dataHandle) {	// we don't have a descriptor, return 0
		*slotID = 0;
		
		return memFullErr;
	} 
	
	if (!*slotID) 
		*slotID = FindScriptSlotID(globals);	// find an empty slot
	else {
		oldDesc = (*globals)->scriptIDSlot[*slotID-1];
		AEDisposeDesc(&oldDesc);				// throw away old contents
	}
	
	if (*slotID) {
		(*globals)->scriptIDSlot[*slotID-1] = *desc;
		
		return noErr;
	} else {
		AEDisposeDesc(desc);
		
		return memFullErr; // no slot available, treat it as memory error because in a more realistic program we would expand the slots
	}
}

OSErr TextToStxt(AEDesc * textDesc, AEDesc * result)
// convert TEXT to styled text
{
	OSErr		err;
	AERecord	theRec;
	
	struct {
		short				scrpNStyles;
		ScrpSTElement	scrpStyle;
	} myStyle;
	
	result->dataHandle					=	nil;
	myStyle.scrpNStyles					=	1;
	myStyle.scrpStyle.scrpStartChar	=	0;
	myStyle.scrpStyle.scrpHeight		=	16;
	myStyle.scrpStyle.scrpAscent		=	12;
	myStyle.scrpStyle.scrpFont			=	1;
	myStyle.scrpStyle.scrpFace			=	bold;
	myStyle.scrpStyle.scrpSize			=	12;
	myStyle.scrpStyle.scrpColor.red	=	0;
	myStyle.scrpStyle.scrpColor.green=	0;
	myStyle.scrpStyle.scrpColor.blue	=	0;
	
	if (err = AECreateList(nil, 0, true, &theRec))
		goto failCreate;
	if (err = AEPutKeyPtr(&theRec, 'ksty', 'styl', &myStyle, sizeof(myStyle)))
		goto failPut;
	if (err = AEPutKeyDesc(&theRec, 'ktxt', textDesc))
		goto failPut;
	err = AECoerceDesc(&theRec, typeStyledText, result);

failPut:
	AEDisposeDesc(&theRec);
failCreate:
	return err;
}

ComponentResult DoOSALoad(
							GlobalsHandle	globals, 
							AEDesc *			scriptData, 
							long 				modeFlags,
							OSAID *			resultingScriptID)
// make a copy, strip the trailer and put it into a slot 
{
	ComponentResult	err;
	AEDesc				descCopy;
	DescType				itsType;
	
	if (scriptData->descriptorType == kOSAGenericScriptingComponentSubtype)
		if (!OSAGetStorageType(scriptData->dataHandle, &itsType))
			if (itsType == MySignature) {
				if (err = AEDuplicateDesc(scriptData, &descCopy))
					return err;
				
				OSARemoveStorageType(descCopy.dataHandle);
				descCopy.descriptorType	= MySignature;
				resultingScriptID 		= 0;
				
				return FindSlotAndPutIt(globals, &descCopy, resultingScriptID);
			}
			
	return errOSABadStorageType;
}
	
ComponentResult	DoOSAStore(
							GlobalsHandle	globals, 
							OSAID				scriptID, 
							DescType			desiredType,
							long 				modeFlags,
							AEDesc *			resultingScriptData)
// make a copy of content in the slot and add trailer
{
	ComponentResult	err;
	
	NukeIt(resultingScriptData);
	if (!scriptID || scriptID>MaxIDSlot)
		return errOSAInvalidID;
	
	*resultingScriptData = (*globals)->scriptIDSlot[scriptID-1];
	
	if (!resultingScriptData->dataHandle)
		return errOSAInvalidID;
	if (err = HandToHand(&resultingScriptData->dataHandle))
		return err;
	if (err = 
			OSAAddStorageType(
				resultingScriptData->dataHandle, 
				resultingScriptData->descriptorType)	
	)
		AEDisposeDesc(resultingScriptData);
	else
		 resultingScriptData->descriptorType = kOSAGenericScriptingComponentSubtype;

	return err;
}

ComponentResult	DoOSADispose(GlobalsHandle globals, OSAID scriptID)
// dispose the descriptor in the slot 
{
	if (scriptID && scriptID <= MaxIDSlot) {
		HLock((Handle) globals);
		AEDisposeDesc(&(*globals)->scriptIDSlot[scriptID-1]);
		HUnlock((Handle) globals);
	}
	
	return noErr;
}

ComponentResult	DoOSAScriptError(
							GlobalsHandle	globals, 
							OSType			selector, 
							DescType			desiredType,
							AEDesc *			resultingErrorDescription)
// fetch the content of the error descriptor 
{
	ComponentResult 	err;
	long					i;
	AERecord				aRec;
	AEDesc				myErrorDesc;
	short					errNum;
	
	NukeIt(resultingErrorDescription);
	switch (selector) {
	case kOSAErrorNumber:
		errNum = (*globals)->errorNumber;
		
		return 
			AECoercePtr(
				typeShortInteger, 
				&errNum, sizeof(short), 
				desiredType, resultingErrorDescription);
	case kOSAErrorMessage:
		myErrorDesc = (*globals)->errorDesc;
		switch (desiredType) {
		case typeChar:
		case typeWildCard:
			return AEDuplicateDesc(&myErrorDesc, resultingErrorDescription);
		case typeStyledText:
			return TextToStxt(&myErrorDesc, resultingErrorDescription);
		default:
			return AECoerceDesc(&myErrorDesc, desiredType, resultingErrorDescription);
		}
	case kOSAErrorRange:
		// in this simple example, we make the error range to include everything
		if (err = AECreateList(nil, 0, true, &aRec))
			return err;
		i = 0;
		AEPutKeyPtr(&aRec, keyOSASourceStart, typeLongInteger, &i, sizeof(i));
		i = 30000;
		AEPutKeyPtr(&aRec, keyOSASourceEnd, typeLongInteger, &i, sizeof(i));
		switch (desiredType) {
		case typeAERecord:
			*resultingErrorDescription = aRec;
			
			return noErr;
		case typeWildCard:
			desiredType = typeOSAErrorRange;
			// Fall through
		default:
			err = AECoerceDesc(&aRec, desiredType, resultingErrorDescription);
			AEDisposeDesc(&aRec);
			return err;
		}
	default:
		return errOSABadSelector;
	}
}

ComponentResult	DoOSAExecute(
							GlobalsHandle	globals, 
							OSAID				compiledScriptID, 
							OSAID				contextID, 
							long 				modeFlags,
							OSAID *			resultingScriptValueID)
// since in this sample program, internal form is the same as source form except 
// for the descriptor type, we can just call CompileExecute 
{
	*resultingScriptValueID = 0;
	if (!compiledScriptID || compiledScriptID > MaxIDSlot)
		return errOSAInvalidID;
	else {
		AEDesc execDesc = (*globals)->scriptIDSlot[compiledScriptID-1];
		
		return 
			DoOSACompileExecute(globals, 
				&execDesc, 
				contextID, modeFlags, resultingScriptValueID);
	}
}

		
ComponentResult	DoOSADisplay(
							GlobalsHandle	globals,
							OSAID				scriptValueID,
							DescType			desiredType,
							long				modeFlags,
							AEDesc *			resultingText)
// in this program, we have no special form for display so just coerce it 
{
	return DoOSACoerceToDesc(globals, scriptValueID, desiredType, modeFlags, resultingText);
}

ComponentResult	DoOSAGetSource(
							GlobalsHandle	globals, 
							OSAID				scriptID, 
							DescType			desiredType,
							AEDesc *			resultingSourceData)
// in this sample program, source is same as internal form and there is no 
// special formatting, so just call coerce
{
	return DoOSACoerceToDesc(globals, scriptID, desiredType, 0, resultingSourceData);
}


ComponentResult	DoOSACompile(
							GlobalsHandle	globals, 
							AEDesc *			sourceData, 
							long 				modeFlags,
							OSAID *			resultingCompiledScriptID)
// since internal form is same as source, just change the dataType and call TryDoScript,
// if it compiles then just store it 
{
	ComponentResult	err;
	AEDesc				descCopy, srcCopy;
	short					num;

	descCopy.dataHandle	= 	nil;
	srcCopy					=  *sourceData;
	
	if (srcCopy.descriptorType == typeChar) {
		srcCopy.descriptorType = MySignature;
		if (err = TryDoScript(globals, &srcCopy, &num))
			return err;
		if (err = AEDuplicateDesc(&srcCopy, &descCopy))
			return err;
		else
			return FindSlotAndPutIt(globals, &descCopy, resultingCompiledScriptID);
	} else
		return errOSABadStorageType;
}

ComponentResult	DoOSACoerceFromDesc(
							GlobalsHandle	globals, 
							AEDesc *			scriptData, 
							long 				modeFlags,
							OSAID *			resultingScriptValueID)
// just store a copy into the slot 
{
	ComponentResult 	err;
	AEDesc				descCopy;
	
	descCopy.dataHandle = nil;
	
	if (err = AEDuplicateDesc(scriptData, &descCopy))
		return err;
	
	*resultingScriptValueID = 0;
	
	return FindSlotAndPutIt(globals, &descCopy, resultingScriptValueID);
}

ComponentResult	DoOSACoerceToDesc(
							GlobalsHandle	globals, 
							OSAID				scriptValueID, 
							DescType			desiredType,
							long 				modeFlags,
							AEDesc *			result)
// fetch from the slot and coerce it, if it is source we rename the type
// because internal form is same as the source text
{
	ComponentResult	err;
	AEDesc				myScriptValue;
	
	if (!scriptValueID || scriptValueID > MaxIDSlot)
		return errOSAInvalidID;
	
	myScriptValue = (*globals)->scriptIDSlot[scriptValueID-1];
	
	if (myScriptValue.descriptorType == MySignature)
		myScriptValue.descriptorType = typeChar;
	if (myScriptValue.descriptorType == desiredType)
		return AEDuplicateDesc(&myScriptValue, result);
	else if (desiredType == typeStyledText)
		return TextToStxt(&myScriptValue, result);
	else
		return AECoerceDesc(&myScriptValue, desiredType, result);
}

ComponentResult	DoOSALoadExecute(
							GlobalsHandle	globals, 
							AEDesc *			scriptData,
							OSAID				contextID, 
							long 				modeFlags,
							OSAID *			resultingScriptValueID)
// strip the trailer, execute it and put back the trailer
// there is chance we cannot restore the original form although we are try our best
// this really calls for a GetScriptDataSize call in OSAComp
{
	ComponentResult	err;
	DescType				itsType;

	if (scriptData->descriptorType != kOSAGenericScriptingComponentSubtype)
		return errOSABadStorageType;
	if (err = OSAGetStorageType(scriptData->dataHandle, &itsType))
		return err;
	if (itsType != MySignature)
		return errOSABadStorageType;
	if (err = OSARemoveStorageType(scriptData->dataHandle))
		return err;
	
	err = DoOSACompileExecute(globals, scriptData, 0, 0, resultingScriptValueID);
	
	if (OSAAddStorageType(scriptData->dataHandle, MySignature)) {
		// we are in deep trouble, we change scriptData and cannot put it back
		// dispose result to get back the memory  
		DoOSADispose(globals, *resultingScriptValueID); 
		resultingScriptValueID = 0;
		// Now try again
		OSAAddStorageType(scriptData->dataHandle, MySignature);
		
		return memFullErr;
	} else
		return err;
}

ComponentResult	DoOSACompileExecute(
							GlobalsHandle	globals, 
							AEDesc *			sourceData,
							OSAID				contextID, 
							long 				modeFlags,
							OSAID *			resultingScriptValueID)
// since source is same as internal form, just execute it and store the result 
{
	ComponentResult	err;
	AEDesc				resultDesc;
	AEDesc				srcDesc;
	
	srcDesc						= *sourceData;
	srcDesc.descriptorType	=	MySignature;
	
	if (err = ActualDoScript(globals, &srcDesc, &resultDesc))
		return err;
	else
		return FindSlotAndPutIt(globals, &resultDesc, resultingScriptValueID);
}

ComponentResult	DoOSADoScript(
							GlobalsHandle	globals, 
							AEDesc *			sourceData,
							OSAID				contextID, 
							DescType			desiredType,
							long 				modeFlags,
							AEDesc *			resultingText)
// since source is same as internal form, just execute it and return the result
{
	ComponentResult	err;
	AEDesc				resultDesc;
	AEDesc				srcDesc;
	
	srcDesc						= *sourceData;
	srcDesc.descriptorType	=	MySignature;
	
   NukeIt(resultingText);

	if (err = ActualDoScript(globals, &srcDesc, &resultDesc))
		return err;
	
	if (desiredType == resultDesc.descriptorType || desiredType == typeWildCard)
		*resultingText = resultDesc;
	else {
		if (desiredType == typeStyledText)
			err = TextToStxt(&resultDesc, resultingText);
		else
			err = AECoerceDesc(&resultDesc, desiredType, resultingText);
		
		AEDisposeDesc(&resultDesc);
	}
	
	return err;
}

ComponentResult	DoOSAMakeContext(
							StringPtr		contextName, 
							OSAID				parentContext, 
							OSAID *			resultingContextID)
// context is not used in this sample program
{
	*resultingContextID = 0;
   
   return noErr;
}

ComponentResult	DoOSAScriptingComponentName(
							GlobalsHandle 	globals, 
							AEDesc *			resultingScriptingComponentName)
{
	static char * componentName = "SampleScript";
   
   return 
   	AECreateDesc(
   		typeChar, componentName, strlen(componentName), 
   		resultingScriptingComponentName);
}

#if GENERATINGPOWERPC
// For some reason, there is no PPC OSAComp glue, so I had to reverse engineer this

struct OSAScriptStorageTrailer {
	DescType			type;
	short				version;
	short				trailerSize;
	unsigned long	cookie;
};

OSAScriptStorageTrailer * OSAGetStorageTypeTrailer(Handle scriptData)
{
	long size		= GetHandleSize(scriptData);
	
	if (size < 12 || size & 1)
		return nil;

	OSAScriptStorageTrailer *	trailer = 
		(OSAScriptStorageTrailer *) (*scriptData + (size - 12));
	
	if (trailer->cookie == 0xFADEDEAD && trailer->version == 1)
		return trailer;
	else
		return nil;
}

pascal OSErr OSAGetStorageType(Handle scriptData, DescType *dscType)
{
	OSAScriptStorageTrailer *	trailer = OSAGetStorageTypeTrailer(scriptData);
	
	if (!trailer)
		return errOSABadStorageType;
	
	*dscType = trailer->type;
	
	return noErr;
}

pascal OSErr OSAAddStorageType(Handle scriptData, DescType dscType)
{
	OSErr	err;
	
	OSAScriptStorageTrailer *	trailer = OSAGetStorageTypeTrailer(scriptData);

	if (trailer) {
		trailer->type = dscType;
		
		return noErr;
	}
	
	short trailerSize = 12;
	long	size			= GetHandleSize(scriptData);
	
	if (size & 1) {
		++trailerSize;
		SetHandleSize(scriptData, size+1);
		if (err = MemError())
			return err;
	}
	
	OSAScriptStorageTrailer nuTrailer;
	
	nuTrailer.type 			= dscType;
	nuTrailer.version			= 1;
	nuTrailer.trailerSize 	= trailerSize;
	nuTrailer.cookie			= 0xFADEDEAD;
	
	if (err = PtrAndHand((Ptr) &nuTrailer, scriptData, sizeof(OSAScriptStorageTrailer)))
		SetHandleSize(scriptData, size);
		
	return err;
}

pascal OSErr OSARemoveStorageType(Handle scriptData)
{
	OSErr	err;
	
	OSAScriptStorageTrailer *	trailer = OSAGetStorageTypeTrailer(scriptData);

	if (trailer)
		SetHandleSize(scriptData, GetHandleSize(scriptData) - trailer->trailerSize);
		
	return noErr;
}

#endif
