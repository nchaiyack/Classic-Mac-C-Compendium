/**

	init.error.CODE.c
	Copyright (c) 1990, joe holt

 **/


/**

	Compile this code in a separate project.  Set the project type to
	Code Resource, type 'RESP' ID 128.  Once compiled, you can build it
	and merge it into your INIT's resource file ("...project.rsrc") or
	build it into a separate file and use ResEdit to copy it in.  This
	code is independent of the INIT, so it can be used as is for any
	INIT you write.

 **/


/**

	ToolScratch is an 8-byte area of low memory used by the Mac toolbox
	and other people as a temporary holding place.

 **/

extern long		ToolScratch : 0x09CE;


/**************************************************************************
 ***
 *** void main ( QElemPtr nmReqPtr );
 ***
 *** This is the code resource type 'RESP' which you need to compile
 *** separately and include among your INIT's resources.
 ***
 *** This code is called when the user closes the Notification Manager's
 *** note dialog.  The NM passes to us the address of the NM record which
 *** was set up by StartupError().  We use this to clean up and leave.
 ***
 *** This is written is assembly for size.  If you have problems with
 *** assembly, I s'pose it could be written in C, but the trick at the
 *** end would be hard to duplicate...
 ***
 *** History:
 ***   jhh 18 jun 90 -- response to news posting
 ***
 ***/

pascal void
main( QElemPtr nmReqPtr )
{
	asm {

/**

	First remove the note from the Notification Manager's notification
	queue.

 **/

			move.l		nmReqPtr, A0
			_NMRemove

/**

	Grab the string's address from the NM rec and dispose of it.  Then
	get rid of the NM rec itself.

 **/

			move.l		nmReqPtr, A0
			move.l		OFFSET(NMRec,nmStr)(A0), A0
			_DisposPtr
			move.l		nmReqPtr, A0
			_DisposPtr

/**

	Now the tricky part.  This code lives within a small block in the
	System Heap which we want to get rid of.  But it's not safe to
	dispose of the very block you're calling from!  So, we create a
	little bit of code in ToolScratch which does the dispose for us
	and execute it last.

 **/

			move.l		4(A7), A1
			move.l		#ToolScratch, A0
			move.l		A0, 4(A7)
			move.l		#0x2040A01F, (A0)	; movea.l D0, A0  /   _DisposPtr
			move.w		#0x4ED1, 4(A0)		; jmp (A1)
			lea			main, A0
			move.l		A0, D0				; Pascal clobbers A0 when returning
	}
}
