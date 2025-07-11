#include <CType.h>
#include <Types.h>
#include <stdio.h>
#include <stream.h>
#include <CursorCtl.h>
#include <StdLib.h>
#include <FCntl.h>

long headPos = 0, bodyPos = 0;
long curPos = 0;
char s[1000], ucs[1000];
char* s2;
Boolean inHead = false, inBody = false;

void FlushBody(ostream& outfile)
{
	if (inBody) {
		outfile << " " << dec(curPos - bodyPos, 10);
		outfile << " " << dec(headPos, 10);
		outfile << " " << dec(curPos - headPos, 10) << "\n";
		inBody = false;
	}
}

int CheckOpen(int fd)
{
	if (fd == 0) {
		cerr << "\nError opening or creating a file\n";
		exit(1);
	}
	return fd;
}

int main()
{
	InitCursorCtl(nil);
	
	int fdArticles	= CheckOpen(open ("articles", 0));
	int fdIndex			= CheckOpen(creat("article_index"));
	int fdFrom			= CheckOpen(creat("qFrom"));
	int fdSubject		= CheckOpen(creat("qSubject"));
	int fdMsgID			= CheckOpen(creat("qMessage-ID"));
	int fdRefs			= CheckOpen(creat("qReferences"));
	int fdRange			= CheckOpen(creat("qRange"));
	
	char articlesBuf[20000], indexBuf[2000];
	char fromBuf[2000], subjectBuf[2000], msgIDBuf[2000], refsBuf[2000];
	char rangeBuf[256];

	filebuf fbArticles	(fdArticles,	articlesBuf,	sizeof(articlesBuf));
	filebuf fbIndex			(fdIndex,			indexBuf,			sizeof(indexBuf));
	filebuf fbFrom			(fdFrom,			fromBuf,			sizeof(fromBuf));
	filebuf fbSubject		(fdSubject,		subjectBuf,		sizeof(subjectBuf));
	filebuf fbMsgID			(fdMsgID,			msgIDBuf,			sizeof(msgIDBuf));
	filebuf fbRefs			(fdRefs,			refsBuf,			sizeof(refsBuf));
	filebuf fbRange			(fdRange,			rangeBuf,			sizeof(rangeBuf));

	istream fileArticles(&fbArticles);
	ostream fileIndex		(&fbIndex);
	ostream fileFrom		(&fbFrom);
	ostream fileSubject	(&fbSubject);
	ostream fileMsgID		(&fbMsgID);
	ostream fileRefs		(&fbRefs);
	ostream fileRange		(&fbRange);

	Boolean gotFirstId = false;
	Boolean gotFrom = false, gotSubject = false, gotMsgID = false, gotRefs = false;
	char IDstr[10];
	char firstIDStr[10];
	long lastID = -1;
	int i;
	while (!fileArticles.eof()) {
		SpinCursor(1);
		fileArticles.getline(s, 900);
		if (s[0] == '@' && strncmp(s, "@@@@ /usr/spool", 15) == 0) {
			gotFrom = false, gotSubject = false, gotMsgID = false, gotRefs = false;
			for (i = strlen(s); s[i - 1] != ' '; i--);
			strcpy(IDstr, s + i);
			if (!gotFirstId) {
				fileRange << IDstr << " ";
				strcpy(firstIDStr, IDstr);
				gotFirstId = true;
			}
			FlushBody(fileIndex);
			long ID;
			sscanf(IDstr, "%ld", &ID);
			if (lastID != -1) {
				while (++lastID < ID) {
					char lastIDstr[200];
					sprintf(lastIDstr, "%ld", lastID);
					fileIndex << dec(lastID, 10) << "          0          0          0          0          0          0\n";
					fileFrom << lastIDstr << " (none)\n";
					fileSubject << lastIDstr << " (none)\n";
					fileMsgID << lastIDstr << " (none)\n";
					fileRefs << lastIDstr << " (none)\n";
				}
			}
			lastID = ID;
			inHead = true;
			s2 = s+(strlen(s)-1);
			while (*(s2-1) != ' ') s2--;
			headPos = curPos + strlen(s) + 1; // after @@@@ line, start with Path:
			for (i = strlen(s2) + 1; i <= 10; i++) fileIndex << " ";
			fileIndex << s2 << " " << dec(headPos, 10);
		}
		else if (inHead) {
			strcpy(ucs, s);
			for (i = 0; ucs[i] != 0; i++) ucs[i] = toupper(ucs[i]);
			if (strlen(s) == 0) {
				fileIndex << " " << dec(curPos - headPos, 10) << " " << dec(curPos + 1, 10);
				inHead = false;
				inBody = true;
				bodyPos = curPos;
				if (!gotFrom)    fileFrom			<< IDstr << " (none)\n";
				if (!gotSubject) fileSubject	<< IDstr << " (none)\n";
				if (!gotMsgID)   fileMsgID		<< IDstr << " (none)\n";
				if (!gotRefs)    fileRefs			<< IDstr << " (none)\n";
			} else if (strncmp(ucs, "FROM: ", 6) == 0) {
				fileFrom << IDstr << " " << s + 6 << "\n";
				gotFrom = true;
			} else if (strncmp(ucs, "SUBJECT: ", 9) == 0) {
				fileSubject << IDstr << " " << s + 9 << "\n";
				gotSubject = true;
			} else if (strncmp(ucs, "MESSAGE-ID: ", 12) == 0) {
				fileMsgID << IDstr << " " << s + 12 << "\n";
				gotMsgID = true;
			} else if (strncmp(ucs, "REFERENCES: ", 12) == 0) {
				fileRefs << IDstr << " " << s + 12 << "\n";
				gotRefs = true;
			}
		}
		else if (inBody) {
		}
		curPos += strlen(s) + 1;
	}
	curPos--; // dette er n�dvendigt for at kunne l�se sidste artikkel, desv�rre!
	FlushBody(fileIndex);
	if (gotFirstId)
	{
		fileRange << IDstr << " ";
		fileRange << firstIDStr << " " << IDstr << "\n";
	}
	else
		fileRange << "1 0 1 0\n";
	return 0;
}
