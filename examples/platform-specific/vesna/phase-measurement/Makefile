CONTIKI_PROJECT = udp-client udp-server
all: $(CONTIKI_PROJECT)


# Provide a path to vesna-drivers
VSNDRIVERS := ../../../../arch/platform/vesna/vesna-drivers

# Select the VESNA node
TARGET=vesna

# Select the extension/radio board (SNR/ISMTV_v1.1)
VESNA_BOARD = SNR

# Select the onboard radio
VESNA_RADIO = AT86RF233


MAKE_MAC = MAKE_MAC_TSCH
MODULES += os/services/shell

CONTIKI := ../../../..
include $(CONTIKI)/Makefile.include