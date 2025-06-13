// Copyright © 1992-1995 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// Nuntius.r

#ifndef qNoCode
#define qNoCode 0
#endif

#ifndef qPowerPC
#define qPowerPC 0
#endif

#if !qNoCode && !qPowerPC
// PowerPC does not make CODE
include "Nuntius" 'CODE';
#endif

#if !qNoCode
// MetroWerks includes in Project file
include "Nuntius.ResEdit";
#endif

include "Nuntius.AdLib" not 'ALIB';
include "Misc.rsrc";
include "Strings.rsrc";
include "faq.rsrc";
