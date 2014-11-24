blidino (nRF51288)
==================

This project was the first prototype in reverse-engineering MIDI over BLE.
Unfortunately the nRF8001 is too slow for extensive tasks like turning knobs (capped at 0.6-0.9kb/s)
but it will work fine for playing keys.

Hardware
========
1. [USB Host Shield by Sparkfun](https://www.sparkfun.com/products/9947) 

2. [RedBearLab Blend Micro](http://redbearlab.com/blendmicro/)


Installation
============
1. Set up the board as instructed on the [RedBearLab Blend Micro](http://redbearlab.com/blendmicro/) page.
2. Replace the ***RBL_services.h*** in the */libraries/RBL_nRF8001* subfolder of your Arduino workspace.
3. Download and import the [USB Host Shield 2.0 Library](https://github.com/felis/USB_Host_Shield_2.0) and the [USBH_MIDI Library](https://github.com/YuuichiAkagawa/USBH_MIDI) into Arduino. You might need to check the pin assignments for SS,INT,MISO,MOSI and CLK within *USBCore.h* and *usbhost.h in the USB Host Shield Library.
4. Compile and upload the sketch to the device.

Known Issues
============

• No MIDI Input

    The SoftDevice S110 Firmware and the SDK used for the Bluetooth chips by **Nordic Semiconductors ASA**
    currently do not support MTU and FAR (Fragmentation and Assembly). iOS and OSX both transfer larger 
    packets (> 20bytes) by fragmenting these on the bluetooth stack. As a result once the system initiates 
    a fragmented transfer, it blocks the output permanently. For data sent from the sketch, it works around 
    this limitation by pre-fragmenting MIDI data on a higher layer.



The [Wiki](https://github.com/sieren/blidino/wiki) will be updated frequently with information about the ongoing development and discoveries about the BLE-MIDI Specificiation.

Click the link to see a demo.

<a href="http://www.youtube.com/watch?feature=player_embedded&v=pk6db6KNVUQ
" target="_blank"><img src="http://img.youtube.com/vi/pk6db6KNVUQ/0.jpg" 
alt="Video" width="240" height="180" border="10" /></a>