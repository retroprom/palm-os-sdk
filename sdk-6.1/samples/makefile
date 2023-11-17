AARD = Aardvark
PINL = addons/SamplePinlet
BASI = BasicApp
BTCHAT = bluetooth/BtChat
BTECHO = bluetooth/BtEchoService
BTHAND = bluetooth/BtHandsfree
BTHEAD = bluetooth/BtHeadset
BTTEST = bluetooth/BtLibTest
BTPICONET = bluetooth/BtPiconet
WHITEBOARD = bluetooth/WhiteBoard
GCDW = GcDraw
GCFN = GcFonts
SCHM = Schemer

NOTEVIEWDIR = Libraries/NoteViewLib
SEARCHDIR = Libraries/SearchLib
PHONEDIR = Libraries/PhoneBookIOLib
IDLOOKDIR = Libraries/IdLookupLib

ADDR = Address
DATE = Datebook
MEMO = memo
TODO = todo
SUBDIRS = $(AARD) $(PINL) $(BASI) $(GCDW) $(GCFN) $(SCHM) \
	$(BTCHAT) $(BTECHO) $(BTHAND) $(BTHEAD) $(BTTEST) $(BTPICONET) $(WHITEBOARD) \
	$(IDLOOKDIR) $(PHONEDIR) $(SEARCHDIR) $(NOTEVIEWDIR) \
	$(ADDR) $(MEMO) $(DATE) $(TODO)
# Default Target
all:
	for i in $(SUBDIRS) ; do \
	(cd $$i ; make; make TARGET_PLATFORM=Device;make \
	DEBUG_OR_RELEASE=Release;make TARGET_PLATFORM=Device DEBUG_OR_RELEASE=Release) ; \
	done
	
	
	
	
clean:
	rm -f *~
	for i in $(SUBDIRS) ; do \
	(cd $$i ; make clean; make clean TARGET_PLATFORM=Device;make clean \
	DEBUG_OR_RELEASE=Release;make clean TARGET_PLATFORM=Device DEBUG_OR_RELEASE=Release) ; \
	done
	 
