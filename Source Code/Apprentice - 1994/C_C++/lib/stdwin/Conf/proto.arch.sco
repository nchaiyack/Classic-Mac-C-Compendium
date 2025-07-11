
#
# Definitions pertaining to architecture SCO
#

# Define the name of this architecture here.
# This is used as a subdirectory name where all the objects for this
# architecture live: $(TOP)/Build/$(ARCH)
#
# Note that there is the silent assumption that there is only one O.S.
# per architecture.  We'll have to invent composite architecture names,
# or insert an extra level in the build tree, if this becomes false...
#
ARCH=		sco

# Define the name of the script used to update the dependencies in
# the Makefile.
# Choices are:
#	slowmkdep -- always works (as far as I know)
#	fastmkdep -- used cc -M, which isn't always available
#	makedepend -- Todd Brunhoff's superfast tool (comes with X11)
#	              (This needs -Dunix because of configure.h)
# You may also place this definition in the proto.os.* file if you
# know a method works for every architecture supported by that OS.
#
MKDEP=		$(CONF)/slowmkdep
#MKDEP=		$(CONF)/fastmkdep
#MKDEP=		makedepend -Dunix

