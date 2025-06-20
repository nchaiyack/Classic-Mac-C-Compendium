# Usage:
#	require "find.pl";
#
#	&find('/foo','/bar');
#
#	sub wanted { ... }
#		where wanted does whatever you want.  $dir contains the
#		current directory name, and $_ the current filename within
#		that directory.  $name contains "$dir/$_".  You are cd'ed
#		to $dir when the function is called.  The function may
#		set $prune to prune the tree.
#
# This library is primarily for find2perl, which, when fed
#
#   find2perl / -name .nfs\* -mtime +7 -exec rm -f {} \; -o -fstype nfs -prune
#
# spits out something like this
#
#	sub wanted {
#	    /^\.nfs.*$/ &&
#	    (($dev,$ino,$mode,$nlink,$uid,$gid) = lstat($_)) &&
#	    int(-M _) > 7 &&
#	    unlink($_)
#	    ||
#	    ($nlink || (($dev,$ino,$mode,$nlink,$uid,$gid) = lstat($_))) &&
#	    $dev < 0 &&
#	    ($prune = 1);
#	}
#
# Converted to Mac by Charlie Reiman <reiman@kaleida.com>
#

sub find {
    chop($cwd = `pwd`);
    foreach $topdir (@_) {
	(($topdev,$topino,$topmode,$topnlink) = stat($topdir))
	  || (warn("Can't stat $topdir: $!\n"), next);
	if (-d _) {
	    if (chdir($topdir)) {
		($dir,$_) = ($topdir,':');
		$name = $topdir;
		&wanted;
		$topdir =~ s,:$,, ;
		&finddir($topdir,$topnlink);
	    }
	    else {
		warn "Can't cd to $topdir: $!\n";
	    }
	}
	else {
	    unless (($dir,$_) = $topdir =~ m#^(.*:)(.*)$#) {
		($dir,$_) = (':', $topdir);
	    }
	    $name = $topdir;
	    chdir $dir && &wanted;
	}
	chdir $cwd;
    }
}

sub finddir {
    local($dir,$nlink) = @_;
    local($dev,$ino,$mode,$subcount);
    local($name);

    # Get the list of files in the current directory.

    opendir(DIR,':') || (warn "Can't open $dir: $!\n", return);
    local(@filenames) = readdir(DIR);
    closedir(DIR);

    if ($nlink == 2) {        # This dir has no subdirectories.
	for (@filenames) {
	    next if $_ eq ':';
	    $name = "$dir:$_";
	    $nlink = 0;
	    &wanted;
	}
    }
    else {                    # This dir has subdirectories.
	$subcount = $nlink - 2;
	for (@filenames) {
	    next if $_ eq ':';
	    $nlink = $prune = 0;
	    $name = "$dir:$_";
	    &wanted;
	    if ($subcount > 0) {    # Seen all the subdirs?

		# Get link count and check for directoriness.

		($dev,$ino,$mode,$nlink) = lstat($_) unless $nlink;
		
		if (-d _) {

		    # It really is a directory, so do it recursively.

		    if (!$prune && chdir $_) {
			&finddir($name,$nlink);
			chdir '::';
		    }
		    --$subcount;
		}
	    }
	}
    }
}
1;
