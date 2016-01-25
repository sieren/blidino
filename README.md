[![Build
Status](https://travis-ci.org/sieren/blidino.svg?branch=master)](https://travis-ci.org/sieren/blidino)

blidino
=======

Arduino USB-MIDI to MIDI over Bluetooth Project.
This repo hosts sketches and instructions to wirelessly connect USB Class compliant MIDI Devices to iOS / OSX Devices based on the recently released MIDI over Bluetooth protocol by Apple.

The [BLEParser.h](https://github.com/auxren/blidino/blob/master/nRF51822-BLEMIDI/BLEParser.h) is fully templatized and can be injected with any class that supports the modelled functions (it is based around USBH_MIDI).

GoogleMock based Unit-Tests ensure its basic feature-functionality (Single MIDI Notes, Running MIDI Status, SysEx).

Simply execute `./run_unit_tests.sh` and the corresponding CMAKE file should take care of the rest (including fetching GoogleMock and GoogleTest).


Projects
========


[nRF52188](https://github.com/sieren/blidino/tree/master/nRF51822-BLEMIDI)

This project holds code specifically aimed at the *nRF51288* by RedBearLab. It is designed to work out of the box with the [Circuits@Home USB Host Shield 2.0](http://www.circuitsathome.com/products-page/arduino-shields/usb-host-shield-2-0-for-arduino). You can get both boards for a total of around 50$.

[nRF 8100](https://github.com/sieren/blidino/tree/unit-tests/Legacy%20Projects/nRF8001-BLE-Proto)

This (legacy) project used to be the very first prototype built around the wide-spread nRF8001 chip. Unfortunately that chip has a throughput limitation of around 0.9kb/s, not enough for the MIDI rate of 31250 baud.

Known Issues
====================


    • USB to BLE Parsing

    MIDI to BLE-MIDI Parsing needs to be rewritten and made testable. This will be added shortly.
  
    • Recent RedbearLabs SDK breaks USB Host Shield compatibility

    The recent RBL nRF51822 breaks compatibility with the USB Host Shield. I am currently investigating
    and fixing this issue, so Blidino will work with the most recent versions of the SDK and Arduino.
    To make it work, checkout an older commit from the RBL Repo.

[Working RBL SDK](https://github.com/RedBearLab/nRF51822-Arduino/tree/fb705923d4c4e628d9db79527d7fbf99328d9f84)

Specifications
==============
[Apple MIDI over Bluetooth
PDF](https://developer.apple.com/bluetooth/Apple-Bluetooth-Low-Energy-MIDI-Specification.pdf)



Videos
=======

<a href="http://www.youtube.com/watch?feature=player_embedded&v=Dgju2BHyXzk
" target="_blank"><img src="http://img.youtube.com/vi/Dgju2BHyXzk/0.jpg" 
alt="Video" width="240" height="180" border="10" /></a>


<a href="http://www.youtube.com/watch?feature=player_embedded&v=pk6db6KNVUQ
" target="_blank"><img src="http://img.youtube.com/vi/pk6db6KNVUQ/0.jpg" 
alt="Video" width="240" height="180" border="10" /></a>
