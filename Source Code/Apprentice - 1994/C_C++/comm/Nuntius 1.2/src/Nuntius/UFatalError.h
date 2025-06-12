// Copyright © 1993 Peter Speck (speck@dat.ruc.dk).  All rights reserved.
// UFatalError.h

const long kPanicBufferSize = 1024;
extern char gPanicBuffer[];

void ReportFatalError(const char *fatalMsg);
void PanicExitToShell(const char *fatalMsg);

void InitUFatalError();