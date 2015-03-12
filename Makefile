#
# Makefile pour generer les executables du repeteur virtuel
#

OBJS_SWITCH=libnet.o virtual_hub.o
OBJS_CLIENT=libnet.o virtual_client.o

CFLAGS += -Wall -DDEBUG

all: virtual_hub virtual_client

#
# La cible de nettoyage
#

clean: 
	rm -f core *.o virtual_hub virtual_client

#
# Les cibles
#

virtual_hub:	$(OBJS_SWITCH)
	$(CC) $(CFLAGS) -o $@ $(OBJS_SWITCH)
virtual_client:	$(OBJS_CLIENT)
	$(CC) $(CFLAGS) -o $@ $(OBJS_CLIENT)

libnet.o: libnet.c libnet.h
virtual_hub.o: virtual_hub.c libnet.h
virtual_client.o: virtual_client.c libnet.h
