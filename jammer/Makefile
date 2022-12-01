CONTIKI_PROJECT = jammer
all: $(CONTIKI_PROJECT)

MAKE_NET = MAKE_NET_NULLNET

ifndef TARGET
	TARGET=sky
endif

ifndef PORT
	PORT=/dev/ttyUSB0
endif

CONTIKI = /home/vagrant/contiki-ng

MODULES += os/services/simple-energest

PLATFORMS_EXCLUDE = nrf52dk

#use this to enable TSCH: MAKE_MAC = MAKE_MAC_TSCH
MAKE_MAC ?= MAKE_MAC_CSMA
MAKE_NET = MAKE_NET_NULLNET
include $(CONTIKI)/Makefile.include


