void foo(void);

void foo(void)
{
	asm {
		moveA.L	@1, A0
		jmp		(A0)
	@1:	DC.L	0
	}
}