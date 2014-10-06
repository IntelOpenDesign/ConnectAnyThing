ConnectAnyThing
============

# Overview

ConnectAnyThing is an easy tool for real-time electronic tinkering on Galileo using a mobile device.

You will need a Galileo GEN1 or GEN2 (preferred) with a wifi card ([details](https://github.com/IntelOpenDesign/ConnectAnyThing/wiki/How-To-Set-Up-Connect-AnyThing)).

0) Download ConnectAnyThing [here](https://github.com/IntelOpenDesign/ConnectAnyThing/releases).

1) Put the ConnectAnyThing software onto a Galileo’s SD card, and power on.

2) Galileo broadcasts a Wi-Fi network called “ConnectAnyThing.” Join with your mobile device.

3) Open a web browser on your device, go to "cat.com". The webpage lets you read inputs and control outputs in real-time from your mobile device.

Good for tinkering, classroom activities, testing sensors, etc.

For more information, photos and video links, please visit [Intel's Maker Community]( https://communities.intel.com/thread/48358 ).

### Uses

* Quickly mock-up an interactive experience.
* Test sensors and simple circuits quickly without a computer or code.
* Read a temperature sensor and control a heater or fan in a garage or attic.
* Make a publicly controllable art installation.
* Interactive window displays.
* Remote monitoring and control for cold/toxic/unpleasant environments.
* Build a mobile-controlled door lock or lights for a shed or outbuilding.

### The Team

Carlos Montesinos <carlos.montesinos@intel.com><br />
Lucas Ainsworth <lucas.b.ainsworth@intel.com><br />
Victoria Fang <victoria.fang@intel.com><br />
Noura Howell <noura.howell@gmail.com><br />

### Credits

Adam Jordan <adam.jordan@intel.com><br />
Seth Hunter <seth.e.hunter@intel.com><br />
Ana Patricia Rosario <ana.p.rosario@intel.com><br />
Clayton Hofrock <clayton.a.hofrock@intel.com><br />
David Shaw <david.shaw@intel.com><br />
Gowri Chitloor Parthasarathy <gowri.chitloor.parthasarathy@intel.com><br />
Sorin Blebea <sorin.blebea@intel.com><br />
Sweta Patel <sweta.patel@intel.com><br />
Adam Pasztory <adampasz@gmail.com><br />
Alex T <alext.mkrs@gmail.com><br />

Our platform is a simplified version of the [LYT project]( https://github.com/secondstory/LYT ) developed by [Second Story] ( http://www.secondstory.com/ ).

## How To Set Up ConnectAnyThing

### Check out the Wiki page for [detailed setup instructions](https://github.com/IntelOpenDesign/ConnectAnyThing/wiki/How-To-Set-Up-Connect-AnyThing).



### ConnectAnyThing Tinkering Kit
We have created a Tinkering Kit with sensors, actuators, and other accessories perfect for CAT. Visit [Intel's Maker Community]( https://communities.intel.com/message/238121#238121 ) to learn more.

## Release Notes

### Rev 0.2.3
* Compatible only with Galileo GEN2.
* Servo control is enabled!

### Rev 0.2.2
* Compatible only with Galileo GEN1.
* Customize the network name on your CAT system.
* Added a 'reset' feature to clear your visual program.
* You can now control analog values from 'Controller Mode'.
* Your visual program is automatically saved every 10 seconds.
* Timer fixes.
* Input invert fixes.

### Rev 0.2.1
* Compatible only with Galileo GEN1.
* ConnectAnyThing now allows the user to connect inputs with outputs in a dynamic fashion.
* Pins are enabled as described in Rev 0.1.0.

### Rev 0.1.0
* Compatible only with Galileo GEN1.
* The Graphical User Interface enables direct access to the pins listed bellow.
* Pins: A0,A1,A2,A3,A4,and A5 are enabled as analog inputs.
* Pins: 2,4,7,8,12,and 13 are enabled as digital outputs.
* Pins: 3,5,6,9,10,and 11 are enabled as analog outputs.

### Known Issues
* Connecting with an iPhone: Apple likes to protect you from Wi-Fi hotspots that are not connected to the internet (such as ConnectAnyThing.)  The first time you connect with an iPhone, go to settings ->WiFi -> choose ConnectAnyThing.  A pop up comes up that says “Log In”, hit “cancel.”  And your phone will disconnect from the network.  Join ConnectAnyThing again.  When you hit “Cancel” the second time, an option will come up that says “Connect without Internet.” Select this and you’re all set.

* Digital pins 0 (RX) and 1 (TX) are currently disabled to avoid RS-232 conflicts.

* Any short circuits on the Galileo board will cause it to reboot. This will break the websockets connection. Avoid placing the board on any conductive materials to prevent any short circuits.  (especially watch out for Wi-Fi antennas with foil backing!)

* GEN 2: Servo control doesn't work on pin 11 for some reason.  Inputs are floating and can be influenced by signals on other input pins.  Doesn't drastically affect function but it's not ideal.  Working to track this down.

## Developers

### Development Work Flow
We welcome developers to contribute to the project. Please push any contributions against the *develop* branch and request a pull.
