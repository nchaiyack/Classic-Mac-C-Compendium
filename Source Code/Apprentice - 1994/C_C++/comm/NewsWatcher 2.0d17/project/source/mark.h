Boolean FindParentCell (WindowPtr wind, short index, Cell *theCell);
void AppendUnreadRange (long first, long last, TGroup *theGroup);
void AdjustUnreadList (TGroup *theGroup);
void UpdateUnreadList (WindowPtr wind);
void MarkAllSubjects (WindowPtr wind, Boolean read);
void MarkArticle (WindowPtr wind, Boolean read);
void MarkThread (WindowPtr wind, short threadHeadIndex, Boolean read);
void DoMarkCommand (WindowPtr wind, Boolean read);
