#
#
NEWSBIN=drn-bin
DIST=../freebsd-dist

DIST_DIR=$(DIST)/dist/www/$(NEWSBIN)/
BIN_DIR=$(DIST)/dist/bin/
SETUP_DIR=$(DIST)/dist/www/drn-setup/

make clean depend install_drn install_ddn:
	@echo 'Use gmake'
	gmake $*

install_dist: $(DIST_DIR)wwwnews $(DIST_DIR)wwwprev $(DIST_DIR)wwwnext \
	$(DIST_DIR)wwwheader $(DIST_DIR)wwwpost $(DIST_DIR)wwwinews \
	$(DIST_DIR)wwwlistgrp $(DIST_DIR)wwwsubscribe \
	$(DIST_DIR)wwwdecode $(DIST_DIR)wwwmdecode \
	$(BIN_DIR)drnclean $(SETUP_DIR)finish

$(DIST_DIR)wwwnews $(DIST_DIR)wwwprev $(DIST_DIR)wwwnext \
  $(DIST_DIR)wwwheader $(DIST_DIR)wwwpost $(DIST_DIR)wwwinews \
  $(DIST_DIR)wwwlistgrp $(DIST_DIR)wwwsubscribe \
  $(DIST_DIR)wwwdecode $(DIST_DIR)wwwmdecode \
: ./${*F}
	install -C -o http -g WWW -s ${*F} $(DIST_DIR)

$(BIN_DIR)drnclean: ./${*F}
	install -C -o http -g WWW -s ${*F} $(BIN_DIR)

$(SETUP_DIR)finish: ./${*F}
	install -C -o 0 -g 0 -m 4555 -s ${*F} $(SETUP_DIR)
