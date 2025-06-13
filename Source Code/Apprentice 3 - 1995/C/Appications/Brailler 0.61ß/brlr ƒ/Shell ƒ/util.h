#define Mymemcpy(a,b,c)			BlockMove(b, a, c);

void Mymemset(Ptr output, unsigned char value, unsigned long len);
Boolean Mymemcompare(Ptr thisThing, Ptr thatThing, unsigned long len);
void AppendStr255(Str255 theStr, Str255 appendStr);
void AppendStr255ToHandle(Handle theHandle, Str255 theStr);
