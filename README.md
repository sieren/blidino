blidino
=======

Arduino USB-MIDI to MIDI over Bluetooth Project.
This repo will soon host sketches and instructions to wirelessly connect USB Class compliant MIDI Devices to iOS / OSX Devices based on the newly released MIDI over Bluetooth protocol.

Projects
========

    [nRF52188](https://github.com/sieren/blidino/tree/master/nRF51822-BLEMIDI)

This project holds code specifically aimed at the *nRF51288* by RedBearLab. It is designed to work out of the box with the [Circuits@Home USB Host Shield 2.0](http://www.circuitsathome.com/products-page/arduino-shields/usb-host-shield-2-0-for-arduino). You can get both boards for a total of around 50$.

    [nRF 8100](https://github.com/sieren/blidino/tree/master/nRF8001-BLE-Proto)

This project used to be the very first prototype built around the wide-spread nRF8001 chip. Unfortunately that chip has a throughput limitation of around 0.9kb/s, not enough for the MIDI rate of 31250 baud.

Known Issues
============


    • No MIDI Input

    The SoftDevice S110 Firmware and the SDK used for the Bluetooth chips by 
    **Nordic Semiconductors ASA** currently do not support MTU and FAR (Fragmentation 
    and Assembly). iOS and OSX both transfer larger packets (> 20bytes) by fragmenting
    these on the bluetooth stack. As a result once the system initiates a fragmented 
    transfer, it blocks the output permanently. For data sent from the sketch, it works 
    around this limitation by pre-fragmenting MIDI data on a higher layer.



The [Wiki](https://github.com/sieren/blidino/wiki) will be updated frequently with information about the ongoing development.

Click the link to see a demo.

<a href="http://www.youtube.com/watch?feature=player_embedded&v=pk6db6KNVUQ
" target="_blank"><img src="http://img.youtube.com/vi/pk6db6KNVUQ/0.jpg" 
alt="Video" width="240" height="180" border="10" /></a>