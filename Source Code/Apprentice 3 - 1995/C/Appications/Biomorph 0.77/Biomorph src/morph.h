#ifndef MORPH_H
#define MORPH_H

typedef void morphfunction(ImagPt *Zold, ImagPt *C, ImagPt *Znew);
typedef morphfunction	*MorphProcP;  // ptr to the function.
typedef MorphProcP		*MorphProcH;  // handle to the function

#endif