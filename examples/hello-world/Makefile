CONTIKI_PROJECT = hello-world
all: $(CONTIKI_PROJECT)

# Location of vesna-drivers
VESNA= ../../arch/platform/vesna/vesna-drivers/

TARGET=vesna
AT86RF2XX_BOARD = ISMTV_v1.1

MAKE_MAC = MAKE_MAC_TSCH
MODULES += os/services/shell


CONTIKI = ../..
include $(CONTIKI)/Makefile.include
