/* Stack6000 is a simple, very simple test for 6000 as the default stack size. 
 			 If it's smaller, it's set to $6000  */

#define DfltStack *(long *)(0x0322) 
#define GOODSIZE 0x6000

pascal void ShowINIT(short iconID, short moveX);

main()
{
/* save all system registers */
	asm {
		movem.l	a1-a5/d0-d7, -(SP)
		}
		
	if(DfltStack<GOODSIZE) {
		DfltStack = GOODSIZE;
		ShowINIT(128,-1); /* did it */
		}
	else ShowINIT(129,-1); /* already there */
	
	asm {
		movem.l	(SP)+, a1-a5/d0-d7		; restore registers
		}

}
