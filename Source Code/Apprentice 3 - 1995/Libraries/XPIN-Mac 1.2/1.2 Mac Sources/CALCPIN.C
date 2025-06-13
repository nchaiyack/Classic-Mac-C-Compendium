/*
  @Project: The cross-platform plug-in toolkit (XPIN).
  @Project-Version: 1.2
  @Project-Date: December 1992.
  @Filename: CALCPIN.C

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
  See DLOGPIN.C for full comments. This one has the comments inside
  stripped out so you can see the basic structure is...
  You can copy this source over to a Windows machine and compile it
  with MSC or Borland C and it will behave exactly the same way...
  @End-Description.
 */

#include "XPIN.h"

#ifdef OS_MAC
#include <Dialogs.h>
#endif

XPIN(xblk)
{
   DescribeXPIN(xblk, "Calculate","Description of Calculate", 1, 0);

#ifdef OS_WIN
   MessageBox(NULL, "I can Add, Multiply, Divide, and Conquer.",
		"Calculated Message",
		MB_ICONSTOP | MB_OK);
#elif OS_MAC
	ParamText("\pI can Add, Multiply, Divide, and Conquer!", NIL, NIL, NIL);
	Alert(128, NIL);
#endif

  XSETSTATw(xblk, 0);	// Return a status back to the caller here

EndXPIN();
}
