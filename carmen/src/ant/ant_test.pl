#!/usr/bin/perl 

use strict;

use Ant;

my $response;
my $err;

my $ant = new Ant(host => 'localhost', port => 3000, 
		  verbose => 0, persistent => 1, name => "test server") 
    or die "Couldn't create Ant object.\n";

if(!defined ($response = $ant->send_client_command("test2"))) {
    print "Error sending client command\n";
}
else {
    print "Response is \"$response\"\n";
}

sleep 3;

if(!defined ($response = $ant->send_client_command("test1"))) {
    print "Error sending client command\n";
}
else {
    print "Response is \"$response\"\n";
}





