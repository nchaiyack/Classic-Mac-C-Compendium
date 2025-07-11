/*
 * File: rwinrsc.r
 *  Icon graphics interface resources
 *
 * Resources are allocated through a layer of internal management
 * routines in order to handle aliasing and resource sharing.
 */
#ifdef Graphics

/*
 * global variables.
 */

wcp wcntxts = NULL;
wsp wstates = NULL;
wbp wbndngs = NULL;
int win_highwater = -1;

#ifdef XWindows
#include "rxrsc.ri"
#endif					/* XWindows */

#ifdef PresentationManager
#include "rpmrsc.ri"
#endif					/* PresentationManager */

/*
 * allocate a window binding structure
 */
wbp alc_wbinding()
   {
   int i;
   wbp w;

   GRFX_ALLOC(w, _wbinding);
   GRFX_LINK(w, wbndngs);
   return w;
   }

/*
 * free a window binding.
 */
novalue free_binding(w)
wbp w;
   {
   w->refcount--;
   if(w->refcount == 0) {
      if (w->window) free_window(w->window);
      if (w->context) free_context(w->context);
      GRFX_UNLINK(w, wbndngs);
      }
   }

#else					/* Graphics */
static char x;			/* avoid empty module */
#endif					/* Graphics */
