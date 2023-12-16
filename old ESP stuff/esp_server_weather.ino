#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_SSD1306.h>

// WiFi-Zugangsdaten
const char* ssid = "";
const char* password = "";

// PHP-Schnittstelle
const char* serverUrl = "http://192.168.178.22:8080/display_weather.php";

// Display
Adafruit_SSD1306 display(128, 64, &Wire, -1);

void setup() {
  // WiFi verbinden
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Verbindung zum WiFi wird hergestellt...");
  }
  Serial.println("WiFi verbunden");

  // Display initialisieren
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
}

void loop() {
  // HTTP-Anfrage an den PHP-Server senden
  HTTPClient http;
  http.begin(serverUrl);
  int httpCode = http.GET();

  // Wenn die Anfrage erfolgreich war
  if (httpCode == HTTP_CODE_OK) {
    String response = http.getString();
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);

    // Text in einzelne Zeilen aufteilen und anzeigen
    int lineIndex = 0;
    while (response.length() > 0) {
      int lineEnd = response.indexOf('\n');
      if (lineEnd != -1) {
        String line = response.substring(0, lineEnd);
        display.println(line);
        response = response.substring(lineEnd + 1);
        lineIndex++;
      } else {
        display.println(response);
        response = "";
      }
    }
    display.display();
  } else {
    Serial.println("Fehler beim Abrufen des Texts");
  }

  http.end();

  delay(500000);
}
