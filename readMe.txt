
I put together the whole house audio amp project back in 2015,
where I had iTunes driven by some php code that I had written (before AI).
We had a web server running on a small laptop in the parent's bedroom
and the web server ran the php and then talked to the amp using tcp
commands.
The amp consists of an arduino uno with an ethernet shield and connected
by i2c to a mux and on the other side of the mux were 6 class D boards.

In 2026, I added a buck step down converter to the amp box so that
the Uno regulator did not have to work too hard from the original
12v that I fed to VIN which overheated the regulator that output
5v and 3.3v.  The $4 buck converter has a led to show the input
voltage and the output voltage.  I tweaked the pot so that the output
voltage is only 7.5 and fed that into VIN so the Uno regulator does 
not overheat and converts the voltages needed in the rest of the
circuit.  I plan to dedicate a cheap ($25) Alexa device to be the
audio input for a room.  The volume control can either be by
asking Alexa to increase the volume or the user can go to
192.168.1.177 and use the slide to increase the volume.
Currently, I just plan to use this for the backyard/patio area
since most of the house has Alexa devices in them.  Though
the house speakers would sound better, there is no need to add
new Alexa devices to the amp.

The software build uses the PlatformIO extension under Visual Code.
It is extremely easy to build and then download the code to the amp.
This would be necessary if the house IP numbers change like to 10.0.0.x.
It is ok to plug in a usb cable to the amp while the 12v power is plugged in.
The Uno regulator handles using the correct voltage inputs.

Jon Norenberg
11/15/2026

