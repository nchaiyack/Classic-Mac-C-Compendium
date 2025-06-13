// Copyright © 1992-1995 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// ProcessTools.h

#define __PROCESSTOOLS__

void LaunchApplication(FSSpec &applSpec, Boolean toFront);
void OpenApplicationDocument(FSSpec &applSpec, AliasHandle aliasH, Boolean toFront);

void TellRunningApplicationToOpenDocument(const ProcessSerialNumber &psn, AliasHandle aliasH, Boolean toFront);

void TellFinderToOpenDocument(AliasHandle aliasH);
void TellFinderToOpenDocument(const FSSpec &spec);