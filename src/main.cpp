// ========================== LIBRARY SETUP ========================== 
#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <WiFi.h>
#include <ESP32Servo.h>

// ========================== OLED SETUP ========================== 
#define PIN_SDA_OLED 21
#define PIN_SCL_OLED 22
#define OLED_ADDRESS 0x3C
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

// ========================== BLYNK SETUP ========================== 
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6nFLZcIDS"
#define BLYNK_TEMPLATE_NAME "Proyek LED on off"
#define BLYNK_AUTH_TOKEN "MfkjtIdO6LV4WNO3RKt2sjikqxI8HlXp"
#include <BlynkSimpleEsp32.h>


// ========================== WIFI SETUP ========================== 
const char* WIFI_SSID = "24";          // SSID WIFI YANG AKAN DI CONNECT
const char* PASS_WIFI = "11113333";    // PASSWORD WIFI YANG AKAN DI CONNECT

// ========================== RFID SETUP PIN ========================== 
#define SS_PIN 5
#define RST_PIN 4
MFRC522 mfrc522(SS_PIN, RST_PIN);

// ========================== PIN LED, SERVO, BUZZER ========================== 
#define PIN_LED_BLUE 2
#define PIN_SERVO 13
#define PIN_RED_LED 12
#define PIN_TRIG_PIN  26
#define PIN_ECHO_PIN  25


// ========================== INISIALISASI LIBRARY SERVO, BUZZER, LED ========================== 
Servo accessServo;

// ========================== INISIALISASI Variabel ULTRASONIK ==========================
float jarak_cm;



// ========================== CUSTOM TEXT OLED 1 LINE ========================== 
void display_text_pintu_darurat(String text, int x, int y) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(x, y);
  display.print(text);
  display.display();
}


// ========================== CUSTOM TEXT OLED 1 LINE ========================== 
void display_OLED_CUSTOM_1(String line1, int x1, int y1) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(x1, y1);
  display.print(line1);
  display.display();
  delay(1000);
  display.clearDisplay();
}

// ========================== KONEKSI BLYNK ========================== 
void blynk_access() {
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, PASS_WIFI);
  Serial.println("Blynk Connected");
}

// ========================== KIRIM UID RFID KE BLYNK ========================== 
void kirim_ID_CARD_blynk(byte length ,int VPIN) {
  String uidString = "";
  for (byte i = 0; i < length; i++) {
    uidString += String(mfrc522.uid.uidByte[i], HEX);
  }
  uidString.toUpperCase();
  Serial.print("UID Kartu HEX: ");
  Serial.println(uidString);
  Blynk.virtualWrite(VPIN, uidString);            // KIRIM UID KE BLYNK VIRTUAL PIN VPIN
}
// ========================== SET PIN MODE ========================== 
void pin_Mode(int LED_BLUE, int SERVO_PIN, int ECHO_PIN, int TRIG_PIN) {
  read_ultrasonic(TRIG_PIN, ECHO_PIN);
  pinMode(PIN_LED_BLUE, OUTPUT);// Triger INPUT
  // Echo OUTPUT 

  accessServo.attach(SERVO_PIN);
}
// ========================== GERAKKAN SERVO UNTUK AKSES ========================== 
void servo_gerak(int value_gerak,int delayTime, int value_kembali) {
  Serial.println("SERVO AKSES ON");
  accessServo.write(value_gerak);
  delay(delayTime);
  accessServo.write(value_kembali);
}

// ========================== DAFTAR UID KARTU YANG DIAUTH ========================== 
byte card_uuid[][4] = {
  {0x6A, 0x2B, 0xC1, 0x01}, // ADMIN CARD
  // {0xDE, 0xDF, 0x32, 0x03}, // CLIENT CARD
};
const int totalUID = sizeof(card_uuid) / sizeof(card_uuid[0]);

// ========================== VERIFIKASI UID KARTU ========================== 
bool isAuthorized(byte *uid) {
  for (int i = 0; i < totalUID; i++) {
    bool match = true;
    for (int j = 0; j < 4; j++) {
      if (uid[j] != card_uuid[i][j]) {
        match = false;
        break;
      }
    }
    if (match) return true;
  }
  return false;
}

// ========================== CETAK UID KE SERIAL MONITOR ========================== 
void printUID(byte *buffer, byte length) {
  for (byte i = 0; i < length; i++) {
    Serial.print(buffer[i] < 0x10 ? "0" : "");
    Serial.print(buffer[i], HEX);
    Serial.print(" ");
  }
}

// ========================== JIKA VERIFIKASI BERHASIL ========================== 
void berhasil_verifikasi(){
  display_OLED_CUSTOM_1("AKSES DITERIMA", 0, 0);
  Serial.println(" → AKSES DITERIMA");
  digitalWrite(PIN_LED_BLUE, HIGH);
  servo_gerak(100,1000,100);
  delay(1000);
  servo_gerak(0,1000,0);
  digitalWrite(PIN_LED_BLUE, LOW);


  Blynk.virtualWrite(4, 1); // KIRIM STATUS TRUE
}

// ========================== JIKA VERIFIKASI GAGAL ========================== 
void gagal_verifikasi(){
  Serial.print("UID Kartu HEX: ");
  Serial.println(isAuthorized(mfrc522.uid.uidByte));
  
  display_OLED_CUSTOM_1("AKSES DITOLAK", 0, 0);
  Serial.println("KARTU TIDAK TERDAFTAR");
  Serial.println(" → AKSES DITOLAK");
  Blynk.virtualWrite(4, 0); // KIRIM STATUS FALSE
  }

// ========================== FUNGSI BUKA GERBANG DARURAT ========================== 
void buka_gerbang_darurat(){
  display_text_pintu_darurat("GERBANG DARURAT DIBUKA", 0, 0);
  Serial.println(" → GERBANG DIBUKA");
  digitalWrite(PIN_LED_BLUE, HIGH);
  servo_gerak(100,1000,100);
  Blynk.virtualWrite(2, 1); // KIRIM STATUS DARURAT
}

void tutup_gerbang_darurat(){
  display.clearDisplay();
  digitalWrite(PIN_LED_BLUE, LOW);
  display_text_pintu_darurat("GERBANG DARURAT DITUTUP", 0, 0);
  Serial.println(" → GERBANG DITUTUP");
  digitalWrite(PIN_LED_BLUE, HIGH);
  servo_gerak(100,1000,0);
  Blynk.virtualWrite(2, 1); // KIRIM STATUS DARURAT
  digitalWrite(PIN_LED_BLUE, LOW);
}

// ========================== BLYNK VIRTUAL PIN ==========================
BLYNK_WRITE(V2) {
  int pinValue = param.asInt();
  if (pinValue == 1) {
    Serial.println("GERBANG DARURAT DIBUKA");
    buka_gerbang_darurat();
  } else {
    Serial.println("GERBANG DARURAT DITUTUP");
    tutup_gerbang_darurat();
    Blynk.virtualWrite(2, 0); // KIRIM STATUS DARURAT
  }
}
// // ========================== MEMBACA ULTRASONIK ==========================
// void read_ultrasonic(int OUTPUT, int INPUT)  {
//  // Triger OUTPUT🙃 // Echo INPUT
//   pinMode(OUTPUT, OUTPUT);
//   pinMode(INPUT, INPUT);
//   digitalWrite(OUTPUT, LOW);
//   delayMicroseconds(2);
//   digitalWrite(OUTPUT, HIGH);
//   delayMicroseconds(10);
//   digitalWrite(OUTPUT, LOW);

//   jarak_cm = pulseIn(INPUT, HIGH) * 0.034 / 2;
//   Serial.print("Jarak: ");
//   Serial.print(jarak_cm);
//   Serial.println(" cm");

//   if (jarak_cm < 5) {
//     Serial.println("GERBANG DITUTUP KARNA TIDAK ADA KENDARAAN");
//     display_OLED_CUSTOM_1("GERBANG DITUTUP KARNA TIDAK ADA KENDARAAN", 0, 0);
//     tutup_gerbang_darurat();
//   }else{
//     Serial.println("Tidak ada Kendaraan");
//   }
// }


// ========================== SETUP ========================== 
void setup() {
  Serial.begin(115200);
  Wire.begin(PIN_SDA_OLED, PIN_SCL_OLED);
  pin_Mode(PIN_LED_BLUE, PIN_SERVO, PIN_ECHO_PIN, PIN_TRIG_PIN);

  
  blynk_access();
  SPI.begin();
  mfrc522.PCD_Init();
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
  display_OLED_CUSTOM_1("KELOMPOK 7", 0, 0);
  display_OLED_CUSTOM_1("TEMPELKAN KARTU UNTUK AUTENTIFIKASI", 0, 20);
  Serial.println("TEMPELKAN KARTU UNTUK AUTENTIKASI...");
}

// ========================== LOOP UTAMA ========================== 
void loop(){
  Blynk.run(); // MENJAGA BLYNK AGAR SELALU UPDATE (AUTO REFRESH)
  display_OLED_CUSTOM_1("MENUNGGU KARTU...", 0, 0);


  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print("UID Kartu: ");
  printUID(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();

  kirim_ID_CARD_blynk(mfrc522.uid.size,0);

  if (isAuthorized(mfrc522.uid.uidByte)) {
    berhasil_verifikasi();    
  } else {
    gagal_verifikasi();
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  delay(200);
}
