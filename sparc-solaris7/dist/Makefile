#
VERSION=3.11
RELEASE=2
MACHINE=sparc-solaris7

DRN=drn

#
$(DRN)-$(MACHINE)-$(VERSION).tar.gz: 
	cd ..; make install_dist
	tar cf $(DRN)-$(MACHINE)-$(VERSION)-$(RELEASE).tar $(DRN)-$(VERSION)
	gzip $(DRN)-$(MACHINE)-$(VERSION)-$(RELEASE).tar
