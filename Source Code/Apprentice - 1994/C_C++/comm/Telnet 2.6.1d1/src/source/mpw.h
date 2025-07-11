// This file basically serves the function that MacHeaders does under Think C
		#include <Controls.h>
		#include <Desk.h>
		#include <Devices.h>
		#include <Dialogs.h>
		#include <DiskInit.h>
		#include <Errors.h>
		#include <Events.h>
		#include <Files.h>
		#include <Fonts.h>
		#include <Lists.h>
		#include <Memory.h>
		#include <Menus.h>
		#include <Notification.h>
		#include <OSEvents.h>
		#include <OSUtils.h>
		#include <Packages.h>
		#include <Quickdraw.h>
		#include <Resources.h>
		#include <Scrap.h>
		#include <Script.h>
		#include <SegLoad.h>
		#include <StandardFile.h>
		#include <String.h>
		#include <Strings.h>
		#include <TextEdit.h>
		#include <Timer.h>
		#include <ToolUtils.h>
		#include <Types.h>
		#include <Windows.h>
		
	/*  Rect macros  */

#define topLeft(r)		(((Point *) &(r))[0])
#define botRight(r)		(((Point *) &(r))[1])

#define CtoPstr c2pstr
#define PtoCstr p2cstr
