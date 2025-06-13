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
	if (-d $topdir) {
	    if (chdir($topdir)) {
		($dir,$_) = ($topdir,':');
		$name = $topdir;
		&wanted;
		$topdir =~ s,:$,, ;
		&finddir($topdir);
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
    local($dir) = @_;
    local($nlink);
    local($name);

    # Get the list of files in the current directory.

    opendir(DIR,':') || (warn "Can't open $dir: $!\n", return);
    local(@filenames) = readdir(DIR);
    closedir(DIR);

    for (@filenames) {
        $nlink = $prune = 0;
        $name = "$dir:$_";
        &wanted;

	# Get link count and check for directoriness.

	if (-d $_) {

	    # It really is a directory, so do it recursively.

	    if (!$prune && chdir $_) {
		 &finddir($name);
		chdir '::';
	    }
    	}
    }
}
1;
