# Perl interface to GUSI, M. Neeracher 28Nov92

package GUSI;

# Address families

sub AF_UNSPEC		{ 0;	}
sub AF_UNIX			{ 1;	}
sub AF_INET			{ 2;	}
sub AF_CTB 			{ 3;	}
sub AF_FILE 		{ 4;	}
sub AF_PPC 			{ 5;	}
sub AF_APPLETALK	{ 16;	}
sub ATALK_SYMADDR	{ 272;}

# Socket types

sub SOCK_STREAM 	{ 1;	}
sub SOCK_DGRAM 	{ 2;	}

# Address formats

sub pack_sockaddr_in	{
	local($family, $addr, $port) = @_;

	if ($addr =~ /^(\d+)+\.(\d+)\.(\d+)\.(\d+)/) {
		$addr = pack('C4', $1, $2, $3, $4);
	} else {
		local(@x) = gethostbyname($addr);
		return undef unless @x;
		$addr = $x[4];
	}
	
	pack("s S a4 x8", $family, $port, $addr);
}

sub unpack_sockaddr_in {
	local($addr) = @_;
	local($family, $port, $a1, $a2, $a3, $a4) = unpack("s S C4 x8", $addr);
	
	($family, "$a1.$a2.$a3.$a4", $port);
}

sub pack_sockaddr_un	{
	local($family, $path) = @_;
	
	pack("s a108", $family, $path);
}

sub unpack_sockaddr_un {
	local($addr) = @_;
	
	return unpack("s a108", $addr);
}

sub pack_sockaddr_atlk	{
	local($family, $net, $node, $socket) = @_;
	
	pack("s s C C", $family, $net, $node, $socket);
}

sub unpack_sockaddr_atlk {
	local($addr) = @_;
	
	unpack("s s C C", $addr);
}

sub pack_sockaddr_atlk_sym	{
	local($family, $obj, $type, $zone) = @_;
	local($fmt);
	
	$fmt	=	"s C a" . ((length($obj) & ~1) + 1);
	$fmt	.=	"C a" . ((length($type) & ~1) + 1);
	$fmt	.=	"C a" . ((length($zone) & ~1) + 1);
	
	pack($fmt, $family, length($obj), $obj, length($type), $type, length($zone), $zone);
}

sub unpack_sockaddr_atlk_sym {
	local($addr) = @_;
	local($len, $obj, $type, $zone);
	
	($len, $addr) = unpack("x2 C a*", $addr);
	($obj, $addr) = unpack("a$len " . ($len & 1 ? "" : "x ") . "a*", $addr);
	($len, $addr) = unpack("x2 C a*", $addr);
	($type, $addr) = unpack("a$len " . ($len & 1 ? "" : "x ") . "a*", $addr);
	($len, $addr) = unpack("x2 C a*", $addr);
	($zone, $addr) = unpack("a$len " . ($len & 1 ? "" : "x ") . "a*", $addr);
	
	($family, $obj, $type, $zone);
}

sub pack_sockaddr_ppc	{
	local($family, $type, $name, $porttype) = @_;
	local($ppcNBPTypeLocation, $smRoman, $ppcByString)	= (2,0,2);
	
	pack("s s C a101 s C a32 x s C a32", 
		$family, 
		$ppcNBPTypeLocation, length($type), $type,
		$smRoman, length($name), $name, $ppcByString, length($porttype), $porttype);
}

sub pack_sa_constr_file	{
	local($count, $constr, $cur) = (0, "");
	
	while ($cur = shift) {
		++$count;
		$constr .= pack("A4", $cur);
	}
	
	pack("s", $count) . $constr;
}

sub pack_sa_constr_atlk	{
	local($count, $constr, $cur) = (0, "");
	
	while ($cur = shift) {
		++$count;
		$constr .= pack("x4 C a33", length($cur), $cur);
	}
	
	pack("s", $count) . $constr;
}

sub pack_sa_constr_ppc	{
	local($type) = @_;
	
	pack("C a33", length($type), $type);
}

# Flags for choose()

sub CHOOSE_DEFAULT	{	1;	}
sub CHOOSE_NEW 		{	2;	}
sub CHOOSE_DIR 		{	4;	}

1;
