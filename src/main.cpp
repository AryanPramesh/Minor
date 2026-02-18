#include <SPI.h>
#include <MFRC522.h>
#include <TFT_eSPI.h>
#include <Adafruit_Fingerprint.h>

/* ---------- RFID ---------- */
#define RFID_MISO  12
#define RFID_MOSI  13
#define RFID_SCK   14
#define RFID_SS    26
#define RFID_RST   27
SPIClass hspi(HSPI);
MFRC522 rfid(RFID_SS, RFID_RST);

/* ---------- TFT ---------- */
TFT_eSPI tft = TFT_eSPI();

/* ---------- Fingerprint ---------- */
#define FINGER_RX 17
#define FINGER_TX 16
HardwareSerial fingerSerial(1); // UART1
Adafruit_Fingerprint finger(&fingerSerial);

/* ---------- FUNCTIONS ---------- */

void tftBoot() {
  tft.fillScreen(TFT_RED); delay(200);
  tft.fillScreen(TFT_GREEN); delay(200);
  tft.fillScreen(TFT_BLUE); delay(200);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(2);
  tft.setCursor(20, 40);
  tft.println("System Booted");
}

int findEmptySlot() {
  for (int id = 1; id <= 200; id++) {
    if (finger.loadModel(id) != FINGERPRINT_OK) return id;
  }
  return -1;
}

void enrollFingerprint() {
  int slot = findEmptySlot();
  if (slot == -1) {
    tft.println("No empty slots!");
    return;
  }

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(20, 40);
  tft.setTextColor(TFT_YELLOW);
  tft.print("Place finger for ID: ");
  tft.println(slot);

  while (finger.getImage() != FINGERPRINT_OK); // wait for finger
  finger.image2Tz();
  finger.createModel();
  finger.storeModel(slot);

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(20, 40);
  tft.setTextColor(TFT_GREEN);
  tft.print("Stored at ID: ");
  tft.println(slot);
}

void setup() {
  Serial.begin(115200);

  // TFT init
  tft.init();
  tft.setRotation(1);
  tftBoot();

  // RFID init
  hspi.begin(RFID_SCK, RFID_MISO, RFID_MOSI, RFID_SS);
  SPI = hspi;
  rfid.PCD_Init();
  tft.println("RFID Ready");

  // Fingerprint init
  fingerSerial.begin(57600, SERIAL_8N1, FINGER_RX, FINGER_TX);
  finger.begin(57600);
  if (finger.verifyPassword()) {
    tft.println("Fingerprint Ready");
  } else {
    tft.println("Fingerprint ERROR");
    while(1) delay(100);
  }
}

void loop() {
  // ----- RFID Check -----
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 40);
    tft.setTextColor(TFT_GREEN);
    tft.println("RFID Detected");

    rfid.PICC_HaltA();
    delay(1000);
  }

  // ----- Fingerprint Check -----
  uint8_t p = finger.getImage();
  if (p == FINGERPRINT_OK) {
    finger.image2Tz();
    finger.fingerFastSearch();

    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 40);
    if (finger.fingerID) {
      tft.setTextColor(TFT_GREEN);
      tft.print("Fingerprint ID: ");
      tft.println(finger.fingerID);
    } else {
      tft.setTextColor(TFT_RED);
      tft.println("Not Found");
    }
    delay(1000);
  }
}
