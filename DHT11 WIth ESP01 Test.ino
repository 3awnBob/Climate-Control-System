//Declare vairable of type String named myAPIkey (Write API key from ThingSpeak Channel)
String myAPIkey = "PWZRGE2EN0AYZEK1";   

// Including Libraries
#include "ThingSpeak.h"                                                       //ThingSpeak Library
#include <SoftwareSerial.h>                                                   //SoftwareSerial Library used to set Arduino GPIO to UART comunication pin
#include <DHT.h>;                                                            //Temperature and Humidity Sensor DHT11 Library


SoftwareSerial ESP8266(2, 3);                                                //Setting pin 2 to RX and pin 3 to TX

#define DHTTYPE DHT11                                                       //Defining Constant to refer to DHT sensor Model  
#define DHTPIN  A0                                                          //Defining the Arduino pin to be used with the Data pin of the DHT sensor

DHT dht(DHTPIN, DHTTYPE,11);       // Initializing DHT function with previously set data
float humidity, temp_f;            // Declare two float variables, named humidity and temp_f, respectively. float is a data type that is used to represent numbers with decimal points.
long writingTimer = 17;            //Declaring Constant of Long type to store time entity in milliseonds initialized at 17
long startTime = 0;                //Same
long waitTime = 0;                 //Same
int led = 12;                      //Declaring Pin used for the LED replacing the Valve command output  

boolean relay1_st = false; 
boolean relay2_st = false; 
unsigned char check_connection=0;
unsigned char times_check=0;
boolean error;                    //ALL of the above serve as variables declared to be used in the communication process below

void setup()                    
{
  pinMode(led, OUTPUT);           //Setting the Led pin to output
  Serial.begin(9600); 
  ESP8266.begin(9600); 
   dht.begin();                   //all the above Initializing Functions
  startTime = millis();           //Storing runtime 
  ESP8266.println("AT+RST");      //Reseting the ESP8266
  delay(2000);
  Serial.println("Connecting to Wifi");   
   while(check_connection==0)         
  {
    Serial.print(".");
   ESP8266.print("AT+CWJAP=\"Kalera Munich\",\"W3R3TH3#ONE\"\r\n");      //Sending WiFi data with an AT Command to ESP8266
 
  ESP8266.setTimeout(5000);
 if(ESP8266.find("WIFI CONNECTED\r\n")==1)
 {
 Serial.println("WIFI CONNECTED");
 break;
 }
 times_check++;                                                          //Incrementing numbers of tries
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
    readSensors();                                                   //Function defined below
    writeThingSpeak();                                               //Function defined below
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
  delay(100);                                                         //All the above: Printing the Values in the Serial Monitor

//if the value is greater than 30, turn the LED on:
  if (temp_f > 26) {
    digitalWrite(led, HIGH);
  }
  else {
    // if the value is not greater than 30, turn the LED off:
    digitalWrite(led, LOW);
  }

  // wait a bit before checking the value again:
  delay(100);                                                         // Activating the Led if Temperature above 26 Deg

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
  GetThingspeakcmd(getStr);                                       //Sending the Values to ThingSpeak
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
