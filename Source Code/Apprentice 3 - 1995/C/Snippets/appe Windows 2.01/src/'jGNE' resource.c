// File "jgnefilter.c" -

// Should the Inline jGNEFilter use MW's FRALLOC macros?
#define ____USE_FRALLOC____

// ***********************************************************************************
// Function Prototype

asm void main(void);

// ***********************************************************************************
// ***********************************************************************************

asm void main() {
	bra.s	Continue
		
Next_Filter:
	dc.l	0						// Saved Address of Next jGNEFilter in the chain.
									//   We jump directly to it, no JSR's or RTS's.
Event_Helper:
	dc.l	0						// Pointer to Helper function in our application.
									//   We clear it to NIL when we quit as a flag
Event_Helper_Data:
	dc.l	0						// Promised storage for the Helper function, 
									//   which can modify the pointer dynamically.
Continue:

// FRALLOC is a MW macro that simplifies ASM local vars and
//   stack maintenance -- use it if we got it, its very easy
#ifdef ____USE_FRALLOC____
	fralloc
#else
	link	a6, #0
#endif ____USE_FRALLOC____

	// Save the Volatile registers for safety
	movem.l	d0-d2/a0-a2, -(a7)			

	// Load the Helper from Inline Storage and test it. If Helper is NIL,
	//   then our handler was released -- and we just jump to the next Filter
	move.l	Event_Helper, a0
	move.l	a0, d0
	tst.l	d0							
	beq		End_Filter
	
	// Straight C Calling Conventions, call the Helper function
	move.l	Event_Helper_Data, -(a7)
	move.l	a1, -(a7)
	move.l	Event_Helper, a0
	jsr		(a0)
	add.l	#8, a7
		
End_Filter:

	// Clean up the same way we got here
	movem.l	(a7)+, d0-d2/a0-a2

#ifdef ____USE_FRALLOC____
	frfree
#else
	unlk	a6
#endif ____USE_FRALLOC____
	
	// Jump to the next filter in the chain
	move.l	Next_Filter, a0
	jmp		(a0)
	}

