#
# CommConnect -- Perl embedding of Alex Metcalf's CommConnect XFCN
#
# Authors: XFCN          Alex Metcalf <alex@metcalf.demon.co.uk>
#          Perl Wrapper  Matthias Neeracher <neeri@iis.ee.ethz.ch>
#
# The CommConnect XFCN is ShareWare! Please refer to the HyperCard
# stack you should have received with this file for details.
#

package CommConnect;

&MacPerl'LoadExternals("CommConnect.pl");

#
# $status = &CommConnect'Prepare([Tool])
#
# Prepare a connection tool (default is first in extensions folder)
# for use.
#

sub Prepare {
   local($tool) = @_;

   if ($tool) {
      &CommConnect("prepare for use", $tool);
   } else {
      &CommConnect("prepare for use");
   }
}

#
# $status = &CommConnect'Choose([X,Y])
#
# Show dialog box to select tool and settings.
#

sub Choose {
   local($x, $y) = @_;

   if ($x) {
      &CommConnect("choose connection", "$x, $y");
   } else {
      &CommConnect("choose connection");
   }
}

#
# $settings = &CommConnect'GetSettings()
#
# Get current tool settings.
#

sub GetSettings {
   &CommConnect("get connection settings");
}

#
# $status = &CommConnect'ChangeSettings($settings)
#
# Change current tool settings.
#

sub ChangeSettings {
   local ($settings) = @_;
   &CommConnect("change connection settings", $settings);
}

#
# $status = &CommConnect'Cleanup()
#
# Clean up after using a tool.
#

sub Cleanup {
   &CommConnect("end of use");
}

#
# $status = &CommConnect'OpenConnection()
#
# Open a connection with the chosen tool.
#

sub OpenConnection {
   &CommConnect("open connection");
}

#
# $status = &CommConnect'CloseConnection()
#
# Open a connection with the chosen tool.
#

sub CloseConnection {
   &CommConnect("close connection");
}

#
# $status = &CommConnect'Idle()
#
# Perform periodical tasks. Call occasionally.
#

sub Idle {
   &CommConnect("idle");
}

#
# $status = &CommConnect'Send($data)
#
# Send Data.
#

sub Send {
   local($data) = @_;
   
   while (length($data)) {
      if ($data =~ /^\0/) {
         &CommConnect("send null");
         $data = $';
      } elsif ($data =~ /^[\1-\377]+/) {
         &CommConnect("send", $&);
         $data = $';
      }
   }
}

#
# $data = &CommConnect'Receive()
#
# Receive Data.
#

sub Receive {
   &CommConnect("receive");
}

1;

