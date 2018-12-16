# ESP32 FastLED Web Server
**Work in progress ESP32 port of https://github.com/jasoncoon/esp8266-fastled-webserver**...

But pretty functional I think. I've fixed many bugs and added other things I need to make a powerfull Christmas Tree.

The UI is a little bit changed, the PR of original project was used and there and there I fixed a little bit.
The WebServer work in the async mode (awesome library) and all assets are stored in the ESP32 SPIFFS.
I'm using 3x WS2811 led strips (50 leds on every).
Original code does not really work on WIFI_AP and WIFI_STA modes, it is more designed for WIFI_STA mode. I fixed that and there is no problem. 
Sometimes when code is uploaded do ESP32 it won't connect to the Wifi. If ESP can't connect to WiFi after 30 retries it will restart and try again.

## Below is _almost_ original README with some of my changes.

Control addressable LEDs with an ESP32 via a web browser over Wi-Fi.

## This is a *PlatformIO* fork of ESP32 FastLED Web Server

With some changes I need. To get started just open project (I'm using VS Code) and install FastLED library to using library manager.

### Upload data/ to SPIFFS:

```
pio run --target buildfs # only for the 1st time or when files changes
pio run --target uploadfs
```

Please remember that the full path for a file must be < 32 bits (it can't be `/js/a-very-very-very-very-long-filename.min.js`).

## Features
### Currently Working:

### On this fork:
* [x] All assets locally in SPIFFS
* [x] Async WebServer
* [x] WebSockets for automatically refreshing/syncing web clients working
* [x] Automatically send WebSocket updates on pattern and pallete change

#### Originally:
* [x] DemoReel100 patterns
* [x] [Sam's multi-core support](https://github.com/samguyer/FastLED/blob/master/examples/DemoReelESP32/DemoReelESP32.ino)
* [x] Static web app file serving from SPIFFS
* [x] Ability to adjust these settings via the HTTP REST API:
   * [x] power on/off
   * [x] brightness
   * [x] pattern
   * [x] autoplay on/off
   * [x] autoplay duration
   * [x] speed
   * [x] palette
   * [x] auto palette cycling
   * [x] palette duration
   * [x] solid color
   * [x] twinkle speed/density
   * [x] fire cooling/sparking
* [x] Setting storage in EEPROM

### Currently Lacking:
* [ ] Build assets by modern Webpack
* [ ] More patterns

### Known issues:
* RGB values does not update properly but Color Changer works.

## Requirements

### Hardware

#### ESP32 Development Board

[![Adafruit HUZZAH32 – ESP32 Feather Board](https://cdn-learn.adafruit.com/assets/assets/000/041/619/thumb100/feather_3405_iso_ORIG.jpg?1494445509)](https://www.adafruit.com/product/3405)

[Adafruit HUZZAH32 – ESP32 Feather Board](https://www.adafruit.com/product/3405)

**or**

[WEMOS LOLIN D32](https://wiki.wemos.cc/products:d32:d32)

**or**

[An ESP32 development board of your choice](https://www.google.com/search?q=esp32+development+board)

#### Addressable LED strip

[![Adafruit NeoPixel Ring](https://www.adafruit.com/images/145x109/1586-00.jpg)](https://www.adafruit.com/product/1586)

[Adafruit NeoPixel Ring](https://www.adafruit.com/product/1586)

#### Other hardware:

* [3.3V to 5V Logic Level Shifter](http://www.digikey.com/product-detail/en/texas-instruments/SN74HCT245N/296-1612-5-ND/277258) (required if LEDs "glitch")
* [Octo Level Shifter FeatherWing](https://www.evilgeniuslabs.org/level-shifter-featherwing) (tidy level shifter PCB)

Recommended by [Adafruit NeoPixel "Best Practices"](https://learn.adafruit.com/adafruit-neopixel-uberguide/best-practices) to help protect LEDs from current onrush:
* [1000µF Capacitor](http://www.digikey.com/product-detail/en/panasonic-electronic-components/ECA-1EM102/P5156-ND/245015)
* [300 to 500 Ohm resistor](https://www.digikey.com/product-detail/en/stackpole-electronics-inc/CF14JT470R/CF14JT470RCT-ND/1830342)
* sn74hct245n

### Software

* [VS Code](https://code.visualstudio.com/download)
* [PlatformIO](https://platformio.org/)

#### Libraries

* [samguyer/FastLED](https://github.com/samguyer/FastLED)

All other required packages are required by PlatformIO