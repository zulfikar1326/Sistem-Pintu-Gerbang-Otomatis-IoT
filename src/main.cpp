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
const char* WIFI_SSID = "24";          // GANTI DENGAN SSID WIFI
const char* PASS_WIFI = "11113333";    // GANTI DENGAN PASSWORD WIFI

// ========================== RFID SETUP PIN ========================== 
#define SS_PIN 5
#define RST_PIN 4
MFRC522 mfrc522(SS_PIN, RST_PIN);

// ========================== PIN LED, SERVO, BUZZER ========================== 
#define PIN_LED_BLUE 2
#define PIN_SERVO 13
#define PIN_BUZZER 15

// ========================== INISIALISASI LIBRARY SERVO, BUZZER, LED ========================== 
Servo accessServo;

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

// ========================== KIRIM STATUS AKSES = TRUE ========================== 
void status_True_Card(int VPIN, int value) {
  Blynk.virtualWrite(VPIN, "True");               // KIRIM STATUS TRUE KE WIDGET DI VPIN
}

// ========================== SET PIN MODE ========================== 
void pin_Mode(int LED_BLUE, int LED_YELLOW, int LED_RED, int SERVO_PIN) {
  pinMode(LED_RED, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);
  accessServo.attach(SERVO_PIN);
}

// ========================== GERAKKAN SERVO UNTUK AKSES ========================== 
void servo_gerak(int value){
  Serial.println("SERVO AKSES ON");
  accessServo.write(value);
  delay(1000);
  accessServo.write(0);
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
  delay(500);
  digitalWrite(PIN_LED_BLUE, LOW);
  servo_gerak(100);
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
  display_OLED_CUSTOM_1("GERBANG DARURAT DIBUKA", 0, 0);
  Serial.println(" → GERBANG DIBUKA");
  digitalWrite(PIN_LED_BLUE, HIGH);
  delay(500);
  digitalWrite(PIN_LED_BLUE, LOW);
  servo_gerak(100);
  Blynk.virtualWrite(4, 1); // KIRIM STATUS DARURAT
}

// ========================== SETUP ========================== 
void setup() {
  Serial.begin(115200);
  Wire.begin(PIN_SDA_OLED, PIN_SCL_OLED);
  pin_Mode(PIN_LED_BLUE,  PIN_SERVO);
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
  delay(500);
}
