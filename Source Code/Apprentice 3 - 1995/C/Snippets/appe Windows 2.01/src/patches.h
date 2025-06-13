// File "patches.h" - 

#ifndef ____PATCHES_HEADER_____
#define ____PATCHES_HEADER_____

// * **************************************************************************** * //
// * **************************************************************************** * //

// Setup calling information for the routines we need
#if !GENERATINGPOWERPC
typedef pascal void (*ExitToShellProcPtr)();
typedef pascal WindowPtr (*NewWindowProcPtr)(Ptr wStorage, Rect *bounds, StringPtr title,
		Boolean vis, short procID, WindowPtr behind, Boolean close, long refCon);
typedef ExitToShellProcPtr ExitToShellUPP;
typedef NewWindowProcPtr NewWindowUPP;
enum {
	uppExitToShellProcInfo = 0,
	uppNewWindowProcInfo = 0
	};
#define NewExitToShellProc(proc) (ExitToShellUPP)(proc)
#define NewNewWindowProc(proc) (NewWindowUPP)(proc)
#define CallExitToShellProc(proc) (* (ExitToShellProcPtr) (proc))()
#define CallNewWindowProc(proc, p1, p2, p3, p4, p5, p6, p7, p8)	\
		(* (NewWindowProcPtr) (proc))(p1, p2, p3, p4, p5, p6, p7, p8)
#else
typedef UniversalProcPtr ExitToShellUPP;
typedef UniversalProcPtr NewWindowUPP;
enum {
	uppExitToShellProcInfo = kPascalStackBased,
	uppNewWindowProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(WindowPtr)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Ptr)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(Rect *)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(StringPtr)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(Boolean)))
		 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(6, SIZE_CODE(sizeof(WindowPtr)))
		 | STACK_ROUTINE_PARAMETER(7, SIZE_CODE(sizeof(Boolean)))
		 | STACK_ROUTINE_PARAMETER(8, SIZE_CODE(sizeof(long)))
	};
#define NewExitToShellProc(proc) (ExitToShellUPP) \
		NewRoutineDescriptor((ProcPtr)(proc), uppExitToShellProcInfo, GetCurrentISA())
#define NewNewWindowProc(proc) (NewWindowUPP) \
		NewRoutineDescriptor((ProcPtr)(proc), uppNewWindowProcInfo, GetCurrentISA())
#define CallExitToShellProc(proc) \
		CallUniversalProc((UniversalProcPtr) (proc), uppExitToShellProcInfo)
#define CallNewWindowProc(proc, p1, p2, p3, p4, p5, p6, p7, p8) \
		(WindowPtr) CallUniversalProc((UniversalProcPtr)(proc), \
		uppNewWindowProcInfo, p1, p2, p3, p4, p5, p6, p7, p8)
#endif GENERATINGPOWERPC

// * **************************************************************************** * //
// * **************************************************************************** * //
// Function Prototypes

void PatchExitToShell(void);
pascal void MyExitToShell(void);

void PatchNewWindow(void);
pascal WindowPtr MyNewWindow(Ptr wStorage, Rect *bounds, StringPtr title, 
		Boolean vis, short procID, WindowPtr behind, Boolean close, long refCon);

#endif  ____PATCHES_HEADER_____
