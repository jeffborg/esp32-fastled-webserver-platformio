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

#pragma once

const bool apMode = false;

#define HOSTNAME "ESP32-" ///< Hostname. The setup function adds the Chip ID at the end.

void setupWifi()
{
  // Set Hostname.
  String hostname(HOSTNAME);

  #ifdef ESP32
  uint64_t chipid = ESP.getEfuseMac();
  #elif ESP8266
  uint32_t chipid = system_get_chip_id();
  #endif
  uint16_t long1 = (unsigned long)((chipid & 0xFFFF0000) >> 16);
  uint16_t long2 = (unsigned long)((chipid & 0x0000FFFF));
  String hex = String(long1, HEX) + String(long2, HEX); // six octets
  hostname += hex;

  char hostnameChar[hostname.length() + 1];
  memset(hostnameChar, 0, hostname.length() + 1);

  for (uint8_t i = 0; i < hostname.length(); i++)
    hostnameChar[i] = hostname.charAt(i);

  #ifdef ESP32
  WiFi.setHostname(hostnameChar);
  #endif

  // Print hostname.
  Serial.println("Hostname: " + hostname);

  WiFi.persistent(false);
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);

  if (apMode)
  {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(hostnameChar, apPassword);
    Serial.printf("Connect to Wi-Fi access point: %s\n", hostnameChar);
    Serial.println("and open http://192.168.4.1 in your browser");
    #ifdef ESP32
    Serial.printf("soft AP IP: %s\n", WiFi.softAPIP());
    #endif
  }
  else
  {
    WiFi.mode(WIFI_STA);
    Serial.printf("Connecting to %s\n", ssid);
    WiFi.begin(ssid, password);
  }
}
