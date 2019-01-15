# Building
Follow the Making the Module Reference under 'Refrences'. Use the modified wire diagram below instead of the one in the guide.

Adapter Wiring Diagram
img0

Don't assume that your GCC extension wires match OEM! Guide is in the doc. I personally recommend the potentiometer method without plugging into the console.

Wire color diagram
img1

Inside of the finished product
img2

Finished product
img3

# Installation
Just open the .ino file in the Arduino IDE and upload via serial.  Instructions for use are at the top of the .ino file.

The arduino should work with a wii or gamecube. In order to get it working with pc you will likely need to download the firmware below and have the device in Wii U mode. The arduino must be powered through either the grey rumble cord, an alternative 5V power source, or the serial bus (micro USB).
www.mayflash.com/File.asp?Id=106

# References
Code Base (v3): https://github.com/OtaK/GCCArduino

Making the Module: https://docs.google.com/document/d/1KZrORDtJBuovVAHRZjteRitBKkwibZOc7VW0PdGedEk/edit

Mayflash Firmware: https://github.com/NicoHood/Nintendo/issues/11
