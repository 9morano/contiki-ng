CONTIKI_PROJECT = hello-world
all: $(CONTIKI_PROJECT)



# Provide a path to vesna-drivers
VSNDRIVERS := ../../../../arch/platform/vesna/vesna-drivers

# Select the VESNA node
TARGET=vesna

# Select the extension/radio board (SNR/ISMTV_v1.1)
VESNA_BOARD = ISMTV_v1.1

# Select the onboard radio
VESNA_RADIO = AT86RF231



#MAKE_MAC = MAKE_MAC_TSCH
MODULES += os/services/shell

CONTIKI := ../../../..
include $(CONTIKI)/Makefile.include