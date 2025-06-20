
/* hitstorage.c */
Boolean FillDEBuffer(ResultHdl resHdl, short first, Boolean bCache);
Boolean NewHitlist(HitmapHdl hitmapHdl, short dbcode, HitlistHdl *new, short *nhits);
Boolean InitResultRec(ResultHdl *new, short dbcode, HitmapHdl hitmapHdl, QueryHdl queryHdl);
void DisposeResRec(ResultHdl resRecHdl);
Boolean GetSelectState(HitlistHdl hlHdl, short pos);
void SetSelectState(HitlistHdl hlHdl, short pos, Boolean state);
Boolean NewHitmap(HitmapHdl *new, short dbcode);
void OrHitmaps(HitmapHdl hmHdl1, HitmapHdl hmHdl2);
void AndHitmaps(HitmapHdl hmHdl1, HitmapHdl hmHdl2);
void NotHitmap(HitmapHdl hmHdl);
