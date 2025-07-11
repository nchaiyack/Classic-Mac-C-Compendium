#include <stream.h>
#include <stdio.h>
#include <stdlib.h>
#include <Types.h>
#include <CursorCtl.h>

const short maxNoFiles = 50;
const short maxNameLen = 40;
char nameTable[maxNoFiles+1][maxNameLen+1];
short noFiles;
short gErr = 0;
Boolean fileHasLF = false;

Boolean progress = false;
Boolean verbose = false;
Boolean vverbose = false;

char my_spaces[100];
char *SP(int x)
{
	char *p = my_spaces;
	if (x > 40)
	{
		x = 40;
		*p++ = '.';
	}
	for (;x;--x)
		*p++ = ' ';
	*p = 0;
	return my_spaces;
}

const char *PP(const char *p)	// skip leading ':' in PartialPaths
{
	if (*p == ':')
		return p + 1;
	else
		return p;
}

const char *FN(const char *p)	// find filename in partial/full path
{
	const char *pp = p;
	while (*pp)
	{
		if (*pp++ == ':')
			p = pp;
	}
	return p;
}

int partcmp(char* line, char* substr)
{
	while (*substr)
		if (*line++ != *substr++)
			return 1;
	return 0;
}

int OpenFile(char *filename, char *partial_path, filebuf &fb)
{
	strcpy(partial_path, filename);
	if (fb.open(filename, input) != 0)
		return 1;
	strcpy(partial_path, ":ES:");
	strcat(partial_path, PP(filename));
	if (fb.open(partial_path, input) != 0)
		return 1;
	return 0;
}

void ProcessFile(char *filename, int indent)
{
	RotateCursor(0);
	if (noFiles >= maxNoFiles)
	{
		cerr << "Too many included files: table overflow\n";
		exit(1);
	}
	char partial_path[400];
	filebuf fb;
	if (!OpenFile(filename, partial_path, fb))
	{
		cerr << "Error opening file: " << filename << "\n";
		gErr = 1;
		return;
	}
	short i;
	for (i = 0; i < noFiles; i++)
	{
		if (strcmp(partial_path, nameTable[i]) == 0)
		{
			if (verbose)
				cerr << form("%3d", indent/2) << SP(indent) << "# " << filename << "\n";
			else if (vverbose)
				cerr << SP(indent+1) << "Avoided multiple includes: " << partial_path << "\n";
			return;
		}
	}
	if (verbose)
		cerr << form("%3d", indent/2) << SP(indent) << "{ " << partial_path << "\n";
	else if (progress)
		cerr << SP(indent) << "Processing file: " << partial_path << "\n";
	if (indent == 0)
		cout << "\"{ObjApp}" << FN(filename) << ".o\" �";
	cout << " �\n\t\t\"{SrcApp}" << PP(partial_path) << "\" ";
	strcpy(nameTable[noFiles++], partial_path);
	fileHasLF = false;
	istream file(&fb);
	while (!file.eof())
	{
		char line[1100];
		file.getline(line, 1000);
		char *p = line;
		while (*p == ' ' || *p == 8) 
			++p;
		if (!fileHasLF && *p == 10)
		{
			cerr << "Warning: file \"" << partial_path << "\" has linefeeds (LF)\n";
			fileHasLF = true;
			while (*p > 0 && *p == 10) 
				++p;
		}
		if (*p != '#')
			continue;
		++p; // skip '#'
		while (*p == ' ' || *p == 8) 
			++p;
		if (partcmp(p, "include"))
			continue;
		p += 7;
		while (*p == ' ' || *p == 8) 
			++p;
		if (*p != '"')
		{
			if (*p != '<')
				cerr << "Bad #include in file \"" << partial_path << "\" in line \"" << line << "\"\n";
			continue;
		}
		if (vverbose)
			cerr << SP(indent+1) << "Found line containing include: " << line << "\n";
		++p; // at first letter in filename
		char *endQ = p;
		while (*endQ != '"')
		{
			if (*endQ == 13)
			{
				cerr << "Missing quote in file: \"" << partial_path << "\" in line \"" << line << "\"\n";
				gErr = 1;
				return;
			}
			endQ++;
		}
		short namelen = endQ - p;
		if (namelen > maxNameLen)
		{
			cerr << "Too long filename in file: " << partial_path << "\n";
			gErr = 1;
			return;
		}
		char iname[maxNameLen + 3];
		strncpy(iname, p, namelen);
		iname[namelen] = 0;
		if (vverbose)
			cerr << SP(indent+1) << "Found file filename (" << namelen << "): " << iname << "\n";
		ProcessFile(iname, indent+2);
	}
	if (verbose)
		cerr << "   " << SP(indent) << "} " << partial_path << "\n";
}

void DoTheFile(char* filename)
{
	noFiles = 0;
	ProcessFile(filename, 0);
	cout << "\n\n";
	if (verbose)
		cerr << "\n";
}

int main(int argc, char *argv[])
{
	InitCursorCtl(nil);
	for (int i = 1; i < argc; i++)
	{
		DoTheFile(argv[i]);
	}
	return gErr;
}
