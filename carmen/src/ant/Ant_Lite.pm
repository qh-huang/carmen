#!/usr/bin/perl -w

package Ant_Lite;

=head1 NAME

Ant - "Ant's Not Tcx" - A simple socket based protocol for module communication.

=head1 SYNOPSIS

use Ant;

=head1 DESCRIPTION

Ant_Lite is a library for simple socket communication between software written
in both C and Perl. Currently the Perl interface only supports client-side
operations.

=head2 METHODS SUMMARY

In brief, the methods are:

    send_client_command(command, [data]);

    send_client_command_begin(command);

    writen(data, n, timeout);

    readn(n, timeout);

    send_client_command_end();

    get_socket();

=cut

require 5.002;
use strict;
use English;
use IO::Socket;

=head2 CONSTRUCTOR

new ( [ARGS] )

There are four useful arguments:

=over 4

=item 'host' => STRING

Set the hostname where the Ant server is running.

=item 'port' => NUM

Set the port where the Ant server is running.

=item 'verbose' => NUM

Set the verbosity level of the software, useful values are 0 or 1.

=item 'persistent' => NUM

If set to zero, the module will connect and disconnect for every client command sent. Otherwise the object will hold a single connection open.

=back

=cut

sub new {
    my $pkg = shift;
    my $object = bless {host => 'localhost', port => 3000, 
			persistent => 1, verbose => 0, 
			name => 'default', @_}, $pkg;

    if ($object->{'no_connect'}) {
#    print "Connecting to $object->{'name'} [60G";
#	print "[[1;34m OFF  [0m]\n";
      return $object;
    }

    print "Connecting to $object->{'name'} [60G";
    if (not $object->connect_to_server()) {
      print "[[1;31mFAILED[0m]\n";
    } else {
      print "[[1;32m  OK  [0m]\n";
    }
    $object->close_connection() unless  $object->{persistent};

    return $object;
}

sub DESTROY {
    my $self = shift;
    $self->send_client_command_begin("QUIT");
    $self->close_connection();
}

=head2 METHODS

=over 4

=item send_client_command(COMMAND, [DATA])

Sends COMMAND, and optionally some DATA to the Ant server. Returns RESPONSE, which includes everything the server said in response to the command.  RESPONSE is undefined if there is an error.  You do not need to add \n or other terminating characters to the end of the command.  The method will take care of that for you.  Also, do not send the prompt command when using this interface.  It should not be necessary and it will not work properly.  The DATA string is written after the Ant command and is not followed with any terminating characters.

=cut

sub send_client_command_core {
    my $self = shift;
    return 0 if defined $self->{'no_connect'};
    my $sock = $self->{sock};
    my $string = shift;
    my $data = shift;
    return 0 unless defined $string;
    my $response;
    my $success;
    
    # send command string
    print "Sending command \"$string\"\n" if $self->{verbose};
    $string .= "\r\0";
    if($self->writen($string, length($string), -1) < 0) {
	print "Error: Could not write to socket.\n" if $self->{verbose};
	return 0;
    }
    # send data string (if any)
    if(defined $data and $self->writen($data, length($data), -1) < 0) {
	print "Error: Could not write to socket.\n" if $self->{verbose};
	return 0;
    }
    return 1;
}

sub send_client_command {
    my $self = shift;
    return undef if defined $self->{'no_connect'};
    my $sock = $self->{sock};
    my $string = shift;
    my $data = shift;
    my $response;
    return undef unless defined $string;

    # if socket is not defined, connect to server
    if(!defined $self->{sock}) {
	$self->connect_to_server() or return undef;
    }

    # send the command. if not successful, connect and try one more time
    if(!$self->send_client_command_core($string, $data)) {
	$self->close_connection();
	$self->connect_to_server() or return undef;
	$self->send_client_command_core($string, $data) or return undef;
    }

    # get the response and the next prompt
    $response = $self->read_prompt();
    return undef unless defined $response;

    # if object is not persistent, close the connection
    $self->close_connection() if !$self->{persistent};
    return $response;
}

=over 4

=item send_client_command_begin(COMMAND)

=item send_client_command_end()

These methods separate the client command into two parts.  The reason for doing this would be if a command is to be followed by arbitrary number of writen and readn commands.  send_client_command_begin sends COMMAND and reads the echo, and send_client_command_end gets the command result (if any) and reads the next prompt.

=cut

sub send_client_command_begin {
    my $self = shift;
    return undef if defined $self->{'no_connect'};
    my $sock = $self->{sock};
    my $string = shift;
    my $response;
    return undef unless defined $string;

    # if socket is not defined, connect to server
    if(!defined $self->{sock}) {
	$self->connect_to_server() or return 0;
    }

    # send command string
    print "Sending command \"$string\"\n" if $self->{verbose};
    $string .= "\r\0";
    if($self->writen($string, length($string), -1) < 0) {
	print "Error: Could not write to socket.\n" if $self->{verbose};
	return 0;
    }

    return 1;
}

sub send_client_command_end {
    my $self = shift;
    return undef if defined $self->{'no_connect'};
    my $response;

    # get the response and the next prompt
    $response = $self->read_prompt();
    return undef unless defined $response;

    # if object is not persistent, close the connection
    $self->close_connection() if !$self->{persistent};
    return $response;
}

=item writen(DATA, N, TIMEOUT)
 
Writes N bytes of data from DATA to the Ant server.  If TIMEOUT microseconds go by without all of the data being written, the function will return. If TIMEOUT is undefined, the function will block indefinitely. The function returns the number of bytes actually written.  If that number is less than zero, an error occurred.  If it is greater than zero but less than the specified number of bytes, the function timed out.

=cut

sub writen {
    my $self = shift;
    my $sock = $self->{sock};
    my $string = shift;
    my $n = shift;
    my $timeout = shift;
    return -1 unless defined $string and defined $n;

    my $win = my $ein = "";
    vec($win, fileno($sock), 1) = 1;
    $ein = $win;

    my $count = 0;
    while($count < $n) {
	my $wout;
	my $eout;
	
	if(!defined $timeout) {
	    select(undef, $wout = $win, $eout = $ein, undef);	   
	}
	else {
	    select(undef, $wout = $win, 
		   $eout = $ein, $timeout / 1000000.0);
	}

	return -1 if vec($eout, fileno($sock), 1);
	if(vec($wout, fileno($sock), 1)) {
	    my $diff = syswrite($sock, $string, $n - $count, $count);
	    return $count unless defined $diff;
	    $count += $diff;
	}
	else {
	    return $count;
	}
    }
    return $count;
}

=item readn(N, TIMEOUT)

Reads N bytes of data from the Ant server.  If TIMEOUT microseconds go by before enough data is read the method will timeout.  If TIMEOUT is undefined, the method will block indefinitely. The function returns the string of data read from the socket. If there is an error, the method will return undef. If the method returns fewer than the requested number of characters, the method timed out.

=cut

sub readn {
    my $self = shift;
    my $sock = $self->{sock};
    my $n = shift;
    my $timeout = shift;
    return undef unless defined $n;
    
    my $result = "";
    my $rin = my $ein = "";
    vec($rin, fileno($sock), 1) = 1;
    $ein = $rin;

    my $count = 0;
    while($count < $n) {
	my $rout;
	my $eout;

	if(!defined $timeout) {
	    select($rout = $rin, undef, $eout = $ein, undef);	   
	}
	else {
#	    print STDERR "timeout is $timeout\n";
	    select($rout = $rin, undef, 
		   $eout = $ein, $timeout / 1000000.0);
	}
	return undef if vec($eout, fileno($sock), 1);
	if(vec($rout, fileno($sock), 1)) {
#	    print STDERR "READING\n";
	    my $diff = sysread($sock, $result, $n - $count, $count);
	    return undef unless defined $diff;
	    $count += $diff;
	}
	else {
	    return $result;
	}
    }
    return $result;
}

=item get_socket()

Returns the socket connected to the Ant server.  If the connection has broken,
this method may return undef.

=cut

sub get_socket {
    my $self = shift;
    return $self->{sock};
}

# private methods from here on down - don't use these.  That means you NICK!

sub connect_to_server {
    my $self = shift;
    my $host = $self->{host};
    my $port = $self->{port};
    my $sock;
    
    print "Connecting to server on $host : $port.\n" if $self->{verbose};
    if(!($sock = IO::Socket::INET->new(Proto     => "tcp",
				       PeerAddr  => $host,
				       PeerPort  => $port))) {
	print "Could not connect to server.\n" if $self->{verbose};
	return 0;
    }
    $sock->autoflush(1);
    $self->{sock} = $sock;
    $self->{prompt} = "";
    print "Connected to server.\n" if $self->{verbose};

    # read the first prompt
    if(!defined $self->read_prompt()) {
	print "Error: Could not read prompt.\n" if $self->{verbose};
	return 0;
    }
    print "Read first prompt.\n" if $self->{verbose};

    return 1;
}

sub close_connection {
    my $self = shift;
    return if !defined $self->{sock};

    close($self->{sock});
    undef $self->{sock};
    print "Disconnected from server.\n" if $self->{verbose};
}

sub get_prompt {
    my $self = shift;
    my $sock = $self->{sock};
    my $read_char = 0;
    my $prompt = "";
    my $rin = "";
    my $c;

    print "Waiting for prompt\n" if $self->{verbose};

    $prompt = $self->readn(1, undef);
    return undef if !defined $prompt or length($prompt) < 1;
    do {
	$c = $self->readn(1, 1);
	return undef if !defined $c;
	$prompt .= $c;
    } while(length($c) == 1);
    print "Read prompt \"$prompt\"\n" if $self->{verbose};
    return $prompt;
}

sub read_prompt {
    my $self = shift;
    my $sock = $self->{sock};
    my $prompt = "";
    my $response;

    if(length($self->{prompt}) == 0) {
	$self->{prompt} = $self->get_prompt();
	return "";
    }
    else {
	until($prompt =~ /$self->{prompt}$/) {
	    my $c = $self->readn(1, undef);
	    return undef if !defined $c or length($c) < 1;
	    $prompt .= $c;
	}
	$prompt =~ /(.*?)(\n|\r)*$self->{prompt}$/s;
	$response = $1;
	return $response;
    }
}

=head1 SEE ALSO

L<Socket>, L<IO::Handle>

=head1 AUTHOR

Michael Montemerlo E<lt>F<mmde@cs.cmu.edu>E<gt>

=head1 COPYRIGHT

Copyright (c) 2001 Michael Montemerlo. All rights reserved. This program is 
free software; you can redistribute it and/or modify it under the same terms
as Perl itself.

=cut

1;
