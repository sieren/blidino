blidino (nRF51288)
==================

This project holds code specifically aimed at the *nRF51288* by RedBearLab. It is designed to work out of the box with the [Circuits@Home USB Host Shield 2.0](http://www.circuitsathome.com/products-page/arduino-shields/usb-host-shield-2-0-for-arduino). You can get both boards for a total of around 50$.

Hardware
========
[USB Host Shield 2.0 for Arduino](http://www.circuitsathome.com/products-page/arduino-shields/usb-host-shield-2-0-for-arduino) (e.g. available at [lipoly](http://www.lipoly.de)
[RedBearLab nRF51822](http://redbearlab.com/redbearlab-nrf51822/)


Installation
============
1. Follow these [instructions](http://redbearlab.com/getting-started-nrf51822/) to set-up Arduino with the [RedBearLab Add-On](https://github.com/RedBearLab/nRF51822-Arduino).
2. Replace the ***projectconfig.h*** in the */hardware/arduino/RBL_nRF51822/libraries/BLE_API/utility/* subfolder of your Arduino IDE installation (on OSX you have to right-click on the Application -> Show Package Contents and then navigate to Resources/Java).
3. Download and import the [USB Host Shield 2.0 Library](https://github.com/felis/USB_Host_Shield_2.0) and the [USBH_MIDI Library](https://github.com/YuuichiAkagawa/USBH_MIDI) into Arduino.
4. Compile and upload the library

Known Issues
============

⋅⋅* No MIDI Input 
The SoftDevice S110 Firmware and the SDK used for the Bluetooth chips by **Nordic Semiconductors ASA** currently do not support MTU and FAR (Fragmentation and Assembly). iOS and OSX both transfer larger packets (> 20bytes) by fragmenting these on the bluetooth stack. As a result once the system initiates a fragmented transfer, it blocks the output permanently. For data sent from the sketch, it works around this limitation by pre-fragmenting MIDI data on a higher layer.



The [Wiki](https://github.com/sieren/blidino/wiki) will be updated frequently with information about the ongoing development and discoveries about the BLE-MIDI Specificiation.

Click the link to see a demo.

<a href="http://www.youtube.com/watch?feature=player_embedded&v=pk6db6KNVUQ
" target="_blank"><img src="http://img.youtube.com/vi/pk6db6KNVUQ/0.jpg" 
alt="Video" width="240" height="180" border="10" /></a>