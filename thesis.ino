#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>
#include <SPI.h>
#include <WiFi.h>
#include <Wire.h>

//AP do którego podlaczam ESP32 - jako client
const char *ssid_cli = "Rydelka";
const char *password_cli = "";
//AP to ktore tworze - ESP32 jako serwer
const char* ssid_ap = "ESP32_ap";
const char* password_ap =  "wroom32";

char thingSpeakAddress[] = "api.thingspeak.com";      // Thingspeak address
String api_key           = "YJ0DFY5ZFW2PWUZA";  // Thingspeak API WRITE Key for your channel

bool sensor_detected;
String DatatoUpload;
float temperature, pressure;
const int interval_min = 15;//  in minutes
#define uS_TO_S_FACTOR 1000000*60  /* Conversion factor for micro seconds to minutes */
const int UpdateInterval = interval_min * uS_TO_S_FACTOR;
#define ADC_input_pin   36  // also known as SVP ADC-0 or SVN Pin-39 
#define pressure_offset 3.9 // Compensates for this location being 40M asl
RTC_DATA_ATTR int bootCount = 0;

Adafruit_BMP280 bmp;

//declaring functions;-----------------

void make_measure();
void connect_to_wifi();
void become_ap();
void push_to_thingspeak(String DatatoUpload);
void BMP280_Sleep();
void esp_go_to_sleep(int UpdateInterval);
void print_wakeup_reason();

///declaring functions;-------------------------------------

//-------------------------------------
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.print("...................................................\n.");
  connect_to_wifi();
}
void loop() {
  make_measure();
  if (sensor_detected = true) {
    push_to_thingspeak(DatatoUpload);
    BMP280_Sleep();
    esp_go_to_sleep(UpdateInterval);
    }
  else {
    //connect_to_wifi();
    Serial.print("\nCould not find a sensor, check wiring!");
    esp_go_to_sleep(UpdateInterval);
  }
}

void make_measure(){
  Serial.print("\n>Checking sensor avaliability.");
  String temperature, pressure;
  if(bmp.begin()) {
    Serial.print("\n|Sensor detected");
  }
  temperature = String(bmp.readTemperature());
  pressure = String(bmp.readPressure()/100);
  DatatoUpload = "field1="+String(temperature)+"&field2="+String(pressure);
  Serial.print("\n|Temperature= " + String(temperature) + "*C");
  Serial.print("\n|Pressure= " + String(pressure) + "hPa");
}

void connect_to_wifi(){
  Serial.println("Starting WiFi setup.");
  WiFi.begin(ssid_cli, password_cli);
  while (WiFi.status() != WL_CONNECTED) {
    delay(900);
    Serial.println("|Connecting to WiFi");
  }
  Serial.print("|Connected to the \"");
  Serial.print(ssid_cli);
  Serial.print("\" WiFi network");
}

void become_ap() {
  Serial.print("\n");
  Serial.print("Configuring ESP as access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid_ap, password_ap);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("ESP AP IP address: ");
  Serial.print("\n"+myIP);
  ///
  //  server.begin();
  //  server.handleClient();
  //  server.send(200, "text/plain", "hello from esp8266!");
  }


void push_to_thingspeak(String DatatoUpload) {
  WiFiClient client;
  if (!client.connect(thingSpeakAddress, 80)) {
    Serial.print("\nConnection failed");
    return;
  }
  else
  {
    Serial.print("\n>Sending string: "+DatatoUpload);
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + api_key + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(DatatoUpload.length());
    client.print("\n\n");
    client.print(DatatoUpload);
    Serial.print(String(temperature));
    Serial.print(String(pressure));
  }
  client.stop();
}

void BMP280_Sleep() {
  //Serial.println("BMP280 to Sleep mode");
  Serial.print("\n>Putting BMP280 to sleep");
  Wire.beginTransmission(0x76); // Check your I2C address, they vary, could be 0x77
  //Wire.write((uint8_t)BMP280_REGISTER_CONTROL);
  Wire.write((uint8_t)0b00);
  Wire.endTransmission();
}

void esp_go_to_sleep(int UpdateInterval){
  Serial.print("\n>Putting ESP32 to sleep for: ");
  Serial.print(interval_min);
  Serial.print("minutes.");
  esp_sleep_enable_timer_wakeup(UpdateInterval);
  esp_deep_sleep_start();
}
