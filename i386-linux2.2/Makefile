#
#
NEWSBIN=drn-bin
DIST=dist

DIST_DIR=$(DIST)/drn/www/$(NEWSBIN)/
BIN_DIR=$(DIST)/drn/bin/
SETUP_DIR=$(DIST)/drn/www/drn-setup/

install_dist: $(DIST_DIR)wwwnews $(DIST_DIR)wwwprev $(DIST_DIR)wwwnext \
	$(DIST_DIR)wwwheader $(DIST_DIR)wwwpost $(DIST_DIR)wwwinews \
	$(DIST_DIR)wwwdecode $(DIST_DIR)wwwmdecode \
	$(DIST_DIR)wwwsubscribe $(DIST_DIR)wwwlistgrp \
	$(SETUP_DIR)finish

#	$(BIN_DIR)drnclean

$(DIST_DIR)wwwnews $(DIST_DIR)wwwprev $(DIST_DIR)wwwnext \
  $(DIST_DIR)wwwheader $(DIST_DIR)wwwpost $(DIST_DIR)wwwinews \
  $(DIST_DIR)wwwdecode $(DIST_DIR)wwwmdecode \
  $(DIST_DIR)wwwsubscribe $(DIST_DIR)wwwlistgrp \
: ./${*F}
	install -C -o http -g WWW -s ${*F} $(DIST_DIR)

$(BIN_DIR)drnclean: ./${*F}
	install -C -o http -g WWW -s ${*F} $(BIN_DIR)

$(SETUP_DIR)finish: ./${*F}
	install -C -o 0 -g 0 -m 4555 -s ${*F} $(SETUP_DIR)
