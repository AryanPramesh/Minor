#include <SPI.h>
#include <MFRC522.h>
#include <TFT_eSPI.h>

/* ---------- RFID PINS ---------- */
#define RFID_MISO  12
#define RFID_MOSI  13
#define RFID_SCK   14
#define RFID_SS    26
#define RFID_RST   27

/* ---------- Objects ---------- */
TFT_eSPI tft = TFT_eSPI();
SPIClass hspi(HSPI);
MFRC522 rfid(RFID_SS, RFID_RST);

void setup() {
  Serial.begin(115200);

  /* ---- TFT init (VSPI via TFT_eSPI) ---- */
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 20);
  tft.println("Booting...");

  /* ---- RFID init (force SPI to HSPI) ---- */
  SPI.end();                              // stop default SPI
  hspi.begin(RFID_SCK, RFID_MISO, RFID_MOSI, RFID_SS);
  SPI = hspi;                             // redirect SPI
  rfid.PCD_Init();
  
  Serial.println("RFID init done");

  tft.println("RFID Ready");
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(10, 40);
  tft.println("Card Detected");

  tft.setCursor(10, 80);
  tft.print("UID: ");

  for (byte i = 0; i < rfid.uid.size; i++) {
    tft.print(rfid.uid.uidByte[i], HEX);
    tft.print(" ");
  }

  rfid.PICC_HaltA();
  delay(1200);
}
