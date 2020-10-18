# AutoOff

A simple timer slightly smarter than a 555 to control 2 individual relays to automatically power off devices after a given amount of time with a single switch.

You can configure the timeout by adjusting the variable `powerOnTime` in the Firmware.

By default a single click on the button will switch on the relays and start the timer.
If the button is pressed again, the timer is stoppend and the relays are opened.
If, however, the button is double-clicked while the relays are on, the timer restarts again.

The PCB is designed for a maximum current flow of 5A / channel, however if you don't need that much current in your application, you should use fuses with a lower, more apropriate rating.
