# 7-Segment-Digital-Clock-V2

This is a new version of a 7 segment digital clocks that uses WS2812B LED's. Instructions how to build can be found in the YouTube video.

## License

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><br />This work is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License</a>.


## Components

| Components                    | x     |
| -------------                 | ----- |
|ESP8266 WeMos Mini D1          | 1x    |
|DS3231 RTC                     | 1x    |
|Micro USB Breakout board       | 1x    |
|Micro USB cable                | 1x    |
|5V 2.5A power supply           | 1x    |
|WS2812B LED Strip 60 LED's per meter     | 2x    |

## 3D Printing

Most files can be printed with 0.3 setting and low infill.


## Schema

Connect D6 from the ESP8266 through a 10 ohm resistor to the data pin of the LED strip. Connect the RTC to gnd, vcc and connect SDA and SCLK to the ESP8266 (check the pin layout of the ESP board you are using). Don't power the LED strip from the ESP board but use a Micro USB breakout board en connect the 5V to the LED strip.

## Arduino code

Upload the .ino file to your ESP board. You also need to upload the html and javascript files with the ESP8266FS tool. Check this link for more info:

http://arduino.esp8266.com/Arduino/versions/2.3.0/doc/filesystem.html#uploading-files-to-file-system

Before uploading the files to your ESP board you have to gzip them with the command:

`gzip -r ./data/`

Afterwards if you want to change something to the html files, just unzip with:

`gunzip -r ./data/`

## YouTube video

<a href="https://youtu.be/tGhew6kTAwc" target="_blank"><img src="https://img.youtube.com/vi/tGhew6kTAwc/0.jpg" 
alt="Click to view: 7 Segment Digital Clock V2" width="500" border="1" /></a>








