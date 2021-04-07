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

const bool apMode = true;


void setupWifi()
{

  const char* hostnameChar = WIFI_NAME;

  WiFi.setHostname(hostnameChar);

  // Print hostname.
  // Serial.println("Hostname: " + hostnameChar);

  WiFi.persistent(false);
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);

  if (apMode)
  {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(hostnameChar, apPassword);
    uint8_t val = 0;
    tcpip_adapter_dhcps_option(TCPIP_ADAPTER_OP_SET, TCPIP_ADAPTER_ROUTER_SOLICITATION_ADDRESS, &val, sizeof(dhcps_offer_t));
    Serial.printf("Connect to Wi-Fi access point: %s\n", hostnameChar);
    Serial.println("and open http://192.168.4.1 in your browser");
    #ifdef ESP32
    Serial.printf("soft AP IP: %s\n", WiFi.gatewayIP());
    #endif
  }
  else
  {
    WiFi.mode(WIFI_STA);
    Serial.printf("Connecting to %s\n", ssid);
    WiFi.begin(ssid, password);
  }
}
