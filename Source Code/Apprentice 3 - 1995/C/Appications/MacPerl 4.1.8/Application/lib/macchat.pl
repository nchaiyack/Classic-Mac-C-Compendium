## macchat.pl: chat with a server over any protocol
## Matthias Neeracher

## Derived from chat2pl
## V2.01.alpha.7 91/06/16
## Randal L. Schwartz

package macchat;

require 'GUSI.ph';

# *S = symbol for current I/O, gets assigned *chatsymbol....
$next = "chatsymbol000000"; # next one
$nextpat = "^chatsymbol"; # patterns that match next++, ++, ++, ++


## $handle = &chat'open_port($family, $addr [, $ouraddr]);
## opens a named or numbered server. 
## $addr may be created with one of the pack_sockaddr… routines in GUSI.pl

sub open_port { ## public
	local($family, $addr, $ouraddr) = @_;

	*S = ++$next;
	unless (socket(S, $family, &GUSI'SOCK_STREAM, 0)) {
		($!) = ($!, close(S)); # close S while saving $!
		return undef;
	}
	if (defined($ouraddr)) {
		unless (bind(S, $ouraddr)) {
			($!) = ($!, close(S)); # close S while saving $!
			return undef;
		}
	}
	unless (connect(S, $addr)) {
		($!) = ($!, close(S)); # close S while saving $!
		return undef;
	}
	select((select(S), $| = 1)[0]);
	$next; # return symbol for switcharound
}

## ($addr, $handle) = &chat'open_listen($family, [$ouraddr]);
## opens a TCP port on the current machine, ready to be listened to
## if $port_number is absent or zero, pick a default port number
## process must be uid 0 to listen to a low port number

sub open_listen { ## public

	*S = ++$next;
	local($family, $ouraddr) = @_;
	local(*NS) = "__" . time;
	unless (socket(NS, $family, &GUSI'SOCK_STREAM, 0)) {
		($!) = ($!, close(NS));
		return undef;
	}
	if (defined $ouraddr) {
		unless (bind(NS, $ouraddr)) {
			($!) = ($!, close(NS));
			return undef;
		}
	}
	unless (listen(NS, 1)) {
		($!) = ($!, close(NS));
		return undef;
	}
	select((select(NS), $| = 1)[0]);
	$S{"needs_accept"} = *NS; # so expect will open it
	(getsockname(NS), $next); # returning this
}

# $S is the read-ahead buffer

## $return = &chat'expect([$handle,] $timeout_time,
## 	$pat1, $body1, $pat2, $body2, ... )
## $handle is from previous &chat'open_*().
## $timeout_time is the time (either relative to the current time, or
## absolute, ala time(2)) at which a timeout event occurs.
## $pat1, $pat2, and so on are regexs which are matched against the input
## stream.  If a match is found, the entire matched string is consumed,
## and the corresponding body eval string is evaled.
##
## Each pat is a regular-expression (probably enclosed in single-quotes
## in the invocation).  ^ and $ will work, respecting the current value of $*.
## If pat is 'TIMEOUT', the body is executed if the timeout is exceeded.
## If pat is 'EOF', the body is executed if the process exits before
## the other patterns are seen.
##
## Pats are scanned in the order given, so later pats can contain
## general defaults that won't be examined unless the earlier pats
## have failed.
##
## The result of eval'ing body is returned as the result of
## the invocation.  Recursive invocations are not thought
## through, and may work only accidentally. :-)
##
## undef is returned if either a timeout or an eof occurs and no
## corresponding body has been defined.
## I/O errors of any sort are treated as eof.

$nextsubname = "expectloop000000"; # used for subroutines

sub expect { ## public
	if ($_[0] =~ /$nextpat/) {
		*S = shift;
	}
	local($endtime) = shift;

	local($timeout,$eof) = (1,1);
	local($caller) = caller;
	local($rmask, $nfound, $timeleft, $thisbuf);
	local($cases, $pattern, $action, $subname);
	$endtime += time if $endtime < 600_000_000;

	if (defined $S{"needs_accept"}) { # is it a listen socket?
		local(*NS) = $S{"needs_accept"};
		delete $S{"needs_accept"};
		$S{"needs_close"} = *NS;
		unless(accept(S,NS)) {
			($!) = ($!, close(S), close(NS));
			return undef;
		}
		select((select(S), $| = 1)[0]);
	}

	# now see whether we need to create a new sub:

	unless ($subname = $expect_subname{$caller,@_}) {
		# nope.  make a new one:
		$expect_subname{$caller,@_} = $subname = $nextsubname++;

		$cases .= <<"EDQ"; # header is funny to make everything elsif's
sub $subname {
	LOOP: {
		if (0) { ; }
EDQ
		while (@_) {
			($pattern,$action) = splice(@_,0,2);
			if ($pattern =~ /^eof$/i) {
				$cases .= <<"EDQ";
		elsif (\$eof) {
	 		package $caller;
			$action;
		}
EDQ
				$eof = 0;
			} elsif ($pattern =~ /^timeout$/i) {
			$cases .= <<"EDQ";
		elsif (\$timeout) {
		 	package $caller;
			$action;
		}
EDQ
				$timeout = 0;
			} else {
				$pattern =~ s#/#\\/#g;
			$cases .= <<"EDQ";
		elsif (\$S =~ /$pattern/) {
			\$S = \$';
		 	package $caller;
			$action;
		}
EDQ
			}
		}
		$cases .= <<"EDQ" if $eof;
		elsif (\$eof) {
			undef;
		}
EDQ
		$cases .= <<"EDQ" if $timeout;
		elsif (\$timeout) {
			undef;
		}
EDQ
		$cases .= <<'ESQ';
		else {
			$rmask = "";
			vec($rmask,fileno(S),1) = 1;
			($nfound, $rmask) =
		 		select($rmask, undef, undef, $endtime - time);
			if ($nfound) {
				$nread = sysread(S, $thisbuf, 1024);
				if ($nread > 0) {
					$S .= $thisbuf;
				} else {
					$eof++, redo LOOP; # any error is also eof
				}
			} else {
				$timeout++, redo LOOP; # timeout
			}
			redo LOOP;
		}
	}
}
ESQ
		eval $cases; die "$cases:\n$@" if $@;
	}
	$eof = $timeout = 0;
	do $subname();
}

## &chat'print([$handle,] @data)
## $handle is from previous &chat'open().
## like print $handle @data

sub print { ## public
	if ($_[0] =~ /$nextpat/) {
		*S = shift;
	}

	if (defined $S{"needs_accept"}) { # is it a listen socket?
		local(*NS) = $S{"needs_accept"};
		delete $S{"needs_accept"};
		$S{"needs_close"} = *NS;
		unless(accept(S,NS)) {
			($!) = ($!, close(S), close(NS));
			return undef;
		}
		select((select(S), $| = 1)[0]);
	}
	
	print S @_;
}

## &chat'close([$handle,])
## $handle is from previous &chat'open().
## like close $handle

sub close { ## public
	local($pid);
	if ($_[0] =~ /$nextpat/) {
		$pid = $PID{$_[0]};
	 	*S = shift;
	} else {
		$pid = $PID{$next};
	}
	close(S);
	if (defined $S{"needs_close"}) { # is it a listen socket?
		local(*NS) = $S{"needs_close"};
		delete $S{"needs_close"};
		close(NS);
	}
}

## @ready_handles = &chat'select($timeout, @handles)
## select()'s the handles with a timeout value of $timeout seconds.
## Returns an array of handles that are ready for I/O.
## Both user handles and chat handles are supported (but beware of
## stdio's buffering for user handles).

sub select { ## public
	local($timeout) = shift;
	local(@handles) = @_;
	local(%handlename) = ();
	local(%ready) = ();
	local($caller) = caller;
	local($rmask) = "";
	for (@handles) {
		if (/$nextpat/o) { # one of ours... see if ready
			local(*SYM) = $_;
				if (defined $SYM{"needs_accept"}) { # is it a listen socket?
					local(*NS) = $SYM{"needs_accept"};
					delete $SYM{"needs_accept"};
					$SYM{"needs_close"} = *NS;
					unless(accept(SYM,NS)) {
						($!) = ($!, close(SYM), close(NS));
						return undef;
					}
					select((select(SYM), $| = 1)[0]);
				}

			if (length($SYM)) {
				$timeout = 0; # we have a winner
				$ready{$_}++;
			}
			$handlename{fileno($_)} = $_;
		} else {
			$handlename{fileno(/'/ ? $_ : "$caller\'$_")} = $_;
		}
	}
	for (sort keys %handlename) {
		vec($rmask, $_, 1) = 1;
	}
	select($rmask, undef, undef, $timeout);
	for (sort keys %handlename) {
		$ready{$handlename{$_}}++ if vec($rmask,$_,1);
	}
	sort keys %ready;
}

1;
