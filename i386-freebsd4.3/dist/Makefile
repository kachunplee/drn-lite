#
VERSION=3.11
RELEASE=2
MACHINE=i386-freebsd4.3

DRN=drn

#
$(DRN)-$(MACHINE)-$(VERSION).tar.gz: 
	cd ..; make install_dist
	tar cf $(DRN)-$(MACHINE)-$(VERSION)-$(RELEASE).tar $(DRN)-$(VERSION)
	gzip $(DRN)-$(MACHINE)-$(VERSION)-$(RELEASE).tar
