// Board tipini belirle
#if defined(ESP32)
  #include <WiFi.h>
  #include <FirebaseESP32.h>
  #define BOARD_TYPE "ESP32"
  #define ANALOG_RESOLUTION 4095  // ESP32 12-bit ADC
  // ESP32'de ek analog pinler
  #define JOYSTICK1_X 36  // VP - ADC1_CH0
  #define JOYSTICK1_Y 39  // VN - ADC1_CH3
  #define JOYSTICK2_X 34  // ADC1_CH6
  #define JOYSTICK2_Y 35  // ADC1_CH7
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <FirebaseESP8266.h>
  #define BOARD_TYPE "ESP8266"
  #define ANALOG_RESOLUTION 1023  // ESP8266 10-bit ADC
  // ESP8266'da sınırlı analog pin
  #define JOYSTICK1_X A0
  #define JOYSTICK1_Y D5
  #define JOYSTICK2_X D6
  #define JOYSTICK2_Y D7
#else
  #error "Bu kod sadece ESP8266 veya ESP32 için tasarlanmıştır!"
#endif

#include <ArduinoJson.h>
#include <Servo.h>

// WiFi bilgileri
#define WIFI_SSID "WiFi_Adınız"
#define WIFI_PASSWORD "WiFi_Şifreniz"

// Firebase proje bilgileri
#define FIREBASE_HOST "your-project.firebaseio.com"
#define FIREBASE_AUTH "Firebase_Secret_Key"

// Servo motor pinleri
#if defined(ESP32)
  #define SERVO1_PIN 13
  #define SERVO2_PIN 12
  #define SERVO3_PIN 14
  #define SERVO4_PIN 27
#else  // ESP8266
  #define SERVO1_PIN D1
  #define SERVO2_PIN D2
  #define SERVO3_PIN D3
  #define SERVO4_PIN D4
#endif

Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Analog okuma fonksiyonu
int readAnalog(int pin) {
  #if defined(ESP32)
    return analogRead(pin);
  #else
    if (pin == A0) {
      return analogRead(pin);
    } else {
      // ESP8266'da dijital pinlerden analog okuma simülasyonu
      return digitalRead(pin) * ANALOG_RESOLUTION;
    }
  #endif
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("Board Tipi: ");
  Serial.println(BOARD_TYPE);
  
  #if defined(ESP32)
    // ESP32 ADC çözünürlüğünü ayarla
    analogReadResolution(12);
  #endif
  
  // Servo motorları başlat
  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);
  servo3.attach(SERVO3_PIN);
  servo4.attach(SERVO4_PIN);
  
  // Başlangıç pozisyonuna getir (90 derece)
  servo1.write(90);
  servo2.write(90);
  servo3.write(90);
  servo4.write(90);
  
  // Joystick pinlerini ayarla
  pinMode(JOYSTICK1_X, INPUT);
  pinMode(JOYSTICK1_Y, INPUT);
  pinMode(JOYSTICK2_X, INPUT);
  pinMode(JOYSTICK2_Y, INPUT);
  
  // WiFi bağlantısı
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("WiFi Bağlanıyor");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nBağlandı");
  Serial.print("IP Adresi: ");
  Serial.println(WiFi.localIP());
  
  // Firebase başlatma
  config.host = FIREBASE_HOST;
  config.api_key = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
}

void loop() {
  // Fiziksel joystick değerlerini oku
  int joy1X = readAnalog(JOYSTICK1_X);
  int joy1Y = readAnalog(JOYSTICK1_Y);
  int joy2X = readAnalog(JOYSTICK2_X);
  int joy2Y = readAnalog(JOYSTICK2_Y);
  
  // Değerleri 0-1023 aralığına normalize et
  #if defined(ESP32)
    joy1X = map(joy1X, 0, ANALOG_RESOLUTION, 0, 1023);
    joy1Y = map(joy1Y, 0, ANALOG_RESOLUTION, 0, 1023);
    joy2X = map(joy2X, 0, ANALOG_RESOLUTION, 0, 1023);
    joy2Y = map(joy2Y, 0, ANALOG_RESOLUTION, 0, 1023);
  #endif
  
  // Firebase'den gelen değerleri oku
  if (Firebase.get(fbdo, "/robot_position")) {
    if (fbdo.dataType() == "json") {
      FirebaseJson json = fbdo.jsonObject();
      double servo1Angle, servo2Angle, servo3Angle, servo4Angle;
      
      FirebaseJsonData jsonData;
      json.get(jsonData, "servo1");
      servo1Angle = jsonData.doubleValue;
      json.get(jsonData, "servo2");
      servo2Angle = jsonData.doubleValue;
      json.get(jsonData, "servo3");
      servo3Angle = jsonData.doubleValue;
      json.get(jsonData, "servo4");
      servo4Angle = jsonData.doubleValue;
      
      // Fiziksel joystickler kullanılıyorsa onları öncelikle kullan
      bool joy1Active = (abs(joy1X - 512) > 50 || abs(joy1Y - 512) > 50);
      bool joy2Active = (abs(joy2X - 512) > 50 || abs(joy2Y - 512) > 50);
      
      if (joy1Active || joy2Active) {
        // Sol joystick aktifse
        if (joy1Active) {
          double angle1 = map(joy1X, 0, 1023, 0, 180);
          double angle2 = map(joy1Y, 0, 1023, 0, 180);
          servo1.write(angle1);  // Sol ön
          servo2.write(angle2);  // Sol arka
          
          Serial.print("Sol Joystick - X: ");
          Serial.print(angle1);
          Serial.print(" Y: ");
          Serial.println(angle2);
        }
        
        // Sağ joystick aktifse
        if (joy2Active) {
          double angle3 = map(joy2X, 0, 1023, 0, 180);
          double angle4 = map(joy2Y, 0, 1023, 0, 180);
          servo3.write(angle3);  // Sağ ön
          servo4.write(angle4);  // Sağ arka
          
          Serial.print("Sağ Joystick - X: ");
          Serial.print(angle3);
          Serial.print(" Y: ");
          Serial.println(angle4);
        }
        
        // Güncel açıları Firebase'e gönder
        guncellemeGonder();
      } else {
        // Fiziksel joystick kullanılmıyorsa Firebase değerlerini kullan
        servo1.write(servo1Angle);
        servo2.write(servo2Angle);
        servo3.write(servo3Angle);
        servo4.write(servo4Angle);
      }
    }
  }
  delay(50);
}

void guncellemeGonder() {
  FirebaseJson json;
  json.set("servo1", servo1.read());
  json.set("servo2", servo2.read());
  json.set("servo3", servo3.read());
  json.set("servo4", servo4.read());
  json.set("timestamp", "timestamp");
  
  Firebase.set(fbdo, "/robot_position", json);
} 