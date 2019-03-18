#include <WiFiClient.h>
#include <WebServer.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include "FS.h"
#include "SPIFFS.h"
#include <HardwareSerial.h>
#define RXD2 16
#define TXD2 17


const char* ssid = "Dcube_web";
const char* password = "D@123456";


//--------- Temp parameters------------//
 static int16_t rms_x;  
 static int16_t rms_y;  
 static int16_t rms_z; 
 static int16_t max_x; 
 static int16_t max_y; 
 static int16_t max_z; 
 static int16_t min_x; 
 static int16_t min_y; 
 static int16_t min_z;
 static float cTemp; 
 
//*********ZigBee Frame**************/
uint8_t data[54];
int k = 10;
int i;
WebServer server(80);

void handleRoot() {
  File  file = SPIFFS.open("/chartThing.html", "r");
  server.streamFile(file,"text/html");
  file.close();
}

void handleDHT(){
  StaticJsonBuffer<100> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["rmsx"] = rms_x;
    root["rmsy"] = rms_y;
    char jsonChar[100];
    root.printTo((char*)jsonChar, root.measureLength() + 1);
    server.send(200, "text/json", jsonChar);
  }

//****************HANDLE NOT FOUND*********************//
void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  server.send(404, "text/plain", message);
}


void setup(void){
  
  Serial.begin(9600);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2); // pins 16 rx2, 17 tx2, 19200 bps, 8 bits no parity 1 stop bit
  SPIFFS.begin();
  WiFi.begin(ssid, password);
  Serial.println("");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/dht22", handleDHT);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
  }

  
void loop(void){
   StaticJsonBuffer<100> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  if (Serial2.available())
  {
    data[0] = Serial2.read();
    delay(k);
   if(data[0]==0x7E)
    {
    Serial.println("Got Packet");
    while (!Serial2.available());
    for ( i = 1; i< 55; i++)
      {
      data[i] = Serial2.read();
      delay(1);
      }
    if(data[15]==0x7F)  /////// to check if the recive data is correct
      {
  if(data[22]==0x08)  //////// make sure the sensor type is correct
  {
  rms_x = ((uint16_t)(((data[24])<<16) + ((data[25])<<8) + (data[26]))/100);
  rms_y = ((uint16_t)(((data[27])<<16) + ((data[28])<<8) + (data[29]))/100);
  rms_z = ((uint16_t)(((data[30])<<16) + ((data[31])<<8) + (data[32]))/100);
  max_x = ((uint16_t)(((data[33])<<16) + ((data[34])<<8) + (data[35]))/100);
  max_y = ((uint16_t)(((data[36])<<16) + ((data[37])<<8) + (data[38]))/100);
  max_z = ((uint16_t)(((data[39])<<16) + ((data[40])<<8) + (data[41]))/100);

  min_x = ((uint16_t)(((data[42])<<16) + ((data[43])<<8) + (data[44]))/100);
  min_y = ((uint16_t)(((data[45])<<16) + ((data[46])<<8) + (data[47]))/100);
  min_z = ((uint16_t)(((data[48])<<16) + ((data[49])<<8) + (data[50]))/100);

  
  cTemp = ((((data[51]) * 256) + data[52]));
  float battery = ((data[18] * 256) + data[19]);
  float voltage = 0.00322 * battery;
  Serial.print("Sensor Number  ");
  Serial.println(data[16]);
  Serial.print("Sensor Type  ");
  Serial.println(data[22]);
  Serial.print("Firmware Version  ");
  Serial.println(data[17]);
  Serial.print("Temperature in Celsius :");
  Serial.print(cTemp);
  Serial.println(" C");
  
  Serial.print("RMS vibration in X-axis :");
  Serial.print(rms_x);
  Serial.println(" g");
  Serial.print("RMS vibration in Y-axis :");
  Serial.print(rms_y);
  Serial.println(" g");
  Serial.print("RMS vibration in Z-axis :");
  Serial.print(rms_z);
  Serial.println(" g");

  Serial.print("Min vibration in X-axis :");
  Serial.print(min_x);
  Serial.println(" g");
  Serial.print("Min vibration in Y-axis :");
  Serial.print(min_y);
  Serial.println(" g");
  Serial.print("Min vibration in Z-axis :");
  Serial.print(min_z);
  Serial.println(" g"); 

  Serial.print("ADC value:");
  Serial.println(battery);
  Serial.print("Battery Voltage:");
  Serial.print(voltage);
  Serial.println("\n");
  if (voltage < 1)
          {
    Serial.println("Time to Replace The Battery");
          }
        }
      }
    }
}
  server.handleClient();
}
