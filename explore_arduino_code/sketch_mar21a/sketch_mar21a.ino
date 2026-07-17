#include <SPI.h>
#include <WiFi101.h>
#include <Arduino_JSON.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

// Wi-Fi Info
const char WIFI_SSID[] = "tesla2000";
const char WIFI_PASSWORD[] = "welcome11701!";

// API Info
const char SERVER[] = "api.openweathermap.org";
const char HOST_STRING[] = "HOST: api.openweathermap.org";
const String API_KEY = "0b0b714dab11efd34e0a6d8b0e3ea722";
const String CITY = "Austin";
String UNITS = "F";                    // Start with F, will toggle

// Onboard LED
const int ONBOARD_LED = 13;

// 7-Segment Display
Adafruit_7segment seven_seg_display = Adafruit_7segment();

WiFiClient client;
int wifi_status = WL_IDLE_STATUS;

void setup()
{
  WiFi.setPins(8,7,4,2);
 
  pinMode(ONBOARD_LED, OUTPUT);
  digitalWrite(ONBOARD_LED, LOW);
 
  // Initialize 7-segment display (default I2C address 0x70)
  seven_seg_display.begin(0x70);
 
  Serial.begin(9600);
  while(!Serial);
 
  Serial.println("Web-Connected Temperature Display");
  Serial.print("Connecting to: ");
  Serial.println(WIFI_SSID);
 
  WiFi.setTimeout(8000);
 
  while (wifi_status != WL_CONNECTED)
  {
    wifi_status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(500);
  }
 
  Serial.println("Connected!\n");
  digitalWrite(ONBOARD_LED, HIGH);
}

void loop()
{
  // Prepare the API Request
  String api_units = (UNITS == "F") ? "imperial" : "metric";
 
  String request = "GET /data/2.5/weather?units=" + api_units +
                   "&q=" + CITY +
                   "&appid=" + API_KEY +
                   " HTTP/1.1";
 
  // Connect and send request
  if (client.connect(SERVER, 80))
  {
    client.println(request);
    client.println(HOST_STRING);
    client.println("Connection: close");
    client.println();
  }
 
  // Wait for response
  while (!client.available()) delay(10);
 
  // Skip headers until JSON starts
  while (client.available())
  {
    if (client.read() == '{') break;
  }
 
  // Read JSON
  String json = "{";
  while (client.available())
  {
    json += (char)client.read();
  }
  client.stop();
 
  // Parse and display
  JSONVar api_object = JSON.parse(json);
  double temp = (double) api_object["main"]["temp"];
 
  Serial.print("Temperature = ");
  Serial.print(temp);
  Serial.println(UNITS);
 
  // Display on 7-segment
  int temp_round = constrain(round(temp), -99, 999);
  seven_seg_display.print(temp_round * 10);           // Shift left for degree symbol
  seven_seg_display.writeDigitRaw(2, 0x10);           // Degree dot
 
  if (UNITS == "F")
  {
    seven_seg_display.writeDigitRaw(4, 0x71);         // "F"
    UNITS = "C";
  }
  else
  {
    seven_seg_display.writeDigitRaw(4, 0x39);         // "C"
    UNITS = "F";
  }
 
  seven_seg_display.writeDisplay();
 
  // Wait ~60 seconds before next update
  Serial.println("Waiting 60 seconds...");
  delay(6000);
}
