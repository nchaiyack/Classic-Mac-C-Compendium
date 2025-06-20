//
//	File:		CWJumpTable.c
//	
//	Contains:	MacsBug external to dump a CodeWarrior JumpTable.
//	
//	Written by:	Stefan Arentz (SMA)
//	
//	Copyright:	� 1994 Stefan Arentz
//
//
//	Modification history:
//
//		94.03.16 SMA Created.
//


#include <Types.h>

#include "dcmd.h"
#include "put.h"


typedef struct {
	short		opcode;
	long		address;
	short		segment;
} JTEntry;


#define kLoadSegOpcode		0xA9F0
#define kJumpOpcode			0x4EF9


void DumpCodeWarriorJumpTable(dcmdBlock* paramPtr, Ptr jtStart);
void DrawHeader(void);
void DrawLoadedEntry(dcmdBlock* paramPtr, JTEntry *jtEntry);
void DrawUnloadedEntry(dcmdBlock* paramPtr, JTEntry *jtEntry);


pascal void CommandEntry(dcmdBlock* paramPtr) {

	long			address;
	Boolean			ok;

	switch (paramPtr->request) {

		case dcmdInit:

			break;
			
		case dcmdHelp:

			dcmdDrawLine("\pCWJumpTable [address]");
			dcmdDrawLine("\p    Display the jumptable of an application");
			dcmdDrawLine("\p    created with CodeWarrior 68K C/C++ 1.0a2");

			break;
			
		case dcmdDoIt:

			if (dcmdPeekAtNextChar() == '\n') {
	
				address = paramPtr->registerFile[A5Register] + 0x40;
	
			} else {
	
				(void) dcmdGetNextExpression(&address, &ok);
				if (!ok) {
	
					dcmdDrawLine ("\pSyntax error");
					return;
	
				}
	
			}

			DumpCodeWarriorJumpTable(paramPtr, (Ptr)address);

			break;

	}

}

void DumpCodeWarriorJumpTable(dcmdBlock* paramPtr, Ptr jtStart) {

	JTEntry			*jtEntry;
	
	DrawHeader();

	jtEntry = (JTEntry*)jtStart;
	while (jtEntry->opcode != 0x0000) {

		switch (jtEntry->opcode) {

			case kLoadSegOpcode:

				DrawUnloadedEntry(paramPtr, jtEntry);
				break;
				
			case kJumpOpcode:

				DrawLoadedEntry(paramPtr, jtEntry);
				break;
				
			default:

				return;
				break;

		}
		
		jtEntry++;

	}

}

//                           1         2         3         4         5         6         7
//                  12345678901234567890123456789012345678901234567890123456789012345678901234567890
//                  JSR Addr  Seg# Location  Offset    Name
//                  --------- ---- --------- --------- ---------------------------------------------
//                  $0000(A5) 0001 $00000000 $00000000 main()
//                  $0008(A5) 0002 Unloaded  $12345678

void DrawHeader(void) {

	dcmdDrawLine("\p");
	dcmdDrawLine("\pJSR Addr  Seg# Location  Offset    Name");
	dcmdDrawLine("\p--------- ---- --------- --------- ---------------------------------------------");

}

void DrawLoadedEntry(dcmdBlock* paramPtr, JTEntry *jtEntry) {

	Str255		desc;

	PutChar('$');
	PutUHexWord((long)jtEntry - paramPtr->registerFile[A5Register]);
	PutCStr("(A5)");
	PutSpace();

	PutUHexWord(jtEntry->segment);
	PutSpace();

	PutChar('$');
	PutUHexZTo(jtEntry->address, 8, 24);
	PutSpace();
	
	PutSpacesTo(36);
	
	dcmdGetNameAndOffset(jtEntry->address, desc);
	PutPStr(desc);

	PutLine();

}

void DrawUnloadedEntry(dcmdBlock* paramPtr, JTEntry *jtEntry) {

	PutChar('$');
	PutUHexWord((long)jtEntry - paramPtr->registerFile[A5Register]);
	PutCStr("(A5)");

	PutSpace();

	PutUHexWord(jtEntry->segment);
	PutSpace();

	PutCStr("Unloaded ");
	PutSpace();

	PutChar('$');
	PutUHexZTo(jtEntry->address, 8, 34);

	PutLine();

}

// This is to make the linker happy. If I add StdCLib.o, then the BuildDcmd complains that
// there are too many entry points...

long strlen(char *s) {

	long len;
	
	len = 0;
	
	while (*s++) len++;
	
	return(len);

}
