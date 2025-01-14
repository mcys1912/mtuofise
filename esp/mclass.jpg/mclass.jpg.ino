#include <WiFi.h>               // ESP32 için Wi-Fi kütüphanesi
#include <FirebaseESP32.h>       // ESP32 için Firebase kütüphanesi
#include <ESP32Servo.h>          // ESP32 için Servo kütüphanesi

// Wi-Fi bağlantı bilgileri
const char* ssid = "13Pro";
const char* password = "testesp32";

// Firebase yapılandırması
FirebaseAuth auth;
FirebaseConfig config;
FirebaseData fbdo;

// Servo motor pinleri
#define SERVO1_PIN 13
#define SERVO2_PIN 12
#define SERVO3_PIN 14
#define SERVO4_PIN 15

// Joystick pinleri
#define JOYSTICK1_X_PIN 34
#define JOYSTICK1_Y_PIN 35
#define JOYSTICK2_X_PIN 32
#define JOYSTICK2_Y_PIN 33

// Servo motorlar
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;

// Firebase URL ve API anahtarı
const char* firebase_host = "https://mturobotcontroller-1d7d9-default-rtdb.firebaseio.com/";
const char* firebase_api_key = "AIzaSyClUMogNYhLMvvDYAnqPw0_qgCOiPK2TQk";

// Kontrol modu (manuel veya otomatik)
String control_mode = "manuel";

void setup() {
  Serial.begin(115200);

  // Wi-Fi bağlantısı
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("WiFi bağlantısı kuruluyor...");
  }
  Serial.println("WiFi bağlantısı başarılı!");

  // Firebase yapılandırma
  config.host = firebase_host;
  config.api_key = firebase_api_key;

  auth.user.email = "esp@gmail.com";
  auth.user.password = "esp12345";

  Firebase.begin(&config, &auth);
  Serial.println("Firebase bağlantısı başarılı!");

  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);
  servo3.attach(SERVO3_PIN);
  servo4.attach(SERVO4_PIN);

  servo1.write(90);
  servo2.write(90);
  servo3.write(90);
  servo4.write(90);
}

void loop() {
  // Firebase'den kontrol modunu oku
  if (Firebase.getString(fbdo, "/control_mode")) {
    control_mode = fbdo.stringData();
  } else {
    Serial.println("Kontrol modu okunamadı.");
    Serial.println(fbdo.errorReason());
  }

  if (control_mode == "manuel") {
    // Joystick değerlerini oku
    int joystick1_x = analogRead(JOYSTICK1_X_PIN);
    int joystick1_y = analogRead(JOYSTICK1_Y_PIN);
    int joystick2_x = analogRead(JOYSTICK2_X_PIN);
    int joystick2_y = analogRead(JOYSTICK2_Y_PIN);

    // Servo pozisyonlarını hesapla
    int servo1_pos = map(joystick1_y, 0, 1023, 0, 180);
    int servo2_pos = map(joystick1_x, 0, 1023, 0, 180);
    int servo3_pos = map(joystick2_y, 0, 1023, 0, 180);
    int servo4_pos = map(joystick2_x, 0, 1023, 0, 180);

    // Servo motorları kontrol et
    servo1.write(servo1_pos);
    servo2.write(servo2_pos);
    servo3.write(servo3_pos);
    servo4.write(servo4_pos);

    // Firebase'e servo pozisyonlarını yaz
    Firebase.setFloat(fbdo, "/robot_position/servo1", servo1_pos);
    Firebase.setFloat(fbdo, "/robot_position/servo2", servo2_pos);
    Firebase.setFloat(fbdo, "/robot_position/servo3", servo3_pos);
    Firebase.setFloat(fbdo, "/robot_position/servo4", servo4_pos);

    Serial.println("Manuel kontrol: Firebase'e pozisyonlar yazıldı.");
  } else if (control_mode == "otomatik") {
    // Firebase'den servo pozisyonlarını oku
    float servo1_pos, servo2_pos, servo3_pos, servo4_pos;

    if (Firebase.getFloat(fbdo, "/robot_position/servo1")) {
      servo1_pos = fbdo.floatData();
      servo1.write(servo1_pos);
    }

    if (Firebase.getFloat(fbdo, "/robot_position/servo2")) {
      servo2_pos = fbdo.floatData();
      servo2.write(servo2_pos);
    }

    if (Firebase.getFloat(fbdo, "/robot_position/servo3")) {
      servo3_pos = fbdo.floatData();
      servo3.write(servo3_pos);
    }

    if (Firebase.getFloat(fbdo, "/robot_position/servo4")) {
      servo4_pos = fbdo.floatData();
      servo4.write(servo4_pos);
    }

    Serial.println("Otomatik kontrol: Firebase'den pozisyonlar okundu.");
  } else {
    Serial.println("Bilinmeyen kontrol modu.");
  }

  delay(500);
}
