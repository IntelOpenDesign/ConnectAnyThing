ConnectAnyThing
============

# Overview

ConnectAnyThing is an open platform aimed to enable non-technical creative people, to control Intel's Galileo development board, through an HTML5 website. The first version of this system was created during an Intel's internal hackthone in December of 2013.

The Team:
Carlos Montesinos <carlos.montesinos@intel.com>
Lucas Ainsworth <lucas.b.ainsworth@intel.com>
Clayton Hofrock <clayton.a.hofrock@intel.com>
David Shaw <david.shaw@intel.com>
Gowri Chitloor Parthasarathy <gowri.chitloor.parthasarathy@intel.com>
Sorin Blebea <sorin.blebea@intel.com>
Sweta Patel <sweta.patel@intel.com>

This documents describes the system's hardware and software architecture, which was based on the [LYT project]( https://github.com/secondstory/LYT ). You can see a video of a practical application of this platform in [the creators project website](http://thecreatorsproject.vice.com/the-makers-series/the-makers-lit-by-second-story).

## How To Run ConnectAnyThing

1) Checkout or download the project's zip file.
2) In the SD_Card directory, unzip the image-full-clanton.zip file (it should contain the image-full-clanton.ext3 file).
3) Copy the entire content of this directory to the root directory of the Galileo Micro SD card, keeping its structure intact ( remove the image-full-clanton.zip if needed ).
4) Put the Micro SD card back in the Galileo board and power up.
5) On your mobile device or computer, connect to the "ConnectAnyThing" network.
6) Start a browser and go to "cat.com" or "192.168.0.10".
7) Control the output pin D13 and read data in from pin A0.

## ConnectAnyThing Architecture 

### Hardware description

* Our setup uses 1 Galileo board as a web server/WiFi hotspot.
* The board uses a half PCI-E Intel N-135 Wifi card and a half-to-full height Mini PCI Express(PCI-E) Card Bracket Adapter to connect it to the board.
* One external 5V (3A) power supply.

### Software description

The server board runs a couple of extra services than the stock installation :

* *hostapd* : for creating the access point
* *udhcpd* : Lightweight DHCP server
* *(busybox) dnsd* : used for DNS resolution of the http:://cat.com address (was customized for acting as a captive portal, but not really working with iPhone so the customization was dropped)

On top of that we installed the libwebsockets library (compiled from source).
Finally the board runs an arduino sketch (compiled against the libwebsockets library) that act as a web-server and an OSC client forwarding touch information to the proper board.

### Networking description

The server board creates an open wifi network called "ConnectAnyThing".
Clients will connect to the WiFi, request the http://cat.com/ webpage and will be delivered with a UI to control Galileo's pins.
Communication from phone clients to server is made through websockets.

####Network setting
 
* WIFI Board : 192.168.0.10  (cat board)

## How To Run ConnectAnyThing

1) Checkout or download the project's zip file.
2) In the SD_Card directory, unzip the image-full-clanton.zip file (it should contain the image-full-clanton.ext3 file).
3) Copy the entire content of this directory to the root directory of the Galileo Micro SD card, keeping its structure intact ( remove the image-full-clanton.zip if needed ).
4) Put the Micro SD card back in the Galileo board and power up.
5) On your mobile device or computer, connect to the ConnectAnyThing network.
6) Start a browser and go to "cat.com" or "192.168.0.10".
7) Control the output pin D13 and read data in from pin A0.