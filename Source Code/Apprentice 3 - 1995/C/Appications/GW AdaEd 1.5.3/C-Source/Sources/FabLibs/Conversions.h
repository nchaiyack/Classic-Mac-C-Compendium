Ptr ShortToHexText(short n, Ptr text);
void ShortToHexString(short n, Str255);
void ShortToOctString(short, Str255);
void ShortToBinString(short n, Str255 out);
void HexStringToShort(ConstStr255Param in, short *p);
void OctStringToShort(ConstStr255Param in, short *p);
void BinStringToShort(ConstStr255Param in, short *p);
/*
pascal Ptr ShortToHexText(short, Ptr);
pascal void ShortToHexString(short, Str255);
pascal void ShortToOctString(short, Str255);
pascal void ShortToBinString(short num, Str255 result);
pascal void HexStringToShort(ConstStr255Param, short *);
pascal void OctStringToShort(ConstStr255Param, short *);
pascal void BinStringToShort(ConstStr255Param, short *);
*/
