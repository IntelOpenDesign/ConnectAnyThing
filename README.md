ConnectAnyThing
============

# Overview

ConnectAnyThing is an open platform intended for easy prototyping of connected appliances, devices, and installations using Intel’s Galileo development board.  When you load ConnectAnyThing onto the Galileo’s SD card, Galileo broadcasts a WiFi network and hosts an HTML5 webpage that allows anyone to connect, read inputs and control outputs from their mobile device. The first version of this system was created during an Intel internal hackthon in December of 2013.

### Potential Uses

* Make a publicly controllable art installation.
* Interactive window displays.
* Remote monitoring and control for cold/toxic/unpleasant environments.
* Build a mobile-controlled door lock or lights for a shed or outbuilding.

### The Team

Carlos Montesinos <carlos.montesinos@intel.com>
Lucas Ainsworth <lucas.b.ainsworth@intel.com>
Clayton Hofrock <clayton.a.hofrock@intel.com>
David Shaw <david.shaw@intel.com>
Gowri Chitloor Parthasarathy <gowri.chitloor.parthasarathy@intel.com>
Sorin Blebea <sorin.blebea@intel.com>
Sweta Patel <sweta.patel@intel.com>
Seth Hunter <seth.e.hunter@intel.com>

### Credits

Our platform is a simplified version of the [LYT project]( https://github.com/secondstory/LYT ) developed by [Second Story] ( http://www.secondstory.com/ ).

## How To Set Up ConnectAnyThing

### Hardware Required

* One [Intel Galileo](http://www.intel.com/content/www/us/en/do-it-yourself/galileo-maker-quark-board.html) board with [firmware version 782 or above]( https://communities.intel.com/docs/DOC-21838 ).
* One Micro SD card. **NOTE:** We've tested the system with a [4 GB SanDisk Micro SDHC card] (http://www.sandisk.com/products/memory-cards/microsd/class4/) .
* One half PCI-E [Intel N-135 WiFi] ( http://www.intel.com/content/www/us/en/wireless-products/centrino-wireless-n-135.html ) card and a half-to-full height Mini PCI Express(PCI-E) Card Bracket Adapter to connect it to the board.
* One external 5V (3A) power supply.

### Software Installation

1) Download the project's zip file or clone the repository.

2) Unpack the content and locate the SD_Card directory.

3) Make sure the [Micro SD card is FAT32 formatted]( http://www.wikihow.com/Format-an-SD-Card ).

4) Copy the entire content of the SD_Card directory into the root directory of the Micro SD card. **NOTE:** Keep the file structure intact.

5) In the SD_Card directory, unzip the image-full-clanton.zip file. **NOTE:** it should contain the image-full-clanton.ext3 file.

6) If successful, the Micro SD card's directory tree should look as follows:
* boot (directory)
* srv_card (directory)
* bzImage (file)
* core-image-minimal-initramfs-claton.cpio.gz (file)
* image-full-clatong.ext3 (file)
* image-full-clatong.zip (file) <- This file can be removed to free up space.

7) Put the Micro SD card in the Galileo board and power it up. **NOTE:** It could take up to 3 minutes for the board to boot-up.

8) On your mobile device or computer, connect to the "ConnectAnyThing" network. **NOTE:** If you don't see the network on your device, make sure your Galileo board has [firmware version 782 or above] ( https://communities.intel.com/docs/DOC-21838 ).

9) Start a web browser on your device and in the address bar type either: "cat", "cat.com", or "192.168.0.10".

10) If successful, you should see a Galileo board image and it's pin controls.

ENJOY...!!!

## Release Notes

### Rev 0.1.0
* The Graphical User Interface enables direct access to the pins listed bellow.
* Pins: A0,A1,A2,A3,A4,and A5 are enabled as analog inputs.
* Pins: 2,4,7,8,12,and 13 are enabled as digital outputs.
* Pins: 3,5,6,9,10,and 11 are enabled as analog outputs.

### Known Issues
* Connecting with an iPhone: If you are using an iPhone, make sure to disable your 4G/LTE radio. If the radio is on when attempting to access ConnectAnyThing's hotspot , Apple may block access to it.
* Digital pins 0 (RX) and 1 (TX) are currently disabled to avoid RS-232 conflicts.

## Developers

### Changing Network Name
If you wish to change the name of the hotspot network, change the SSID name in the file hostapd.conf located in /etc/hostapd/.

### How To Edit the HTML5 Website
Copy the file index.html from: ~/srv to /media/mmcblk0p1/srv_card/home/root/srv/ use the following command on the Linux command line:
cp -r /media/mmcblk0p1/srv_card/home/root/srv/* ./srv/