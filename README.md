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

This documents describes the system's hardware and software architecture, which was based on the LYT project ( https://github.com/secondstory/LYT ). You can see a video of a practical application of this platform in creators project website(http://thecreatorsproject.vice.com/the-makers-series/the-makers-lit-by-second-story).

## ConnectAnyThing Architecture 

### Hardware description

### Software description

## How To Run ConnectAnyThing

1) Checkout or download the project's zip file.
2) In the SD_Card directory, unzip the image-full-clanton.zip file (it should contain the image-full-clanton.ext3 file).
3) Copy the entire content of this directory to the root directory of the Galileo Micro SD card, keeping its structure intact ( remove the image-full-clanton.zip if needed ).
4) Put the Micro SD card back in the Galileo board and power up.
5) On your mobile device or computer, connect to the ConnectAnyThing network.
6) Start a browser and go to "cat.com" or "192.168.0.10".
7) Control the output pin D13 and read data in from pin A0.