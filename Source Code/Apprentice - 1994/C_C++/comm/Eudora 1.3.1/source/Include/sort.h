/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
void QuickSort(UPtr array, int size, int f, int l, int (*compare)(), void (*swap)());
int StrCompar(UPtr s1, UPtr s2);
void StrSwap(UPtr s1, UPtr s2);
int CStrCompar(UPtr s1, UPtr s2);
void CStrSwap(UPtr s1, UPtr s2);
