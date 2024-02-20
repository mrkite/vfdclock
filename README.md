## Simple Arduino Clock

This is a simple arduino clock, hooked up to a Logic Controls TD3000 Vacuum
Fluorescent Display (VFD).

This clock is made up of 5 different components.  The arduino nano itself, the
VFD, a buck converter, a max3232 converter, and a DS3231 timer.  There are
also 3 buttons connected to the arduino for setting the time and date.

### Connections

The VFD requires 9VAC power, so I use a 9VAC power plug that goes to the VFD,
and splits off and goes to a 5V buck converter to power everything else.

The arduino nano has GND and 5V connected to the buck converter.

The max3232 converter has two sides, the TTL side and the RS232 side.  The TTL
side has 3 pins connected.  The VCC and GND are connected to the arduino nano
3v3 pin and ground.  Serial In is connected to the TX1 pin of the arduino.
The RS232 side has 2 pins connected.  The GND and Serial In is connected
to the Ground and Serial In of the VFD.

The DS3231 timer has 4 pins connected.  The VCC and GND are connected to the
buck converter's GND and 5V lines.  The SCL and SDA pins are connected to the
SCL and SDA pins of the arduino.

Finally 3 buttons are connected to digital pins 2, 3, and 4 of the arduino, and GND.

### Setting the time

Hold the "set" button, connected to pin 2 until the hours start flashing.
The "-" button is hooked up to pin 3, and "+" button is hooked up to pin 4.
One decreases the hours, the other increases the hours.

Hitting the set button then starts flashing the minutes and so on.
Cycle through all the fields until the year.. hitting the set button after
the year is flashing will exit setting the time.
