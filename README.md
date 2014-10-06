ConnectAnyThing
============

## Overview ##

**Connect AnyThing allows remote-control and real-time simple programming of your Galileo development board from a phone, tablet or laptop.**  

![Connect AnyThing](http://s26.postimg.org/ekqtsrbp5/CAT_Small.jpg)

**Connect AnyThing is software that makes your Galileo broadcast a WiFi hotspot and webpage.** Through the webpage, you can see sensor values in real-time, and directly control the output pins, or use the sensor values to automatically control the outputs.

Connect AnyThing uses the Galileo board's 6 Analog Input pins (labeled A0 – A5) to read incoming sensor values, and 12 Output pins (labeled 2 - 13) to turn things on and off, or send variable signals that can control the brightness of a light, the position of a servo, or the speed of a motor. 


### Uses

* Rapid prototyping! Quickly mock-up a project before digging in.
* Test sensors and simple circuits quickly without writing code.
* Read a temperature sensor and control a heater or fan in a garage or attic.
* Make a publicly controllable game or art project.
* Make an interactive exhibit.
* Great in classrooms or offices- doesn't need internet access.
* Allows novices to quickly build and understand basic concepts of hardware and logic.

For more information, photos and video links, please visit [Intel's Maker Community]( https://communities.intel.com/thread/48358 ).

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
Our experinece taking ConnectAnyThing into classrooms and after-school programs has led us to a preferred set of easy-to-use sensors, actuators, and other accessories perfect for rapid prototyping with CAT. Visit [Intel's Maker Community]( https://communities.intel.com/message/238121#238121 ) to learn more.

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
