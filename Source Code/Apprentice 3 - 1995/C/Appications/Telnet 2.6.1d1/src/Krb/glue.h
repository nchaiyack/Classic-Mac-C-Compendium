#pragma parameter __D0 getA4
long getA4(void);
long getA4 () = {
	0x200c					/* move.l a4, d0 */
};

#pragma parameter __D0 swapA4(__D0)
long swapA4(long);
long swapA4 () = {
	0xc18c					/* exg d0, a4 */
};

#pragma parameter setD0(__D0)
void setD0(long);
void setD0 () = {
	0x4e71					/* nop */
};
