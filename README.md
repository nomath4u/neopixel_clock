Binary clock based on neopixels.

This uses an ESP8266 and gets network time so that we don't have to deal with DST and time zones and drift and stuff.

reqires you to make a file named ssid.h with a #define SSID and #define SSIDPWD which are string with your network SSID and network password respectivly.

If you don't like having the background lights to see which ones are off then change light magenta to all 0s.

Based on a 30 pixel strip but as long as you have 20 in a row it will work
fine. However you will want 21 if you want to show the WiFi connection status

Things for the future: photo resisotr for auto brightness. External button to change color scheme.
