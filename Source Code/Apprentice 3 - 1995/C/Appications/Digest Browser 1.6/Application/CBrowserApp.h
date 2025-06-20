/*****
 * CBrowserApp.h
 *
 *	Application class for a typical application.
 *
 *****/

#define	_H_CBrowserApp		/* Include this file only once */
#include <CApplication.h>

struct CBrowserApp : CApplication {

	/* No instance variables */

	void	IBrowserApp(void);
	void	SetUpFileParameters(void);

	void	DoCommand(long theCommand);
	void	OpenDocument(SFReply *macSFReply);

	void	InitToolbox(void);
};
