struct AddressRange {UPtr start, end;};		/* location of CODE in memory */

long UnloadUnneeded(long needed);	/* unload code segments not ref'ed from stack */
