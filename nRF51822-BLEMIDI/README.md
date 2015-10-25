blidino (nRF51288)
==================

This project holds code specifically aimed at the *nRF51288* by RedBearLab. It is designed to work out of the box with the [Circuits@Home USB Host Shield 2.0](http://www.circuitsathome.com/products-page/arduino-shields/usb-host-shield-2-0-for-arduino). Both boards are available for a total of around 50$.

Hardware
========
1. [USB Host Shield 2.0 for Arduino](http://www.circuitsathome.com/products-page/arduino-shields/usb-host-shield-2-0-for-arduino) (e.g. available at [lipoly](http://www.lipoly.de)

2. [RedBearLab nRF51822](http://redbearlab.com/redbearlab-nrf51822/)


Installation
============
1. Follow these [instructions](http://redbearlab.com/getting-started-nrf51822/) to set-up Arduino with the [RedBearLab Add-On](https://github.com/RedBearLab/nRF51822-Arduino).
2. Replace the ***projectconfig.h*** in the */hardware/arduino/RBL_nRF51822/libraries/BLE_API/utility/* subfolder of your Arduino IDE installation (on OSX you have to right-click on the Application -> Show Package Contents and then navigate to Resources/Java).
3. Download and import the [USB Host Shield 2.0 Library](https://github.com/felis/USB_Host_Shield_2.0) and the [USBH_MIDI Library](https://github.com/YuuichiAkagawa/USBH_MIDI) into Arduino.
4. Compile and upload the sketch.

Known Issues
============

Please check the [README within the root directory](https://github.com/sieren/blidino) for known issues.

