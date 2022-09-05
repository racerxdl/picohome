# PicoHome (WIP)

Raspberry Pi Pico Home Automation using CAN Bus

**PLEASE READ LICENSE WARNING SECTION BEFORE USE**

This project is a improvement over my older home automation projects such as [espshift](https://github.com/racerxdl/esp32-shift). It all started when I wanted to make my own home automation project for learning and I didn't wanted to rely on Wireless Connections for my automation.

This project consists of few parts:

* [picohome-input](/pico-input/README.md) => Board with opto-coupled inputs which works from 12V to 24V switches.
* `picohome-relay` => Board that fits on cheap chinese 16 relay board

The main idea is to be able to put everything inside a metal box and forget about it. So far I'm still iterating over designs (I'm running my own home automation for a few years already) and trying to make it less dependent of computers and such.

Work-in-progress = More details to come.


# LICENSE WARNING

Although all the code and schematics I did here is released under Apache / Creative Commons, the CAN2040 library is released under GPL. The creator of can2040 library doesn't seen to be willing to release it in another license (see https://github.com/KevinOConnor/can2040/issues/5 ) so basically since GPL is more restritive than the one I released, you should follow the GPL for this project.

So how that affects you? It will only affect you if you do distribute a binary version of the firmware. In this case GPL demands you to also release the source code. That's only likely to affect you if you're doing a commercial project and doesn't want to share the source code. Otherwise, you don't need to worry about it.

Although the can2040 library is GPL, everything else here will be as described in LICENSE. If anyone knows or want to do a replacement for can2040 library, let me know :D
