/* StandardFile.h */

#pragma once

MyBoolean	FGetFile(FSSpec* FileInfo, pascal Boolean (*FileFilter)(CInfoPBRec* pb),
						OSType TypeList[4], short NumTypes);
MyBoolean	FPutFile(PString DefaultFileName, FSSpec* FileInfo, MyBoolean* Replacing);
