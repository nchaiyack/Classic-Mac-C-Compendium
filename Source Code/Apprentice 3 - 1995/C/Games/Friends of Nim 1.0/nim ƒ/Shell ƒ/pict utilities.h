#ifndef __MY_PICT_UTILS_H__
#define __MY_PICT_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

extern	PicHandle DrawThePicture(PicHandle thePict, short whichPict, short x, short y);
extern	PicHandle ReleaseThePict(PicHandle thePict);
extern	Boolean DrawPictureCentered(PicHandle thePict, Rect boundsRect, Rect *destRect);

#ifdef __cplusplus
}
#endif

#endif
