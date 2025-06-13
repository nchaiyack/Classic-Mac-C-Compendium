ICError ICMDeleteEntry (Handle entries, long pos);
ICError	ICMCountEntries (Handle entries, long *count);
ICError ICMGetEntry (Handle entries, long pos, ICMapEntry *entry);
ICError	ICMGetIndEntry (Handle entries, long ndx, long *pos, ICMapEntry *entry);
ICError ICMAddEntry (Handle entries, ICMapEntry *entry);
ICError ICMSetEntry (Handle entries, long pos, ICMapEntry *entry);
