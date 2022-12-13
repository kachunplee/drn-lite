#! /bin/sh  

if [ -d /var/local/etc/httpd/decoded ]; then
	cd /var/local/etc/httpd/decoded/000 && {
	find . -type f -mtime +1 -exec rm -f -- {} \; ; }
	cd /var/local/etc/httpd/decoded/temp && {
	find . -type f -mtime +1 -exec rm -f -- {} \; ; }
	cd /var/local/etc/httpd/decoded/lock && {
	find . -type f -mtime +1 -exec rm -f -- {} \; ; }
fi

/var/local/www/bin/checkcache
