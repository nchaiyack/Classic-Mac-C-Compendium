#ifndef __HGR_FX_H__
#define __HGR_FX_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	void HGRFade(Rect *boundsRect);
extern	void HGRReversed(GrafPtr sourceGrafPtr, GrafPtr destGrafPtr, Rect *boundsRect);

#ifdef __cplusplus
}
#endif

#endif
