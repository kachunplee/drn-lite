#
VERSION=3.11
RELEASE=2
MACHINE=i386-linux2.2

DRN=drn-$(MACHINE)

#
$(DRN)-$(VERSION).tar.gz: 
	cd ..; make install_dist
	cd $(DRN)-$(VERSION); find . ! \( -type d -o -name 'flist' -o -name 'README' -o -name 'INSTALL' \) | sed -n 's/^./\/var\/local\/drn/p' > flist
	tar cf $(DRN)-$(VERSION)-$(RELEASE).tar $(DRN)-$(VERSION)
	gzip $(DRN)-$(VERSION)-$(RELEASE).tar
