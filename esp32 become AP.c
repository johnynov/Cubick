#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>

//AP do którego podlaczam ESP32 - jako client
const char *ssid_cli = "Rydelka";
const char *password_cli = "rydla4444";

const char* ssid_ap = "ESP32_ap";
const char* password_ap =  "wroom32";
int sensor_return;
int i;

char ThingSpeakAddress[] = "api.thingspeak.com";      // Thingspeak address
char api_key[]           = "6WQBX9QR4GSVXLPJ";  // Thingspeak API WRITE Key for your channel
const int UpdateInterval = 15 * 60 * 1000000;  // e.g. 15 * 60 * 1000000; for a 15-Min update interval (15-mins x 60-secs * 1000000uS)
#define ADC_input_pin   36  // also known as SVP ADC-0 or SVN Pin-39 
#define pressure_offset 3.9 // Compensates for this location being 40M asl
Adafruit_BMP280 bme;
//AP to ktore tworze - ESP32 jako serwer

//declaring functions;-----------------

void connect_to_wifi();
void become_ap();
void make_measure();
void push_to_thingspeak(String DatatoUpload);
void BME280_Sleep();
void esp_go_to_sleep();

//-------------------------------------

//-------------------------------------
void setup() {
	make_measure();
	if (sensor_return == 1) {
		connect_to_wifi();
		BME280_Sleep();
		esp_go_to_sleep();
	}
	else {

	}
	make_measure();
	push_to_thingspeak();
}

void connect_to_wifi(){
	Serial.begin(115200);
	WiFi.begin(ssid_cli, password_cli);

	Serial.print("\nStarting ESP32 in client mode");
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print("\nConnecting to WiFi..");
		i += 1;
		if (i = 4){
			return;
		}
	}
	Serial.print("Connected to Wifi Network");
	IPAddress myIP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.print("\n"+myIP);
}

void become_ap() {
	Serial.print("\n");
	Serial.print("Configuring ESP as access point...");
	/* You can remove the password parameter if you want the AP to be open. */
	WiFi.softAP(ssid_ap, password_ap);
	IPAddress myIP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.print("\n"+myIP);
	///
	server.begin();
	server.handleClient();
	
}

void make_measure() {
	Serial.print("\nChecking sensor avaliability...");
	Wire.begin(17,16); // (sda,scl) 
	if (!bme.begin()) {
	Serial.print("\nCould not find a sensor, check wiring!");
	float sensor = 1;
	} 
	else {
	Serial.print("\nFound a sensor continuing");
	float temperature = bme.readTemperature(); // 3 example variables, ideally supplied by a sensor, see my examples for the BMP180, 	BME280 or DS18B20
	float pressure    = bme.readPressure() / 100.0F + pressure_offset; // Result is in hPA
	DatatoUpload = "field1="+String(temperature)+"&field2="+String(pressure)
	return 0;
	}
}

void push_to_thingspeak(String DatatoUpload) {
	WiFiClient client;
	if (!client.connect(ThingSpeakAddress, 80)) {
	 Serial.print("\nConnection failed");
	return;
	}
	else
	{
	Serial.print("\n"+DataForUpload);
	client.print("POST /update HTTP/1.1\n");
	client.print("Host: api.thingspeak.com\n");
	client.print("Connection: close\n");
	client.print("X-THINGSPEAKAPIKEY: " + api_key + "\n");
	client.print("Content-Type: application/x-www-form-urlencoded\n");
	client.print("Content-Length: ");
	client.print(DataForUpload.length());
	client.print("\n\n");
	client.print(DataForUpload);
	}
	client.stop();
}

void BME280_Sleep() {
	//Serial.println("BME280 to Sleep mode");
	Wire.beginTransmission(0x76); // Check your I2C address, they vary, could be 0x77
	Wire.write((uint8_t)BME280_REGISTER_CONTROL);
	Wire.write((uint8_t)0b00);
	Wire.endTransmission();
}

void esp_go_to_sleep(){
	esp_sleep_enable_timer_wakeup(UpdateInterval);
	esp_deep_sleep_start();
}

void loop() {
	make_measure();
	if (sensor_return == 1) {
		connect_to_wifi();
		BME280_Sleep();
		esp_go_to_sleep();
	}
	else {
		///////??????
	}
	make_measure();
	push_to_thingspeak();
	esp_go_to_sleep();
}