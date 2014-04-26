big red button
==============

An arduino powered, wifi-enabled Big Red Button.   
The button connects to your wifi network, and when you press it, it sends a payload to a user-defined webservice. From there, the possibilities are endless!

![Button](https://github.com/snrub/big-red-button/raw/master/button.jpg)&nbsp;
![Schematics - small](https://github.com/snrub/big-red-button/raw/master/schematics/BigRedButton_sml.png)

This is just a prototype, so the enclosure is big and ugly. Also changing the network settings is a pain in the arse. You need to dismantle the enclosure, plug the arduino in, and re-upload the `Configuration.h` file. There is a [wirelessSetup branch](https://github.com/snrub/big-red-button/tree/wirelessSetup) which will allow you to edit that config over wifi. That's still a work in progress.

# Hardware

  * An Arduino. Any Arduino would be fine, but I used this one:
    * 3.3v Pro Micro https://www.sparkfun.com/products/12587
  * Networking
    * WiFly RN-131C https://www.sparkfun.com/products/10050
    * Antenna (RP-SMA) https://www.sparkfun.com/products/145
    * Antenna Cable (RP-SMA to U.FL) https://www.sparkfun.com/products/662
  * Power
    * 3.7v LiPo Battery https://www.sparkfun.com/products/10718
    * USB LiPo Charger https://www.sparkfun.com/products/10161
  * Other stuff
    * Button https://www.sparkfun.com/products/9181
    * Enclosure http://www.jaycar.com.au/productView.asp?ID=HB6230
    * On/Off Switch https://www.sparkfun.com/products/10727

# Drivers

The Pro Micro board isn't natively supported by the Arduino IDE, so you need to install some drivers for it.   
Install these into the `<sketchbook-root>/hardware/` directory

  * 3.3v Pro Micro Board driver
    * https://github.com/sparkfun/SF32u4_boards

# Libraries

Install these into the `<sketchbook-root>/libraries/` directory.   
Complete installation instructions: http://arduino.cc/en/Guide/Libraries

  * WiFlySerial
    * Modified: [I've modified it to enable me to use an external antenna](https://github.com/snrub/big-red-button/tree/master/src/libraries/WiFlySerial)
    * Original: http://arduinowifly.sourceforge.net
  * PString
    * http://arduiniana.org/libraries/PString
  * Streaming
    * http://arduiniana.org/libraries/streaming

# License

The MIT License (MIT)   
Copyright (c) 2014 Julian Lloyd-Phillips   
Full license: https://github.com/snrub/big-red-button/blob/master/LICENSE
