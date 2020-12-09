// Structure example to receive data
// Must match the sender structure
typedef struct field_update_message {
    uint8_t power;
    uint8_t brightness;
    uint8_t speed;
    uint8_t currentPatternIndex;
    uint8_t autoplay;
    uint8_t autoplayDuration;
    uint8_t currentPaletteIndex;
    uint8_t cyclePalettes;
    uint8_t paletteDuration;
    CRGB solidColor;
    uint8_t cooling;
    uint8_t sparking;
    uint8_t twinkleSpeed;
    uint8_t twinkleDensity;
    uint8_t mirrored;
} field_update_message;

// typedef struct field_update_message {
//     CRGB leds[NUM_LEDS];
// } field_update_message;

// update other clients (note may conflict with receieving)
#ifdef ESP8266
  #include <espnow.h>
#elif ESP32
  #include <esp_now.h>
#endif


#ifdef ESP_NOW_MASTER

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x5C, 0xCF, 0x7F, 0xEF, 0x4C, 0x06};

#endif

void updateOtherClients () {
    #ifdef ESP_NOW_MASTER
    // Create a struct_message called myData
    field_update_message myData;
    // fill struct up
    myData.power = power;
    myData.brightness = brightness;
    myData.speed = speed;
    myData.currentPatternIndex = currentPatternIndex;
    myData.autoplay = autoplay;
    myData.autoplayDuration = autoplayDuration;
    myData.currentPaletteIndex = currentPaletteIndex;
    myData.cyclePalettes = cyclePalettes;
    myData.paletteDuration = paletteDuration;
    myData.solidColor = solidColor;
    myData.cooling = cooling;
    myData.sparking = sparking;
    myData.twinkleSpeed = twinkleSpeed;
    myData.twinkleDensity = twinkleDensity;
    myData.mirrored = mirrored;

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    
    if (result == ESP_OK) {
      Serial.println("Sent with success");
      Serial.println(String(sizeof(myData)));
    }
    else {
      Serial.println("Error sending the data");
    }
    #endif
}

