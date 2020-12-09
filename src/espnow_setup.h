
#ifdef ESP_NOW_MASTER

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

#endif

#ifdef ESP_NOW_SLAVE

// Callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  // Create a struct_message called myData
  field_update_message myData;
  // String newValue = setFieldValue(name, value, fields, fieldCount);
  memcpy(&myData, incomingData, sizeof(myData));

  power = myData.power;
  brightness = myData.brightness;
  speed = myData.speed;
  currentPatternIndex = myData.currentPatternIndex;
  autoplay = myData.autoplay;
  autoplayDuration = myData.autoplayDuration;
  currentPaletteIndex = myData.currentPaletteIndex;
  cyclePalettes = myData.cyclePalettes;
  paletteDuration = myData.paletteDuration;
  solidColor = myData.solidColor;
  cooling = myData.cooling;
  sparking = myData.sparking;
  twinkleSpeed = myData.twinkleSpeed;
  twinkleDensity = myData.twinkleDensity;
  mirrored = myData.mirrored;

  Serial.println(String(sizeof(myData)));
}
#endif 

void initEspNow() {

  // Init ESP-NOW
  Serial.print("WIFI MAC ADDRESS FOR ESPNOW: ");
  Serial.println(WiFi.macAddress());

  #ifdef ESP_NOW_SLAVE
  WiFi.mode(WIFI_STA);
  #endif

  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
      // ESP_NOW_ROLE_CONTROLLER, ESP_NOW_ROLE_SLAVE, ESP_NOW_ROLE_COMBO, ESP_NOW_ROLE_MAX
    #ifdef ESP_NOW_SLAVE
    // esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
    esp_now_register_recv_cb(OnDataRecv);
    #endif 

    #ifdef ESP_NOW_MASTER
    // esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
    esp_now_register_send_cb(OnDataSent);
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    // Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("Failed to add peer");
      return;
    }

    #endif
  };


