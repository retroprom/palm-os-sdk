NOTEVIEWDIR = NoteViewLib
BOOKSDIR = BooksLib
SEARCHDIR = SearchLib
PHONEDIR = PhoneBookIOLib
IDLOOKDIR = IdLookupLib

SUBDIRS = $(IDLOOKDIR) $(PHONEDIR) $(SEARCHDIR) $(NOTEVIEWDIR)
# Default Target
all:
	for i in $(SUBDIRS) ; do \
	(cd $$i ; make ) ; \
	done
	
	
	
clean:
	rm -f *~
	for i in $(SUBDIRS) ; do \
	( cd $$i ; make clean) ; \
	done