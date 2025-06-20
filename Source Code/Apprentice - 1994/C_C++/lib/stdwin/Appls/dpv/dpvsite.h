/* dpv -- ditroff previewer.  Site-dependent definitions. */

/* Declare the printers we want */

#define PRINT_MENU_TABLE \
	{"Print on Oce",		"psc",	"psdit %s | lpr -Poce"}, \
	{"Print on PostScript",		"psc",	"psdit %s | lpr -Ppsc"}, \
	{"Print on Ag",			"psc",	"psdit %s | lpr -Pag"}, \
	{"Print on Fa"	,		"psc",	"psdit %s | lpr -Pfa"}, \
	{"Print on Wowbagger",		"psc",	"psdit %s | lpr -Pwow"}, \
	{"Print on ${PRINTER-psc}",	"psc", \
				"psdit %s | lpr -P${PRINTER-psc}"}, \
	/**/
