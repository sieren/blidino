[![Build
Status](https://travis-ci.org/sieren/blidino.svg?branch=master)](https://travis-ci.org/sieren/blidino)

blidino
=======

Arduino USB-MIDI to MIDI over Bluetooth Project.
This repo hosts sketches and instructions to wirelessly connect USB Class compliant MIDI Devices to iOS / OSX Devices based on the recently released MIDI over Bluetooth protocol by Apple.

I am not that experienced with vanilla C, any suggestions to improve the code quality are welcome.

Projects
========


[nRF52188](https://github.com/sieren/blidino/tree/master/nRF51822-BLEMIDI)

This project holds code specifically aimed at the *nRF51288* by RedBearLab. It is designed to work out of the box with the [Circuits@Home USB Host Shield 2.0](http://www.circuitsathome.com/products-page/arduino-shields/usb-host-shield-2-0-for-arduino). You can get both boards for a total of around 50$.

[nRF 8100](https://github.com/sieren/blidino/tree/unit-tests/Legacy%20Projects/nRF8001-BLE-Proto)

This (legacy) project used to be the very first prototype built around the wide-spread nRF8001 chip. Unfortunately that chip has a throughput limitation of around 0.9kb/s, not enough for the MIDI rate of 31250 baud.

Known Issues
====================


    • BLE to USB

    Due to the lack of proper devices to test this with, I cannot guarantee this is working.
    Any feedback or PRs with the proper implementation would be appreciated.

    • Official Spec implementation
    The sketches are based on weeks of reverse-engineering the BLE MIDI Standard in late 2014.
    As of mid-december the specifications have been officially released. The reverse engineered
    and the official specs are close to a state that they are working.
    However a couple of features (e.g. SysEx, Timing information) might need further work.


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
