// Structure example to receive data
// Must match the sender structure
typedef struct field_update_message {
    String name; // field name
    String value; // field value max is 255,255,255 for color type
} field_update_message;

// update other clients (note may conflict with receieving)
#ifdef ESP8266
  #include <espnow.h>
#elif ESP32
  #include <esp_now.h>
#endif


#ifdef ESP_NOW_MASTER

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

#endif

void updateOtherClients (String name, String value) {
    #ifdef ESP_NOW_MASTER
    // Create a struct_message called myData
    field_update_message myData;
    myData.name = name;
    myData.value = value;

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }
    #endif
}

