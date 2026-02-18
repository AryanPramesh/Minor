#include <SPI.h>
#include <MFRC522.h>
#include <TFT_eSPI.h>
#include <Adafruit_Fingerprint.h>

/* ---------- RFID ---------- */
#define RFID_MISO 12
#define RFID_MOSI 13
#define RFID_SCK  14
#define RFID_SS   26
#define RFID_RST  27
SPIClass hspi(HSPI);
MFRC522 rfid(RFID_SS, RFID_RST);

/* ---------- TFT ---------- */
TFT_eSPI tft = TFT_eSPI();

/* ---------- Fingerprint ---------- */
#define FINGER_RX 17
#define FINGER_TX 16
HardwareSerial fingerSerial(1);
Adafruit_Fingerprint finger(&fingerSerial);

/* ---------- Voting System ---------- */
enum State { STATE_IDLE, STATE_ELIGIBLE, STATE_SELECTED, STATE_VERIFIED, STATE_ERROR };
State currentState = STATE_IDLE;

struct Voter {
  byte rfidUID[4];
  uint8_t fingerID;
  bool hasVoted;
};
struct Candidate {
  const char* name;
  uint16_t votes;
};
#define MAX_VOTERS 5
#define MAX_CANDIDATES 3

Voter voters[MAX_VOTERS];
Candidate candidates[MAX_CANDIDATES];
int selectedCandidate = -1;
int currentVoter = -1;

/* ---------- Setup ---------- */
void setup() {
  Serial.begin(115200);

  // TFT
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(2);
  tft.setCursor(20, 40);
  tft.println("Booting...");

  // RFID
  hspi.begin(RFID_SCK, RFID_MISO, RFID_MOSI, RFID_SS);
  SPI = hspi;
  rfid.PCD_Init();

  // Fingerprint
  fingerSerial.begin(57600, SERIAL_8N1, FINGER_RX, FINGER_TX);
  finger.begin(57600);
  if (!finger.verifyPassword()) {
    tft.println("Fingerprint ERROR");
    while(1) delay(100);
  }

  // Sample Voters
  voters[0] = {{0xDE,0xAD,0xBE,0xEF}, 1, false};
  voters[1] = {{0xBA,0xAD,0xF0,0x0D}, 2, false};
  voters[2] = {{0x12,0x34,0x56,0x78}, 3, false};

  // Sample Candidates
  candidates[0] = {"Alice",0};
  candidates[1] = {"Bob",0};
  candidates[2] = {"Charlie",0};
}

/* ---------- Helper Functions ---------- */
int findVoterByRFID(byte *uid) {
  for(int i=0;i<MAX_VOTERS;i++){
    bool match = true;
    for(int j=0;j<4;j++){
      if(voters[i].rfidUID[j] != uid[j]) match=false;
    }
    if(match) return i;
  }
  return -1;
}

void showCandidates() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(10,20);
  tft.println("Select Candidate:");
  for(int i=0;i<MAX_CANDIDATES;i++){
    tft.setCursor(10,60+i*40);
    tft.println(candidates[i].name);
  }
}

/* ---------- Main Loop ---------- */
void loop() {

  switch(currentState){

    case STATE_IDLE:
      if(rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()){
        int idx = findVoterByRFID(rfid.uid.uidByte);
        if(idx==-1 || voters[idx].hasVoted){
          tft.fillScreen(TFT_BLACK);
          tft.setCursor(20,40);
          tft.setTextColor(TFT_RED);
          tft.println("Not Eligible!");
        } else {
          currentVoter = idx;
          currentState = STATE_ELIGIBLE;
          showCandidates();
        }
        rfid.PICC_HaltA();
        delay(1000);
      }
      break;

    case STATE_ELIGIBLE:
      // Replace this with your touch detection
      // Example: if touch detected, set selectedCandidate = index;
      // For testing, auto-select first candidate
      selectedCandidate = 0;
      currentState = STATE_SELECTED;
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(20,40);
      tft.setTextColor(TFT_YELLOW);
      tft.println("Place Finger");
      break;

    case STATE_SELECTED:
      if(finger.getImage() == FINGERPRINT_OK){
        finger.image2Tz();
        finger.fingerFastSearch();
        if(finger.fingerID == voters[currentVoter].fingerID){
          // Verified
          candidates[selectedCandidate].votes++;
          voters[currentVoter].hasVoted = true;
          tft.fillScreen(TFT_BLACK);
          tft.setCursor(20,40);
          tft.setTextColor(TFT_GREEN);
          tft.println("Vote Recorded!");
        } else {
          tft.fillScreen(TFT_BLACK);
          tft.setCursor(20,40);
          tft.setTextColor(TFT_RED);
          tft.println("Fingerprint Mismatch!");
        }
        delay(1500);
        currentState = STATE_IDLE;
      }
      break;

    default: currentState = STATE_IDLE;
  }
}
