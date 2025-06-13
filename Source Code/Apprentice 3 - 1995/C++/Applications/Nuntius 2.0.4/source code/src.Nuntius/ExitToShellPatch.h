// Copyright © 1994-1995 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// ExitToShellPatch.h

typedef void (*ExitProcFunc)();

void MakeExitToShellPatch(ExitProcFunc exitProc);
void ExecuteExitToShellPatches();
