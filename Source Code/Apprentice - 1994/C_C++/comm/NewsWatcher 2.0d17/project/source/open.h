void CalcWindowHCoords (WindowPtr wind);
void CalcPanelHeight (WindowPtr wind);
WindowPtr MakeNewWindow (EWindowKind kind, Point topLeft, StringPtr title);
WindowPtr NewUserGroupWindow (StringPtr title, TGroup **groupArray, short numGroups);
