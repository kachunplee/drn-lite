#! /usr/bin/perl

my $pwd = $0;
$pwd =~ s/\/[^\/]*$//;
chdir $pwd;

$| = 1;

#
print <<EOM;
Content-type: text/html

<HTML>
<HEAD>
<TITLE>DRN Setup - Input</TITLE>
</HEAD>
<body vlink="#000000" alink="#000000" basefont="3" topmargin="0" leftmargin="0" link="#000000" bgcolor="#FFFFFF" text="#000000">
<div align="center"><center>

<table border="0" cellpadding="0" width="602" bgcolor="#FFFFFF" bordercolordark="#FFFFFF" cellspacing="0" bordercolor="#000000" bordercolorlight="#000000">
  <tr>
    <td align="center" bgcolor="#FFFFFF" width="582"><img border="0" src="/drn-setup/images/startupbar.gif" width="600" height="47"></td>
  </tr>
  <tr>
    <td valign="top" bgcolor="#FFFFFF" align="left" width="582"><div align="center"><center><table border="1" cellpadding="10" cellspacing="0" width="600" bordercolor="#326698">
      <tr>
        <td bordercolor="#6380A7">
    </center>
</center>
      <table border="0" cellpadding="7" cellspacing="0" width="560">
        <tr>
          <td valign="top" width="180"><img border="0" src="/drn-setup/images/startside4.gif" width="173" height="290"></td>
          <td valign="top">
            <p align="center">&nbsp;</p>
            <p align="center"><strong><font face="Arial" size="2" color="#326698">
EOM

#
# Load config file
#
$AdminHost = 'localhost';

if(open(CONF, '../etc/drn.conf'))
{
	while(<CONF>) { eval; print STDERR $@; }
}

%FORM = ();
if ($ENV{'REQUEST_METHOD'} eq 'POST')
{
	my $inbuf;
	if ($ENV{'CONTENT_LENGTH'})
	{
		read(STDIN, $inbuf, $ENV{'CONTENT_LENGTH'});
	}

	my @pairs = split(/&/, $inbuf);
	my $name, $value;
	foreach $pair (@pairs)
	{
		($name, $value) = split(/=/, $pair);
		$value =~ tr/+/ /;
		$value =~ s/%([a-fA-F0-9][a-fA-F0-9])/pack("C", hex($1))/eg;
		$FORM{$name} = $value;
	}
}

$VirtualDomain = $FORM{'VirtualDomain'} if(defined($FORM{'VirtualDomain'}));

$DrnServer = ($FORM{'DrnServer'} ne "") ? $FORM{'DrnServer'} : $VirtualDomain;
$DrnServer = "http://$DrnServer";

$NNTPservers = $FORM{'NNTPservers'} if(defined($FORM{'NNTPservers'}));
$NNTPauths = $FORM{'NNTPauths'} if(defined($FORM{'NNTPauths'}));
$OverviewDir = $FORM{'OverviewDir'} if(defined($FORM{'OverviewDir'}));
$AccessHost = $FORM{'AccessHost'} if(defined($FORM{'AccessHost'}));
$AdminHost = $FORM{'AdminHost'} if(defined($FORM{'AdminHost'}));

#
$ApacheConf = '/var/local/apache/conf/httpd.conf'
	if(!defined($ApacheConf) || $ApacheConf eq "");

$WWWRoot = "/var/local/drn/www" if(!defined($WWWRoot) || $WWWRoot eq "");
&SaveDrnConf($ApacheConf, $WWWRoot, $NNTPservers, $OverviewDir, $NNTPauths,
		$AccessHost, $AdminHost);

#
# Setting up Apache Server
#
print "<strong><font face=\"Arial\" size=\"2\" color=\"#326698\">Setting up Apache web server with config file at $ApacheConf</font></strong></br>";

if(!open(CONF, $ApacheConf))
{
	my $filename = "/var/local/drn/tmp/httpd.conf";
	if(open(SAVE, "> $filename"))
	{
		my $oldHandle = select(SAVE);
		&AddApacheConf($WWWRoot, $AccessHost, $AdminHost, '');
		select($oldHandle);
		close(SAVE);
		print "<strong><font face=\"Arial\" size=\"2\" color=\"#326698\">Cannot open the $ApacheConf file you provided. The ";
		print "DRN configuration are saved in \"$filename\". ";
		print "Please copy it to your Apache configuration file</font></strong></br>\n";
		&OutEndLinks();
	}
	else
	{
		print "<strong><font face=\"Arial\" size=\"2\" color=\"#326698\">Cannot open $ApacheConf file you provided and cannot ";
		print "open temporary $filename file to save ";
		print "the DRN configuration.</font></strong></br>\n";
		print "<strong><font face=\"Arial\" size=\"2\" color=\"#326698\">Please re-run setup and provide the correct Apache ";
		print "Configuration Directory.</font></strong></br>\n";
		print '<p><center><a href="javascript:close();"><img border="0" src="/drn-setup/images/installfinish.gif"></a></center>';
		print "</td>";
       	print "</tr>";
      	print "</table>";
        print "</td>";
      	print "</tr>";
    	print "</table>";
        print "</div></td>";
  		print "</tr>";
		print "</table>";
		print "</div>";
		print "</body>";
	}
	
	print "</html>";
	exit(1);
}

# scan through the config file and setup all the document roots
if(!open(CTEMP, ">$ApacheConf.tmp"))
{
	my $filename = "/var/local/drn/tmp/httpd.conf";
	if(open(SAVE, "> $filename"))
	{
		my $oldHandle = select(CONF);
		&AddApacheConf($WWWRoot, $AccessHost, $AdminHost, '');
		select($oldHandle);
		close(SAVE);
		print "<strong><font face=\"Arial\" size=\"2\" color=\"#326698\">Cannot open the $ApacheConf.tmp file to save the new ";
		print "DRN configurations. The ";
		print "DRN configurations are saved in \"$filename\". ";
		print "Please copy it to your Apache configuration file</font></strong></br>\n";
		&OutEndLinks();
	}
	else
	{
		print "<strong><font face=\"Arial\" size=\"2\" color=\"#326698\">Cannot open $ApacheConf file you provided and cannot ";
		print "open temporary $filename file to save ";
		print "the DRN configuration.</font></strong></br>\n";
		print "<strong><font face=\"Arial\" size=\"2\" color=\"#326698\">Please re-run setup and provide the correct Apache ";
		print "Configuration Directory.</font></strong></br>\n";
		print '<p><center><a href="javascript:close();"><img border="0" src="/drn-setup/images/installfinish.gif"></a></center>';
	}
	print "</td>";
       	print "</tr>";
      	print "</table>";
        print "</td>";
      	print "</tr>";
    	print "</table>";
        print "</div></td>";
  	print "</tr>";
	print "</table>";
	print "</div>";
	print "</body>";
	print "</html>";
	close(CONF);
	exit(1);
}

my $stg = "";
my $pidFile = "";
my $bVHost = 0;
my $bMatch = 0;
my $bAdded = 0;
my $bSkip = 0;
my $line;
while(<CONF>)
{
	chop;
	if($bSkip)
	{
		$bSkip = 0 if(/#-->/);
		next;
	}

	if(/#<-- DRN-setup - $VirtualDomain/o)
	{
		$bSkip = 1;
		next;
	}

	$line = $_;
	if($line =~ /^\s*<\/VirtualHost>$/)
	{
		# end of a virtual host section
		if($bMatch == 1)
		{
			# end of the matching virtual host
			print CTEMP "\n";
			my $oldHandle = select(CTEMP);
			&AddApacheConf($WWWRoot, $AccessHost, $AdminHost, '    ');
			select($oldHandle);
			print CTEMP $line, "\n";
			$bAdded = 1;
			last;
		}
		$bVHost = 0;
	}
	elsif($line =~ /^\s*<VirtualHost\s+(.+)>/)
	{
		$stg = $1;
		if(defined($VirtualDomain) && $VirtualDomain ne "" &&
			$stg eq $VirtualDomain)
		{
			$bMatch = 1;
		}
		$bVhost = 1;
	}
	elsif($line =~ /^\s*DocumentRoot\s+(\S+)/)
	{
		$stg = $1;
		if($bMatch)
		{
			# in the matching virtual section, find the document root
			$DocRoot = $stg;
		}
		else
		{
			if(!defined($VirtualDomain) || $VirtualDomain ne "" && $bVHost == 0)
			{
				# no virtual domina was specified and in the root section,
				#	find the document root
				$DocRoot = $stg;
			}
		}
		#
		# Did not have a use for $DocRoot for now
		#
	}
	elsif($line =~ /^\s*PidFile\s+(\S+)/)
	{
		$pidFile = $1;
	}
	elsif($line =~ /^\s*User\s+(\S+)/)
	{
		$Userid = $1;
	}
	elsif($line =~ /^\s*Group\s+(\S+)/)
	{
		$Groupid = $1;
	}
	print CTEMP $line,"\n";
}
while(<CONF>)
{
	# finish up copy the rest of the conf file
	print CTEMP $_;
}
if($bAdded == 0)
{
	# the new DRN section has not added to the tmp conf file yet, added it as
	#	comment with a leading '#' if virtual domain is defined but not found
	if(!defined($VirtualDomain) || $VirtualDomain eq "")
	{
		$bAdded = 1;
	}
	print CTEMP "\n";
	my $oldHandle = select(CTEMP);
	&AddApacheConf($WWWRoot, $AccessHost, $AdminHost, ($bAdded == 1) ? '' : '#');
	select($oldHandle);
}
close(CTEMP);
close(CONF);

rename $ApacheConf, "$ApacheConf.old";
rename "$ApacheConf.tmp", $ApacheConf;

if($Userid ne "")
{
	my $dirs = "www ov.index";
	my $chggroups = "chgrp -R $Groupid $dirs" if($Groupid ne "");
	system("
		cd /var/local/drn
		chown -R $Userid $dirs
		$chggroups
	    ");
}

if($bAdded == 1)
{
	# added the DRN config section and found the pid file, reset HTTP now
	if($pidFile ne "")
	{
		print "<strong><font face=\"Arial\" size=\"2\" color=\"#326698\">Send signal to httpd to reload $pidFile</font></strong></br>\n";
		system("kill -USR1 `cat $pidFile`");
	}
}

#
#
#
print "<strong><font face=\"Arial\" size=\"2\" color=\"#326698\">Direct Read News setup completed</font></strong></br>";

#
#
#
&OutEndLinks();
exit(0);

sub OutEndLinks
{
	print <<EOM;
<p><center><a href=/drn-admin/formdefault target="_blank"><img border="0" src="/drn-setup/images/installdefaults.gif"></a>
<a href="$DrnServer/drn/index.html" target="_blank"><img border="0" src="/drn-setup/images/installtrydrn.gif"></a>
<a href="javascript:close();"><img border="0" src="/drn-setup/images/installfinish.gif"></a></center>
          </td>
        </tr>
      </table>
        </td>
      </tr>
    </table>
        </div></td>
  </tr>
</table>
</div>
</body>

EOM
}

#
# Add DRN config to Apache config file
#
sub AddApacheConf
{
	my($WWWRoot, $AccessHost, $AdminHost, $lchar) = @_;

	print $lchar, "#<-- DRN-setup - $VirtualDomain\n";
	print $lchar, "#\n";
	print $lchar, "Alias /drn/ $WWWRoot/html/\n";
	if($AccessHost ne "")
	{
		print $lchar, "<Location /drn>\n";
		print $lchar, "    options none\n";
		print $lchar, "    AllowOverride None\n";
		print $lchar, "    order deny,allow\n";
		print $lchar, "    deny from all\n";
		print $lchar, "    allow from $AccessHost\n";
		print $lchar, "</Location>\n";
	}
	print $lchar, "\n";
	print $lchar, "ScriptAlias /drn-bin/ $WWWRoot/drn-bin/\n";
	if($AccessHost ne "")
	{
		print $lchar, "<Location /drn-bin>\n";
		print $lchar, "    order deny,allow\n";
		print $lchar, "    deny from all\n";
		print $lchar, "    allow from $AccessHost\n";
		print $lchar, "</Location>\n";
	}
	print $lchar, "\n";
	print $lchar, "ScriptAlias /drn-admin/ $WWWRoot/drn-admin/\n";
	if($AdminHost ne "")
	{
		print $lchar, "<Location /drn-admin>\n";
		print $lchar, "    order deny,allow\n";
		print $lchar, "    deny from all\n";
		print $lchar, "    allow from $AdminHost\n";
		print $lchar, "</Location>\n";
	}
	print $lchar, "\n";
	print $lchar, "Alias /decoded/ $WWWRoot/decoded/\n";
	if($AccessHost ne "")
	{
		print $lchar, "<Location /decoded>\n";
		print $lchar, "    options none\n";
		print $lchar, "    AllowOverride None\n";
		print $lchar, "    order deny,allow\n";
		print $lchar, "    deny from all\n";
		print $lchar, "    allow from $AccessHost\n";
		print $lchar, "</Location>\n";
	}
	print $lchar, "#-->\n";
}

#
# Save DRN configuration into DRN config file
#
sub SaveDrnConf
{
	my ($ApacheConf, $WWWRoot, $NNTPservers, $OverviewDir, $NNTPauths,
		$AccessHost, $AdminHost) = @_;
	my $DRNCONF="../etc/drn.conf";

	rename $DRNCONF, "$DRNCONF.old";
	if(!open(CONF, "> $DRNCONF"))
	{
		print "<strong><font face=\"Arial\" size=\"2\" color=\"#326698\">Cannot write into drn.conf</font></strong></br>";
		return;
	}

	#
	# Should save user default
	#
	print "<strong><font face=\"Arial\" size=\"2\" color=\"#326698\">Creating drn.conf</font></strong></br>";

	print CONF "#[System]\n";
	print CONF "\$ApacheConf=\"$ApacheConf\";\n" if(defined($ApacheConf));
	print CONF "\$VirtualDomain=\"$VirtualDomain\";\n" if(defined($VirtualDomain));
	print CONF "\$DrnServer=\"$DrnServer\";\n" if(defined($DrnServer));
	print CONF "\$WWWRoot=\"$WWWRoot\";\n" if(defined($WWWRoot));
	print CONF "\n";
	print CONF "\#[Global]\n";
	print CONF "\$NNTPservers=\"$NNTPservers\";\n" if(defined($NNTPservers));
	print CONF "\$NNTPauths=\"$NNTPauths\";\n" if(defined($NNTPauths));
	print CONF "\$OverviewDir=\"$OverviewDir\";\n" if(defined($OverviewDir));
	print CONF "\$AccessHost=\"$AccessHost\";\n" if(defined($AccessHost));
	print CONF "\$AdminHost=\"$AdminHost\";\n" if(defined($AdminHost));
	print CONF "\n";
	print CONF "#[User]\n";
	print CONF "\n";
	print CONF "#[End]\n";
	close(CONF);
}
