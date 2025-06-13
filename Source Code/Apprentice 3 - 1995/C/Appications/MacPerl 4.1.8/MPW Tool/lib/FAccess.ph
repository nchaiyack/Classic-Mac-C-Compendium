# Faccess.ph -- constants to pass to &MacPerl'FAccess
#
# Matthias Neeracher 13Dec93
#

# $TAB = &MacPerl'FAccess(FILE, &F_GTABINFO)

sub F_GTABINFO 	{	0x6500;	}

# &MacPerl'FAccess(FILE, &F_STABINFO, TAB)

sub F_STABINFO 	{	0x6501;	}

# ($FONTNAME, $FONTSIZE) = &MacPerl'FAccess(FILE, &F_GFONTINFO)
# $FONTNUM = &MacPerl'FAccess(FILE, &F_GFONTINFO)

sub F_GFONTINFO	{	0x6502;	}

# &MacPerl'FAccess(FILE, &F_SFONTINFO, FONT [, SIZE])

sub F_SFONTINFO	{	0x6503;	}

# ($STARTSEL, $ENDSEL, $DISPLAYTOP) = &MacPerl'FAccess(FILE, &F_GSELINFO)
# $STARTSEL = &MacPerl'FAccess(FILE, &F_GSELINFO)

sub F_GSELINFO		{	0x6506;	}

# &MacPerl'FAccess(FILE, &F_SSELINFO, $STARTSEL, $ENDSEL [, $DISPLAYTOP])

sub F_SSELINFO		{	0x6507;	}

# ($LEFT, $TOP, $RIGHT, $BOTTOM) = &MacPerl'FAccess(FILE, &F_GWININFO)
# $TOP = &MacPerl'FAccess(FILE, &F_GWININFO)

sub F_GWININFO		{	0x6508;	}

# &MacPerl'FAccess(FILE, &F_SWININFO, LEFT, TOP [, RIGHT, BOTTOM])

sub F_SWININFO		{	0x6509;	}

1;
