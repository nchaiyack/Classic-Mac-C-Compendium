#ifndef __MY_MEMORY_LAYER_H__
#define __MY_MEMORY_LAYER_H__

#define Mymemcpy(a,b,c)			BlockMove(b, a, c);

#ifdef __cplusplus
extern "C" {
#endif

extern	void Mymemset(Ptr output, unsigned char value, unsigned long len);
extern	Boolean Mymemcompare(Ptr thisThing, Ptr thatThing, unsigned long len);
extern	void AppendStr255(Str255 theStr, Str255 appendStr);
extern	void AppendStr255ToHandle(Handle theHandle, Str255 theStr);

#ifdef __cplusplus
}
#endif

#endif
