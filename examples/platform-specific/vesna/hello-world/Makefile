CONTIKI_PROJECT = hello-world
all: $(CONTIKI_PROJECT)



# Provide a path to vesna-drivers
VSNDRIVERS := ../../../../arch/platform/vesna/vesna-drivers

# Select the VESNA node
TARGET=vesna

# Select the extension/radio board (snr/sne-ismtv-v1-1)
BOARD = sne-atasw
# Select the onboard radio
RADIO = AT86RF215



MAKE_MAC = MAKE_MAC_TSCH
MODULES += os/services/shell

CONTIKI := ../../../..
include $(CONTIKI)/Makefile.include
