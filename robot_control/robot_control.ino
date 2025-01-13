#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ESP32Servo.h>

const char* ssid = "WIFI_ADI";
const char* password = "WIFI_SIFRESI";

WebSocketsServer webSocket = WebSocketsServer(81);
Servo servos[4];
const int servoPins[4] = {13, 12, 14, 27}; // ESP32 pinleri

void setup() {
  Serial.begin(115200);
  
  // WiFi bağlantısı
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());
  
  // Servoları başlat
  for(int i = 0; i < 4; i++) {
    servos[i].attach(servoPins[i]);
    servos[i].write(90);
  }
  
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if(type == WStype_TEXT) {
    String message = String((char*)payload);
    int index = 0;
    int lastIndex = 0;
    int servoIndex = 0;
    
    // Gelen veriyi parse et (format: "angle1,angle2,angle3,angle4")
    while(lastIndex >= 0 && servoIndex < 4) {
      index = message.indexOf(',', lastIndex);
      String angleStr = index < 0 ? 
                       message.substring(lastIndex) : 
                       message.substring(lastIndex, index);
      float angle = angleStr.toFloat();
      servos[servoIndex].write(angle);
      lastIndex = index + 1;
      servoIndex++;
    }
  }
} 