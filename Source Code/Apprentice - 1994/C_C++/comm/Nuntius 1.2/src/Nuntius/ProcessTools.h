// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// ProcessTools.h

#define __PROCESSTOOLS__

void LaunchApplication(FSSpec &applSpec, Boolean toFront);
void OpenApplicationDocument(FSSpec &applSpec, AliasHandle aliasH, Boolean toFront);
