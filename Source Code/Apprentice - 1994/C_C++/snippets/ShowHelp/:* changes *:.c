#if justacomment

2.0
---

Eliminated use of A4-based globals, in favor of tacking more information
onto the dialog record.

Made it possible to switch to other applications or use the application
or help menu, if the dialog is not of type dBoxProc. Made it possible
for the dialog to be moved, if it has a drag region.  Added the callback
for window updating to the Show_help API.  Added Fake_ModalDialog().



1.8 (unreleased, but used in Carpetbag 1.2.2)
---

Changed programmatic interface.  Now a 'Hlp?' resource contains most of
the information that was formerly being passed as parameters.

In Save_text, remove base_pict_id parameter, and get pict information
from the array tacked onto the dialog record.

PICTS can now be compressed.

In Scroll_text, changed a parameter declaration from int to short.
This prevents problems for those using 4-byte ints.

Disposed of the handle to the deferred highlight array, which was
formerly a memory leak.

Saved and restored the GrafPort around the SFPutFile call in Save_text.

Removed LoadResource call from Draw_picts, since the PicHandles may now
be the results of decompression, not resources.

Eliminated High_hook_glue routine, by modifying High_hook.


1.7
---

Changed handling of A4 in the main routine.


1.6
---

Added the Push_highlight and Pop_highlight routines, and made the
array of deferred highlights into a handle that can be resized at need.
This was to fix a crash that occurred when drag-selecting under
Japanese system software.

#endif
