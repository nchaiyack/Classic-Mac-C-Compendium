/*
  @Project: The cross-platform plug-in toolkit (XPIN).
  @Project-Version: 1.2
  @Project-Date: December 1992.
  @Filename: DLOGPIN.C

  @Author: Ramin Firoozye - rp&A Inc.
  @Copyright: (c) 1992, rp&A Inc. - San Francisco, CA.
  @License.
  The portable plug-in toolkit source and library code is
  presented without any warranty or assumption of responsibility
  from rp&A Inc. and the author. You can not sell this toolkit in
  source or library form as a standalone product without express
  permission of rp&A Inc. You are, however, encouraged to use this
  code in your personal or commercial application without any
  royalties. You can modify the sources as long as the license
  and credits are included with the source.

  For bugs, updates, thanks, or suggestions you can contact the
  author at:
	CompuServe: 70751,252
	Internet: rpa@netcom.com

  @End-License.

  @Description.
  This is a sample plug-in that puts up a simple dialog. It is
  kept simple to help demonstrate how an XPIN is built. You can,
  of course, give yours dripping fangs...
  @End-Description.
 */

#include "XPIN.h"
#ifdef OS_MAC
#include <QuickDraw.h>
#include <Dialogs.h>
#endif
/*
  * This is the standard plug-in function declaration.
  * The "xblk" is the variable name you want to use to a pass
  * parameters back and forth between the main application and
  * the xtension. xblk is a pointer to an XBlock (defined in XPINSHR.H).
  * You can use the XSETx and XGETx macros to get at the values in the
  * parameter block. To return a result, you can use the XSETSTATx
  * macro.
  */
XPIN(xblk)
{
int localVarsGoHere;
LPSTR	label, desc;
/*
  * This section is required for each plug-in. The first string is
  * the 32 character label you want assigned to this function.
  * The XPINLabel function returns this value. The second string
  * is a description string that you can obtain in the main app
  * by calling the XPINDesc function. The last two numbers are the
  * major and minor version numbers assigned to this plug-in. You can
  * use them in any way you want. The current version of the toolkit
  * doesn't use them (but will).
  *
  * NOTE: Generally, local variables should go before the DescribeXPIN
  * macro. You should avoid calling DescribeXPIN more than
  * once inside the app.
  * The "xblk" value is a variable you can name. Just remember to use
  * whatever name you choose whenever you use the XSET#/XGET# macros.
  */

 DescribeXPIN(xblk, "Dialog","Description of Dialog", 1, 0);

// *********** This is the actual body of code... ************

#ifdef OS_WIN 
     MessageBox(NULL, "This is a sample dialog box...",
		"Dialog Plug-in Message",
		MB_ICONSTOP | MB_YESNO);
#elif OS_MAC
		ParamText("\pCool!", NIL, NIL, NIL);
		Alert(128, NIL);
#endif

// You can return a status value back to the caller this way.
     XSETSTATw(xblk, 0);

// *********** End of main body of XPIN ***********************

/*
  The EndXPIN() macro HAS to be put at the very end. You can also use
  ReturnXPIN() if you need to return before you hit the end point.
  If you forget it, your application WILL DIE. Maybe not immediately,
  but soon.
*/

EndXPIN();
}
