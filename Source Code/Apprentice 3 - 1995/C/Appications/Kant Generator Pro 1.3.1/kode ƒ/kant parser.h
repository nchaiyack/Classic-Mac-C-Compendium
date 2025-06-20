#ifndef __KANT_PARSER_H__
#define __KANT_PARSER_H__

enum ParserError
{
	kNoError=0, kNoReferenceName, kBadReference, kBadChoiceFormat, kBadOptionalFormat,
		kNoInstantiations, kBadBackslash, kBadProbabilityFormat, kUserCancelled
};

typedef short		ParserError;

#ifdef __cplusplus
extern "C" {
#endif

extern	void InitTheParser(WindowRef theWindow, short rawStart, short rawEnd);
extern	ParserError ParseLoop(void);

#ifdef __cplusplus
}
#endif

#endif
