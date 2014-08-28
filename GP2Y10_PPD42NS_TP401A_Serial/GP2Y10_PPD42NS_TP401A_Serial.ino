/*
References:
http://www.seeedstudio.com/wiki/images/e/eb/TP-401A_Indoor_Air_quality_gas_sensor.pdf
http://www.sparkfun.com/datasheets/Sensors/gp2y1010au_e.pdf
http://www.sca-shinyei.com/pdf/PPD42NS.pdf 
https://www.sparkfun.com/datasheets/Sensors/Temperature/DHT22.pdf
*/

#include "DHT.h"

#define DHTPIN A3        // A3
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define GP2ReadPin A0    // A0
#define GP2LedPin 2      // D2
#define GPdelayTime1 280
#define GPdelayTime2 40
#define GPoffTime 9680
unsigned long GP2sum = 0;

#define TP4pin A1         // A1
unsigned long TP4sum = 0;

#define PPDpin 8          // D8
#define PPDsample 30e6   // us
unsigned long PPDsum = 0; // us

// general storage for Arduino analog reads
unsigned long time0 = 0; // time in ms
unsigned long i = 0; // number of samples

// general output string pointer
String out = "";

void setup() {
  Serial.begin(9600);
  
  pinMode(GP2ReadPin, INPUT);
  pinMode(GP2LedPin, OUTPUT);
  
  pinMode(TP4pin, INPUT);
  
  pinMode(PPDpin, INPUT);
  
  dht.begin();
  
  Serial.println("# Warm up...");
  
  out += "# GPdust, TPaq, PPDdust, humidity, temp";
  
  Serial.println(out);
    time0 = micros();
}

void loop() {

  i++;
  
  // GP2Y10 dust sensor
  digitalWrite(GP2LedPin, LOW); // power on the LED
  delayMicroseconds(GPdelayTime1);
  GP2sum += analogRead(GP2ReadPin);
  delayMicroseconds(GPdelayTime2);
  digitalWrite(GP2LedPin, HIGH); // turn the LED off
  delayMicroseconds(GPoffTime);
  
  // DHT22 (AM2302) temperature & humidity sensor
  float h = dht.readHumidity();
  float t = dht.readTemperature();  
  
  // TP401A air quality sensor
  TP4sum += analogRead(TP4pin);

  // PPD42NS dust sensor
  PPDsum += pulseIn(PPDpin, LOW);

  if( (micros() - time0) > PPDsample) {
     out = "";
     out += String(GP2sum/i);
     out += ",";
     out += String(TP4sum/i);
     out += ",";
     out += String(100*PPDsum/(float)PPDsample);
     out += ",";
     out += String(h);
     out += ",";
     out += String(t);
     
     Serial.println(out); 
     
     GP2sum = TP4sum = PPDsum = i = 0;
     
     time0 = micros();
  }

}
