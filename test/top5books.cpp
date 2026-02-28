#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>

// Wi-Fi
const char* ssid = "KILL!";
const char* password = "12345678";

// API URL
const char* apiURL = "https://www.krantikarilibrary.com/api/books";

// TFT
TFT_eSPI tft = TFT_eSPI();

// --- Function to print text with wrapping and return number of lines used ---
int printWrappedText(TFT_eSPI &tft, int x, int yStart, int maxWidth, const char* text, int lineSpacing) {
  String str = String(text);
  int y = yStart;
  int linesUsed = 0;

  while (str.length() > 0) {
    int cut = str.length();
    // reduce cut until text fits
    while (tft.textWidth(str.substring(0, cut).c_str()) > maxWidth && cut > 0) {
      cut--;
    }
    String line = str.substring(0, cut);
    // remove leading spaces from remaining string
    while (str.length() > cut && str[cut] == ' ') cut++;
    tft.setCursor(x, y);
    tft.println(line);
    y += lineSpacing;
    linesUsed++;
    str = str.substring(cut);
  }
  return linesUsed;
}

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_CYAN);
  tft.setCursor(10, 10);
  tft.println("Connecting to Wi-Fi...");

  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Connected!");

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(10, 10);
  tft.setTextColor(TFT_YELLOW);
  tft.println("Wi-Fi Connected!");
}

void loop() {
  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;
    Serial.println("\n--- Making HTTP GET Request ---");
    http.begin(apiURL);

    int httpResponseCode = http.GET();
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    if(httpResponseCode > 0){
      String payload = http.getString();

      DynamicJsonDocument doc(65536);
      DeserializationError error = deserializeJson(doc, payload);

      if(!error){
        JsonArray resources = doc["resource"];
        Serial.println("\n--- Book Names ---");

        // Clear screen once
        tft.fillScreen(TFT_BLACK);
        tft.setTextSize(2);
        tft.setTextColor(TFT_YELLOW);
        tft.setCursor(10, 10);
        tft.println("Top 5 Books:");

        int y = 40;
        int lineSpacing = 20;
        int maxWidth = tft.width() - 20;

        for(int i = 0; i < 5 && i < resources.size(); i++){
          const char* bookName = resources[i]["name"];

          // Print in Serial
          Serial.print(i + 1); Serial.print(". "); Serial.println(bookName);

          // Numbered title
          String numbered = String(i + 1) + ". " + String(bookName);

          // Print on TFT with wrapping and get lines used
          int lines = printWrappedText(tft, 10, y, maxWidth, numbered.c_str(), lineSpacing);

          // Update y for next book
          y += lines * lineSpacing;

          // If we exceed screen height, stop (or implement scroll later)
          if(y > tft.height() - lineSpacing){
            break;
          }
        }

      } else {
        Serial.print("JSON parse error: ");
        Serial.println(error.c_str());
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(10,10);
        tft.setTextColor(TFT_RED);
        tft.println("JSON Parse Error");
      }

    } else {
      Serial.print("HTTP GET failed, error code: ");
      Serial.println(httpResponseCode);
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(10,10);
      tft.setTextColor(TFT_RED);
      tft.println("HTTP GET Failed");
    }

    http.end();
  } else {
    Serial.println("Wi-Fi Disconnected!");
  }

  delay(30000);
}
