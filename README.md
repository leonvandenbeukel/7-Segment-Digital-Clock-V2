# 7-Segment-Digital-Clock-V2

This is a new version of a 7 segment digital clocks that uses WS2812B LED's. Instructions how to build can be found in the YouTube video.


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

