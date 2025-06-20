#ifndef __MY_TEXT_SEARCH_H__
#define __MY_TEXT_SEARCH_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	void SetLastFindPosition(short val);
extern	short SearchForwards(TEHandle hTE, short *foundLength);
extern	Boolean DoFindDialog(void);
extern	Boolean AnythingToFindQQ(void);
extern	Boolean AnythingToReplaceQQ(void);
extern	Boolean DoFindAgain(void);
extern	void DoReplace(void);
extern	void DoReplaceAll(void);
extern	void DoEnterString(Boolean isFindString);

#ifdef __cplusplus
}
#endif

#endif
