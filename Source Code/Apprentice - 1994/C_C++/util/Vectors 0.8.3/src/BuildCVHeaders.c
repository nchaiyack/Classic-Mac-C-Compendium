/* Use this file to precompile your headers in MPW C */
/* Putting this in a source file (bracketed by #if's for conditional building) */
/* is causing the C compiler to choke, so I put it in a seperate file */
/* If you get a bomb box when you are precompiling headers, that means that the */
/* C compiler is running out of memory when it does the dump.  Just increase the  */
/* size of your MPW partition (3 meg is good) and it'll work fine.   */
/* You can then reduce the size of your MPW partition. */
#define __REDUMP__
#include <Types.h>
#include <Quickdraw.h>
#include <Errors.h>
#include <Fonts.h>
#include <Events.h>
#include <Controls.h>
#include <Windows.h>
#include <Menus.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <Memory.h>
#include <Folders.h>
#include <OSUtils.h>
#include <SegLoad.h>
#include <Files.h>
#include <StandardFile.h>
#include <ToolUtils.h>
#include <OSEvents.h>
#include <Resources.h>
#include <Script.h>
#include <StdLib.h>
#include <String.h>
#include <Strings.h>
#include <Packages.h>
#include <Desk.h>
#include <QDOffscreen.h>
#include <CType.h>
#include <stdio.h>
#include <Printing.h>
#include <Palettes.h>
#include <Time.h>
#include <Math.h>
#include <Lists.h>
#include <SysEqu.h>
#include <GestaltEqu.h>
#include <Printing.h>
#include <AppleEvents.h>

#include <TE32K.h>
#include <ACurs.h>

#include "CVectors.h"

#undef __REDUMP__
#pragma dump "CVHeaders"

