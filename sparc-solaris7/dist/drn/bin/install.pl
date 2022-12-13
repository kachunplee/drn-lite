#! /usr/bin/perl

my $DRN_DIR;
if($0 =~ /^\//)
{
	$DRN_DIR = $0;
}
else
{
	$DRN_DIR = `pwd`;
	chop($DRN_DIR);

	$0 =~ s/^\.\///;
	$DRN_DIR .= '/' . $0;
}

$DRN_DIR =~ s/\/bin\/[^\/]+$//;
print "Using DRN dir @ ", $DRN_DIR, "\n";

#
#
my $Brower;
if($ARGV[0] eq "-b")
{
	shift;
	$Brower = shift;
}

#
# Make directories
#
chdir $DRN_DIR || die "Cannot chdir to $DRN_DIR";
$dirs = "tmp www/db www/ov.index www/dd.index www/decoded www/user";
system("for dir in $dirs
	do
		if [ ! -d \$dir ]
		then
			mkdir -p \$dir
		fi
	done
");

chdir "www/decoded" || die "Cannot chdir to www/decoded";
my $i;
for($i = 0; $i <= 256; $i++)
{
	mkdir sprintf("%03d", $i), 0775;
}

mkdir "lock", 0775;
mkdir "temp", 0775;

#
# Looking for httpd config files
#
my $HTTP_CONF;

if($ARGV[0] && -f $ARGV[0])
{
	$HTTP_CONF = $ARGV[0];
}
elsif($ENV{"HTTP_CONF"} && -f $ENV{"HTTP_CONF"})
{
	$HTTP_CONF = $ENV{"HTTP_CONF"};
}
elsif( -f ($HTTP_CONF = "/var/local/apache/conf/httpd.conf"))
{
}
elsif( -f ($HTTP_CONF = "/var/local/etc/httpd/conf/httpd.conf"))
{
}
elsif( -f ($HTTP_CONF = "/etc/httpd/conf/httpd.conf"))
{
}
else
{
	print "Cannot find apache config file...\n";
	$HTTP_CONF = "";
}

if($HTTP_CONF ne "" && !open(CONF, $HTTP_CONF))
{
	print "Cannot open $HTTP_CONF\n";
	$HTTP_CONF = "";
}

if($HTTP_CONF eq "")
{
	print <<EOM;

You can continue DRN install by:

	$DRN_DIR/bin/install1.pl CONF_FILE
EOM
	exit 1;
}

print "Using Apache config @ ", $HTTP_CONF, "\n";

my $TMP_CONF = "$HTTP_CONF.tmp";
if(!open(TMPCONF, ">$TMP_CONF"))
{
	print "Cannot create $TMP_CONF\n";
	exit 1;
}

while(<CONF>)
{
	last if(/^#<-- DRN-install/);

	if(/^\s*PidFile\s+(\S+)/)
	{
		$PidFile = $1;
	}

	print TMPCONF $_;

}

while(<CONF>)
{
	last if(/#-->/);
}

while(<CONF>)
{
	print TMPCONF $_;
}

print TMPCONF <<EOM;
#<-- DRN-install
#
Alias /drn-install/ /var/local/drn/drn-install/
Alias /drn-setup/images/ /var/local/drn/www/drn-setup/images/

ScriptAlias /drn-setup/ $DRN_DIR/www/drn-setup/
<Location /drn-setup/>
	options none
	AllowOverride None
	order deny,allow
	deny from all
	allow from localhost
</Location>

ScriptAlias /drn-admin/ $DRN_DIR/www/drn-admin/
<Location /drn-admin>
	options none
	AllowOverride None
	order deny,allow
	deny from all
	allow from localhost
</Location>
#-->
EOM

close CONF;
if(! close TMPCONF)
{
	print "Error closing new HTTP config file ($TMPCONF)\n";
	exit 1;
}

rename $HTTP_CONF, "$HTTP_CONF.old";
rename $TMP_CONF, $HTTP_CONF;

#
# Generate drn.conf
#
my $DRN_CONF = "$DRN_DIR/www/etc/drn.conf";
if(!open(DRNCONF, "> $DRN_CONF.tmp"))
{
	print "Cannot create $DRN_CONF.tmp\n";
	exit 1;
}

print DRNCONF <<EOM;
#[System]
\$ApacheConf="$HTTP_CONF";
\$WWWRoot="$DRN_DIR/www";

#[Global]
\$NNTPservers="news";
\$NNTPauths="";
\$AccessHost="";
\$AdminHost="localhost";

#[User]
\$sort="D";
\$AttachSplitSize=7000;
\$MaxSplitSize=10000;
\$PageLength=50;

#[End]
EOM

close DRNCONF;

rename $DRN_CONF, "$DRN_CONF.old";
rename "$DRN_CONF.tmp", $DRN_CONF;

if($PidFile && -f $PidFile)
{
	#
	# if running in X-Window
	#
	if(system("kill -USR1 `cat $PidFile`") == 0)
	{
		my $installURL = "file:$DRN_DIR/drn-install/setup.html";

		if($Brower ne "")
		{
			exec("$Brower $installURL&")
				if(-f $Brower);
		}
		else
		{
			exec("/usr/local/bin/netscape $installURL&")
				if(-f "/usr/local/bin/netscape");

			exec("/usr/bin/netscape $installURL&")
				if(-f "/usr/bin/netscape");
		}
	}
}
else
{
	print "Cannot find HTTP pid file... please reload HTTP config manually\n";
}

print <<EOM;

You may want to review $HTTP_CONF.

Currently, permission for DRN setup directories are only enabled for localhost.
If you want to setup DRN remotely with a brower, you will need to adjust
the permission settings of the DRN setup directories.

When ready, you can continue install DRN with a brower, locally:

	file:$DRN_DIR/drn-install/setup.html

or, remotely:

	http://hostname/drn-install/setup.html
EOM
