/* DocDialogs - Implementation of the configuration dialogs                   */

#ifndef __DOCDIALOGS__
#define __DOCDIALOGS__

#ifndef __MUDDOC__
#include "MUDDoc.h"
#endif


pascal void Communication (TMUDDoc *theDoc);
pascal void ConfigTCP (TMUDDoc *theDoc);
pascal void Preferences (TMUDDoc *theDoc);
pascal void Setup (TMUDDoc *theDoc);

pascal void InitDocDialogs (void);

#endif
