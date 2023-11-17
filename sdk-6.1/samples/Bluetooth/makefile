BTCHAT = BtChat
BTECHO = BtEchoService
BTHAND = BtHandsfree
BTHEAD = BtHeadset
BTTEST = BtLibTest
BTPICONET = BtPiconet
WHITEBOARD = WhiteBoard

SUBDIRS = $(BTCHAT) $(BTECHO) $(BTHAND) $(BTHEAD) $(BTTEST) $(BTPICONET) $(WHITEBOARD) 
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