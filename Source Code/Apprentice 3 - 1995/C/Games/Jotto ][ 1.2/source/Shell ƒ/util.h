#define Mymemcpy(a,b,c)			BlockMove(b, a, c);

void Mymemset(Ptr output, unsigned char value, unsigned long len);
Boolean Mymemcompare(Ptr thisThing, Ptr thatThing, unsigned long len);
