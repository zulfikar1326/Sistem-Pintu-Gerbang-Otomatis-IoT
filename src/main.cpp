// ========================== Libray setup ========================== 
#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <WiFi.h>
#include <ESP32Servo.h>
#include <Buzzer.h>
#include <EasyLed.h>

// ========================== OLED setup ========================== 
#define PIN_SDA_OLED 21
#define PIN_SCL_OLED 22
#define OLED_ADDRESS 0x3C
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

// ========================== Blynk setup ========================== 
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6nFLZcIDS"
#define BLYNK_TEMPLATE_NAME "Proyek LED on off"
#define BLYNK_AUTH_TOKEN "MfkjtIdO6LV4WNO3RKt2sjikqxI8HlXp"
#include <BlynkSimpleEsp32.h>

// ========================== WIFI SETUP ========================== 
const char* WIFI_SSID = "24";          // Ganti dengan SSID WiFi
const char* PASS_WIFI = "11113333";  // Ganti dengan password WiFi

// ========================== RFID SETUP PIN ========================== 
// RFID setup
#define SS_PIN 5
#define RST_PIN 4
MFRC522 mfrc522(SS_PIN, RST_PIN);

// LED pins
#define PIN_LED_RED 33
#define PIN_LED_YELLOW 32
#define PIN_LED_BLUE 2
#define PIN_SERVO 13
#define PIN_BUZZER 15



// ========================== INISIALISASI LIBRARY ==========================
Servo accessServo;
// ========================== INISIALISASI BUZZER ==========================
Buzzer Buzzer(PIN_BUZZER);
EasyLed led_red(PIN_LED_YELLOW, EasyLed::ActiveLevel::Low, EasyLed::State::Off)



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

// ========================== BLYNK SETUP ==========================
void blynk_access() {
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, PASS_WIFI);
  Serial.println("Blynk Connected");
}

void kirim_ID_CARD_blynk(byte length ,int VPIN) {
  String uidString = "";
  for (byte i = 0; i < length; i++) {
    uidString += String(mfrc522.uid.uidByte[i], HEX);
  }
  uidString.toUpperCase();
  Serial.print("UID Kartu HEX: ");
  Serial.println(uidString);
  Blynk.virtualWrite(VPIN, uidString);            // Kirim VALUE ke widget di Virtual Pin VPIN
}

void status_True_Card(int VPIN, int value) {
  Blynk.virtualWrite(VPIN, "True");            // Kirim VALUE ke widget di Virtual Pin VPIN
}

// ========================== PIN setup ==========================
void pin_Mode(int LED_BLUE, int LED_YELLOW, int LED_RED, int SERVO_PIN) {
  
  // INISIALISASI LED
  pinMode(LED_RED, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);

  // INISIALISASI SERVO
  accessServo.attach(SERVO_PIN);

}
// ========================== SETUP SERVO ==========================
void servo_gerak(int value){
  Serial.println("SERVO AKSES ON");
  accessServo.write(value);
  delay(1000);
  accessServo.write(0);
}
// ========================== CARD UIDsetup ==========================
byte card_uuid[][4] = {
  {0x6A, 0x2B, 0xC1, 0x01}, // Admmin Card
  // {0xDE, 0xDF, 0x32, 0x03}, // Client Card
};
const int totalUID = sizeof(card_uuid) / sizeof(card_uuid[0]);

// ========================== CARD VERIFIKASI ==========================
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

void printUID(byte *buffer, byte length) {
  for (byte i = 0; i < length; i++) {
    Serial.print(buffer[i] < 0x10 ? "0" : "");
    Serial.print(buffer[i], HEX);
    Serial.print(" ");
  }
}

void berhasil_verifikasi(){
  display_OLED_CUSTOM_1("Akses Diterima", 0, 0);
  Serial.println(" → AKSES DITERIMA");

  // MENHIDUPKAN LED BIRU
  digitalWrite(PIN_LED_BLUE, HIGH);
  delay(500);
  digitalWrite(PIN_LED_BLUE, LOW);
  
  // TURN ON SERVO
  servo_gerak(100);

  // MENGIRIM KAN STATUS KARTU DITERIMA KE BYLNK
  Blynk.virtualWrite(4, 1); 
}

void gagal_verifikasi(){

  Serial.print("UID Kartu HEX: ");
  Serial.println(isAuthorized(mfrc522.uid.uidByte));
  display_OLED_CUSTOM_1("Akses Ditolak", 0, 0);

  // MENGHIDUPKAN LED MERAH
  digitalWrite(PIN_LED_RED, HIGH);

  // MENAMPILKAN KE SERIAL_MONITOR
  Serial.println("Kartu tidak terdaftar");
  Serial.println(" → AKSES DITOLAK");

  // MENGIRIM KAN STATUS KARTU GAGAL KE BYLNK
  Blynk.virtualWrite(4, 0);
}

void buka_gerbang_darurat(){
  display_OLED_CUSTOM_1("Gerbang Darurat Dibuka", 0, 0);
  Serial.println(" → GERBANG DIBUKA");

  // MENHIDUPKAN LED BIRU
  digitalWrite(PIN_LED_BLUE, HIGH);
  delay(500);
  digitalWrite(PIN_LED_BLUE, LOW);
  
  // TURN ON SERVO
  servo_gerak(100);

  // MENGIRIM KAN STATUS KARTU DITERIMA KE BYLNK
  Blynk.virtualWrite(4, 1); 
}



void setup() {
    Serial.begin(115200); // Initialize serial communications with the PC

    digitalWrite(PIN_BUZZER, LOW);
    Wire.begin(PIN_SDA_OLED, PIN_SCL_OLED);
    pin_Mode(PIN_LED_BLUE, PIN_LED_YELLOW, PIN_LED_RED, PIN_SERVO);
    blynk_access();
    
    SPI.begin();        // Init SPI bus
    mfrc522.PCD_Init(); // Init MFRC522 card
    display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);

    display_OLED_CUSTOM_1("KELOMPOK 7", 0, 0);

    display_OLED_CUSTOM_1("Tempelkan Kartu untuk Autentifikasi", 0, 20);
    Serial.println("Tempelkan kartu untuk autentikasi...");
}

void loop(){
  
  digitalWrite(PIN_LED_RED, HIGH);
  display_OLED_CUSTOM_1("Menunggu Kartu...", 0, 0);
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return; // No new card present or error reading card
  }

  Serial.print("UID Kartu: ");
  printUID(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();

  kirim_ID_CARD_blynk(mfrc522.uid.size,0);


  // MODIFIKASI SOURCE
  if (isAuthorized(mfrc522.uid.uidByte)) {
    berhasil_verifikasi();    
  } else if(!isAuthorized(mfrc522.uid.uidByte)) {
    gagal_verifikasi();
  }


  
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  delay(500);
}
