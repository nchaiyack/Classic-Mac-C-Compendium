#include "program globals.h"

enum ErrorTypes GetComputerWord(void);
Boolean CheckDup(void);
enum ErrorTypes GetHumanWord(char *thisWord, unsigned long index);
Boolean GetCustomWord(char *thisWord, unsigned long index);
enum ErrorTypes SaveCustomWordToDisk(char *thisWord);
enum ErrorTypes OpenTheFiles(void);
void CloseTheFiles(void);
