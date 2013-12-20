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

## How To Run ConnectAnyThing

1) Download the project's zip file or clone the repository.

2) Unpack the content and locate the SD_Card directory.

3) Make sure the [Micro SD card is FAT32 formatted.] ( http://www.wikihow.com/Format-an-SD-Card )

4) Copy the entire content of the SD_Card directory into the root directory of the Micro SD card. **NOTE:** Keep the file structure intact.

5) In the SD_Card directory, unzip the image-full-clanton.zip file. **NOTE:** it should contain the image-full-clanton.ext3 file.

6) If successful, the Micro SD card's directory tree should look as follows:
* boot (directory)
* srv_card (directory)
* bzImage (file)
* core-image-minimal-initramfs-claton.cpio.gz (file)
* image-full-clatong.ext3 (file)
* image-full-clatong.zip (file) <- This can be removed

7) Put the Micro SD card in the Galileo board and power it up. **NOTE:** It could take up to 3 minutes for the board to boot-up.

8) On your mobile device or computer, connect to the "ConnectAnyThing" network. **NOTE:** If you don't see the network on your device, make sure your Galileo board has [firmware version 782 or above.] ( https://communities.intel.com/docs/DOC-21838 )

9) Start a web browser on your device and in the address bar type either: "cat", "cat.com", or "192.168.0.10".

10) If successful, you should see a Galileo board image and it's pin controls.

ENJOY...!!!

## ConnectAnyThing Architecture 

### Hardware description

* Our set up uses 1 Galileo board as a web server/WiFi hotspot.
* The board uses a half PCI-E Intel N-135 WiFi card and a half-to-full height Mini PCI Express(PCI-E) Card Bracket Adapter to connect it to the board.
* One external 5V (3A) power supply.

### Software description

The server board runs a couple of extra services than the stock installation :

* *hostapd* : for creating the access point
* *udhcpd* : Lightweight DHCP server
* *(busybox) dnsd* : used for DNS resolution of the http:://cat.com address (was customized for acting as a captive portal, but not really working with iPhone so the customization was dropped)

On top of that we installed the libwebsockets library (compiled from source).
Finally the board runs an arduino sketch (compiled against the libwebsockets library) that act as a web-server and an OSC client forwarding touch information to the proper board.

### Networking description

The server board creates an open WiFi network called "ConnectAnyThing".
Clients will connect to the WiFi, request the http://cat.com/ webpage and will be delivered with a UI to control Galileo's pins.
Communication from phone clients to server is made through websockets.
If you wish to change the name of the hotspot network, change the SSID name in the file hostapd.conf located in /etc/hostapd/.

####Network setting
 
* WIFI Board : 192.168.0.10  (C.A.T. board)

## How To Edit the HTML5 Website
Copy the file index.html from: ~/srv to /media/mmcblk0p1/srv_card/home/root/srv/ use the following command on the Linux command line:
cp -r /media/mmcblk0p1/srv_card/home/root/srv/* ./srv/