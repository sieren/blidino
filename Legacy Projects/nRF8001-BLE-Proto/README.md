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
2. Replace the ***RBL_services.h, RBL_nRF8001.cpp and RBL_nRF8001.h*** in the */libraries/RBL_nRF8001* subfolder of your Arduino workspace.
3. Download and import the [USB Host Shield 2.0 Library](https://github.com/felis/USB_Host_Shield_2.0) and the [USBH_MIDI Library](https://github.com/YuuichiAkagawa/USBH_MIDI) into Arduino. You might need to check the pin assignments for SS,INT,MISO,MOSI and CLK within *USBCore.h* and *usbhost.h in the USB Host Shield Library.
4. Compile and upload the sketch to the device.

Known Issues
============

• Slow MIDI Output

    The nRF8001 chip is designed for low-performance bluetooth applications, rendering it suitable for low
    performance MIDI applications only. Exceeding the bandwidth will result in dropped packets or stuck notes.



The [Wiki](https://github.com/sieren/blidino/wiki) will be updated frequently with information about the ongoing development and discoveries about the BLE-MIDI Specificiation.

Click the link to see a demo.

<a href="http://www.youtube.com/watch?feature=player_embedded&v=pk6db6KNVUQ
" target="_blank"><img src="http://img.youtube.com/vi/pk6db6KNVUQ/0.jpg" 
alt="Video" width="240" height="180" border="10" /></a>