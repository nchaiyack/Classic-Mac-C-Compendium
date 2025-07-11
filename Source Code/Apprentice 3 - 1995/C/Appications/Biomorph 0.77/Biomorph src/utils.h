#ifndef UTILS_H
#define UTILS_H

void Error( int s1, int s2, int s3, int s4, int severity);
void ReadCoords(void);
void WriteCoords(void);
void UserItem(DialogPtr theDialog,short itemNum,void *theProc);
pascal void BoxItem(WindowPtr theWindow,short itemNum);
pascal void OutlineButton(DialogPtr theDialog, short itemNum);
void Print(char *s);
MathType dabs(MathType x);


#endif