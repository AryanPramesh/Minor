#include <SPI.h>
#include <MFRC522.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// ================= Pins =================
#define RFID_SS   5
#define RFID_RST  27
MFRC522 rfid(RFID_SS, RFID_RST);

TFT_eSPI tft = TFT_eSPI();
#define TOUCH_CS 13
XPT2046_Touchscreen ts(TOUCH_CS);

// ================= Voter List =================
#define TOTAL_VOTERS 2
String eligibleUIDs[TOTAL_VOTERS] = {
"73DAA413",
  "D3D490E2"
};

// ================= Admin UID =================
String adminUID = "DEADBEEF"; // Special card to show tally

// ================= Candidates =================
#define TOTAL_CANDIDATES 6
String candidates[TOTAL_CANDIDATES] = {
  "Alice", "Bob", "Charlie", "David", "Eva", "Pramesh"
};
int votes[TOTAL_CANDIDATES] = {0};

// ================= Track Voters =================
bool hasVoted[TOTAL_VOTERS] = {false};

// ================= Touch Buttons =================
struct ButtonArea {
  int y_start;
  int y_end;
  int candidateIndex;
};
ButtonArea buttons[TOTAL_CANDIDATES] = {
  {50, 90, 0},
  {90, 130, 1},
  {130, 170, 2},
  {170, 210, 3},
  {210, 250, 4},
  {250, 290, 5}
};

// ================= Functions =================
String readUID() {
  if (!rfid.PICC_IsNewCardPresent()) return "";
  if (!rfid.PICC_ReadCardSerial()) return "";

  String uidStr = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uidStr += "0";
    uidStr += String(rfid.uid.uidByte[i], HEX);
  }
  uidStr.toUpperCase();
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  return uidStr;
}

int getVoterIndex(String uid) {
  for (int i = 0; i < TOTAL_VOTERS; i++) if (eligibleUIDs[i] == uid) return i;
  return -1;
}

bool checkEligibility(String uid) {
  int index = getVoterIndex(uid);
  return index != -1 && !hasVoted[index];
}

TS_Point getMappedTouch() {
  TS_Point p = ts.getPoint();
  p.x = map(p.x, 200, 3900, 0, tft.width());
  p.y = map(p.y, 200, 3900, 0, tft.height());
  return p;
}

int getTouchedCandidate() {
  if (!ts.touched()) return -1;
  TS_Point p = getMappedTouch();
  for (int i = 0; i < TOTAL_CANDIDATES; i++) {
    if (p.y >= buttons[i].y_start && p.y < buttons[i].y_end) return buttons[i].candidateIndex;
  }
  return -1;
}

void showMessage(const char* msg, int color=TFT_WHITE) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(color, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(20, 120);
  tft.println(msg);
}

void showCandidates() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  for (int i = 0; i < TOTAL_CANDIDATES; i++) {
    tft.setCursor(20, buttons[i].y_start);
    tft.println(candidates[i]);
  }
}

void showTally() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 20);
  tft.println("Vote Tally:");
  for (int i = 0; i < TOTAL_CANDIDATES; i++) {
    tft.setCursor(20, 60 + i*30);
    tft.printf("%s : %d\n", candidates[i].c_str(), votes[i]);
  }
  delay(5000);
}

// ================= Setup =================
void setup() {
  Serial.begin(115200);
  SPI.begin(18, 19, 23);

  pinMode(RFID_SS, OUTPUT); digitalWrite(RFID_SS, HIGH);
  pinMode(TOUCH_CS, OUTPUT); digitalWrite(TOUCH_CS, HIGH);

  rfid.PCD_Init();
  tft.init();
  tft.setRotation(1);
  ts.begin();
  ts.setRotation(1);

  showMessage("Scan RFID to vote");
}

// ================= Loop =================
void loop() {
  String uid = readUID();
  if (uid != "") {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 100);
    tft.setTextColor(TFT_WHITE);
    tft.println("UID:");
    tft.println(uid);
    delay(1000);

    // Check if admin card
    if (uid == adminUID) {
      showTally();
     
      return;
    }

    // Check voter eligibility
    int voterIndex = getVoterIndex(uid);
    if (voterIndex == -1) {
      showMessage("Not Eligible!", TFT_RED);
      delay(2000);
      showMessage("Scan RFID to vote");
      return;
    } 
    if (hasVoted[voterIndex]) {
      showMessage("Already Voted!", TFT_RED);
      delay(2000);
      showMessage("Scan RFID to vote");
      return;
    }

    showMessage("Eligible!", TFT_GREEN);
    delay(1000);

    // Voting
    showCandidates();
    int choice = -1;
    while (choice == -1) choice = getTouchedCandidate();

    votes[choice]++;
    hasVoted[voterIndex] = true;

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_YELLOW);
    tft.setCursor(20, 120);
    tft.print("Vote registered for ");
    tft.println(candidates[choice]);
    delay(2000);

    showMessage("Scan RFID to vote");
  }
}
