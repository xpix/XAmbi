#!/usr/bin/perl -w
# Reads data from serial port and posts to emoncms

# Run apt-get install libdevice-serialport-perl
# if you ger "Can't locate device/SerialPort.pm in @INC (@INC includes ..."

# use lib '/usr/lib/perl5/Device'
# sudo apt-get install libwww-mechanize-perl

# To install Proc:Daemon
# perl -MCPAN -e 'install Proc::Daemon' OR sudo apt-get install libproc-daemon-perl

# Martin Harizanov
# http://harizanov.com

my $APIKEY = '0206c9e46a7b85773dbb9268c4a0217a';
my $EMONURL = sprintf('http://emoncms.org/input/post.json&apikey=%s', $APIKEY);

# Types for sensors in housemon
use constant LetterBox  => 1;
use constant Window     => 2;
use constant Door       => 3;
use constant DoorBell   => 4;

# Types for switchers in housemon
use constant PowerPlug  => 101;
use constant Window     => 102;
use constant Markise    => 103;
use constant DoorBell   => 104;

use lib
'/usr/lib/perl5';
use strict;
use Device::SerialPort qw( :PARAM :STAT 0.07 );
use WWW::Mechanize;
use Time::localtime;
use Scalar::Util 'looks_like_number';
use Proc::Daemon;

print "Serial to EmonCMS gateway for RaspberryPi with TinySensor\r\n";

my $PORT = "/dev/ttyUSB0";

my $ob = Device::SerialPort->new($PORT);
$ob->baudrate(57600);
$ob->parity("none");
$ob->databits(8);
$ob->stopbits(1);
#$ob->handshake("xoff");
$ob->write_settings;

#Configure the TinySensor
$ob->write("\r\n");
$ob->write("30i");   #Node ID=22
sleep 1;
$ob->write("4b");    #868Mhz
sleep 1;
$ob->write("210g");  #Group=210
sleep 1;


open(SERIAL, "+>$PORT");

my $continue = 1;
$SIG{TERM} = sub { $continue = 0 };

while ($continue) {
   my $line = trim(<SERIAL>);
   print "$line\n";
   my @values = split(' ', $line);

   my $nodenr = int(shift @values) or next;

   my $bindata = "";
   for(my $j=1; $j<@values; $j++) {
      $bindata.=sprintf ("%.2x",$values[$j]);
   }
   #print $bindata . "\r\n";
   $bindata=pack("H*",$bindata);

   # call routines for every sensor Group
   sensor_enviroment ($bindata, $nodenr) if($nodenr >=  5 and $nodenr <=  9); # Node  5 -  9 -> Enviroment Sensor (temperature, air pressure, ...)
   sensor_housemon   ($bindata, $nodenr) if($nodenr >= 10 and $nodenr <= 14); # Node 10 - 14 -> House Sensor (Letterbox,Doors,Windows, ...)
   switch_house      ($bindata, $nodenr) if($nodenr >= 20 and $nodenr <= 24); # Node 20 - 24 -> House Switcher (Lamps, Heater, TV, ...)

   #Example of decoding packet content for nodes 17 and 18
   if($values[0]==18 || $values[0]==17) {
      my($temperature, $battery) = unpack("ss",$bindata);
      $temperature /=100;
      print "Temperature $temperature\n";
      print "Battery $battery\n";
   }

   #Example of decoding packet content for node 10
   if($values[0]==10) {
      my($realpower, $apparentpower, $vrms, $powerfactor, $irms) = unpack("ssssf",$bindata);
      print "Real power: $realpower W Apparent Power: $apparentpower W VRMS: $vrms V Power Factor: $powerfactor IRMS: $irms \n\r";
   }


   sleep(1);
}

sub sensor_enviroment {
   my $bindata = shift || return;
   my $nodenr = shift || return warn "No Nodenumber!";

   my($temperature, $battery) = unpack("ss",$bindata);
   $temperature /=100;
print "Temperature $temperature\n";
print "Battery $battery\n";

   return post2emoncms($nodenr,$temperature, $battery);
}

sub sensor_housemon {
   my $bindata = shift || return;
   my $nodenr = shift || return warn "No Nodenumber!";

   # in Example:
   # type=1 (LetterBox), LetterboxNr 1, State filled
   my($type, $nr, $state) = unpack("sss",$bindata);

   if($type == Letterbox){
      printf "Letterbox Nr %d is %s!\n", $nr, ($state ? 'filled' : 'empty');
   }
   elsif($type == Window){
      printf "Window Nr %d is %s!\n", $nr, ($state ? 'open' : 'closed');
   }
   else {
      printf "Type %s unknow!\n", $type;
   }
   return post2emoncms($nodenr, $type,$nr,$state);
}

sub switch_house {
   my $bindata = shift || return;
   my $nodenr = shift || return warn "No Nodenumber!";

   # in Example:
   # type=104 (DoorBell), DoorBellNr 1, Switch on(0|1)
   my($type, $nr, $state) = unpack("sss",$bindata);

   return post2emoncms($nodenr, $type,$nr,$state);
}

sub post2emoncms {
   my $nodenr = shift || die "No Nodenumber!";
   my @values = @_;

   my $ua = WWW::Mechanize->new();
   my $url = $EMONURL.'&node='.$nodenr.'&csv='.join(',',@values);
print $url; print "\r\n";
   my $response = $ua->get($url);
   if ($response->is_success)
   {
      print "Success!\n";
      #my $c = $ua->content;
      #print ("$c");
   }
   else
   {
      print "Failed to update emoncms!";
      die $response->status_line;
   }
   return 1;
}


# Perl trim function to remove whitespace from the start and end of the string
sub trim($)
{
   my $string = shift;
   $string =~ s/^\s+//;
   $string =~ s/\s+$//;
   return $string;
}
#
