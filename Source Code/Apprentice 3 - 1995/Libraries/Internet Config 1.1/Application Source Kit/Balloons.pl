#!perl

$balloons_strh_id = 26724;

# chdir "Rocky:Peter:Pascal:TCP Work:TCP Programs:Anarchie";

open(STDIN,"Balloons Data") || die "Failed to open input";
open(STDOUT,">Balloons.r") || die "Failed to open output";

print <<INCLUDES;
#include "Types.r"
#include "BalloonTypes.r"

INCLUDES

@strings=();

while (<>) {
  chop;
  next if /^$/;
  last if /^END$/;
  die "Bad line '$_'" unless /(DIALOG|MENU)\s+(\d+)\s*(.*)/;
  $dialog = $1 eq "DIALOG"; $id = $2; $name=$3;
  @items=();
	@menus=();
	$menuitem=0;
  while (<>) {
    chop;
    next if /^$/;
    if ($dialog) {
	    last if /^END-DIALOG$/;
	    die "Bad dialog line '$_'" unless /^(\d+)\.(\d+)\s+(.*)/;
	    die "Quote in line" if /"/;
	    $base=($1-1)*4;
	    $item=$base+$2-1;
	    $index = &find_string($3);
	    $items[$item] = $index;
	    $items[$base+0] = 0 unless $items[$base+0];
	    $items[$base+1] = 0 unless $items[$base+1];
	    $items[$base+2] = 0 unless $items[$base+2];
	    $items[$base+3] = 0 unless $items[$base+3];
    } else {
	    last if /^END-MENU$/;
	    die "Quote in line '$_'" if /"/;
	    die "Bad menu line '$_'" unless /^(\d) (.*)/ || /^(\d)$/;
			if ($1 == 0) {
			  if ($1 eq $_) {
			    $menus[$menuitem]='-';
				} else {
				  $menus[$menuitem]= $2;
				}
				$menuitem++;
			} else {
				$base=($menuitem-1)*4;
				$item=$base+$1-1;
				$index = &find_string($2);
				$items[$item] = $index;
				$items[$base+0] = 0 unless $items[$base+0];
				$items[$base+1] = 0 unless $items[$base+1];
				$items[$base+2] = 0 unless $items[$base+2];
				$items[$base+3] = 0 unless $items[$base+3];
			}
    }
  }
	if ($dialog) {
  	print <<"HEADER";
resource 'hdlg' ($id,"$name") {
\t2,0,0,0,0,
\tHMSkipItem { },
\t{
HEADER
	} else {
		print <<"HEADER";
resource 'hmnu' ($id,"$name") {
\t2,0,0,0,
\tHMSkipItem { },
\t{
HEADER
	}
  for $item (1..@items/4) {
    $base = ($item-1)*4;
    if ($items[$base+0] || $items[$base+1] ||
        $items[$base+2] || $items[$base+3]) {
			print "\t\tHMStringResItem { /* $item */\n";
			if ($dialog) {
      	print <<"ITEM";
\t\t\t{0,0},
\t\t\t{0,0,0,0},
ITEM
			}
      for $k (0..3) {
        $index = $items[$base+$k];
        if ($index) {
          print "\t\t\t$balloons_strh_id,$index,\n";
        } else {
          print "\t\t\t0,0,\n";
        }
      }
      print "\t\t},\n";
    } else {
	     print "\t\tHMSkipItem { }, /* $item */\n";
    }
  }
  print <<"TRAILER";
\t}
};

TRAILER
}

print "resource 'STR#' ($balloons_strh_id,\"Balloon Help Strings\") {\n";
print "\t{\n";
for $index (1..@strings) {
  print "\t\t/* $index */\n";
  print "\t\t\"$strings[$index-1]\",\n";
}
print "\t}\n";
print "};\n\n";

close(STDOUT);
close(STDIN);

sub find_string {
  local($s) = @_;
  local($i);
  for $i (1..@strings) {
    return $i if $s eq $strings[$i-1];
  }
  $i = @strings;
  $strings[$i] = $s;
  return $i+1;
}
