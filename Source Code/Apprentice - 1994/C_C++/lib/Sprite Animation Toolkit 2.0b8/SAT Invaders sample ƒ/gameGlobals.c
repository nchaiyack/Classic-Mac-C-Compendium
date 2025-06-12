//¥ C translation from Pascal source file: gameGlobals.p

//¥ =================================================.
//¥ ========= GameGlobals, globals for SATInvaders ==========.
//¥ =================================================.

//¥ Example file for Ingemars Sprite Animation Toolkit.
//¥ © Ingemar Ragnemalm 1992.
//¥ See doc files for legal terms for using this code.

//¥ This file defines the resource numbers and global variables for SATInvaders.

//¥ GameGlobals;

//¥ Prototypes, etc.

#include <SAT.h>
#include "InvadeSAT.h"


//¥ Menu handles.  There isn't any apple menu here, since TransSkel will.
//¥ be told to handle it itself.

MenuHandle fileMenu;

long level;
 
Boolean stillRunning;

//Point globalSpeed;
//Boolean turnFlag;
