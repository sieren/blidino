blidino
=======

Arduino USB-MIDI to MIDI over Bluetooth Project.
This repo will soon host sketches and instructions to wirelessly connect USB Class compliant MIDI Devices to iOS / OSX Devices based on the newly released MIDI over Bluetooth protocol.

The protoype was built using a **RedBearLab Blend Micro** and a **SparkFun USB Host Shield**. The RBL Blend Micro is based on the **nRF8001** Bluetooth Chip which during development has proven to be too slow for MIDI (max. throughput of 0.6-1.0kb/s as opposed to a 31250 baud maximum on MIDI).

The final project will be targeted for a ***RedBearLab nRF51822** Board with a ***Circuits@Home USB Host Shield 2.0*** for maximum compatibility and ease of use for less-adventurous users (as the big advantage is, that this won't involve any soldering).

Sources based on the rNF8001 will be uploaed, too.

The [Wiki](https://github.com/sieren/blidino/wiki) will be updated frequently with information about the ongoing development.

Click the link to see a demo.

<a href="http://www.youtube.com/watch?feature=player_embedded&v=pk6db6KNVUQ
" target="_blank"><img src="http://img.youtube.com/vi/pk6db6KNVUQ/0.jpg" 
alt="Video" width="240" height="180" border="10" /></a>