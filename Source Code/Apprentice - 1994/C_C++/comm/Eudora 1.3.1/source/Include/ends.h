/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
void Initialize(void);
void FigureOutFont(void);
void Cleanup(void);
void BuildBoxMenus(void);
void RememberOpenWindows(void);
void RecallOpenWindows(void);
void TrashMenu(MenuHandle mh, short beginAtItem);
void OpenNewSettings(short vRef,long dirId,UPtr name);
void SetSendQueue(void);
void GetBoxLines(void);
