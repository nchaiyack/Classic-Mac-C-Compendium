#pragma once

#include "ppp.h"
#include <GestaltEqu.h>
#include <CommResources.h>
#include <CRMSerialDevices.h>
#include <Folders.h>
#include <Menus.h>
#include <ToolUtils.h>



pascal OSErr pppopen (void);
pascal OSErr pppclose(short hard);
pascal short pppup(void);
void bzero(b_8 *ptr, short cnt);
