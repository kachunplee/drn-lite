#
VERSION=3.11
RELEASE=1

DRN=drn-i386-linux2.2-$(VERSION)-$(RELEASE)

/var/ftp/pub/rpm/$(DRN).i386.rpm: \
  /usr/src/redhat/RPMS/i386/$(DRN).i386.rpm
	cp -p /usr/src/redhat/RPMS/i386/$(DRN).i386.rpm /var/ftp/pub/drn

/usr/src/redhat/RPMS/i386/$(DRN).i386.rpm: \
  /usr/src/redhat/SOURCES/$(DRN).tar.gz $(DRN).spec
	rpm -bb $(DRN).spec

/usr/src/redhat/SOURCES/$(DRN).tar.gz: ../$(DRN).tar.gz
	cp -p ../$(DRN).tar.gz /usr/src/redhat/SOURCES
	cp -p ../$(DRN).tar.gz /var/ftp/pub/drn

#../$(DRN).tar.gz:
#	cd ..; make
