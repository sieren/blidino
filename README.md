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


Requirements
====================

As of 02.2016 this sketch requires the recent RBL SDK (based on S130). To make sure you are using the right firmware,
drop the [bootloader.hex](https://github.com/RedBearLab/nRF51822-Arduino/blob/S130/bootloader/bootloader.hex) to the mBED drive.
Older RBL Boards sold in 2014/2015 may still be using S110, thus updating is recommended if not done already. 

FAQ
====================

    • Do I have to use the USB Shield to make this work?

    Certainly not, removing all the USB Routines from the Code and replacing them with other functions is quite easy.
    The BLEParser takes in a Receiver Type which can be anything. To see an example of a fake-receiver, see the unit-test code.


    • Does this work with other Bluetooth boards?

    I suppose, but not out of the box. You'd have to re-implement the nRF SDK specific bluetooth functions for your board.
    The only supported boards right now are the ones based on the nRF51288 (and marketed through RBL). 
    Rewriting this code natively for the nRF SDK is quite easy though and proved an ideal reference in the past.


Known Issues
====================


    • USB to BLE Parsing

    MIDI to BLE-MIDI Parsing needs to be rewritten and made testable. This will be added shortly.

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
