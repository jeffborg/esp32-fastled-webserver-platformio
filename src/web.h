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

uint8_t connectTry = 0;

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    Serial.printf("[%u] Disconnected!\n", num);
    break;

  case WStype_CONNECTED:
  {
    IPAddress ip = webSocketsServer.remoteIP(num);
    Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

    // send message to client
    // webSocketsServer.sendTXT(num, "Connected");
  }
  break;

  case WStype_TEXT:
    Serial.printf("[%u] get Text: %s\n", num, payload);

    // send message to client
    // webSocketsServer.sendTXT(num, "message here");

    // send data to all connected clients
    // webSocketsServer.broadcastTXT("someone else connected");
    break;

  case WStype_BIN:
    Serial.printf("[%u] get binary length: %u\n", num, length);
    //  hexdump(payload, length);

    // send message to client
    // webSocketsServer.sendBIN(num, payload, lenght);
    break;
  }
}

void setupWeb()
{
  webServer.on("/all", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(LED_BUILTIN, HIGH);
    String json = getFieldsJson(fields, fieldCount);
    request->send(200, "text/json", json);
    digitalWrite(LED_BUILTIN, LOW);
  });

  webServer.on("/fieldValue", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(LED_BUILTIN, HIGH);
    String name = request->getParam("name")->value();
    String value = getFieldValue(name, fields, fieldCount);
    request->send(200, "text/json", value);
    digitalWrite(LED_BUILTIN, LOW);
  });

  webServer.on("/fieldValue", HTTP_POST, [](AsyncWebServerRequest *request) {
    digitalWrite(LED_BUILTIN, HIGH);
    String name = request->getParam("name", true)->value();

    Field field = getField(name, fields, fieldCount);
    String value;
    if (field.type == ColorFieldType)
    {
      String r = request->getParam("r", true)->value();
      String g = request->getParam("g", true)->value();
      String b = request->getParam("b", true)->value();
      value = r + "," + g + "," + b;
    }
    else
    {
      value = request->getParam("value", true)->value();
    }

    String newValue = setFieldValue(name, value, fields, fieldCount);
    request->send(200, "text/json", newValue);
    digitalWrite(LED_BUILTIN, LOW);
  });

  webServer.serveStatic("/", SPIFFS, "/").setDefaultFile("index.htm").setCacheControl("max-age=864000");

  webServer.begin();
  Serial.println("HTTP server started");

  webSocketsServer.begin();
  webSocketsServer.onEvent(webSocketEvent);
  Serial.println("WebSockets server started");
}

void handleWeb()
{
  static bool webServerStarted = false;

  // check for connection
  if (apMode == true || (apMode == false && WiFi.status() == WL_CONNECTED))
  {
    if (!webServerStarted)
    {
      // turn off the board's LED when connected to wifi
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.println();
      Serial.println("WiFi connected");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      webServerStarted = true;
      digitalWrite(LED_BUILTIN, LOW);
    }

    webSocketsServer.loop();
  }
  else
  {
    // blink the board's LED while connecting to wifi
    // static uint8_t ledState = 0;
    EVERY_N_MILLIS(125)
    {
      // ledState = ;
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      connectTry = connectTry + 1;
      if (connectTry > 30)
      {
        Serial.println("");
        Serial.println("could not connect to wifi, restarting...");
        ESP.restart();
      }
      Serial.print(".");
    }
  }
}
