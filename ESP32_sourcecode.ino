void Line_Notify1(String message1) ;

#include <WiFi.h>
#include <WiFiClientSecure.h>

#define WIFI_SSID "iphone13 promax"        // Name of your wifi or hotspot
#define WIFI_PASSWORD "123456789"     // Password of your wifi or hotspot

#define LINE_TOKEN_IR "X08U4IOm5YZKKPvcF0CXm66oizEoGEdSbC5OJB6weZ5"    // Input your token

#define irPin 16


String message1 = "Food Tray FULL. Please check if your pet has finished the food... 🐶🦴";        // Message to line
uint32_t time1, time2;

void setup() {

  Serial.begin(115200);
  Serial.println();




  Serial.println("connecting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("connecting");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  delay(5000);
  Serial.println("IR Ready!!");

  time1 = time2 = millis();
}

void loop() {

  time1 = millis();

 
  


  if ((digitalRead(irPin) == HIGH) && (WiFi.status() == WL_CONNECTED)) {
    while (digitalRead(irPin) == HIGH) delay(100);
    Serial.println("Detect !");
    
    Line_Notify1(message1);
  }
  delay(50);

}

void Line_Notify1(String message) {
  WiFiClientSecure client;
  client.setInsecure();           // If ESP32 version 1.0.4 and lower erase this!!!
  if (!client.connect("notify-api.line.me", 443)) {
    Serial.println("connection failed");
    delay(2000);
    return;
  }

  String req = "";
  req += "POST /api/notify HTTP/1.1\r\n";
  req += "Host: notify-api.line.me\r\n";
  req += "Authorization: Bearer " + String(LINE_TOKEN_IR) + "\r\n";
  req += "Cache-Control: no-cache\r\n";
  req += "User-Agent: ESP8266\r\n";
  req += "Content-Type: application/x-www-form-urlencoded\r\n";
  req += "Content-Length: " + String(String("message=" + message1).length()) + "\r\n";
  req += "\r\n";
  req += "message=" + message1;
  // Serial.println(req);
  client.print(req);

  delay(20);

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }
}
