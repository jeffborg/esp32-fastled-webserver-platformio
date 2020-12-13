/*
   ESP32 FastLED WebServer: https://github.com/jasoncoon/esp32-fastled-webserver
   Copyright (C) 2017 Jason Coon

   Built upon the amazing FastLED work of Daniel Garcia and Mark Kriegsman:
   https://github.com/FastLED/FastLED

   ESP32 support provided by the hard work of Sam Guyer:
   https://github.com/samguyer/FastLED

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define FASTLED_INTERRUPT_RETRY_COUNT 0
#define FASTLED_ALLOW_INTERRUPTS 0

#include <Arduino.h>
#include <FastLED.h>
#include <CircularBuffer.h>

#ifdef ESP8266
// #define WEBSERVER_H
#include <ESP8266WiFi.h>
#endif

#ifdef ESP32
#define CONFIG_LITTLEFS_CACHE_SIZE 512
#define SPIFFS LITTLEFS
#include <LITTLEFS.h>
#include <WiFi.h>
#include <AsyncUDP.h>
#endif

#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <FS.h>
#include <EEPROM.h>

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3003000)
#warning "Requires FastLED 3.3 or later; check github for latest code."
#endif


AsyncWebServer webServer(80);
WebSocketsServer webSocketsServer = WebSocketsServer(81);
AsyncUDP udp;

const int led = 32;
// const int LED_BUILTIN = 2;

uint8_t gMaxPower = 10; // 200 millamps max is 2.5 amps at 255
#define MCU_POWER 250
#define MAX_TOTAL_POWER 2750
#define MAX_POWER_CONVERSION (gMaxPower * 20) < MAX_TOTAL_POWER - MCU_POWER ? (gMaxPower * 20) : MAX_TOTAL_POWER - MCU_POWER
uint8_t mirrored = 0;
uint8_t autoplay = 1;
uint8_t autoplayDuration = 60;
unsigned long autoPlayTimeout = 0;
uint8_t currentPatternIndex = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
uint8_t power = 1;
uint8_t brightness = 150;
uint8_t speed = 20;

// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100
uint8_t cooling = 50;

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
uint8_t sparking = 120;

CRGB solidColor = CRGB::Blue;

uint8_t cyclePalettes = 0;
uint8_t paletteDuration = 10;
uint8_t currentPaletteIndex = 0;
unsigned long paletteTimeout = 0;

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#ifdef ESP32
#define DATA_PIN 23 // LED_BUILTIN // pins tested so far on the Feather ESP32: 13, 12, 27, 33, 15, 32, 14, SCL
#endif
#ifdef ESP8266
#define DATA_PIN LED_BUILTIN // LED_BUILTIN // pins tested so far on the Feather ESP32: 13, 12, 27, 33, 15, 32, 14, SCL
#endif

//#define CLK_PIN   4
#define LED_TYPE WS2812
#define COLOR_ORDER GRB

#ifndef SKATE_LED_LENGTH
#define SKATE_LED_LENGTH 10
#endif

#define BUFFER_SIZE 20

// THIS is because every sketch has different counts
#define NUM_LEDS (mirrored == 1 ? SKATE_LED_LENGTH : (SKATE_LED_LENGTH * 2))
// animation buffer for routines to write into
CRGB leds[SKATE_LED_LENGTH * 2];

#define FRAMES_PER_SECOND 120


// Structure example to receive data
// Must match the sender structure
// Structure example to receive data
// Must match the sender structure
// 39 * 2 * 3 bytes each = 234bytes
typedef struct field_update_message {
    uint8_t brightness; // brightness to use
    uint8_t mxPower; // current power setting
    uint8_t ledCount; // led count
    unsigned long millis; // time message was sent
    CRGB leds[SKATE_LED_LENGTH];
} field_update_message;

enum response_types{ACK_PACKET, PLAYED_FRAME};
typedef struct struct_response {
  response_types responseType;
  unsigned long playedFrameMillis;
  unsigned long localTimeIn;
  unsigned long localTimePlayed;
} struct_response;

CircularBuffer<field_update_message, BUFFER_SIZE> buffer;
// playback buffer just recycle the output as it's much easier
field_update_message playback;


#include "patterns.h"

#include "field.h"
#include "fields.h"

#include "secrets.h"
#include "wifi_setup.h"

#include "web.h"

// wifi ssid and password should be added to a file in the sketch named secrets.h
// the secrets.h file should be added to the .gitignore file and never committed or
// pushed to public source control (GitHub).
// const char* ssid = "........";
// const char* password = "........";

// -- Task handles for use in the notifications
#ifdef ESP32

static TaskHandle_t FastLEDshowTaskHandle = 0;
static TaskHandle_t userTaskHandle = 0;

/** show() for ESP32
    Call this function instead of FastLED.show(). It signals core 0 to issue a show,
    then waits for a notification that it is done.
*/
void FastLEDshowESP32()
{
  if (userTaskHandle == 0)
  {
    // -- Store the handle of the current task, so that the show task can
    //    notify it when it's done
    userTaskHandle = xTaskGetCurrentTaskHandle();

    // -- Trigger the show task
    xTaskNotifyGive(FastLEDshowTaskHandle);

    // -- Wait to be notified that it's done
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(200);
    ulTaskNotifyTake(pdTRUE, xMaxBlockTime);
    userTaskHandle = 0;
  }
}

/** show Task
    This function runs on core 0 and just waits for requests to call FastLED.show()
*/
void FastLEDshowTask(void *pvParameters)
{
  // -- Run forever...
  for (;;)
  {
    // -- Wait for the trigger
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // -- Do the show (synchronously)
    FastLED.show();

    // -- Notify the calling task
    xTaskNotifyGive(userTaskHandle);
  }
}
#endif


void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
  Serial.printf("Listing directory: %s\n", dirname);
  File root = fs.open(dirname, "r");
  if (!root)
  {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels)
      {
        listDir(fs, file.name(), levels - 1);
      }
    }
    else
    {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  currentPatternIndex = (currentPatternIndex + 1) % patternCount;
  // updateOtherClients(); // broadcast esp now
  String json = "{\"name\":\"pattern\",\"value\":\"" + String(currentPatternIndex) + "\"}";
  webSocketsServer.broadcastTXT(json);
}

void nextPalette()
{
  currentPaletteIndex = (currentPaletteIndex + 1) % paletteCount;
  targetPalette = palettes[currentPaletteIndex];
  // updateOtherClients(); // broadcast esp now
  String json = "{\"name\":\"palette\",\"value\":\"" + String(currentPaletteIndex) + "\"}";
  webSocketsServer.broadcastTXT(json);
}

const char * udpAddress = "192.168.4.2";
const int udpPort = 4210;

void udpSendTest() {
    // send back a reply, to the IP address and port we got the packet from
    field_update_message myData;
    myData.mxPower = gMaxPower;
    myData.brightness = brightness;
    myData.ledCount = SKATE_LED_LENGTH;
    myData.millis = millis();
    if (mirrored) {
      memcpy(&myData.leds, &leds[0], sizeof(myData.leds));
    } else {
      std::reverse_copy(&leds[SKATE_LED_LENGTH], &leds[SKATE_LED_LENGTH * 2], myData.leds);
    }
    // memcpy(&myData.leds, &leds[mirrored ? 0 : (NUM_LEDS_PER_STRIP * 2)], sizeof(myData.leds));
    // buffer.push(myData);
    // AsyncUDPMessage message = AsyncUDPMessage(sizeof(myData));
    udp.broadcastTo((uint8_t *) &myData, sizeof(myData), 4210);
    // udp.writeTo((uint8_t *) &myData, sizeof(myData), IP_ADDR_BROADCAST , TCPIP_ADAPTER_IF_MAX);
    // message.
    // udp.write()
    // udp.writeTo((uint8_t *) &myData, sizeof(myData), udpAddress, udpPort);
    // udp.beginPacket(udpAddress, udpPort);
    // udp.write(, );
    // udp.endPacket();  
}

unsigned long networkDelay = 0;
uint16_t incomingCount = 0;
unsigned long playDelay = 0;
uint16_t playincomingCount = 0;

#include "esp32-hal-cpu.h"

void udpHandler(AsyncUDPPacket packet) {
  struct_response response;
  memcpy(&response, packet.data(), packet.available());
  if (response.responseType == ACK_PACKET) {
    networkDelay += millis() - response.playedFrameMillis;
    incomingCount++;
  } else if (response.responseType == PLAYED_FRAME) {
    playDelay += millis() - response.playedFrameMillis;
    playincomingCount++;
  }
  // packet.length()
  EVERY_N_MILLIS(1000) {
    Serial.printf("Delay: %d, Count: %d, Average: %d, HEAP %d\n", networkDelay, incomingCount, incomingCount > 0 ? networkDelay / incomingCount : 0, ESP.getFreeHeap());
    Serial.printf("Delay: %d, Count: %d, Average: %d\n", playDelay, playincomingCount, playincomingCount > 0 ? playDelay / playincomingCount : 0);
    networkDelay = 0;
    incomingCount = 0;
    playDelay = 0;
    playincomingCount = 0;
  }
}


void setup()
{
  // delay(5000);
  pinMode(led, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(led, 1);

  Serial.begin(115200);

  SPIFFS.begin();
  // listDir(SPIFFS, "/", 1);

  // restore from memory
  loadFieldsFromEEPROM(fields, fieldCount);
  setupWifi();
  setupWeb();

  // three-wire LEDs (WS2811, WS2812, NeoPixel)
  // playback from inside a struct
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(playback.leds, SKATE_LED_LENGTH).setCorrection(TypicalLEDStrip);

  // four-wire LEDs (APA102, DotStar)
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // Parallel output: 13, 12, 27, 33, 15, 32, 14, SCL
  // FastLED.addLeds<LED_TYPE, 13, COLOR_ORDER>(leds, 0, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
  // FastLED.addLeds<LED_TYPE, 12, COLOR_ORDER>(leds, NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
  // FastLED.addLeds<LED_TYPE, 27, COLOR_ORDER>(leds, 2 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
  // FastLED.addLeds<LED_TYPE, 33, COLOR_ORDER>(leds, 3 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
  // FastLED.addLeds<LED_TYPE, 15, COLOR_ORDER>(leds, 4 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
  // FastLED.addLeds<LED_TYPE, 32, COLOR_ORDER>(leds, 5 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
  // FastLED.addLeds<LED_TYPE, 14, COLOR_ORDER>(leds, 6 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
  // FastLED.addLeds<LED_TYPE, SCL, COLOR_ORDER>(leds, 7 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);

  FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_POWER_CONVERSION);
  set_max_power_indicator_LED(LED_BUILTIN);
  // set master brightness control
  FastLED.setBrightness(brightness);

  autoPlayTimeout = millis() + (autoplayDuration * 1000);

  //  if(udp.listen(4210)) {
  //       Serial.print("UDP Listening on IP: ");
  //       Serial.println(WiFi.localIP());
  //       udp.onPacket([](AsyncUDPPacket packet) {
  //       }
  if (udp.listen(4211)) {
    Serial.print("UDP Listening on port 4211: ");
    udp.onPacket(udpHandler);
  }
}

void loop()
{
  handleWeb();

  if (power == 0)
  {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
  }
  else
  {
    // Call the current pattern function once, updating the 'leds' array
    patterns[currentPatternIndex].pattern(leds, NUM_LEDS);

    EVERY_N_MILLISECONDS(40)
    {
      // slowly blend the current palette to the next
      nblendPaletteTowardPalette(currentPalette, targetPalette, 8);
      gHue++; // slowly cycle the "base color" through the rainbow
    }

  // if esp slave don't auto move forward
    if (autoplay == 1 && (millis() > autoPlayTimeout))
    {
      nextPattern();
      autoPlayTimeout = millis() + (autoplayDuration * 1000);
    }

    if (cyclePalettes == 1 && (millis() > paletteTimeout))
    {
      nextPalette();
      paletteTimeout = millis() + (paletteDuration * 1000);
    }
  }

  // send the 'leds' array out to the actual LED strip
  // FastLEDshowESP32();
  // mirror the 1st half of leds into the 2nd half if setup

  // if (mirrored == 1) {
  //   // copy the 2nd half over
  //   for( u8_t i = 0; i < NUM_LEDS; i++) {
  //     leds[SKATE_LED_LENGTH - i - 1] = leds[i];
  //   }
  // }
  // EVERY_N_MILLISECONDS(1000 / 30) {
  udpSendTest(); // buffer.push done inside here of 2nd half

  field_update_message myData;
  myData.brightness = brightness;
  myData.mxPower = gMaxPower;
  myData.ledCount = SKATE_LED_LENGTH;
  myData.millis = millis();
  memcpy(&myData.leds, leds, sizeof(myData.leds));
  buffer.push(myData);

  // }
  // updateOtherClients();
  if (buffer.isFull()) {
    playback = buffer.shift();
    FastLED.show();
  }
  // insert a delay to keep the framerate modest
  // FastLED.
  delay(1000 / FRAMES_PER_SECOND);
}
