#
# File:   CommConnect.t  -- Test program for CommConnect.pl
# Author: XFCN           -- Alex Metcalf <alex@metcalf.demon.co.uk>
#         Perl Embedding -- Matthias Neeracher <neeri@iis.ee.ethz.ch>
#
# The CommConnect XFCN is ShareWare! Please refer to the HyperCard 
# stack you should have received with this file for details.
#

&MacPerl'DebugExternals(1);

require "CommConnect.pl";

print &CommConnect'Prepare(), "\n";
print &CommConnect'Choose(), "\n";
print &CommConnect'OpenConnection(), "\n";

while (1) {
   print &CommConnect'Receive(), "\n";
   $_ = <STDIN>;

   last if ($_ eq ".\n");
   next if ($_ eq "?\n");

   print &CommConnect'Send($_), "\n";
}

&CommConnect'CloseConnection();
&CommConnect'Cleanup();
