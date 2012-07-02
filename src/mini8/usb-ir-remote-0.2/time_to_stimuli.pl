#/**
# *  _   _ ____  ____    ___        ____                      _       
# * | | | / ___|| __ )  |_ _|_ __  |  _ \ ___ _ __ ___   ___ | |_ ___ 
# * | | | \___ \|  _ \   | || '__| | |_) / _ \ '_ ` _ \ / _ \| __/ _ \
# * | |_| |___) | |_) |  | || |    |  _ <  __/ | | | | | (_) | ||  __/
# *  \___/|____/|____/  |___|_|    |_| \_\___|_| |_| |_|\___/ \__\___|
# *   
# * 
# * Copyright 2007 Jörgen Birkler
# * jorgen.birkler@gmail.com
# */
#

use strict;
use warnings;


$ARGV[0] =~ /--fcpu=(\d+)/i || die;
my $F_CPU = $1;
$ARGV[1] =~ /--portbit=(\d)/i || die;	
my $P_BIT = $1;
my $total_time_uS = 0;

while (<STDIN>) {
	if (/^\s*(0|1)\s*:\s*(\d+)\s*(ms|us|s)\s*/i) 
	{
		my $value = $1;
		my $time = $2;
		my $factor = $3;
		
		if ($factor =~ /^ms$/i) { $time *= 1000; }
		if ($factor =~ /^s$/i) { $time *= 1000000; }
		
		printf("%09d:%02x\n",$total_time_uS * $F_CPU / 1000000,$value << $P_BIT); 
		$total_time_uS += $time;
	}
	elsif (/^\s*$/) {
	}
	elsif (/^\s*\/\//) {
	}
	else {
		die;
	}
}
printf("999999999:FF\n");
