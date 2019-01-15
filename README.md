# Building

My Purchases (Don't have to match):

Nano V3: https://www.amazon.com/gp/product/B00SGMEH7G/ref=ppx_yo_dt_b_asin_title_o00__o00_s00?ie=UTF8&psc=1

Bi-Directional Logic Level Converter: https://www.amazon.com/gp/product/B00NAY3J7O/ref=ppx_yo_dt_b_asin_title_o00__o00_s01?ie=UTF8&psc=1

GCC Extender: https://www.amazon.com/gp/product/B00E1LX3CQ/ref=ppx_yo_dt_b_asin_title_o00__o00_s01?ie=UTF8&psc=1

Junction Boxes (Case): https://www.amazon.com/gp/product/B07HMPWNLM/ref=ppx_yo_dt_b_asin_title_o00__o00_s00?ie=UTF8&psc=1


Follow the Making the Module Reference under 'Refrences'. Use the modified wire diagram below instead of the one in the guide.

Adapter Wiring Diagram
![](https://raw.githubusercontent.com/Oafish1/GCCArduino/master/img/ModifiedChart.png)

Don't assume that your GCC extension wires match OEM! Guide is in the doc. I personally recommend the potentiometer method without plugging into the console.

Wire color diagram for extension cable - may vary
![](https://raw.githubusercontent.com/Oafish1/GCCArduino/master/img/ExtenderWires.JPG)

Inside of the finished product
![](https://raw.githubusercontent.com/Oafish1/GCCArduino/master/img/FinishedInside.JPG)

Finished product
![](https://raw.githubusercontent.com/Oafish1/GCCArduino/master/img/Finished.JPG)

# Installation

Just open the .ino file in the Arduino IDE and upload via serial.  Instructions for use are at the top of the .ino file.

The arduino should work with a wii or gamecube. In order to get it working with pc you will likely need to download the firmware below and have the device in Wii U mode. The arduino must be powered through either the grey rumble cord, an alternative 5V power source, or the serial bus (micro USB).
www.mayflash.com/File.asp?Id=106

# References

Code Base (v3): https://github.com/OtaK/GCCArduino

Making the Module: https://docs.google.com/document/d/1KZrORDtJBuovVAHRZjteRitBKkwibZOc7VW0PdGedEk/edit

Mayflash Firmware: https://github.com/NicoHood/Nintendo/issues/11
