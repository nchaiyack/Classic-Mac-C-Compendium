UHandle  ExpandAliases(UHandle fromH,short depth,Boolean wantExpansion);
long FindAliasFor(UHandle aliases,UPtr name,short size);
long FindExpansionFor(UHandle aliases,UPtr name,short size);
void FinishAlias(MyWindowPtr win, Boolean wantExpansion, Boolean findOnly);
void InsertAlias(MyWindowPtr win, long foundOffset,Boolean wantExpansion);
