#include <PatchNewControl.h>

static pascal ControlHandle MyNewControl( WindowPtr theWindow, Rect *ctrlRect, Str255 title, Boolean visible,
											short val, short min, short max, short ctrlType, long refCon );

typedef pascal ControlHandle (*NewControlProcPtr) ( WindowPtr, Rect*, Str255,
Boolean, short, short, short, short, long );

enum {
        uppNewControlProcInfo = kPascalStackBased
                 | RESULT_SIZE(SIZE_CODE(sizeof(long)))
                 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(WindowPtr)))
                 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(Rect*)))
                 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(StringPtr)))
                 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(Boolean)))
                 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(short)))
                 | STACK_ROUTINE_PARAMETER(6, SIZE_CODE(sizeof(short)))
                 | STACK_ROUTINE_PARAMETER(7, SIZE_CODE(sizeof(short)))
                 | STACK_ROUTINE_PARAMETER(8, SIZE_CODE(sizeof(short)))
                 | STACK_ROUTINE_PARAMETER(9, SIZE_CODE(sizeof(long)))
};

#if GENERATINGCFM
        typedef UniversalProcPtr NewControlUPP;
        
        #define CallNewControlProc(userRoutine,w,rect,title,vis,val,min,max,ctrlType,refCon)           \
                        (ControlHandle) CallUniversalProc((UniversalProcPtr)(userRoutine), uppNewControlProcInfo,\
                                                        (w),(rect),(title),(vis),(val),(min),(max),(ctrlType),(refCon))
                                                        
        #define NewNewControlProc(userRoutine)          \
                        (NewControlUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppNewControlProcInfo, GetCurrentISA())
#else
        typedef NewControlProcPtr NewControlUPP;
        
        #define CallNewControlProc(userRoutine,w,rect,title,vis,val,min,max,ctrlType,refCon)   \
			(*(userRoutine))((w),(rect),(title),(vis),(val),(min),(max),(ctrlType),(refCon))
        #define NewNewControlProc(userRoutine)          \
			(NewControlUPP)(userRoutine)
#endif

static NewControlUPP sOldNewControl;
static NewControlUPP myNewControlUPP;

static pascal ControlHandle MyNewControl( WindowPtr theWindow, Rect *ctrlRect, Str255 title, Boolean visible,
											short val, short min, short max, short ctrlType, long refCon )
{
        if( ctrlType==checkBoxProc || ctrlType==radioButProc )
                ctrlType |= useWFont;                                   // Make check/radio buttons use window's font
        return CallNewControlProc(sOldNewControl, theWindow,ctrlRect,title,visible,val,min,max,ctrlType,refCon);
}

void	PatchNewControl( void )
{

	myNewControlUPP = NewNewControlProc(&MyNewControl);

	sOldNewControl = (NewControlUPP)GetToolboxTrapAddress(_NewControl);
	SetToolboxTrapAddress((UniversalProcPtr)myNewControlUPP,_NewControl);// Patch NewControl
}

void	UnPatchNewControl( void )
{
	SetToolboxTrapAddress((UniversalProcPtr)sOldNewControl,_NewControl);
	DisposeRoutineDescriptor(myNewControlUPP);
}