Binary clock based on neopixels.

Setup requires a button between PIN 4 and GND as well as PIN 10 and GND.
	PIN4 : Mode changer
	PIN10: Modifier in mode

If you don't like having the background lights to see which ones are off then change light magenta to all 0s.

Based on a 30 pixel strip but as long as you have 20 in a row it will work
fine. EXCEPT if you have debugging on. You will need to change the
debugging LED number to be something that is available on your strand
