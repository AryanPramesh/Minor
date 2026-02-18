#include <SPI.h>
#include <MFRC522.h>

#define RFID_MISO 12
#define RFID_MOSI 13
#define RFID_SCK  14
#define RFID_SS   26
#define RFID_RST  27

SPIClass hspi(HSPI);
MFRC522 rfid(RFID_SS, RFID_RST);

void setup() {
  Serial.begin(115200);

  // Start SPI bus for RFID
  hspi.begin(RFID_SCK, RFID_MISO, RFID_MOSI, RFID_SS);
  SPI = hspi;

  // Initialize RFID
  rfid.PCD_Init();
  Serial.println("Place your RFID card near the reader...");
}

void loop() {
  // Look for new cards
  if (!rfid.PICC_IsNewCardPresent()) return;

  // Select one of the cards
  if (!rfid.PICC_ReadCardSerial()) return;

  // Print UID
  Serial.print("RFID UID: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) Serial.print("0");
    Serial.print(rfid.uid.uidByte[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  // Halt PICC
  rfid.PICC_HaltA();
}
