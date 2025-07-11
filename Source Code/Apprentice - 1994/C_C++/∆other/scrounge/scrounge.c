/* ----------------------------------------------------------------------

	scrounge
	version 1.0.0
	30 March 1994
	
	Copyright (C) 1994 Celestin Company.
	All Rights Reserved.
	
	Written by: Paul Celestin
	
	This utility will grab all new news articles in the subscribed
	groups specified in your .newsrc file and output them in rnews
	format. Output is to the console. You can redirect it to a file,
	if you wish, by typing "scrounge > filename" or something similar.
	
	The.newsrc file is updated to reflect the newly scrounged articles.
	
	Change SPOOL_DIR to the location of the news spool files for your
	host. It's usually in the /usr/spool/news directory, but this is
	subject to change depending on how your host is configured. For my
	host, it's /news/spool.
	
	scrounge has been tested on various Unix hosts, from Suns to BSDI
	flavors. If it doesn't compile under 'cc', try 'gcc'. Your mileage
	may vary. Neither Paul Celestin nor Celestin Company is responsible
	for any damaged caused by using this source file. Compile and use
	it at your own risk.
	
	Restrictions: You may freely distribute this source file. However,
	please do not distribute modified versions, unless you place a note
	in the source file stating that it has been modified and by whom.
	
	Send questions, comments, and bugs to: celestin@pt.olympus.net
	
	940330 - 1.0.0 - initial release
	
---------------------------------------------------------------------- */

# include	<stdio.h>
# include	<string.h>
# include	<stdlib.h>
# include	<dirent.h>

# define	NEWSRC_OLD	".newsrc"
# define	NEWSRC_NEW	".newsrc.new"
# define	MSG_BEGIN	"#! rnews "
# define	SPOOL_DIR	"/news/spool"

/* ----------------------------------------------------------------------
prototypes
---------------------------------------------------------------------- */

int main();
void GetGroupName(char theLine[150],char theGroup[150]);
int GetLastArticle(char theLine[150]);
int ProcessGroup(char theGroup[150],int lastArticle);
void Dot2Slash(char src[150],char dst[150]);
void FileCopy(char fileOne[150], char fileTwo[150]);

/* ----------------------------------------------------------------------
main
---------------------------------------------------------------------- */
int main ()
{
	FILE *theOldFile,*theNewFile;
	char theOld[150],theNew[150],homeDir[150],currentDir[150];
	char theLine[150],theGroup[150];
	int err,lastArticle,newLast;

	strcpy(homeDir,getenv("HOME"));
	strcpy(currentDir,getenv("PWD"));
	sprintf(theOld,"%s/%s",homeDir,NEWSRC_OLD);
	sprintf(theNew,"%s/%s",homeDir,NEWSRC_NEW);

	theOldFile = fopen(theOld,"r");
	if (theOldFile == NULL)
		return(-1);
	remove(theNew);

	theNewFile = fopen(theNew,"w");
	if (theNewFile == NULL)
		return(-1);

	fflush(stdout);
	while(fgets(theLine,150,theOldFile) != NULL)
	{
		if (strchr(theLine,':') != NULL) /* subscribed group */
		{
			GetGroupName(theLine,theGroup);
			if (theGroup != NULL)
			{
				lastArticle = GetLastArticle(theLine);
				newLast = ProcessGroup(theGroup,lastArticle);
			}
		}
		if (strchr(theLine,':') != NULL)
			sprintf(theLine,"%s: 1-%d\n",theGroup,newLast);
		err = fputs(theLine,theNewFile);
	}
	fclose(theOldFile);
	fclose(theNewFile);
	remove(theOld);
	FileCopy(theNew,theOld);
}

/* ----------------------------------------------------------------------
GetGroupName
---------------------------------------------------------------------- */
void GetGroupName(char from[150],char to[150])
{
	int i = 0;
	
	while ((from[i] != ':') && (from[i] != '\0'))
		to[i] = from[i++];
	to[i] = '\0';
}

/* ----------------------------------------------------------------------
GetLastArticle
---------------------------------------------------------------------- */
int GetLastArticle(char from[150])
{
	char to[150];
	int i,j = 0;
	
	i = strlen(from);
	while ((from[i] != '-') && (from[i] != ',') && (from[i] != ' '))
		--i;
	++i;
	while (from[i] != '\0')
		to[j++] = from[i++];
	to[j] = '\0';
	return(atoi(to));
}

/* ----------------------------------------------------------------------
ProcessGroup
---------------------------------------------------------------------- */
int ProcessGroup(char theGroup[150],int lastArticle)
{
	struct dirent *theDir;
	DIR *oneDir;
	FILE *oneMsg;
	char spoolDir[150],theSlash[150],message[150],oneLine[255];
	int message_no,lastProcessed = lastArticle,offset;

	Dot2Slash(theGroup,theSlash);
	sprintf(spoolDir,"%s/%s",SPOOL_DIR,theSlash);
	if (!chdir(spoolDir))
	{
		oneDir = opendir(spoolDir);
		if (oneDir != NULL)
		{
			while ((theDir = readdir(oneDir)) != NULL)
			{
				message_no = atoi(theDir->d_name);
				if (message_no > lastProcessed)
				{
					lastProcessed = message_no;
					sprintf(message,"%s/%s",spoolDir,theDir->d_name);
					oneMsg = fopen(message,"r");
					if (oneMsg != NULL)
					{
						offset = 0;	/* figure out how big fileTwo is */
						while (fgets(oneLine,255,oneMsg) != NULL)
							offset += strlen(oneLine);
						fseek(oneMsg,0L,0); /* rewind to beginning */
						printf("%s %d\n",MSG_BEGIN,offset);
						while (fgets(oneLine,255,oneMsg) != NULL)
							printf("%s",oneLine);
						fclose(oneMsg);
					}
				}
			}
		}
	}
	return(lastProcessed);
}


/* ----------------------------------------------------------------------
Dot2Slash
---------------------------------------------------------------------- */
void Dot2Slash(char src[150],char dst[150])
{
	int i = 0;

	while (src[i] != '\0')
	{
		dst[i] = src[i];
		if (dst[i] == '.')
			dst[i] = '/';
		++i;
	}
	dst[i] = '\0';
}

/* ----------------------------------------------------------------------
FileCopy
---------------------------------------------------------------------- */
void FileCopy(char fileOne[150], char fileTwo[150])
{
	FILE *src,*dst;
	char line[255];
	int err;
	
	src = fopen(fileOne,"r");
	if (src != NULL)
	{
		dst = fopen(fileTwo,"w");
		if (dst != NULL)
		{
			while (fgets(line,255,src) != NULL)
				err = fputs(line,dst);
			fclose(dst);
		}
		fclose(src);
	}
}
