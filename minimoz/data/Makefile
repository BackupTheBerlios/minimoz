#
# (c) 2004 Andreas Steinmetz, ast@domdv.de
#
DEFS=../Makedefs
SOURCE=http://rdf.dmoz.org/rdf
CONTENT=content.rdf.u8.gz
STRUCTURE=structure.rdf.u8.gz
REDIRECT=redirect.rdf.u8.gz
TERMS=terms.rdf.u8.gz
DATA=$(CONTENT) $(STRUCTURE) $(REDIRECT) $(TERMS)

include $(DEFS)

.phony: db download clean

ifeq ($(RETRIEVECMD),)

db:
	@echo
	@echo ====================================================
	@echo "No retrieve command specified, can't build database."
	@echo ====================================================
	@echo
	@false
else

db: download rdf.verified dmoz.db

endif

dmoz.db: $(DATA)
	../src/buildmoz && touch dmoz.db || { rm -f dmoz.db ; false ; }

rdf.verified: $(DATA)
ifneq ($(DUMPCHECK),)
	cd .. ; [ ! -x $(DUMPCHECK) ] && true || $(DUMPCHECK) data/$(CONTENT)
	cd .. ; [ ! -x $(DUMPCHECK) ] && true || $(DUMPCHECK) data/$(STRUCTURE)
endif
	touch rdf.verified

download:
	for i in $(DATA) ; do $(RETRIEVECMD) $(SOURCE)/$$i || exit 1 ; done

clean:
	rm -f $(DATA) rdf.verified dmoz.db
