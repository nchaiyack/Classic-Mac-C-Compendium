short FindGroupIndex (const char *name);
Boolean	ReadGroupsFromPrefs (short fRefNum);
void MakeGroupList (short numGroups, ListHandle theList);
void CreateNewGroupListWindow (void);
Boolean CreateFullGroupListWindow (void);
Boolean	CheckForNewGroups (void);
void CheckForDeletedGroups (void);
Boolean ReadGroupsFromServer (void);
