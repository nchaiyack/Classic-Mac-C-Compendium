Boolean CustomGet(FSSpec *fSpec,
				void (*DoUpdate)(EventRecord *),
				void (*DoActivate)(EventRecord *),
				short SFDlgID, short SelectStrRsrc/*, short DeskStrRsrc*/);
