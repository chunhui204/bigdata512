
VERSION := 1.0.0
PACKAGE := linuxtv-dvb-$(VERSION)


all:  dvb 

dvb:
	(cd driver; make)
	(cd apps/av7110_loadkeys; make)
	(cd apps/dvbnet; make)
	(cd apps/scan; make)
	(cd apps/test; make)
	(cd apps/szap; make)


CVSROOT     := $(shell cat CVS/Root)
RELEASE_TAG := LINUXTV-DVB-$(subst .,_,$(subst -,_,$(VERSION)))

release dist:
	rm -rf release-tmp $(PACKAGE).tar.gz
	mkdir release-tmp
	( cd release-tmp; cvs -d$(CVSROOT) export -r$(RELEASE_TAG) -d$(PACKAGE) DVB )
	# work around CVS braindeadness wrt file permissions
	chmod -x release-tmp/$(PACKAGE)/Makefile
	chmod -x release-tmp/$(PACKAGE)/driver/av7110/av7110.c
	chmod -x release-tmp/$(PACKAGE)/include/linux/em8300.h
	find release-tmp -name .cvsignore | xargs rm -v
	( cd release-tmp; tar czf ../$(PACKAGE).tar.gz $(PACKAGE) )
	rm -rf release-tmp
	@echo
	@echo --------------------------------------------------------------------------------
	@echo
	@echo "driver package: ./$(PACKAGE).tar.gz"
	@echo
	@echo --------------------------------------------------------------------------------
	@echo


clean:
	(cd driver; make clean)
	(cd apps/av7110_loadkeys; make clean)
	(cd apps/dvbnet; make clean)
	(cd apps/scan; make clean)
	(cd apps/test; make clean)
	(cd apps/szap; make clean)



