CONTIKI_PROJECT = hello-world
all: $(CONTIKI_PROJECT)



# Provide a path to vesna-drivers
VSNDRIVERS := ../../../../arch/platform/vesna/vesna-drivers

# Select the VESNA node
TARGET=vesna

# Select the extension/radio board
AT86RF2XX_BOARD = ISMTV_v1.1



MAKE_MAC = MAKE_MAC_TSCH
MODULES += os/services/shell

CONTIKI := ../../../..
include $(CONTIKI)/Makefile.include