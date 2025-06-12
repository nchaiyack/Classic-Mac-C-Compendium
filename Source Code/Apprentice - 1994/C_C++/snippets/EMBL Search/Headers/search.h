
/* search.c */
HitmapHdl DoSearch(short what, StringPtr key, short dbcode);
Boolean FindHits(char *key, short dbcode, HitmapHdl hitmapHdl, u_short target_recsize, StringPtr targetFName, u_long target_nrec, short (*compare )(void *, void *), StringPtr hitFName, short maxkeylen);
Boolean FindEntryname(char *key, short dbcode, HitmapHdl hitmapHdl);
short ename_compare(void *key, void *rec);
short general_compare(void *key, void *rec);
