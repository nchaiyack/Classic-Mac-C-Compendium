/* DMHelper.r - Defines all the resources needed for DMHelper                 */

/* ¥ Auto-Include the requirements for this source */
#ifndef __TYPES.R__
#include "Types.r"
#endif

#ifndef __MacAppTypes__
#include "MacAppTypes.r"
#endif


// Get the application's CODE.
include $$Shell("ObjApp")"MUDDweller" 'CODE';


// Basic resources for various purposes.
include "MacApp.rsrc";
include "Dialog.rsrc";
include "Printing.rsrc";

#if qDebug
include "Debug.rsrc";
#endif


// Include our own resources

include "Alerts.rsrc";
include "Finder.rsrc";
include "Strings.rsrc";
include "Menus.rsrc";
include "Views.rsrc" 'view';
include "USizerView.rsrc";
