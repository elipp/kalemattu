#!/usr/local/bin/perl -w
# irc.pl A simple IRC robot. Usage: perl irc.pl
use strict;
# We will use a raw socket to connect to the IRC server.
use IO::Socket;
# The server to connect to and our details.
my $server = "open.ircnet.net"; my $nick = "Du_Mu"; my $login = "gallentauska";
# The channel which the bot will join.
my $channel = "#raugallentauska";
# Connect to the IRC server.
my $sock = new IO::Socket::INET(PeerAddr => $server,
                                PeerPort => 6667,
                                Proto => 'tcp') or
                                    die "Can't connect\n";

# Log on to the server.
print $sock "NICK $nick\r\n"; print $sock "USER $login linoud IRCNet :poetry machine\r\n";

# Read lines from the server until it tells us we have connected.
while (my $input = <$sock>) {
    # Check the numerical responses from the server.
    if ($input =~ /004/) {
        # We are now logged in.
        last;
    }
    elsif ($input =~ /433/) {
        die "Nickname is already in use.";
    }
}

print "sending JOIN\n";

# Join the channel.
print $sock "JOIN $channel\r\n";
# Keep reading lines from the server.
while (my $input = <$sock>) {
    chop $input;
    if ($input =~ /^PING(.*)$/i) {
        # We must respond to PINGs to avoid being disconnected.
        print $sock "PONG $1\r\n";
    }
    elsif (index($input, "!poem") != -1) {
        # Print the raw line received by the bot.
	my @verses = split /\n/, `cargo run --release`;
	foreach my $verse (@verses) {
		if ($verse eq "") {
			print $sock "PRIVMSG $channel : \n";
		} else {
			print $sock "PRIVMSG $channel :$verse\n";
		}
	}

	print $sock "PRIVMSG $channel : \n";
    }
    elsif (index($input, "!boem") != -1) {
	my @verses = split /\n/, `cargo run --release -- --chaos`;
	foreach my $verse (@verses) {
		if ($verse eq "") {
			print $sock "PRIVMSG $channel : \n";
		} else {
			print $sock "PRIVMSG $channel :$verse\n";
		}
	}
	print $sock "PRIVMSG $channel : \n";

    }
    else {
        print "$input\n";
    }
}
