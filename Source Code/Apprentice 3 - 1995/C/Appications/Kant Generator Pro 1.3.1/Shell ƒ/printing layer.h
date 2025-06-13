#ifndef __MY_PRINTING_LAYER_H__
#define __MY_PRINTING_LAYER_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	void InitThePrinting(void);
extern	void DoThePageSetup(void);
extern	void PrintText(TEHandle theText);

#ifdef __cplusplus
}
#endif

#endif
