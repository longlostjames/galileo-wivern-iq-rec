#!/usr/bin/perl -w 
# Owain Davies 08062004
# Darcy Ladd  14012011
# Alan Doo - 20130430 - Update to accept single command line arg

MAIN:
{

while(1)
{
	print "invoking iq recording program\n";
	$program = "/usr/local/bin/radar-galileo-iq-rec @ARGV[0] > /dev/null";
	system($program);

	print "sleeping for 2 seconds before restart\n";
	sleep 2;
}

}

