// a short program to just hang
ProcPtr ResetVector;

void rebootMe ()
{
   asm { movea.l  ResetVector,a3
		 jmp (a3) } 
		
        
}

main()
{ 
	ResetVector = ROMBase + 10;
	rebootMe(); 
}
