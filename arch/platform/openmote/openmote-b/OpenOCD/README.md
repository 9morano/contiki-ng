# OpenOCD

Programming OpenMote-B via JTAG using Olimex adapter works with openocd version JIM: https://git.jim.sh/jim/openocd
It includes flash loader and glue code for cc2538, while official version of OpenOCD does not.

## JTAG selection

OpenMote-B includes multiplexer (TS3A27518E) to select between two JTAG interfaces. To program the platform using the onboard JTAG connector, the JTAG_SEL pin on the OpenMote-B board must be disconnected (it is by default).

## Wiring

10-pin JTAG connector on the board (orientation: antenna down, USB connector up):
+-----+-----+
| VCC | TMS |
| GND | TCLK|
| GND | TDO |
|  *  | TDI |
| GND | RST |
+-----+-----+

Connection with Olimex 20-pin JTAG
_________________________
TMS       -->  TTMS/SWDIO
TCK       -->  TTCK/SWCLK
TDO       -->  TTDO/SWO
TDI       -->  TTDI
RST       -->  TnSRST
VCC       -->  VREF
GND       -->  GND

The device must be powered (via USB).

## Adapter support

Current implementation supports Olimex ARM-USB-OCD and ARM-USB-OCD-H adapters. To flash the code to the OpenMoteB, run either:

$ make hello-world.olimex 
$ make hello-world.olimex-h

While installing the OpenOCD, do not forget to use the contrib/60-openocd.rules file, otherwise your access to the USB device will be denied. The file belongs somewhere in /etc/udev/rules.d.