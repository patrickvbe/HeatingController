# Main pomp controller

The main pump controller is the unit that is visible to the user. It has a display, is positioned in the room to control and has a temperature sensor to measure the room temperature. It is communicating with the actual pump controller via 433MHz.
Because I want the controller to optionally do more stuff to put on the display (and just wanted to try it out :-) ), I use a WeMos ESP8266 board for it. Among the ideas are:
* Get time with NTP
* Get google agenda items
* Get the days to put the garbage out from the website of the company that collects it.

An other optional thing is to receive the outside temperature from my el-cheapo weather station.

I first tried to use microPython. I quickly managed getting data from the internet (of course, that's the strong part of the ESP and microPython). But then... Receiving 433MHz was also quite doable with interrupts. But sending turned out to be a whole differ ball game. The sleep_us() and other timing functions are just too unreliable in the 'cooperative' multi-tasking environment. Even just using spin-loops turned out to be too slow (??) to time the few hundreds of micro seconds the signal needs. So I switched to using the Arduino environment with it's C++ like language to program the ESP. It's way faster, the microsecond delay is not doing any other code execution (only the milliseconds sleep, yield() and leaving the loop() are effectively cooperating with the other code on the board) so it seems to better fit real-time programming than microPython.

If I had know everything on forehand, I'd probably used an ESP32 (more usable IO's) or an Arduino Nano combined with an AT-flashed simple ESP.
