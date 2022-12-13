String myAPIkey = "PWZRGE2EN0AYZEK1";  

#include "ThingSpeak.h"
#include <SoftwareSerial.h>
#include <DHT.h>;
SoftwareSerial ESP8266(2, 3); // Rx,  Tx

#define DHTTYPE DHT11
#define DHTPIN  A0

DHT dht(DHTPIN, DHTTYPE,11);
float humidity, temp_f;  
long writingTimer = 17; 
long startTime = 0;
long waitTime = 0;
int led = 12; //LED Replacing valve

boolean relay1_st = false; 
boolean relay2_st = false; 
unsigned char check_connection=0;
unsigned char times_check=0;
boolean error;

void setup()
{
  pinMode(led, OUTPUT);
  Serial.begin(9600); 
  ESP8266.begin(9600); 
   dht.begin();
  startTime = millis(); 
  ESP8266.println("AT+RST");
  delay(2000);
  Serial.println("Connecting to Wifi");
   while(check_connection==0)
  {
    Serial.print(".");
   ESP8266.print("AT+CWJAP=\"Kalera Munich\",\"W3R3TH3#ONE\"\r\n");
 
  ESP8266.setTimeout(5000);
 if(ESP8266.find("WIFI CONNECTED\r\n")==1)
 {
 Serial.println("WIFI CONNECTED");
 break;
 }
 times_check++;
 if(times_check>3) 
 {
  times_check=0;
   Serial.println("Trying to Reconnect..");
  }
  }
}

void loop()
{
  waitTime = millis()-startTime;   
  if (waitTime > (writingTimer*1000)) 
  {
    readSensors();
    writeThingSpeak();
    startTime = millis();   
  }
}


void readSensors(void)
{
  temp_f = dht.readTemperature();
  humidity = dht.readHumidity();

  //Added testing for the output
  Serial.print("Temperature: ");
  Serial.println(temp_f);
  delay(100);
  Serial.print("Humidity: ");
  Serial.println(humidity);
  delay(100);

//if the value is greater than 30, turn the LED on:
  if (temp_f > 26) {
    digitalWrite(led, HIGH);
  }
  else {
    // if the value is not greater than 30, turn the LED off:
    digitalWrite(led, LOW);
  }

  // wait a bit before checking the value again:
  delay(100);

}


void writeThingSpeak(void)
{
  startThingSpeakCmd();
  // preparacao da string GET
  String getStr = "GET /update?api_key=";
  getStr += myAPIkey;
  getStr +="&field1=";
  getStr += String(temp_f);
  getStr +="&field2=";
  getStr += String(humidity);
  getStr += "\r\n\r\n";
  GetThingspeakcmd(getStr); 
}

void startThingSpeakCmd(void)
{
  ESP8266.flush();
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // api.thingspeak.com IP address
  cmd += "\",80";
  ESP8266.println(cmd);
  Serial.print("Start Commands: ");
  Serial.println(cmd);

  if(ESP8266.find("Error"))
  {
    Serial.println("AT+CIPSTART error");
    return;
  }
}

String GetThingspeakcmd(String getStr)
{
  String cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  ESP8266.println(cmd);
  Serial.println(cmd);

  if(ESP8266.find(">"))
  {
    ESP8266.print(getStr);
    Serial.println(getStr);
    delay(500);
    String messageBody = "";
    while (ESP8266.available()) 
    {
      String line = ESP8266.readStringUntil('\n');
      if (line.length() == 1) 
      { 
        messageBody = ESP8266.readStringUntil('\n');
      }
    }
    Serial.print("MessageBody received: ");
    Serial.println(messageBody);
    return messageBody;
  }
  else
  {
    ESP8266.println("AT+CIPCLOSE");     
    Serial.println("AT+CIPCLOSE"); 
  } 
   
  

}