blidino
=======

Arduino USB-MIDI to MIDI over Bluetooth Project.
This repo hosts sketches and instructions to wirelessly connect USB Class compliant MIDI Devices to iOS / OSX Devices based on the recently released MIDI over Bluetooth protocol by Apple.

Projects
========


[nRF52188](https://github.com/sieren/blidino/tree/master/nRF51822-BLEMIDI)

This project holds code specifically aimed at the *nRF51288* by RedBearLab. It is designed to work out of the box with the [Circuits@Home USB Host Shield 2.0](http://www.circuitsathome.com/products-page/arduino-shields/usb-host-shield-2-0-for-arduino). You can get both boards for a total of around 50$.


<a href="http://www.youtube.com/watch?feature=player_embedded&v=Dgju2BHyXzk
" target="_blank"><img src="http://img.youtube.com/vi/Dgju2BHyXzk/0.jpg" 
alt="Video" width="240" height="180" border="10" /></a>



[nRF 8100](https://github.com/sieren/blidino/tree/master/nRF8001-BLE-Proto)

This (legacy) project used to be the very first prototype built around the wide-spread nRF8001 chip. Unfortunately that chip has a throughput limitation of around 0.9kb/s, not enough for the MIDI rate of 31250 baud.

Known Issues & To Do
====================


    • No MIDI Input

    I initially thought MIDI Input was not possible due to the lack of fragmentation. It turns out this
    is caused by a bug in OS X MIDIServer Daemon which will hopefully be fixed in a future release.
    Please be aware that you need to kill the MIDIServer if you have previously used any devices with a larger MTU (> 23)
    before connecting to a device with a fixed standard MTU, as the MIDIServer will not recognize lower MTUs
    and thus will not start pre-fragmenting outgoing MIDI Packets.
    MIDI Input functionality as well as SysEx (to support e.g. Ableton Push Controllers wirelessly) will be added
    in a forthcoming update.

    • Implementation of official specifications
    The sketches are based on weeks of reverse-engineering the BLE MIDI Standard.
    As of mid-december the standard has been officially released. The reverse engineered
    and the official specs need to be compared and subsequently implemented as per documentation.
    This will be one of the next steps in this project.


The [Wiki](https://github.com/sieren/blidino/wiki) will be updated frequently with information about the ongoing development.

Click the link to see a demo.

<a href="http://www.youtube.com/watch?feature=player_embedded&v=pk6db6KNVUQ
" target="_blank"><img src="http://img.youtube.com/vi/pk6db6KNVUQ/0.jpg" 
alt="Video" width="240" height="180" border="10" /></a>
