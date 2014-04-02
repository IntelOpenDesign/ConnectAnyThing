ConnectAnyThing
============

# Overview

ConnectAnyThing is an easy tool for real-time electronic tinkering on Galileo using a mobile device.

1)	Put the ConnectAnyThing software onto a Galileo’s SD card, and power on. 
2)	Galileo broadcasts a Wi-Fi network called “ConnectAnyThing.”  Join with your mobile device.
3)	Open a web browser on your device, go to http://CAT.  Webpage lets you read inputs and control outputs in real-time from your mobile device.

Good for tinkering, classroom activities, testing sensors, etc. 

For more information, photos and video links, please visit: https://communities.intel.com/thread/48358

The first version of this system was created during an Intel internal hackthon in December of 2013.

### Uses

* Quickly mock-up an interactive experience.
* Test sensors and simple circuits quickly without code.
* Read a temperature sensor and control a heater or fan in a garage or attic

building on CAT,
* Make a publicly controllable art installation.
* Interactive window displays.
* Remote monitoring and control for cold/toxic/unpleasant environments.
* Build a mobile-controlled door lock or lights for a shed or outbuilding.

### The Team

Carlos Montesinos <carlos.montesinos@intel.com><br />
Lucas Ainsworth <lucas.b.ainsworth@intel.com><br />
Clayton Hofrock <clayton.a.hofrock@intel.com><br />
David Shaw <david.shaw@intel.com><br />
Gowri Chitloor Parthasarathy <gowri.chitloor.parthasarathy@intel.com><br />
Sorin Blebea <sorin.blebea@intel.com><br />
Sweta Patel <sweta.patel@intel.com><br />
Seth Hunter <seth.e.hunter@intel.com>

### Credits

Our platform is a simplified version of the [LYT project]( https://github.com/secondstory/LYT ) developed by [Second Story] ( http://www.secondstory.com/ ).

## How To Set Up ConnectAnyThing

### Hardware Required

* One [Intel Galileo](http://www.intel.com/content/www/us/en/do-it-yourself/galileo-maker-quark-board.html) board with [firmware version 782 or above]( https://communities.intel.com/docs/DOC-21838 ).
* One Micro SD card. **NOTE:** We've tested the system with a [4 GB SanDisk Micro SDHC card] (http://www.sandisk.com/products/memory-cards/microsd/class4/) .
* One half PCI-E [Intel N-135 WiFi] also tested and working with N-6205 and N-6235  ( http://www.intel.com/content/www/us/en/wireless-products/centrino-wireless-n-135.html ) card and a half-to-full height Mini PCI Express(PCI-E) Card Bracket Adapter to connect it to the board.  (a rubber band can work in a pinch)
* antenna for the WiFi card.
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

10) If successful, you should see a Galileo board image and pin controls.

ENJOY...!!!

## Release Notes

### Rev 0.1.0
* The Graphical User Interface enables direct access to the pins listed bellow.
* Pins: A0,A1,A2,A3,A4,and A5 are enabled as analog inputs.
* Pins: 2,4,7,8,12,and 13 are enabled as digital outputs.
* Pins: 3,5,6,9,10,and 11 are enabled as analog outputs.

### Known Issues
* Connecting with an iPhone: – Apple likes to protect you from Wi-Fi hotspots that are not connected to the internet (such as ConnectAnyThing.)  The first time you connect with an iPhone, go to settings ->WiFi -> choose ConnectAnyThing.  A pop up comes up that says “Log In”, hit “cancel”,  And your phone will disconnect from the network.  Join ConnectAnyThing again.  When you hit “Cancel” the second time, an option will come up that says “Connect without Internet.” Select this and you’re all set.

* Digital pins 0 (RX) and 1 (TX) are currently disabled to avoid RS-232 conflicts.

## Developers

### Development Work Flow
We welcome developers to contribute to the project. Please push any contributions against the *develop* branch and request a pull.

### Changing Network Name
If you wish to change the name of the hotspot network, change the SSID name in the file hostapd.conf located in /etc/hostapd/.

### How To Edit the HTML5 Website
Copy the file index.html from: ~/srv to /media/mmcblk0p1/srv_card/home/root/srv/ use the following command on the Linux command line:
cp -r /media/mmcblk0p1/srv_card/home/root/srv/* ./srv/
