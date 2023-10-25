<img src="https://github.com/contiki-ng/contiki-ng.github.io/blob/master/images/logo/Contiki_logo_2RGB.png" alt="Logo" width="256">

# Contiki-NG: The OS for Next Generation IoT Devices

[![Github Actions](https://github.com/contiki-ng/contiki-ng/workflows/CI/badge.svg?branch=develop)](https://github.com/contiki-ng/contiki-ng/actions)
[![Documentation Status](https://readthedocs.org/projects/contiki-ng/badge/?version=master)](https://contiki-ng.readthedocs.io/en/master/?badge=master)
[![license](https://img.shields.io/badge/license-3--clause%20bsd-brightgreen.svg)](https://github.com/contiki-ng/contiki-ng/blob/master/LICENSE.md)
[![Latest release](https://img.shields.io/github/release/contiki-ng/contiki-ng.svg)](https://github.com/contiki-ng/contiki-ng/releases/latest)
[![GitHub Release Date](https://img.shields.io/github/release-date/contiki-ng/contiki-ng.svg)](https://github.com/contiki-ng/contiki-ng/releases/latest)
[![Last commit](https://img.shields.io/github/last-commit/contiki-ng/contiki-ng.svg)](https://github.com/contiki-ng/contiki-ng/commit/HEAD)

[![Stack Overflow Tag](https://img.shields.io/badge/Stack%20Overflow%20tag-Contiki--NG-blue?logo=stackoverflow)](https://stackoverflow.com/questions/tagged/contiki-ng)
[![Gitter](https://img.shields.io/badge/Gitter-Contiki--NG-blue?logo=gitter)](https://gitter.im/contiki-ng)
[![Twitter](https://img.shields.io/badge/Twitter-%40contiki__ng-blue?logo=twitter)](https://twitter.com/contiki_ng)

Contiki-NG is an open-source, cross-platform operating system for Next-Generation IoT devices. It focuses on dependable (secure and reliable) low-power communication and standard protocols, such as IPv6/6LoWPAN, 6TiSCH, RPL, and CoAP. Contiki-NG comes with extensive documentation, tutorials, a roadmap, release cycle, and well-defined development flow for smooth integration of community contributions.

Unless explicitly stated otherwise, Contiki-NG sources are distributed under
the terms of the [3-clause BSD license](LICENSE.md). This license gives
everyone the right to use and distribute the code, either in binary or
source code format, as long as the copyright license is retained in
the source code.

Contiki-NG started as a fork of the Contiki OS and retains some of its original features.

Find out more:

* GitHub repository: https://github.com/contiki-ng/contiki-ng
* Documentation: https://docs.contiki-ng.org/
* List of releases and changes: https://github.com/contiki-ng/contiki-ng/releases
* Web site: http://contiki-ng.org

Engage with the community:

* Discussions on GitHub: https://github.com/contiki-ng/contiki-ng/discussions
* Contiki-NG tag on Stack Overflow: https://stackoverflow.com/questions/tagged/contiki-ng
* Gitter: https://gitter.im/contiki-ng
* Twitter: https://twitter.com/contiki_ng


# Support for VESNA platform

[VESNA](https://log-a-tec.eu/hw-vesna.html) is a modular and fully flexible platform for the development of wireless sensor networks developed at the [Department of Communication Systems](https://e6.ijs.si/), Jozef Stefan Institute.

This branch includes support for VESNA devices

Usage: `TARGET=vesna`


## SNC

Sensor Node Core (SNC) is a standalone, base board of VESNA platform. It provides connectivity to various peripherals (UART, I2C, SPI, USB, ADC, ...) and provides various energy supply options (battery, solar, external power supply). 
The board can be upgraded with various extensions, that are grouped in: Sensor Node Radios (SNR) and Sensor Node Extension (SNE).

Usage: `BOARD=snc`

## SNR

SNR is used to connect VESNA nodes into a wireless management network. 
SNR can be equipped with multiple different radios: 
- Texas Instrument CC1101 & CC2500
- Atmel AT86RF212 (868 MHz), AT86RF231 (2.4 GHz) and AT86RF233 (2.4 GHz)

This port currently supports interface only for Atmel's radios

Usage:`BOARD=snr` `RADIO=AT86RF2xx`


## SNE-ISMTV

SNE-ISMTV adds general-purpose radio-frequency transceiver hardware to the VESNA wireless sensor node. 
There are 2 versions of the SNE-ISMTV boards: v1.0 and v1.1.
SNE_ISMTV can be equipped with multiple different radios: 
- Texas Instrument CC1101 & CC2500
- Atmel AT86RF212 (868 MHz), AT86RF231 (2.4 GHz) and AT86RF233 (2.4 GHz)

This port currently supports interface only for Atmel's radios

Usage:`BOARD=sne-ismtv-v1-1` `RADIO=AT86RF2xx`


## SNE-ATASW

Equipped with two band AT86RF215 (868 MHz and 2.4 GHz), SNE-ATASW enables experimentation with multiple RF bands and different types of modulation. The 2.4 GHz band is further extended with antenna switch, that can connect the radio to up to 8 antennas.

Usage:`BOARD=sne-atasw` `RADIO=AT86RF215`
