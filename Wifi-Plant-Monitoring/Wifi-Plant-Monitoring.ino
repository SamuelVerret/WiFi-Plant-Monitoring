//Libraries
#include <Wire.h>
#include "Arduino.h"
#include "SI114X.h"
#include <DHT.h>
#include <ESP8266WiFi.h>

//Constants
#define DHTPIN 7     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define Frequent            5    // Frequency of upload in Minutes */
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino
SI114X SI1145 = SI114X(); // Initialize Grove Sunlight Sensor
String apiKey         = "";   /* Replace with your thingspeak API key */
const char* ssid      = "";      /* Your Router's SSID */
const char* password  = "";      /* Your Router's WiFi Password */
const char* server    = "api.thingspeak.com";

//Variables
float humidityPercent;  //Stores humidity value
float tempCelsius; //Stores temperature value
float visibleLight; // Stores visible light value
float infraredLight; // Stores infrared light value
float uvLight; // Stores uv light value
float soilMoisture; // Stores soil moisture value
int soilPin = A0;//Declare a variable for the soil moisture sensor 
int soilPower = 6;//Variable for Soil moisture Power

WiFiClient client;

void setup()   {    
  
  //Setting up the serial communication     
  Serial.begin(9600);

  //Initializing DHT22 sensor
  dht.begin();

  //Initializing Grove Sunlight Sensor sensor
  Serial.println("Beginning Si1145!");
  while (!SI1145.Begin()) {
    Serial.println("Si1145 is not ready!");
    delay(1000);
    //Initialize soil moisture sensor
    pinMode(soilPower, OUTPUT);//Set D6 as an OUTPUT
    digitalWrite(soilPower, LOW);//Set to LOW so no power is flowing through the sensor
  }
  
  Serial.println("Si1145 is ready!");
  
  delay(250);
  ConnectAP();
}

void loop() {
  delay(2000);
  tempCelsius = dht.readTemperature();
  humidityPercent = dht.readHumidity();
  visibleLight = SI1145.ReadVisible();
  infraredLight = SI1145.ReadIR();
  uvLight = (float)SI1145.ReadUV()/100;
  soilMoisture = readSoil();
  Serial.print("Temp: "); 
  Serial.print("Humidity: ");
  Serial.print(humidityPercent);
  Serial.print(" %, Temp: ");
  Serial.print(tempCelsius);
  Serial.println(" Celsius");
  Serial.print("//--------------------------------------//\r\n");
  Serial.print("Vis: "); Serial.println(visibleLight);
  Serial.print("IR: "); Serial.println(infraredLight);
  Serial.print("UV: "); Serial.println(uvLight);
  Serial.print("Soil Moisture = ");  //get soil moisture value from the function below and print it
  Serial.println(soilMoisture);
   
  if (client.connect(server,80)) {  
    String postStr = apiKey;
           postStr +="&field1=";
           postStr += String(tempCelsius);
           postStr +="&field2=";
           postStr += String(soilMoisture);
           postStr +="&field3=";
           postStr += String(visibleLight);
           postStr +="&field4=";
           postStr += String(infraredLight);
           postStr +="&field5=";
           postStr += String(uvLight);
           postStr +="&field6=";
           postStr += String(humidityPercent);
           postStr += "\r\n\r\n\r\n\r\n\r\n\r\n";
     client.print("POST /update HTTP/1.1\n"); 
     client.print("Host: api.thingspeak.com\n"); 
     client.print("Connection: close\n"); 
     client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n"); 
     client.print("Content-Type: application/x-www-form-urlencoded\n"); 
     client.print("Content-Length: "); 
     client.print(postStr.length()); 
     client.print("\n\n"); 
     client.print(postStr);
  }
  client.stop();

  //Deepsleep mode to reduce power consumption
  //Wakes every "Frequent" minutes specified in variables
  ESP.deepSleep(Frequent * 60 * 1000000, WAKE_RF_DEFAULT);
}


// Connect to WiFi Router             
void ConnectAP(void) {
  WiFi.mode(WIFI_STA);    // Set WiFi to station mode
  WiFi.disconnect();     // disconnect from an AP if it was Previously connected
  delay(100);
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);
}

//This is a function used to get the soil moisture value
float readSoil()
{
    float val;
    digitalWrite(soilPower, HIGH);//turn D6 "On"
    delay(10);//wait 10 milliseconds 
    val = analogRead(soilPin);//Read the SIG value form sensor 
    digitalWrite(soilPower, LOW);//turn D6 "Off"
    return val;//send current moisture value
}
